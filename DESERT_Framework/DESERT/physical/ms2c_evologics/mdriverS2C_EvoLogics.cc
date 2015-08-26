//
// Copyright (c) 2015 Regents of the SIGNET lab, University of Padova.
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
 * @file mdriverS2C_EvoLogics.cc
 * @author Riccardo Masiero and Federico Favaro
 * \version 1.0.0
 * \brief Implementation of the MdriverS2C_EvoLogics class.
 */


#include "mdriverS2C_EvoLogics.h"
#include <uwmphy_modem.h>

static void hexdump(std::string name, std::string str) {
    int len = str.size();
    const char *data = str.c_str();

    std::cout << name << "[" << len << "]: " << std::hex;
    for (int i = 0; i < len; i++) {
        std::cout.fill('0');
        std::cout.width(2);
        std::cout << std::right << (unsigned int) (unsigned char) data[i];

        if (std::isalnum(data[i]) || std::ispunct(data[i]))
            std::cout << "(" << data[i] << ")";
        std::cout << " ";
    }

    std::cout.width(0);
    std::cout << std::dec << std::endl;
}

static std::string hexdumplog(std::string str) {
    int len = str.size();
    const char *data = str.c_str();
    std::string str_out = "";

    for (int i = 0; i < len; i++) {
        if (std::isalnum(data[i]) || std::ispunct(data[i]))
            str_out += data[i];
        else {
            std::string str;
            std::stringstream sstr("");
            sstr << "[" << std::hex << (unsigned int) (unsigned char) data[i] << std::dec << "]";
            sstr >> str;
            str_out += str;
        }
    }
    return str_out;
}

MdriverS2C_EvoLogics::MdriverS2C_EvoLogics(UWMPhy_modem* pmModem_) : UWMdriver(pmModem_), mInterpreter(this), mConnector(this, pmModem_->getPathToDevice()) {
    m_status_tx = _IDLE;
    m_status_rx = _IDLE;
    setConnections(&mInterpreter, &mConnector);
}

MdriverS2C_EvoLogics::~MdriverS2C_EvoLogics() {
}

void MdriverS2C_EvoLogics::start() {

    if (debug_ >= 2) {
        cout << NOW << "MS2C_EVOLOGICS(" << ID << ")::START" << endl;
    }
    mConnector.openConnection();
    if (getKeepOnlineMode()) {
        if (debug_ >= 0) cout << "MS2C_EVOLOGICS(" << ID << ")::SETTING_KEEP_ONLINE_MODALITY" << endl;
        status = _CFG;
        m_status_tx = _TXKO;
        modemTxManager();
    } else if (SetModemID) {
        if (debug_ >= 0) cout << NOW << "MS2C_EVOLOGICS(" << ID << ")::SETTING_MODEM_ID" << endl; 
        status = _CFG;
        m_status_tx = _SETID;
        modemTxManager();
    } else {
        status = _IDLE;
    }
}

void MdriverS2C_EvoLogics::emptyModemQueue() {
    status = _RESET;
    m_status_tx = _DROPBUFFER;
    modemTxManager();
}

void MdriverS2C_EvoLogics::stop() {
    if (getKeepOnlineMode())
    {
        m_status_tx = _CLOSE;
        status = _QUIT;
        modemTxManager();
    } else {
        mConnector.closeConnection();
    }
}

void MdriverS2C_EvoLogics::modemTx() {
    status = _TX;
    m_status_tx = _IM;
    modemTxManager();
}

void MdriverS2C_EvoLogics::modemTxBurst() {
    status = _TX;
    m_status_tx = _BURST;
    modemTxManager();
}

void MdriverS2C_EvoLogics::modemTxPBM() {
    status = _TX;
    m_status_tx = _PBM;
    modemTxManager();
}

void MdriverS2C_EvoLogics::modemSetID() {
    modemTxManager();
}

