#pragma once

#include <torch/torch.h>

#include "rl/Types.h"
#include "rl/CircularArray.h"
#include "rl/State.h"

namespace rl {
    
class Agent {
public:
    Agent(const AgentConfig& config);
    virtual ~Agent() = default;

    virtual ActionResult act(const torch::Tensor& state) = 0;
    virtual void observe(const Transition& transition) = 0;
    virtual TrainMetrics train_step() = 0;
    virtual void set_mode(AgentMode mode) = 0;
    virtual AgentMode mode() const = 0;
    virtual void save(const std::string& path) = 0;
    virtual void load(const std::string& path) = 0;
    virtual void reset() = 0;

    void store_state(const torch::Tensor& state);
    torch::Tensor get_nth_last_state(std::size_t n) const;
    void write_header();
    void logTransition(int uid, State s, int a, double r, double generation_time, double action_time, double rx_time, bool received, double delay);

protected:
    CircularArray<torch::Tensor> state_buffer_{2}; // Buffer to store the last 2 states
    const AgentConfig config_;
};

} // namespace rl
