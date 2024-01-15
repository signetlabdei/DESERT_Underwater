//
// Copyright (c) 2019 Regents of the SIGNET lab, University of Padova.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. Neither the name of the University of Padova (SIGNET lab) nor the
//    names of its contributors may be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/**
 * @file uwmodamodem.h
 * @author Roberto Francescon
 * @version 0.0.1
 * @brief Driver for the MODA SDM acoustic UW modem
 */

#ifndef UWMODAMODEM_H
#define UWMODAMODEM_H

#include <uwmodem.h>
#include <uwconnector.h>
#include <uwsocket.h>

#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <vector>
#include <map>
#include <string>

class UwMODAModem : public UwModem
{

public:

    /**
     * Enum type for the modem general state.
     * AVAILABLE: immediately available to perform an operation
     * TRANSMITTING: executing a tranmission command: must wait for it ot complete
     * CONFIGURING: executing a configuration command: must wait for it to complete
     */
    enum class ModemState {AVAILABLE = 0, TRANSMITTING, RECEIVING, CONFIGURING};

    /**
     * Enum type representing modem responses.
     * UNDEFINED: unrecognized signaling
     * TX_END: modem signals that it has ended a transmission
     * RX_BEG: modem signals that it has begun receiving data
     * CFG_END: modem signals that it has ended an ongoing configuration
     */
    enum class ModemResponse {
      UNDEFINED = 0,
      TX_END,
      RX_BEG,
      CFG_END
    };

    /**
     * Constructor of the UwMODAModem class
     */
    UwMODAModem();

    /**
     * Destructor of the UwMODAModem class
     */
    virtual ~UwMODAModem();

    /**
     * Method that handles the reception of packets arriving from upper layers
     * of the network simulator.
     * @param p pointer to the packet that has been received from the simulator
     *        upper layers
     */
    virtual void recv(Packet *p);

    /**
     * Tcl command interpreter: Method that maps Tcl commands into C++ methods.
     *
     * @param argc number of arguments in <i> argv </i>
     * @param argv array of strings which are the command parameters
     * 		  (Note that <i>argv[0]</i> is the name of the object).
     * @return TCL_OK or TCL_ERROR whether the command has been dispatched
     *		   successfully or not
     */
    virtual int command(int argc, const char *const *argv);

    /**
     * Method that returns the modulation type used for the packet being
     * transmitted. Inherited from MPhy, in NS-MIRACLE, could be left empty if
     * no way exists to retrieve this information
     * @param p Packet pointer to the given packet being transmitted
     * @param modulation type represented by an integer
     */
    virtual int
    getModulationType(Packet *p);

    /**
     * Cross-Layer messages synchronous interpreter.
     *
     * @param ClMessage* an instance of ClMessage that represent the
     * message received
     * @return <i>0</i> if successful.
     */
    virtual int recvSyncClMsg(ClMessage *m);

protected:

     /**
      * Method that triggers the transmission of a packet through a specified
      * modem.
      * @param p Packet pointer to the packet to be sent
      */
     virtual void startTx(Packet *p);

     /**
      * Method that starts a packet reception. This method is also in charge of
      * sending a CrLayerMsg, Phy2MacStartRx(p), to notify the upper layers of
      * the simulator about the start of the reception
      * @param p Packet pointer to the packet to be received
      */
     virtual void startRx(Packet *p);

     /**
      * Method that ends a packet reception. This method is also in charge of
      * sending the received NS-MIRACLE packet to the upper layers
      * @param p Packet pointer to the packet being sent
      */
     virtual void endRx(Packet *p);

     ModemState status; /**< Variable holding the current status of the modem */

private:

    /**
     * Method that starts the driver operations. It performs all the needed
     * operations to correctly fire up the device's driver.
     */
    void start();

    /**
     * Method that stops the driver operations. It performs all the needed
     * operations to correctly stop the device's driver before closing
     * operations.
     */
    void stop();

    /**
     * Method that dispacthes a thread dedicated to receiving signaling from the
     * signaling connector
     * Allows to be compliant with modem opeations: if channel fails, driver
     * must stop operations until signaling resumes
     */
    void receivingSignaling();

    /**
     * Method that dispatch a thread dedicated to receiving data from the data 
     * connector
     */
    void receivingData();

   /**
    * Method that creates a packet from the received stream of bytes
    * @param p allocated empty packet to fill in with the received bytes
    */
    void createRxPacket(Packet *p);

    /**
     * Method that dispatches a thread dedicated to transmitting data through
     * the data connector
     */
    void transmittingData();

    /**
     * Method that parses the content of the signaling data buffer to retrieve
     * signaling messages
     */
    ModemResponse parseSignaling(std::vector<char>::iterator& end_it);

    /**
     * Method that, based on the received signaling, updates the state machine
     * of the driver
     * @param response ModemRresponse response from the modem
     */
    void updateStatus(ModemResponse response);

    /** Mutex associated with the state machine of the modem */
    std::mutex status_m;
    /** Condition variable that is linked with the status variable */
    std::condition_variable status_cv;
    /** Mutex associated with the transmission queue */
    std::mutex tx_queue_m;
    /** Condition variable that is linked with the transmitting queue */
    std::condition_variable tx_queue_cv;
    /** Atomic boolean variable that controls the receiving looping thread */
    std::atomic<bool> receiving;
    /** Atomic boolean variable that controls the transmitting looping thread */
    std::atomic<bool> transmitting;

    /** String that is updated with each new received message */
    std::string rx_payload;
    /** Size of each new received message, coming from signaling */
    int rx_size;

    std::thread sig_thread; /**< Thread managing the signaling reception process */
    std::thread rx_thread; /**< Thread managing the data reception process */
    std::thread tx_thread; /**< Thread managing the data transmission process */

    /** Dictionary of accepted signaling states */
    static std::vector<std::pair<std::string, ModemResponse> > signaling_dict;

   
   /** Dictionary for converting a state to a printable string */
   static std::map<ModemState, std::string> stateToString;

   /**
    * Signaling connector: used ot retrieve signaling coming from the modem
    * signaling's channel
    */
    std::unique_ptr<UwConnector> signal_conn;
    static const int SIGNALING_ADDRESS; /**< Port of the signaling channel */

   /**
    * Data connector: used ot retrieve data coming from the modem data socket
    */
    std::unique_ptr<UwConnector> data_conn;
    static const int DATA_ADDRESS; /**< Port of the data channel */

    /** Bytes buffer for the signaling channel (unparsed data) */
    std::vector<char> signal_buffer;

    /** Maximum time to wait for modem to become ModemState::AVAILABLE */
    const static std::chrono::milliseconds MODEM_TIMEOUT;

    /** String separator used in reception signaling */
    const std::string sep = {"::"};
    /** String end delimiter used in reception signaling */
    const std::string end_delim = {";"};

    /**
     * Signaling tag to recognize signaling from the modem. It is not necessary
     * but added in case of future additional features that want to use the
     * signaling channel
     */
    std::string signal_tag;

    std::string signal_address;

};

#endif
