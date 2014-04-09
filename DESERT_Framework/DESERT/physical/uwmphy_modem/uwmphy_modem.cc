//
// Copyright (c) 2012 Regents of the SIGNET lab, University of Padova.
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
 * @file uwmphy_modem.cc
 * \author Riccardo Masiero
 * \version 2.0.0
 * \brief Implementation of the UWMPhy_modem class.
 */

#include "uwmphy_modem.h"

UWMPhy_modem::UWMPhy_modem(std::string pToDevice_) {

    pToDevice = pToDevice_;

    bind("ID_", &ID);
    bind("period_", &period);
    bind("bin_", &bin_);
    bind("debug_", &debug_);
    bind("log_", &log_);
    bind("SetModemID_", &SetModemID);
    t = -1;
    PktRx = NULL;
    pcheckTmr = NULL;
    pmDriver = NULL;

    for (int i = 0; i < _MTBL; i++) {
        modemTxBuff[i] = NULL;
    }
}

UWMPhy_modem::~UWMPhy_modem() {
}

int UWMPhy_modem::command(int argc, const char*const* argv) {
    Tcl& tcl = Tcl::instance();

    if (argc == 2) {
        if (!strcmp(argv[1], "start")) {
            start();
            return TCL_OK;
        }
        if (!strcmp(argv[1], "stop")) {
            stop();
            return TCL_OK;
        }
    }

    return Module::command(argc, argv);
}

void UWMPhy_modem::updatePktRx(Packet* p) {
    PktRx = p;
}

void UWMPhy_modem::recv(Packet* p) {

    if (debug_ >= 1) {
        cout << NOW << "UWMPHY_MODEM(" << ID << ")::RECV_PKT_FROM_MAC" << endl;
    }

    if (log_) {
        outLog.open(logFile.c_str(), ios::app);
        outLog << left << "[" << getEpoch() << "]::" << NOW << "::UWMPHY_MODEM("<<ID<<")::RECV::RECV_PKT_FROM_UPPER_LAYER" << endl;
        outLog.close();
    }

    int modemStatus = pmDriver -> getStatus();
    if (t >= -1 && t < _MTBL) {
        if (t == _MTBL - 1) {
            free(p);
            cerr << this << ": WARNING, packet dropped beacuse no space in modem Tx queue\n";
            if (log_) {
                outLog.open(logFile.c_str(), ios::app);
                outLog << left << "[" << getEpoch() << "]::" << NOW << "::UWMPHY_MODEM("<<ID<<")::RECV::NO_SPACE_IN_THE_QUEUE " << endl;
                outLog.close();
            }
        } else {
            modemTxBuff[++t] = p;
        }
    } else {
        cout << "ERROR in modem TX buffer: index t out of bounds! \n";
    }

    while (modemStatus == _CFG) {
        modemStatus = check_modem();
    }

    while (modemStatus == _RESET) {
        modemStatus = check_modem();
    }
    if (modemStatus == _RX || modemStatus == _IDLE_RX) {
        pmDriver -> resetModemStatus();
        free(PktRx);
        PktRx = NULL;
        modemStatus = _IDLE;
        cout << this << " UWMPhy_modem: WARNING, RX packet not delivered to upper layers because it is arrived a concurrent packet to transmit\n";
        if (log_) {
            outLog.open(logFile.c_str(), ios::app);
            outLog << left << "[" << getEpoch() << "]::" << NOW << "::UWMPHY_MODEM("<<ID<<")::RX_PACKET_NOT_DELIVERED_CONCURRENT_PACKET" << endl;
            outLog.close();
        }
    }
    if (modemStatus == _IDLE) {
        hdr_mac* mach = HDR_MAC(modemTxBuff[0]);
        hdr_uwal* uwalh = HDR_UWAL(modemTxBuff[0]);
        std::string payload_string;
        payload_string.assign(uwalh->binPkt(),uwalh->binPktLength());
        pmDriver->updateTx(mach->macDA(),payload_string);
        startTx(modemTxBuff[0]);
       if (pmDriver -> getStatus() != _TX) {
            endTx(popTxBuff());
        }
    }
}


void UWMPhy_modem::setConnections(CheckTimer* pcheckTmr_, UWMdriver* pmDriver_) {
    pcheckTmr = pcheckTmr_;
    pmDriver = pmDriver_;
}

void UWMPhy_modem::start() {

    std::stringstream str("");
    str << "MODEM_log_" << ID;
    str >> logFile;

    if (log_) {
        outLog.open(logFile.c_str());
        if (!outLog) {
            std::cerr << "WARNING: Opening error ( " << strerror(errno) << " ). It was not possible to create " << logFile.c_str() << "\n";
        }
        outLog.close();
    }
    pmDriver -> setID(ID);
    if (SetModemID == 1) {
        pmDriver->setModemID(true);
    } else {
        pmDriver->setModemID(false);
    }
    pmDriver -> start();
    pcheckTmr -> resched(period);

}

