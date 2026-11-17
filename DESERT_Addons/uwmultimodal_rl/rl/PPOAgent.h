#pragma once

#include <memory>
#include <random>

#include "rl/Agent.h"
#include "rl/Networks.h"
#include "rl/RolloutBuffer.h"

namespace rl {

class PPOAgent final : public Agent {
public:
    explicit PPOAgent(const AgentConfig& config);

    ActionResult act(const torch::Tensor& state) override;
    void observe(const Transition& transition) override;
    TrainMetrics train_step() override;
    void set_mode(AgentMode mode) override;
    AgentMode mode() const override;
    void reset() override;
    void save(const std::string& path) override;
    void load(const std::string& path) override;

    [[nodiscard]] std::size_t rollout_size() const noexcept;

private:
    std::pair<torch::Tensor, torch::Tensor> evaluate_policy(const torch::Tensor& states);
    torch::Tensor compute_last_value();

    AgentMode mode_ = AgentMode::Train;

    ActorCritic policy_;
    torch::optim::Adam optimizer_;
    RolloutBuffer rollout_buffer_;
    mutable std::mt19937 rng_;
};

} // namespace rl
