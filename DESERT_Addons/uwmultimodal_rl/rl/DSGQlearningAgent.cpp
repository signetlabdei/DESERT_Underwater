#include "rl/DSGQlearningAgent.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace rl {

namespace {

std::vector<double> flatten_state(const torch::Tensor& state) {
    const auto tensor = ensure_float_state(state).squeeze(0).contiguous();
    std::vector<double> values;
    values.reserve(static_cast<std::size_t>(tensor.numel()));

    for (int64_t index = 0; index < tensor.numel(); ++index) {
        values.push_back(tensor[index].item<double>());
    }

    return values;
}

} // namespace

DSGQlearningAgent::DSGQlearningAgent(const AgentConfig& config)
    : Agent(config),
      rng_(std::random_device{}()),
      output_path_(config.output_path) {
    if (config_.state_dim <= 0 || config_.action_dim <= 0) {
        throw std::invalid_argument("state_dim and action_dim must be greater than zero");
    }

    weights_.resize(static_cast<std::size_t>(config_.action_dim));
    for (auto& row : weights_) {
        row.assign(static_cast<std::size_t>(config_.state_dim), 0.0);
        row.front() = 1.0;
    }
}

ActionResult DSGQlearningAgent::act(const torch::Tensor& state) {
    ActionResult result;
    result.action = mode_ == AgentMode::Eval ? greedy_action(state) : greedy_action(state);

    if (mode_ != AgentMode::Eval && uniform_(rng_) < config_.epsilon) {
        result.action = sample_action();
    }

    return result;
}

void DSGQlearningAgent::observe(const Transition& transition) {
    if (mode_ == AgentMode::Eval) {
        return;
    }

    pending_.push_back(transition);
}

TrainMetrics DSGQlearningAgent::train_step() {
    TrainMetrics metrics;
    if (mode_ == AgentMode::Eval || pending_.empty()) {
        return metrics;
    }

    double total_loss = 0.0;
    std::size_t updates = 0;

    for (const auto& transition : pending_) {
        if (!transition.next_state.defined() || transition.done) {
            continue;
        }

        const auto hs1 = tensor_to_vector(transition.state);
        const auto hs2 = tensor_to_vector(transition.next_state);
        if (transition.action < 0 || transition.action >= config_.action_dim) {
            continue;
        }

        const int64_t best_next_action = greedy_action(transition.next_state);
        const double q_s2a2 = dot_product(weights_[static_cast<std::size_t>(best_next_action)], hs2);
        const double q_s1a1 = dot_product(weights_[static_cast<std::size_t>(transition.action)], hs1);
        const double delta = transition.reward - r_bar_ + q_s2a2 - q_s1a1;

        r_bar_ = r_bar_ + config_.beta * delta;
        auto& selected_row = weights_[static_cast<std::size_t>(transition.action)];
        for (std::size_t i = 0; i < selected_row.size() && i < hs1.size(); ++i) {
            selected_row[i] = selected_row[i] + config_.alpha * delta * hs1[i];
        }

        append_log_line(transition, transition.reward);
        total_loss += std::abs(delta);
        ++updates;
    }

    pending_.clear();

    if (updates > 0) {
        metrics.trained = true;
        metrics.loss = total_loss / static_cast<double>(updates);
    }

    return metrics;
}

void DSGQlearningAgent::set_mode(AgentMode mode) {
    mode_ = mode;
}

AgentMode DSGQlearningAgent::mode() const {
    return mode_;
}

void DSGQlearningAgent::reset() {
    pending_.clear();
}

void DSGQlearningAgent::save(const std::string& path) {
    save_model(path);
}

void DSGQlearningAgent::load(const std::string& path) {
    load_model(path);
}

void DSGQlearningAgent::set_output_path(const std::string& output_path) {
    output_path_ = output_path;
}

std::vector<double> DSGQlearningAgent::tensor_to_vector(const torch::Tensor& state) const {
    return flatten_state(state);
}

double DSGQlearningAgent::dot_product(const std::vector<double>& lhs, const std::vector<double>& rhs) const {
    const std::size_t limit = std::min(lhs.size(), rhs.size());
    double product = 0.0;
    for (std::size_t i = 0; i < limit; ++i) {
        product += lhs[i] * rhs[i];
    }
    return product;
}

int64_t DSGQlearningAgent::greedy_action(const torch::Tensor& state) const {
    const auto values = tensor_to_vector(state);
    std::vector<double> q_values;
    q_values.reserve(weights_.size());

    for (const auto& row : weights_) {
        q_values.push_back(dot_product(row, values));
    }

    const auto best_it = std::max_element(q_values.begin(), q_values.end());
    return static_cast<int64_t>(std::distance(q_values.begin(), best_it));
}

int64_t DSGQlearningAgent::sample_action() const {
    std::uniform_int_distribution<int64_t> distribution(0, config_.action_dim - 1);
    return distribution(rng_);
}

void DSGQlearningAgent::append_log_line(const Transition& transition, double normalized_reward) {
    if (output_path_.empty() || config_.mac_addr != 254) {
        return;
    }

    std::ofstream outfile(output_path_, std::ios_base::app);
    outfile << "ACTOR_LOG" << config_.mac_addr << ":";

    const auto state_values = tensor_to_vector(transition.state);
    for (std::size_t i = 0; i < state_values.size(); ++i) {
        if (i != 0) {
            outfile << ",";
        }
        outfile << state_values[i];
    }

    for (int64_t action = 0; action < config_.action_dim; ++action) {
        outfile << "," << (transition.action == action ? 1 : 0);
    }
}

void DSGQlearningAgent::save_model(const std::string& path) const {
    std::string complete_path = path + ".txt";
    std::ofstream outfile(complete_path);
    for (const auto& row : weights_) {
        for (std::size_t index = 0; index < row.size(); ++index) {
            if (index != 0) {
                outfile << ",";
            }
            outfile << row[index];
        }
        outfile << "\n";
    }
    outfile << "\n" << r_bar_ << "\n\n";
}

void DSGQlearningAgent::load_model(const std::string& path) {
    std::string complete_path = path + ".txt";
    std::ifstream infile(complete_path);
    std::string line;
    std::string word;
    std::vector<double> row;
    std::size_t line_index = 0;

    while (std::getline(infile, line)) {
        if (line.empty()) {
            break;
        }

        row.clear();
        std::stringstream stream(line);
        while (std::getline(stream, word, ',')) {
            row.push_back(std::stod(word));
        }

        if (line_index < weights_.size()) {
            weights_[line_index++] = row;
        }
    }

    if (std::getline(infile, line)) {
        std::stringstream stream(line);
        std::getline(stream, word, '\n');
        r_bar_ = std::stod(word);
    }
}

} // namespace rl