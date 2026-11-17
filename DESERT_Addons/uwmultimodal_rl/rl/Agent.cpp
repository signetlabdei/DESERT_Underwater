#include "rl/Agent.h"
#include <fstream>



namespace rl {

Agent::Agent(const AgentConfig& config) : config_(config) {
}

void Agent::store_state(const torch::Tensor& state) {
    state_buffer_.push_back(state);
}

torch::Tensor 
Agent::get_nth_last_state(std::size_t n) const {
    return state_buffer_.get_nth_last(n);
}

void
Agent::write_header() {
    std::ofstream outfile;
    outfile.open(config_.output_path);
    // outfile << "Acoustic State/Acoustic Last Noise,"
    //         << "Acoustic State/Acoustic Instant Noise,"
    //         << "Optical State/Optical Last Noise,"
    //         << "Optical State/Optical Instant Noise,"
    //         << "Acoustic State/Acoustic Last Rx Power,"
    //         << "Optical State/Optical Last Rx Power,"
    //         << "State/Depth,"
    //         << "Optical State/Attenuation,"
    //         << "State/Distance,"
    //         << "State/Packet Size,"
    //         << "Acoustic State/Acoustic Queue Length,"
    //         << "Optical State/Optical Queue Length,"
    //         << "Optical State/Temperature,"
    //         << "Acoustic State/Shipping,"
    //         << "Acoustic State/Wind Speed,"
    //         << "State/Data Type,"
    //         << "State/Wait Queue Length,"
    //         << "Action/Acoustic,"
    //         << "Action/Optical,"
    //         << "Action/Wait,"
    //         << "Action/Drop,"
    //         << "Reward/Reward,"
    //         << "Timestamp/Action Time,"
    //         << "Packet_uid/Packet_uid,"
    //         << "Acoustic State/Normalized Acoustic Last Noise,"
    //         << "Acoustic State/Normalized Acoustic Instant Noise,"
    //         << "Optical State/Normalized Optical Last Noise,"
    //         << "Optical State/Normalized Optical Instant Noise,"
    //         << "Acoustic State/Normalized Acoustic Last Rx Power,"
    //         << "Optical State/Normalized Optical Last Rx Power,"
    //         << "State/Normalized Depth,"
    //         << "Optical State/Normalized Attenuation,"
    //         << "State/Normalized Distance,"
    //         << "State/Normalized Packet Size,"
    //         << "Acoustic State/Normalized Acoustic Queue Length,"
    //         << "Optical State/Normalized Optical Queue Length,"
    //         << "Optical State/Normalized Temperature,"
    //         << "Acoustic State/Normalized Shipping,"
    //         << "Acoustic State/Normalized Wind Speed,"
    //         << "State/Normalized Data Type,"
    //         << "State/Normalized Wait Queue Length,"
    //         << "State/Rx Time\n";

    outfile << "Packet_uid/Packet_uid,"
            << "Timestamp/Generation Time,"
            << "Timestamp/Action Time,"
            << "Timestamp/Rx Time,"
            << "State/Acoustic Last Noise,"
            << "State/Acoustic Instant Noise,"
            << "State/Optical Last Noise,"
            << "State/Optical Instant Noise,"
            << "State/Acoustic Last Rx Power,"
            << "State/Optical Last Rx Power,"
            << "State/Depth,"
            << "State/Attenuation,"
            << "State/Distance,"
            << "State/Packet Size,"
            << "State/Acoustic Queue Length,"
            << "State/Optical Queue Length,"
            << "State/Temperature,"
            << "State/Shipping,"
            << "State/Wind Speed,"
            << "State/Data Type,"
            << "State/Wait Queue Length,"
            << "Action/Acoustic,"
            << "Action/Optical,"
            << "Action/Wait,"
            << "Action/Drop,"
            << "Normalized/Acoustic Last Noise,"
            << "Normalized/Acoustic Instant Noise,"
            << "Normalized/Optical Last Noise,"
            << "Normalized/Optical Instant Noise,"
            << "Normalized/Acoustic Last Rx Power,"
            << "Normalized/Optical Last Rx Power,"
            << "Normalized/Depth,"
            << "Normalized/Attenuation,"
            << "Normalized/Distance,"
            << "Normalized/Packet Size,"
            << "Normalized/Acoustic Queue Length,"
            << "Normalized/Optical Queue Length,"
            << "Normalized/Temperature,"
            << "Normalized/Shipping,"
            << "Normalized/Wind Speed,"
            << "Normalized/Data Type,"
            << "Normalized/Wait Queue Length,"
            << "Reward/Received,"
            << "Reward/Delay,"
            << "Reward/Reward\n";
            
    outfile.close();
}

void
Agent::logTransition(int uid,
                     State s, 
                     int a, 
                     double r, 
                     double generation_time, 
                     double action_time, 
                     double rx_time,
                     bool received,
                     double delay) {

    std::ofstream outfile;
    outfile.open(config_.output_path, std::ios_base::app);
    int in = 0;

    outfile << uid << "," ;
    outfile << generation_time << ",";
    outfile << action_time << ",";
    if (received) {
        outfile << rx_time << ",";
    } else {
        outfile << "NaN" << ",";
    }

    for (auto& v : s.getWholeState()) {
        outfile << v << ",";
    }

    for (int i = 0; i < config_.action_dim; i++) {
        if (a == i) {
            outfile << "1,";
        } else {
            outfile << "0,";
        }
    }
    
    int t = 0;
    for (auto& v : s.getHomogeneousWholeState()) {
        if (t != 0) { 
            outfile << v << ",";
        }
        t++;
    }

    if (received) {
        outfile << "1,";
        outfile << delay << "," ;
    } else {
        outfile << "0,";
        outfile << "NaN," ;
    }
    
    outfile << r << "\n";
    outfile.close();
}

} // namespace rl