int MdriverS2C_EvoLogics::updateStatus() {
    if (debug_ >= 2) {
        cout << NOW << "MS2C_EVOLOGICS(" << ID << ")::STATUS_" << status << "_M_STATUS_TX_" << m_status_tx << "_M_STATUS_RX_" << m_status_rx << endl;
    }
    std::string rx_msg;
    bool cread = true;

    while (cread) {
        rx_msg = mConnector.readFromModem();

        if (rx_msg != "") {

            if (getLog()) {
                outLog.open((getLogFile()).c_str(), ios::app);
                outLog << left << "[" << pmModem->getEpoch() << "]::" << NOW << "::MS2C_EVOLOGICS_DRIVER(" << ID << ")::UPDATE_STATUS::RX_MSG = " << hexdumplog(rx_msg) << endl;
                outLog.flush();
                outLog.close();
            }

            std::string pr_msg;
            std::string parser("\r\n");
            size_t p_offset = 0;
            size_t p_parser = rx_msg.find(parser);

            while (p_parser != std::string::npos) {
                pr_msg = rx_msg.substr(p_offset, p_parser + parser.size() - p_offset);

                if (pr_msg.find("RECVIM") != string::npos) {

                    queue_rx.push(pr_msg);

                    if (debug_ >= 2) {
                        hexdump("MS2C_EVOLOGICS::UPDATESTATUS::RECVIM::", pr_msg);
                    }
                    if (getLog()) {
                        outLog.open((getLogFile()).c_str(), ios::app);
                        outLog << left << "[" << pmModem->getEpoch() << "]::" << NOW << "::MS2C_EVOLOGICS_DRIVER(" << ID << ")::UPDATE_STATUS::RECVIM = " << hexdumplog(pr_msg) << endl;
                        outLog.flush();
                        outLog.close();
                    }

                } else if (pr_msg.find("RECV") != string::npos) {

                    queue_rx.push(pr_msg);
                    if (debug_ >= 2) {
                        hexdump("MS2C_EVOLOGICS::UPDATESTATUS::RECV::", pr_msg);
                    }
                    if (getLog()) {
                        outLog.open((getLogFile()).c_str(), ios::app);
                        outLog << left << "[" << pmModem->getEpoch() << "]::" << NOW << "::MS2C_EVOLOGICS_DRIVER(" << ID << ")::UPDATE_STATUS::RECV = " << hexdumplog(pr_msg) << endl;
                        outLog.flush();
                        outLog.close();
                    }
                } else if (pr_msg.find("OK") != string::npos) {

                    queue_tx.push(pr_msg);
                    if (debug_ >= 2) {
                        hexdump("MS2C_EVOLOGICS::UPDATESTATUS::", pr_msg);
                    }

                } else if (pr_msg.find("BUSY CLOSING CONNECTION") != string::npos) {
                    if (debug_ >= 2) {
                        hexdump("MS2C_EVOLOGICS::UPDATESTATUS::", pr_msg);
                    }
                    if (getLog()) {
                        outLog.open((getLogFile()).c_str(), ios::app);
                        outLog << left << "[" << pmModem->getEpoch() << "]::" << NOW << "::MS2C_EVOLOGICS_DRIVER(" << ID << ")::UPDATE_STATUS::BUSY = " << hexdumplog(pr_msg) << endl;
                        outLog.flush();
                        outLog.close();
                    }

                    queue_tx.push(pr_msg);

                } else if (pr_msg.find("BUSY BACKOFF STATE") != string::npos) {
                    if (debug_ >= 2) {
                        hexdump("MS2C_EVOLOGICS::UPDATESTATUS::", pr_msg);
                    }

                    if (getLog()) {
                        outLog.open((getLogFile()).c_str(), ios::app);
                        outLog << left << "[" << pmModem->getEpoch() << "]::" << NOW << "::MS2C_EVOLOGICS_DRIVER(" << ID << ")::UPDATE_STATUS::BUSY = " << hexdumplog(pr_msg) << endl;
                        outLog.flush();
                        outLog.close();
                    }
                    queue_tx.push(pr_msg);
                } else if (pr_msg.find("BUSY DELIVERING") != string::npos) {
                    if (getLog()) {
                        outLog.open((getLogFile()).c_str(), ios::app);
                        outLog << left << "[" << pmModem->getEpoch() << "]::" << NOW << "::MS2C_EVOLOGICS_DRIVER(" << ID << ")::UPDATE_STATUS::BUSY = " << hexdumplog(pr_msg) << endl;
                        outLog.flush();
                        outLog.close();
                    }
                    queue_tx.push(pr_msg);
                } else if (pr_msg.find("ERROR") != string::npos) {

                    queue_tx.push(pr_msg);

                    if (debug_ >= 2) {
                        hexdump("MS2C_EVOLOGICS::UPDATESTATUS::", pr_msg);
                    }

                    if (getLog()) {
                        outLog.open((getLogFile()).c_str(), ios::app);
                        outLog << left << "[" << pmModem->getEpoch() << "]::" << NOW << "::MS2C_EVOLOGICS_DRIVER(" << ID << ")::UPDATE_STATUS::ERROR = " << hexdumplog(pr_msg) << endl;
                        outLog.flush();
                        outLog.close();
                    }
                    
                } else if (pr_msg.find("DELIVERED") != string::npos) {
/*                    if (debug_ >= 1) {
                        hexdump("MS2C_EVOLOGICS::UPDATESTATUS::", pr_msg);
                    }*/
                    if (getLog()) {
                        outLog.open((getLogFile()).c_str(), ios::app);
                        outLog << left << "[" << pmModem->getEpoch() << "]::" << NOW << "::MS2C_EVOLOGICS_DRIVER(" << ID << ")::UPDATE_STATUS::DELIVERED = " << hexdumplog(pr_msg) << endl;
                        outLog.flush();
                        outLog.close();
                    }
                    //queue_tx.push(pr_msg);

                } else if (pr_msg.find("FAILEDIM") != string::npos) {
                    if (debug_ >= 0) {
                        hexdump("MS2C_EVOLOGICS::UPDATESTATUS::", pr_msg);
                    }
                    if (getLog()) {
                        outLog.open((getLogFile()).c_str(), ios::app);
                        outLog << left << "[" << pmModem->getEpoch() << "]::" << NOW << "::MS2C_EVOLOGICS_DRIVER(" << ID << ")::UPDATE_STATUS::FAILEDIM = " << hexdumplog(pr_msg) << endl;
                        outLog.flush();
                        outLog.close();
                    }

                    queue_tx.push(pr_msg);

                } else if (pr_msg.find("FAILED") != string::npos) {
                    if (debug_ >= 0) {
                        hexdump("MS2C_EVOLOGICS::UPDATESTATUS::", pr_msg);
                    }
                    if (getLog()) {
                        outLog.open((getLogFile()).c_str(), ios::app);
                        outLog << left << "[" << pmModem->getEpoch() << "]::" << NOW << "::MS2C_EVOLOGICS_DRIVER(" << ID << ")::UPDATE_STATUS::FAILED = " << hexdumplog(pr_msg) << endl;
                        outLog.flush();
                        outLog.close();
                    }
                    queue_tx.push(pr_msg);


                } else if ((pr_msg.find("USBLANGLES") != string::npos) && (pr_msg.find("USBLLONG") != string::npos)) { //positioning messages, we don't care about it

                    if (debug_ >= 0) {
                        hexdump("MS2C_EVOLOGICS::UPDATESTATUS::UNKNOWN_PACKET_", pr_msg);
                    }
                    if (getLog()) {
                        outLog.open((getLogFile()).c_str(), ios::app);
                        outLog << left << "[" << pmModem->getEpoch() << "]::" << NOW << "::MS2C_EVOLOGICS_DRIVER(" << ID << ")::UPDATE_STATUS::UNKNOWN = " << hexdumplog(pr_msg) << endl;
                        outLog.flush();
                        outLog.close();
                    }

                }
                p_offset += pr_msg.size();
                p_parser = rx_msg.find(parser, p_offset + 1);
            } // End while (p_parser!=std::string::npos)

        } else {
            cread = false; // exit the loop
        } // End if (rx_msg!="")
    } // End of while (cread)
    cread = true;
    if (status == _TX || status == _CFG || status == _RESET || status == _QUIT) {// read from queue_tx only

        if (m_status_rx == _RXIM && status == _TX) {
            // Update S2C RX status
            m_status_rx = _IDLE;
        }


        while (cread) {

            if (!queue_tx.empty()) {

                // Read the possible received message
                rx_msg = queue_tx.front();
                queue_tx.pop();

                if (rx_msg.find("OK") != string::npos) {
                    if (debug_ >= 2) {
                        cout << NOW << "MS2C_EVOLOGICS(" << ID << ")::UPDATE_STATUS::OK_" << m_status_tx << "_" << status << endl;
                    }
                    if ((m_status_tx == _SETIDS && status == _CFG) || (m_status_tx == _IMS && status == _TX) || (m_status_tx == _DROPBUFFERS && status == _RESET) || (m_status_tx == _BURSTS && status == _TX) || (m_status_tx == _PBMS && status == _TX)) {
                        // Update modem status
                        status = _IDLE;
                        m_status_tx = _IDLE;
                        cread = false;
                    } else if ((m_status_tx == _TXKOD && status == _CFG)) {
                        if (SetModemID)
                        {
                            if (debug_ >= 0) cout << NOW << "MS2C_EVOLOGICS(" << ID << ")::SETTING_MODEM_ID" << endl;
                            status = _CFG;
                            m_status_tx = _SETID;
                            cread = false;
                        } else {
                            status = _IDLE;
                            m_status_tx = _IDLE;
                            cread = false;
                        }
                    } else if (m_status_tx == _CLOSED && status == _QUIT) {
                        status = _IDLE;
                        m_status_tx = _IDLE;
                        if (debug_ >= 0) cout << NOW << "MS2C_EVOLOGICS(" << ID << ")::ACOUSTIC_CONNECTION_CLOSED" << endl;
                        cread = false;
                        mConnector.closeConnection();
                    } else {
                        if (debug_ >= 0) {
                            cout << NOW << "MS2C_EVOLOGICS(" << ID << ")::UPDATE_STATUS::OK_WRONG_STATUS_" << m_status_tx << "_" << status << endl;
                        }
                        if (getLog()) {
                            outLog.open((getLogFile()).c_str(), ios::app);
                            outLog << left << "[" << pmModem->getEpoch() << "]::" << NOW << "::MS2C_EVOLOGICS_DRIVER(" << ID << ")::OK_WRONG_STATUS_TX_" << m_status_tx << "_STATUS_" << status << endl;
                            outLog.flush();
                            outLog.close();
                        }
                    }

                } else if (rx_msg.find("ERROR") != string::npos) {
                    if ((m_status_tx == _SETIDS && status == _CFG) || (m_status_tx == _IMS && status == _TX) || (m_status_tx == _DROPBUFFERS && status == _RESET) || (m_status_tx == _BURSTS && status == _TX) || (m_status_tx == _PBMS && status == _TX)) {
                        if (debug_ >= 0) {
                            cout << NOW << "MS2C_EVOLOGICS(" << ID << ")::UPDATE_STATUS::ERROR" << endl;
                        }
                        status = _IDLE;
                    } else {
                        if (debug_ >= 0) {
                            cout << NOW << "MS2C_EVOLOGICS(" << ID << ")::UPDATE_STATUS::ERROR_WRONG_STATUS_TX_" << m_status_tx << "_STATUS_" << status << endl;
                        }
                        if (getLog()) {
                            outLog.open((getLogFile()).c_str(), ios::app);
                            outLog << left << "[" << pmModem->getEpoch() << "]::" << NOW << "::MS2C_EVOLOGICS_DRIVER(" << ID << ")::ERROR_WRONG_STATUS_TX_" << m_status_tx << "_STATUS_" << status << endl;
                            outLog.flush();
                            outLog.close();
                        }
                    }
                    m_status_tx = _IDLE;
                    cread = false;

                } else if (rx_msg.find("BUSY CLOSING CONNECTION") != string::npos) {
                    if (debug_ >= 0) {
                        cout << NOW << "MS2C_EVOLOGICS(" << ID << ")::BUSY_CLOSING_CONNECTION" << endl;
                    }
                    status = _IDLE;

                } else if (rx_msg.find("BUSY BACKOFF STATE") != string::npos) {
                    if (debug_ >= 0) {
                        cout << NOW << "MS2C_EVOLOGICS(" << ID << ")::BUSY_BACKOFF_STATE" << endl;
                    }
                    status = _IDLE;

                } else if (rx_msg.find("FAILEDIM") != string::npos) {
                    if (debug_ >= 0) {
                        cout << NOW << "MS2C_EVOLOGICS(" << ID << ")::UPDATE_STATUS::FAILEDIM" << endl;
                    }
                    status = _IDLE;
                } else if (rx_msg.find("FAILED") != string::npos) {
                    if (debug_ >= 0) {
                        cout << NOW << "MS2C_EVOLOGICS(" << ID << ")::UPDATE_STATUS::FAILED" << endl;
                    }
                    status = _IDLE;
                } else if (rx_msg.find("DELIVERED") != string::npos) {
                    if (debug_ >= 0) {
                        cout << NOW << "MS2C_EVOLOGICS(" << ID << ")::UPDATE_STATUS::DELIVERED" << endl;
                    }
                } else if  (rx_msg.find("BUSY DELIVERING") != string::npos) {
                    if (debug_ >= 0) {
                        cout << NOW << "MS2C_EVOLOGICS(" << ID << ")::UPDATE_STATUS::MODEM BUSY DELIVERING PIGGYBACK MESSAGE" << endl;   
                    }
                    if (m_status_tx == _PBMS && status == _TX)
                    {
                        status = _IDLE;
                        m_status_tx = _IDLE;
                        cread = false;
                    }
                } else {
                    if (debug_ >= 0) cout << NOW << "MS2C_EVOLOGICS(" << ID << ")::UNKNOWN_PACKET_IN_QUEUE_" << rx_msg << endl;
                }
            } else {
                cread = false;
            }
        }
    } else if ((m_status_rx == _RXIM && status == _RX) || (m_status_rx == _RXBURST && status == _RX) || (m_status_rx == _RXPBM && status == _RX)) {// if something has been previously received
        // Update S2C RX status
        m_status_rx = _IDLE;
        // Update modem status
        status = _IDLE_RX;
        // Exit from updateStatus() because status has changed;
        cread = false;

    }
    else {// read from queue_rx only

        while (cread) {

            if (!queue_rx.empty()) {

                // Read the possible received message
                rx_msg = queue_rx.front();
                queue_rx.pop();

                if (rx_msg.find("RECVIM") != string::npos) {// Acoustic message detected
                    m_status_rx = _RXIM;
                    status = _RX;
                    mInterpreter.parse_recvim(rx_msg);
                    cread = false;
                } else if (rx_msg.find("RECV") != string::npos) {
                    m_status_rx = _RXBURST;
                    status = _RX;
                    mInterpreter.parse_recv(rx_msg);
                    cread = false;
                } else if (rx_msg.find("RECVPBM") != string::npos) {
                    m_status_tx = _RXPBM;
                    status = _RX;
                    mInterpreter.parse_recvpbm(rx_msg);
                    cread = false;
                } else {// Any other AT messages (ignored for the moment)    
                    if (debug_ >= 0) cout << NOW << "MS2C_EVOLOGICS(" << ID << ")::UPDATE_STATUS::IGNORED_AT_MESSAGE_" << rx_msg << endl;
                }
            } else {
                // Exit from the while loop because no packets in queue_rx
                cread = false;
            } // End if (!queue_rx.empty())
        }// End while (cread)

    }// End if (status == _TX || status == _CFG)        

    return status;
}

