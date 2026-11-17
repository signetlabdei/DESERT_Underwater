#pragma once

#include <random>
#include <string>
#include <vector>

#include "rl/Agent.h"
#include "rl/Types.h"

namespace rl {

class DSGSARSAAgent final : public Agent {
public:
    explicit DSGSARSAAgent(const AgentConfig& config);

    ActionResult act(const torch::Tensor& state) override;
    void observe(const Transition& transition) override;
    TrainMetrics train_step() override;
    void set_mode(AgentMode mode) override;
    AgentMode mode() const override;
    void reset() override;
    void save(const std::string& path) override;
    void load(const std::string& path) override;

    void set_output_path(const std::string& output_path);

private:
    std::vector<double> tensor_to_vector(const torch::Tensor& state) const;
    double dot_product(const std::vector<double>& lhs, const std::vector<double>& rhs) const;
    int64_t greedy_action(const torch::Tensor& state) const;
    int64_t sample_action() const;
    void append_log_line(const Transition& transition, double normalized_reward);
    void save_model(const std::string& path) const;
    void load_model(const std::string& path);

    AgentMode mode_ = AgentMode::Train;
    std::vector<std::vector<double>> weights_;
    double r_bar_ = 3000.0;
    std::vector<Transition> pending_;
    mutable std::mt19937 rng_;
    std::uniform_real_distribution<double> uniform_{0.0, 1.0};
    std::string output_path_;
};

} // namespace rl