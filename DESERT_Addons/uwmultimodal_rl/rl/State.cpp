#include "rl/State.h"

torch::Tensor
State::toTensor() const {
    const std::vector<double> values = getHomogeneousState();
    return torch::tensor(values, torch::TensorOptions().dtype(torch::kFloat32)).unsqueeze(0);
}


State::State(int mask_type) 
    : acoustic_last_noise_(0.0)
    , acoustic_instant_noise_(0.0)
    , optical_last_noise_(0.0)
    , optical_instant_noise_(0.0)
    , acoustic_last_rx_power_(0.0)
    , optical_last_rx_power_(0.0)
    , depth_(0.0)
    , attenuation_coefficient_(0.0)
    , distance_(0.0)
    , packet_size_(0)
    , acoustic_queue_length_(0)
    , optical_queue_length_(0)
    , temperature_(0.0)
    , shipping_(0.0)
    , wind_speed_(0.0)
    , wait_queue_length_(0)
    , mask_() 
    {
    switch(mask_type) {
        case 0:
            mask_[0]  = 1;// acoustic_last_noise_
            mask_[1]  = 1;// acoustic_instant_noise_
            mask_[2]  = 1;// optical_last_noise_
            mask_[3]  = 1;// optical_instant_noise_
            mask_[4]  = 1;// acoustic_last_rx_power_
            mask_[5]  = 1;// optical_last_rx_power_
            mask_[6]  = 1;// depth_
            mask_[7]  = 1;// attenuation_coefficient_
            mask_[8]  = 1;// distance_
            mask_[9]  = 1;// packet_size_
            mask_[10] = 1;// acoustic_queue_length_
            mask_[11] = 1;// optical_queue_length_
            mask_[12] = 1;// temperature_
            mask_[13] = 1;// shipping_
            mask_[14] = 1;// wind_speed_
            mask_[15] = 1;// wait_queue_length_
            break;
        case 1:
            mask_[0]  = 0;// acoustic_last_noise_
            mask_[1]  = 0;// acoustic_instant_noise_
            mask_[2]  = 0;// optical_last_noise_
            mask_[3]  = 0;// optical_instant_noise_
            mask_[4]  = 0;// acoustic_last_rx_power_
            mask_[5]  = 0;// optical_last_rx_power_
            mask_[6]  = 1;// depth_
            mask_[7]  = 1;// attenuation_coefficient_
            mask_[8]  = 1;// distance_
            mask_[9]  = 1;// packet_size_
            mask_[10] = 1;// acoustic_queue_length_
            mask_[11] = 1;// optical_queue_length_
            mask_[12] = 1;// temperature_
            mask_[13] = 1;// shipping_
            mask_[14] = 1;// wind_speed_
            mask_[15] = 1;// wait_queue_length_
            break;
        case 2:
            mask_[0]  = 0;// acoustic_last_noise_
            mask_[1]  = 0;// acoustic_instant_noise_
            mask_[2]  = 0;// optical_last_noise_
            mask_[3]  = 0;// optical_instant_noise_
            mask_[4]  = 0;// acoustic_last_rx_power_
            mask_[5]  = 0;// optical_last_rx_power_
            mask_[6]  = 1;// depth_
            mask_[7]  = 0;// attenuation_coefficient_
            mask_[8]  = 1;// distance_
            mask_[9]  = 1;// packet_size_
            mask_[10] = 1;// acoustic_queue_length_
            mask_[11] = 1;// optical_queue_length_
            mask_[12] = 0;// temperature_
            mask_[13] = 0;// shipping_
            mask_[14] = 0;// wind_speed_
            mask_[15] = 1;// wait_queue_length_
            break;
        default:
            throw std::invalid_argument("Invalid mask type");
            break;
    }
}

void
State::setOpticalLastNoise(double op_last_noise) {
    optical_last_noise_ = op_last_noise;
}

void
State::setOpticalInstantNoise(double op_inst_noise) {
    optical_instant_noise_ = op_inst_noise;
}

void
State::setOpticalLastRxPower(double op_last_rx_pw) {
    optical_last_rx_power_ = op_last_rx_pw;
}

void
State::setAcousticLastNoise(double ac_last_noise) {
    acoustic_last_noise_ = ac_last_noise;
}

void
State::setAcousticInstantNoise(double ac_inst_noise) {
    acoustic_instant_noise_ = ac_inst_noise;
}

void
State::setAcousticLastRxPower(double ac_last_rx_pw) {
    acoustic_last_rx_power_ = ac_last_rx_pw;
}

void
State::setDataType(int data_type) {
    data_type_ = data_type;
}

void
State::setPacketSize(int packet_size) {
    packet_size_ = packet_size;
}

