#pragma once

#include <torch/torch.h>

namespace rl {

struct QNetworkImpl : torch::nn::Module {
    QNetworkImpl(int64_t state_dim, int64_t action_dim, int64_t hidden_dim);
    torch::Tensor forward(const torch::Tensor& state);

    torch::nn::Sequential layers{nullptr};
};

TORCH_MODULE(QNetwork);

struct ActorCriticImpl : torch::nn::Module {
    ActorCriticImpl(int64_t state_dim, int64_t action_dim, int64_t hidden_dim);
    std::pair<torch::Tensor, torch::Tensor> forward(const torch::Tensor& state);

    torch::nn::Sequential shared{nullptr};
    torch::nn::Linear policy_head{nullptr};
    torch::nn::Linear value_head{nullptr};
};

TORCH_MODULE(ActorCritic);

} // namespace rl