void UWMPhy_modem::stop() {
    pmDriver -> stop();
    pcheckTmr -> force_cancel();

}

int UWMPhy_modem::check_modem() {

    int modemStatus_old = pmDriver -> getStatus();
    int modemStatus = pmDriver -> updateStatus();

    if (debug_ >= 2) {
        cout << NOW << "UWMPHY_MODEM(" << ID << ")::CHECK_MODEM::TRANSITION_FROM_" << modemStatus_old << "_TO_" << modemStatus << endl;
    }


    if (modemStatus == _IDLE && modemStatus_old == _TX) {


        endTx(popTxBuff());

    } else if (modemStatus == _RX && modemStatus_old == _IDLE)

        startRx(PktRx);

    else if (modemStatus == _IDLE_RX && modemStatus_old == _RX)
        endRx(PktRx);

    else if (modemStatus == _IDLE_RX && modemStatus_old == _TX_RX) {
        endTx(popTxBuff());
        startRx(PktRx);
        endRx(PktRx);
    } else if ((modemStatus == modemStatus_old) || ((modemStatus == _TX_RX) && modemStatus_old == _TX_PAUSED) || (modemStatus == _TX_PAUSED && modemStatus_old == _TX)) {
        // Do nothing
        return modemStatus;

    }
    else if (modemStatus == _IDLE && modemStatus_old == _CFG) {
        if (debug_ >= 0) cout << NOW << "UWMPHY_MODEM(" << ID << ")::CONFIGURATION DONE!!!" << endl;
        pmDriver->emptyModemQueue();
    } else if (modemStatus == _IDLE && modemStatus_old == _RESET)
        return modemStatus;

    else {
        if (debug_ >= 0) {
            cout << NOW << "UWMPHY_MODEM(" << ID << ")::CHECK_MODEM::ERROR_UNEXPECTED_STATE_TRANSITION_FROM_" << modemStatus_old << "_TO_" << modemStatus << endl;
        }
        if (log_) {
            outLog.open(logFile.c_str(), ios::app);
            outLog << left << "[" << getEpoch() << "]::" << NOW << "::UWMPHY_MODEM("<<ID<<")::CHECK_MODEM::UNEXPECTED_TRANSITION_FROM_" << modemStatus_old << "_TO_" << modemStatus << endl;
            outLog.close();
        }
    }

    return modemStatus;
}

void UWMPhy_modem::startTx(Packet* p) 
{
    if (debug_ >= 2) cout << NOW << "UWMPHY_MODEM(" << ID << ")::CHECK_MODEM::START_TX " << endl;
    hdr_cmn *ch = HDR_CMN(p);
    if (ch->direction() == hdr_cmn::UP) {
         cout << NOW << "UWMPHY_MODEM(" << ID << ")::CHECK_MODEM::ERROR_DIRECTION_SET_UP " << endl;
    } else {
        pmDriver -> modemTx();
    }
}

void UWMPhy_modem::endTx(Packet* p) 
{
    Phy2MacEndTx(p);
    if (debug_ >= 2)
        cout << NOW << "UWMPHY_MODEM(" << ID << ")::CHECK_MODEM::END_TX " << endl;

    free(p);
}

void UWMPhy_modem::startRx(Packet* p) {
    if (debug_ >= 2)
        cout << NOW << "UWMPHY_MODEM(" << ID << ")::CHECK_MODEM::START_RX " << endl;

    Phy2MacStartRx(p);
}

void UWMPhy_modem::endRx(Packet* p) {
    std::string str = pmDriver->getRxPayload();
    unsigned char *buf = (unsigned char *) str.c_str();
    Packet* p_rx = Packet::alloc();
    hdr_uwal* uwalh = HDR_UWAL(p_rx);
    uwalh->binPktLength() = str.length();
    memset(uwalh->binPkt(),0,sizeof(uwalh->binPkt()));
    memcpy(uwalh->binPkt(),buf,uwalh->binPktLength());
    this->updatePktRx(p_rx);
    if (debug_ >= 2)
        cout << NOW << "UWMPHY_MODEM(" << ID << ")::CHECK_MODEM::END_RX " << endl;
    sendUp(PktRx);
    PktRx = NULL;
    pmDriver -> resetModemStatus();
}

Packet* UWMPhy_modem::popTxBuff() {

    Packet* temp = modemTxBuff[0];
    for (int i = 0; i < t; i++) {
        modemTxBuff[i] = modemTxBuff[i + 1];
    }

    t--;
    return temp;
}

void CheckTimer::expire(Event *e) {
    pmModem->check_modem();

    resched(pmModem->getPeriod());
}