void
State::setTemperature(double temperature) {
    temperature_ = temperature;
}

void
State::setDepth(float depth) {
    depth_ = depth;
}

void
State::setDistance(float distance) {
    distance_ = distance;
}

void
State::setWindSpeed(float windspeed) {
    wind_speed_ = windspeed;
}

void
State::setShipping(float shipping) {
    shipping_ = shipping;
}

void
State::setAcousticQueueLength(int aq_length) {
    acoustic_queue_length_ = aq_length;
}

void
State::setOpticalQueueLength(int oq_length) {
    optical_queue_length_ = oq_length;
}

void
State::setAttenuationCoefficient(float c) {
    attenuation_coefficient_ = c;
}

void
State::setWaitQueueLength(int wq_length) {
    wait_queue_length_ = wq_length;
}

double 
State::clip(double value, double a, double b) const {
    if (value < a)
        return -1;
    if (value > b)
        return 1;
    double out = ((2 * (value - a)) / (b - a)) - 1;
    return out;
}

std::vector<double>
State::getHomogeneousState() const {
    std::vector<double> whole_homogeneous_state = getHomogeneousWholeState();
    std::vector<double> homogeneous_state;
    homogeneous_state.push_back(1);
    for (int i = 1; i < 18; i++) {
        if (mask_[i - 1] == 1)
            homogeneous_state.push_back(whole_homogeneous_state[i]);
    }
    return homogeneous_state;
}

std::vector<double>
State::getHomogeneousWholeState() const {
    std::vector<double> whole_state = getWholeState();
    std::vector<double> normalized_state;
    normalized_state.push_back(1);
    for (int i = 0; i < 17; i++) {
        double temp = whole_state[i];
        switch (i) {
            case StateLabels::AcousticLastNoise:
                temp = clip(temp, 0, 5 * 10e9);
                break;
            case StateLabels::AcousticInstantNoise:
                temp = clip(temp, 0, 5 * 10e9);
                break;
            case StateLabels::OpticalLastNoise:
                temp = clip(temp, 0, 10e-15);
                break;
            case StateLabels::OpticalInstantNoise:
                temp = clip(temp, 0, 10e-15);
                break;
            case StateLabels::AcousticLastRxPower:
                temp = clip(log(temp + 1), 21.41, 36.61);
                break;
            case StateLabels::OpticalLastRxPower:
                temp = clip(temp, 0, 1.5);
                break;
            case StateLabels::Depth:
                temp = clip(temp, -1000, 0);
                break;
            case StateLabels::AttenuationCoefficient:
                temp = clip(temp, 0.15, 0.40);
                break;
            case StateLabels::Distance:
                temp = clip(log(temp+1), 0, 8.7);
                break;
            case StateLabels::PacketSize:
                temp = clip(temp, 0, 1024);
                break;
            case StateLabels::AcousticQueueLength:
                temp = clip(temp, 0, 1000);
                break;
            case StateLabels::OpticalQueueLength:
                temp = clip(temp, 0, 1000);
                break;
            case StateLabels::Temperature:
                temp = clip(temp, 273.15, 313.15);
                break;
            case StateLabels::Shipping:
                temp = clip(temp, 0, 1);
                break;
            case StateLabels::Windspeed:
                temp = clip(temp, 0, 20);
                break;
            case StateLabels::DataType:
                temp = clip(temp, 0, 1);
                break;
            case StateLabels::WaitQueueLength:
                temp = clip(temp, 0, 1000);
                break;        
            default:
                break;
        }
        normalized_state.push_back(temp);
    }
    return normalized_state;
}

std::vector<double>
State::getWholeState() const {
    std::vector<double> whole_state;
    whole_state.push_back(acoustic_last_noise_);
    whole_state.push_back(acoustic_instant_noise_);
    whole_state.push_back(optical_last_noise_);
    whole_state.push_back(optical_instant_noise_);
    whole_state.push_back(acoustic_last_rx_power_);
    whole_state.push_back(optical_last_rx_power_);
    whole_state.push_back(depth_);
    whole_state.push_back(attenuation_coefficient_);
    whole_state.push_back(distance_);
    whole_state.push_back(packet_size_);
    whole_state.push_back(acoustic_queue_length_);
    whole_state.push_back(optical_queue_length_);
    whole_state.push_back(temperature_);
    whole_state.push_back(shipping_);
    whole_state.push_back(wind_speed_);
    whole_state.push_back(data_type_);
    whole_state.push_back(wait_queue_length_);
    return whole_state;
}

float
State::getDistance() const {
    return distance_;
}

int
State::getPacketSize() const {
    return packet_size_;
}