void MdriverS2C_EvoLogics::modemTxManager() {
    std::string tx_msg;
    if (status == _TX || status == _CFG || status == _RESET || status == _QUIT) {
        switch (m_status_tx) {
            case _SETID:
                // Build the configuration message
                if (debug_ >= 2) cout << NOW << "MS2C_EVOLOGICS(" << ID << ")::BUILDING_SETAL!" << ID << "_MESSAGE" << endl;
                tx_msg = mInterpreter.build_setAL(ID);
                break;
            case _IM:
                // Build the instant message
                if (debug_ >= 2) cout << NOW << "MS2C_EVOLOGICS(" << ID << ")::BUILDING_AT*SENDIM_MESSAGE" << endl;
                tx_msg = mInterpreter.build_sendim(payload_tx.length(), dest, "noack", payload_tx);
                if (getLog()) {
                    outLog.open((getLogFile()).c_str(), ios::app);
                    outLog << left << "[" << pmModem->getEpoch() << "]::" << NOW << "::MS2C_EVOLOGICS_DRIVER(" << ID << ")::MODEM_TX_MANAGER::SENDIM = " << hexdumplog(tx_msg) << endl;
                    outLog.flush();
                    outLog.close();
                }
                break;
            case _BURST:
                //build a burst data
                if (debug_ >= 2) cout << NOW << "MS2C_EVOLOGICS(" << ID << ")::BUILDING_AT*SEND_MESSAGE" << endl;
                tx_msg = mInterpreter.build_atsend(payload_tx.length(), dest, payload_tx);
                cout << "AT*SEND_MESSAGE = " << hexdumplog(tx_msg) << endl;
                if (getLog()) {
                    outLog.open((getLogFile()).c_str(), ios::app);
                    outLog << left << "[" << pmModem->getEpoch() << "]::" << NOW << "::MS2C_EVOLOGICS_DRIVER(" << ID << ")::MODEM_TX_MANAGER::SEND = " << hexdumplog(tx_msg) << endl;
                    outLog.flush();
                    outLog.close();
                }
                break;
            case _PBM:
                if (debug_ >= 2) cout << NOW << "MS2C_EVOLOGICS(" << ID << ")::BUILDING_AT*SENDPBM_MESSAGE" << endl;
                tx_msg = mInterpreter.build_atsendpbm(payload_tx.length(), dest, payload_tx);
                if (getLog()) {
                    outLog.open((getLogFile()).c_str(), ios::app);
                    outLog << left << "[" << pmModem->getEpoch() << "]::" << NOW << "::MS2C_EVOLOGICS_DRIVER(" << ID << ")::MODEM_TX_MANAGER::SENDPBM = " << hexdumplog(tx_msg) << endl;
                    outLog.flush();
                    outLog.close();
                }
                break;
            case _DROPBUFFER:
                if (debug_ >= 2) cout << NOW << "MS2C_EVOLOGICS(" << ID << ")::BUILDING_ATZ" << _DROPBUFFER << "_MESSAGE" << endl;
                tx_msg = mInterpreter.build_atzn(_DROPBUFFER);
                break;
            case _TXKO:
                if (debug_ >= 2) cout << NOW << "MS2C_EVOLOGICS(" << ID << ")::BUILDING_AT!KO" << endl;
                tx_msg = mInterpreter.build_atko(0);
                break;
            case _CLOSE:
                if (debug_ >= 2) cout << NOW << "MS2C_EVOLOGICS(" << ID << ")::BUILDING_ATH0 TO CLOSE CONNECTION" << endl;
                tx_msg = mInterpreter.build_ath(0);
                break;
            default:

                if (debug_ >= 2) cout << NOW << "MS2C_EVOLOGICS(" << ID << ")::MODEM_TX_MANAGER::UNEXPECTED_CALL" << endl;

                if (getLog()) {
                    outLog.open((getLogFile()).c_str(), ios::app);
                    outLog << left << "[" << pmModem->getEpoch() << "]::" << NOW << "::MS2C_EVOLOGICS_DRIVER(" << ID << ")::MODEM_TX_MANAGER_WRONG_CALL_" << m_status_tx << endl;
                    outLog.flush();
                    outLog.close();
                }
        }//end switch
        if (m_status_tx == _IM || m_status_tx == _SETID || m_status_tx == _DROPBUFFER || m_status_tx == _TXKO || m_status_tx == _BURST || m_status_tx == _PBM || m_status_tx == _CLOSE) {
            mConnector.writeToModem(tx_msg);
            m_status_tx++;
        }

    }
}

double MdriverS2C_EvoLogics::getIntegrity() {
    return mInterpreter.getIntegrity();
}