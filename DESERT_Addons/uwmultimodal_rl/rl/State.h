#include "torch/torch.h"

/**
 * Enum that assigns labels to state parameters for easier referral
 */
enum StateLabels {
    AcousticLastNoise,
    AcousticInstantNoise,
    OpticalLastNoise,
    OpticalInstantNoise,
    AcousticLastRxPower,
    OpticalLastRxPower,
    Depth,
    AttenuationCoefficient,
    Distance,
    PacketSize,
    AcousticQueueLength,
    OpticalQueueLength,
    Temperature,
    Shipping,
    Windspeed,
    DataType,
    WaitQueueLength
};

/**
 * Class that represents the state of a node
 */
class State {

    public:
        /**
         * Constructor of the State class
         */
        State(int mask_type);
        /**
         * Destructor of the State class
         */
        ~State() = default;
        /**
         * Set the acoustic last noise parameter
         */
        void setAcousticLastNoise(double ac_last_noise);
        /**
         * Set the acoustic instant noise parameter
         */
        void setAcousticInstantNoise(double ac_inst_noise);
        /**
         * Set the optical last noise parameter
         */
        void setOpticalLastNoise(double op_last_noise);
        /**
         * Set the optical instant noise parameter
         */
        void setOpticalInstantNoise(double op_inst_noise);
        /**
         * Set the acoustic last rx power parameter
         */
        void setAcousticLastRxPower(double ac_last_rx_pw);
        /**
         * Set the optical last rx power parameter
         */
        void setOpticalLastRxPower(double op_last_rx_pw);
        /**
         * Set the depth parameter
         */
        void setDepth(float depth);
        /**
         * Set the attenuation coefficient parameter
         */
        void setAttenuationCoefficient(float c);
        /**
         * Set the distance parameter
         */
        void setDistance(float distance);
        /**
         * Set the packet size parameter
         */
        void setPacketSize(int packet_size);
        /**
         * Set the acoustic queue length parameter
         */
        void setAcousticQueueLength(int aq_length);
        /**
         * Set the optical queue length parameter
         */
        void setOpticalQueueLength(int oq_length);
        /**
         * Set the temperature parameter
         */
        void setTemperature(double temperature);
        /**
         * Set the shipping parameter
         */
        void setShipping(float shipping);
        /**
         * Set the windspeed parameter
         */
        void setWindSpeed(float windspeed);
        /**
         * Set the data type parameter
         */
        void setDataType(int data_type);
        /**
         * Set the wait queue length parameter
         */
        void setWaitQueueLength(int wq_length);
        /**
         * Clip a value to the [a, b] interval
         * @param value Value to be clipped
         * @param a Lower edge of the interval
         * @param b Upper edge of the interval
         */
        double clip(double value, double a, double b) const;
        /**
         * Return a homogeneous vector containing the normalized state
         * parameters filtered using the mask
         * @return homogeneous state vector with selected parameters
         */
        std::vector<double> getHomogeneousState() const;
        /**
         * Return a homogeneous vector containing the entire normalized state
         * @return homogeneous state vector containing all parameters
         */
        std::vector<double> getHomogeneousWholeState() const;
        /**
         * Return a vector containing all unnormalized state parameters
         * @return vector containing all unnormalized state parameters
         */
        std::vector<double> getWholeState() const;

        torch::Tensor toTensor() const;
        /**
         * Return distance
         */
        float getDistance() const;
        /**
         * Return packet size
         */
        int getPacketSize() const;

    protected:
        double acoustic_last_noise_;        /**< Acoustic last noise parameter */
        double acoustic_instant_noise_;     /**< Acoustic instant noise parameter */
        double optical_last_noise_;         /**< Optical last noise parameter */
        double optical_instant_noise_;      /**< Optical instant noise parameter */
        double acoustic_last_rx_power_;     /**< Acoustic last rx power parameter */
        double optical_last_rx_power_;      /**< Optical last rx power parameter */
        float depth_;                       /**< Depth parameter */
        double attenuation_coefficient_;    /**< Attenuation coefficient parameter */
        float distance_;                    /**< Distance parameter */
        int packet_size_;                   /**< Packet size parameter */
        int acoustic_queue_length_;         /**< Acoustic queue length parameter */
        int optical_queue_length_;          /**< Optical queue length parameter */
        double temperature_;                /**< Temperature parameter */
        float shipping_;                    /**< Shipping parameter */
        float wind_speed_;                  /**< Wind speed parameter */
        int data_type_;                     /**< Data type parameter */
        int wait_queue_length_;             /**< Wait queue length parameter */
    
    private:
        int mask_[17];                      /*!< Mask that allows the selection of which
                                                        state parameters to use */
};
