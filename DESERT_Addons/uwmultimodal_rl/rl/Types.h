#pragma once

#include <cstdint>
#include <stdexcept>

#include <torch/torch.h>

namespace rl {

enum class AgentMode {
    Train,
    Eval,
};

struct Transition {
    torch::Tensor state;
    int64_t action = 0;
    int64_t next_action = -1;  // Next action for SARSA-style algorithms
    float reward = 0.0F;
    torch::Tensor next_state;
    bool done = false;
    torch::Tensor log_prob;
    torch::Tensor value;
    bool has_log_prob = false;
    bool has_value = false;
};

struct ActionResult {
    int64_t action = 0;
    torch::Tensor log_prob;
    torch::Tensor value;
};

struct TrainMetrics {
    bool trained = false;
    double loss = 0.0;
    double policy_loss = 0.0;
    double value_loss = 0.0;
    double entropy = 0.0;
};

struct AgentConfig {
    int64_t state_dim = 0;
    int64_t action_dim = 0;
    int64_t hidden_dim = 128;

    double learning_rate = 3e-4;
    double gamma = 0.99;

    std::size_t batch_size = 64;

    std::size_t replay_capacity = 10000;
    std::size_t target_update_steps = 1000;

    std::size_t rollout_capacity = 2048;
    std::size_t ppo_epochs = 4;
    std::size_t minibatch_size = 64;
    double clip_ratio = 0.2;
    double value_coef = 0.5;
    double entropy_coef = 0.01;
    double gae_lambda = 0.95;

    double alpha = 0.95;
    double beta = 0.95;
    double epsilon = 1.0;
    
    int mac_addr = -1;
    std::string output_path = "";
};

inline torch::Tensor ensure_float_state(const torch::Tensor& state) {
    if (!state.defined()) {
        throw std::invalid_argument("state tensor must be defined");
    }

    auto tensor = state.to(torch::kFloat32);
    if (tensor.dim() == 1) {
        return tensor.unsqueeze(0);
    }

    if (tensor.dim() == 2 && tensor.size(0) == 1) {
        return tensor;
    }

    throw std::invalid_argument("expected a 1D state tensor or a batch of size 1");
}

inline torch::Tensor scalar_float(float value) {
    return torch::tensor(value, torch::TensorOptions().dtype(torch::kFloat32));
}

inline torch::Tensor scalar_int(int64_t value) {
    return torch::tensor(value, torch::TensorOptions().dtype(torch::kInt64));
}

} // namespace rl
