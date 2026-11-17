#include "rl/DDQNAgent.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace rl
{

    QNetworkImpl::QNetworkImpl(int64_t state_dim, int64_t action_dim, int64_t hidden_dim)
    {
        layers = torch::nn::Sequential(
            torch::nn::Linear(state_dim, hidden_dim),
            torch::nn::ReLU(),
            torch::nn::Linear(hidden_dim, hidden_dim),
            torch::nn::ReLU(),
            torch::nn::Linear(hidden_dim, action_dim));

        register_module("layers", layers);
    }

    torch::Tensor QNetworkImpl::forward(const torch::Tensor &state)
    {
        return layers->forward(state);
    }

    DDQNAgent::DDQNAgent(const AgentConfig &config)
        : Agent(config),
          online_net_(QNetwork(config.state_dim, config.action_dim, config.hidden_dim)),
          target_net_(QNetwork(config.state_dim, config.action_dim, config.hidden_dim)),
          optimizer_(online_net_->parameters(), torch::optim::AdamOptions(config.learning_rate)),
          replay_buffer_(config.replay_capacity),
          rng_(std::random_device{}())
    {
        if (config.state_dim <= 0 || config.action_dim <= 0)
        {
            throw std::invalid_argument("state_dim and action_dim must be greater than zero");
        }

        sync_target_network();
        target_net_->eval();

    }

    ActionResult DDQNAgent::random_action()
    {
        ActionResult result;
        std::uniform_int_distribution<int64_t> action_dist(0, config_.action_dim - 1);
        result.action = action_dist(rng_);
        return result;
    }

    ActionResult DDQNAgent::greedy_action(const torch::Tensor &state)
    {
        const auto input = ensure_float_state(state);
        const auto q_values = online_net_->forward(input);
        const auto action = std::get<1>(q_values.max(1, false));

        ActionResult result;
        result.action = action.item().toLong();
        return result;
    }

    ActionResult DDQNAgent::act(const torch::Tensor &state)
    {
        if (mode_ == AgentMode::Eval)
        {
            return greedy_action(state);
        }

        const double epsilon = config_.epsilon;

        if (uniform_(rng_) < epsilon)
        {
            return random_action();
        }

        return greedy_action(state);
    }

    void DDQNAgent::observe(const Transition &transition)
    {
        replay_buffer_.push(transition);
    }

    TrainMetrics DDQNAgent::train_step()
    {
        TrainMetrics metrics;
        if (mode_ == AgentMode::Eval || replay_buffer_.size() < config_.batch_size)
        {
            return metrics;
        }

        auto batch = replay_buffer_.sample(config_.batch_size, rng_);

        std::vector<torch::Tensor> states;
        std::vector<torch::Tensor> next_states;
        std::vector<int64_t> actions;
        std::vector<float> rewards;
        std::vector<float> dones;
        states.reserve(batch.size());
        next_states.reserve(batch.size());
        actions.reserve(batch.size());
        rewards.reserve(batch.size());
        dones.reserve(batch.size());

        for (const auto &transition : batch)
        {
            states.push_back(ensure_float_state(transition.state));
            next_states.push_back(ensure_float_state(transition.next_state));
            actions.push_back(transition.action);
            rewards.push_back(transition.reward);
            dones.push_back(transition.done ? 1.0F : 0.0F);
        }

        const auto state_batch = torch::cat(states, 0);
        const auto next_state_batch = torch::cat(next_states, 0);
        const auto action_batch = torch::tensor(actions, torch::TensorOptions().dtype(torch::kInt64)).unsqueeze(1);
        const auto reward_batch = torch::tensor(rewards, torch::TensorOptions().dtype(torch::kFloat32));
        const auto done_batch = torch::tensor(dones, torch::TensorOptions().dtype(torch::kFloat32));

        auto q_values = online_net_->forward(state_batch);
        auto chosen_q = q_values.gather(1, action_batch).squeeze(1);

        torch::Tensor targets;
        {
            torch::NoGradGuard no_grad;
            const auto online_next_actions = online_net_->forward(next_state_batch).argmax(1).unsqueeze(1);
            const auto target_q_values = target_net_->forward(next_state_batch);
            const auto next_q = target_q_values.gather(1, online_next_actions).squeeze(1);
            targets = reward_batch + (1.0F - done_batch) * static_cast<float>(config_.gamma) * next_q;
        }

        const auto loss = torch::mse_loss(chosen_q, targets.detach());

        optimizer_.zero_grad();
        loss.backward();
        optimizer_.step();

        ++steps_;
        if (config_.target_update_steps > 0 && steps_ % config_.target_update_steps == 0)
        {
            sync_target_network();
        }

        metrics.trained = true;
        metrics.loss = loss.item().toDouble();
        return metrics;
    }

    void DDQNAgent::set_mode(AgentMode mode)
    {
        mode_ = mode;
        if (mode_ == AgentMode::Eval)
        {
            online_net_->eval();
            target_net_->eval();
        }
        else
        {
            online_net_->train();
            target_net_->eval();
        }
    }

    AgentMode DDQNAgent::mode() const
    {
        return mode_;
    }

    void DDQNAgent::reset()
    {
        replay_buffer_.clear();
        steps_ = 0;
    }

    std::size_t DDQNAgent::replay_size() const noexcept
    {
        return replay_buffer_.size();
    }

    void DDQNAgent::sync_target_network()
    {
        torch::NoGradGuard no_grad;
        auto source_parameters = online_net_->parameters();
        auto target_parameters = target_net_->parameters();
        const auto parameter_count = std::min(source_parameters.size(), target_parameters.size());

        for (std::size_t index = 0; index < parameter_count; ++index)
        {
            target_parameters[index].copy_(source_parameters[index]);
        }
    }

    void DDQNAgent::save(const std::string &path)
    {
        torch::save(online_net_, path + "_online.pt");
        torch::save(target_net_, path + "_target.pt");
    }

    void DDQNAgent::load(const std::string &path)
    {
        torch::load(online_net_, path + "_online.pt");
        torch::load(target_net_, path + "_target.pt");
    }

} // namespace rl
