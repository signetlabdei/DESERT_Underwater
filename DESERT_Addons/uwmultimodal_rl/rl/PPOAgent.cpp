#include "rl/PPOAgent.h"

#include <algorithm>
#include <numeric>
#include <stdexcept>

namespace rl {

ActorCriticImpl::ActorCriticImpl(int64_t state_dim, int64_t action_dim, int64_t hidden_dim) {
    shared = torch::nn::Sequential(
        torch::nn::Linear(state_dim, hidden_dim),
        torch::nn::ReLU(),
        torch::nn::Linear(hidden_dim, hidden_dim),
        torch::nn::ReLU()
    );

    policy_head = torch::nn::Linear(hidden_dim, action_dim);
    value_head = torch::nn::Linear(hidden_dim, 1);

    register_module("shared", shared);
    register_module("policy_head", policy_head);
    register_module("value_head", value_head);
}

std::pair<torch::Tensor, torch::Tensor> ActorCriticImpl::forward(const torch::Tensor& state) {
    const auto features = shared->forward(state);
    const auto logits = policy_head->forward(features);
    const auto value = value_head->forward(features).squeeze(-1);
    return {logits, value};
}

PPOAgent::PPOAgent(const AgentConfig& config)
    : Agent(config),
      policy_(ActorCritic(config.state_dim, config.action_dim, config.hidden_dim)),
      optimizer_(policy_->parameters(), torch::optim::AdamOptions(config.learning_rate)),
      rollout_buffer_(config.rollout_capacity),
      rng_(std::random_device{}()) {
    if (config.state_dim <= 0 || config.action_dim <= 0) {
        throw std::invalid_argument("state_dim and action_dim must be greater than zero");
    }
}

std::pair<torch::Tensor, torch::Tensor> PPOAgent::evaluate_policy(const torch::Tensor& states) {
    return policy_->forward(states);
}

ActionResult PPOAgent::act(const torch::Tensor& state) {
    const auto input = ensure_float_state(state);

    // Check for NaN/Inf in state
    if (!torch::isfinite(input).all().item<bool>()) {
        throw std::runtime_error("State contains NaN or Inf values");
    }

    const auto [logits, value] = policy_->forward(input);

    if (!torch::isfinite(logits).all().item<bool>() || !torch::isfinite(value).all().item<bool>()) {
        throw std::runtime_error("Policy forward produced NaN or Inf values" + std::string(" logits: ") + logits.toString() + " value: " + value.toString());
    }

    // Clip logits for numerical stability
    const auto safe_logits = torch::clamp(logits, -80.0, 80.0);

    std::cout << "UwMultiTrafficRl::act: logits: " << safe_logits << std::endl;

    if (!torch::isfinite(safe_logits).all().item<bool>()) {
        throw std::runtime_error("Clamped logits contain NaN or Inf values" + std::string(" logits: ") + safe_logits.toString());
    }

    ActionResult result;
    result.value = value;

    const auto probabilities = safe_logits.softmax(-1, torch::kFloat32);
    if (!torch::isfinite(probabilities).all().item<bool>()) {
        throw std::runtime_error("Softmax produced NaN or Inf probabilities" + std::string(" probabilities: ") + probabilities.toString());
    }

    if (mode_ == AgentMode::Eval) {
        result.action = std::get<1>(probabilities.squeeze(0).max(0, false)).item<int64_t>();
        return result;
    }

    const auto action_tensor = torch::multinomial(probabilities, 1);
    result.action = action_tensor.item().toLong();
    result.log_prob = probabilities.gather(1, action_tensor).clamp_min(1e-8).log().squeeze(1);
    return result;
}

void PPOAgent::observe(const Transition& transition) {
    if (mode_ == AgentMode::Eval) {
        return;
    }

    if (!transition.has_log_prob || !transition.has_value) {
        throw std::invalid_argument("PPO transitions must contain log_prob and value from act()");
    }

    rollout_buffer_.push(transition);
}

torch::Tensor PPOAgent::compute_last_value() {
    if (rollout_buffer_.size() == 0) {
        return torch::zeros({1}, torch::TensorOptions().dtype(torch::kFloat32));
    }

    const auto& last_transition = rollout_buffer_.data().back();
    if (last_transition.done) {
        return torch::zeros({1}, torch::TensorOptions().dtype(torch::kFloat32));
    }

    const auto input = ensure_float_state(last_transition.next_state);
    return policy_->forward(input).second.detach();
}

TrainMetrics PPOAgent::train_step() {
    TrainMetrics metrics;
    if (mode_ == AgentMode::Eval || rollout_buffer_.size() < config_.minibatch_size) {
        return metrics;
    }

    const auto& rollout = rollout_buffer_.data();
    const std::size_t rollout_size = rollout.size();

    std::vector<torch::Tensor> states;
    std::vector<int64_t> actions;
    std::vector<torch::Tensor> old_log_probs;
    std::vector<torch::Tensor> values;
    std::vector<float> rewards;
    std::vector<float> dones;
    states.reserve(rollout_size);
    actions.reserve(rollout_size);
    old_log_probs.reserve(rollout_size);
    values.reserve(rollout_size);
    rewards.reserve(rollout_size);
    dones.reserve(rollout_size);

    for (const auto& transition : rollout) {
        states.push_back(ensure_float_state(transition.state));
        actions.push_back(transition.action);
        old_log_probs.push_back(transition.log_prob.reshape({1, 1}).detach());
        values.push_back(transition.value.reshape({1, 1}).detach());
        rewards.push_back(transition.reward);
        dones.push_back(transition.done ? 1.0F : 0.0F);
    }

    const auto state_batch = torch::cat(states, 0);
    const auto action_batch = torch::tensor(actions, torch::TensorOptions().dtype(torch::kInt64)).unsqueeze(1);
    const auto old_log_prob_batch = torch::cat(old_log_probs, 0);
    const auto value_batch = torch::cat(values, 0);
    const auto reward_batch = torch::tensor(rewards, torch::TensorOptions().dtype(torch::kFloat32));
    const auto done_batch = torch::tensor(dones, torch::TensorOptions().dtype(torch::kFloat32));

    const auto last_value = compute_last_value();
    auto advantages = torch::zeros_like(reward_batch);
    auto returns = torch::zeros_like(reward_batch);

    double gae = 0.0;
    double next_value = last_value.item().toDouble();

    for (std::size_t index = rollout_size; index-- > 0;) {
        const double reward = reward_batch[index].item().toDouble();
        const double done = done_batch[index].item().toDouble();
        const double value = value_batch[index].item().toDouble();
        const double mask = 1.0 - done;
        const double delta = reward + config_.gamma * next_value * mask - value;
        gae = delta + config_.gamma * config_.gae_lambda * mask * gae;
        advantages[index] = scalar_float(static_cast<float>(gae));
        returns[index] = scalar_float(static_cast<float>(gae + value));
        next_value = value;
    }

    advantages = (advantages - advantages.mean()) / (advantages.std() + 1e-8);

    const std::size_t minibatch_size = std::min(config_.minibatch_size, rollout_size);
    std::vector<std::size_t> indices(rollout_size);
    std::iota(indices.begin(), indices.end(), 0);

    double total_loss = 0.0;
    double total_policy_loss = 0.0;
    double total_value_loss = 0.0;
    double total_entropy = 0.0;
    std::size_t updates = 0;

    for (std::size_t epoch = 0; epoch < config_.ppo_epochs; ++epoch) {
        std::shuffle(indices.begin(), indices.end(), rng_);

        for (std::size_t start = 0; start < rollout_size; start += minibatch_size) {
            const auto end = std::min(start + minibatch_size, rollout_size);
            std::vector<int64_t> minibatch_indices(indices.begin() + static_cast<std::ptrdiff_t>(start), indices.begin() + static_cast<std::ptrdiff_t>(end));

            std::vector<torch::Tensor> minibatch_states;
            std::vector<int64_t> minibatch_actions;
            std::vector<torch::Tensor> minibatch_old_log_probs;
            std::vector<torch::Tensor> minibatch_returns;
            std::vector<torch::Tensor> minibatch_advantages;
            minibatch_states.reserve(minibatch_indices.size());
            minibatch_actions.reserve(minibatch_indices.size());
            minibatch_old_log_probs.reserve(minibatch_indices.size());
            minibatch_returns.reserve(minibatch_indices.size());
            minibatch_advantages.reserve(minibatch_indices.size());

            for (auto index : minibatch_indices) {
                minibatch_states.push_back(states[index]);
                minibatch_actions.push_back(actions[index]);
                minibatch_old_log_probs.push_back(old_log_prob_batch[index].reshape({1, 1}));
                minibatch_returns.push_back(returns[index].reshape({1, 1}));
                minibatch_advantages.push_back(advantages[index].reshape({1, 1}));
            }

            const auto mini_state_batch = torch::cat(minibatch_states, 0);
            const auto mini_action_batch = torch::tensor(minibatch_actions, torch::TensorOptions().dtype(torch::kInt64)).unsqueeze(1);
            const auto mini_old_log_prob_batch = torch::cat(minibatch_old_log_probs, 0).detach();
            const auto mini_return_batch = torch::cat(minibatch_returns, 0).detach();
            const auto mini_advantage_batch = torch::cat(minibatch_advantages, 0).detach();

            const auto [logits, value_prediction] = evaluate_policy(mini_state_batch);
            const auto safe_logits = torch::clamp(logits, -80.0, 80.0);
            if (!torch::isfinite(safe_logits).all().item<bool>() || !torch::isfinite(value_prediction).all().item<bool>()) {
                continue;
            }

            const auto probabilities = safe_logits.softmax(-1, torch::kFloat32);
            if (!torch::isfinite(probabilities).all().item<bool>()) {
                continue;
            }

            const auto action_probabilities = probabilities.gather(1, mini_action_batch);
            const auto new_log_prob = action_probabilities.clamp_min(1e-8).log();
            const auto entropy = -(probabilities * probabilities.clamp_min(1e-8).log()).sum(-1).mean();

            const auto ratio = (new_log_prob - mini_old_log_prob_batch).exp();
            const auto unclipped = ratio * mini_advantage_batch;
            const auto clipped = torch::clamp(ratio, 1.0 - config_.clip_ratio, 1.0 + config_.clip_ratio) * mini_advantage_batch;
            const auto policy_loss = -torch::min(unclipped, clipped).mean();
            const auto value_loss = torch::mse_loss(value_prediction.unsqueeze(1), mini_return_batch);
            const auto loss = policy_loss + static_cast<float>(config_.value_coef) * value_loss - static_cast<float>(config_.entropy_coef) * entropy;

            if (!torch::isfinite(loss).all().item<bool>()) {
                continue;
            }

            optimizer_.zero_grad();
            loss.backward();
            torch::nn::utils::clip_grad_norm_(policy_->parameters(), 1.0);
            optimizer_.step();

            total_loss += loss.item().toDouble();
            total_policy_loss += policy_loss.item().toDouble();
            total_value_loss += value_loss.item().toDouble();
            total_entropy += entropy.item().toDouble();
            ++updates;
        }
    }

    if (updates > 0) {
        metrics.trained = true;
        metrics.loss = total_loss / static_cast<double>(updates);
        metrics.policy_loss = total_policy_loss / static_cast<double>(updates);
        metrics.value_loss = total_value_loss / static_cast<double>(updates);
        metrics.entropy = total_entropy / static_cast<double>(updates);
    }

    rollout_buffer_.clear();
    return metrics;
}

void PPOAgent::set_mode(AgentMode mode) {
    mode_ = mode;
    if (mode_ == AgentMode::Eval) {
        policy_->eval();
    } else {
        policy_->train();
    }
}

AgentMode PPOAgent::mode() const {
    return mode_;
}

void PPOAgent::reset() {
    rollout_buffer_.clear();
}

std::size_t PPOAgent::rollout_size() const noexcept {
    return rollout_buffer_.size();
}

void PPOAgent::save(const std::string& path) {
    torch::save(policy_, path + "_policy.pt");
}

void PPOAgent::load(const std::string& path) {
    torch::load(policy_, path + "_policy.pt");
}

} // namespace rl
