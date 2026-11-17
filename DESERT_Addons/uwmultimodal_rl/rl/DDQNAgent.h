#pragma once

#include <memory>
#include <random>

#include "rl/Agent.h"
#include "rl/Networks.h"
#include "rl/ReplayBuffer.h"

namespace rl {

class DDQNAgent final : public Agent {
public:
    explicit DDQNAgent(const AgentConfig& config);

    ActionResult act(const torch::Tensor& state) override;
    void observe(const Transition& transition) override;
    TrainMetrics train_step() override;
    void set_mode(AgentMode mode) override;
    AgentMode mode() const override;
    void reset() override;
    void save(const std::string& path) override;
    void load(const std::string& path) override;

    [[nodiscard]] std::size_t replay_size() const noexcept;

private:
    ActionResult greedy_action(const torch::Tensor& state);
    ActionResult random_action();
    void sync_target_network();

    AgentMode mode_ = AgentMode::Train;

    QNetwork online_net_;
    QNetwork target_net_;
    torch::optim::Adam optimizer_;
    ReplayBuffer replay_buffer_;
    mutable std::mt19937 rng_;
    std::uniform_real_distribution<double> uniform_{0.0, 1.0};
    std::size_t steps_ = 0;
};

} // namespace rl
