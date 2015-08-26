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
//

/**
 * @file   uwApplication_TCP_socket.cc
 * @author Federico Favaro
 * @version 1.1.0
 *
 * \brief Provides the implementation of the TCP socket between the protocol and any other application media.
 *
 */

#include <sstream>
#include <time.h>
#include "uwApplication_cmn_header.h"
#include "uwApplication_module.h"
#include <error.h>
#include <errno.h>

pthread_mutex_t mutex_tcp = PTHREAD_MUTEX_INITIALIZER;

int uwApplicationModule::openConnectionTCP() {
    int sockoptval = 1;
    
    //Create socket for incoming connections
    if((servSockDescr=socket(AF_INET,SOCK_STREAM,0)) < 0){
        if (debug_ >= 0) std::cout << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::OPEN_CONNECTION_TCP::SOCKET_CREATION_FAILED" << endl;
        if (logging) out_log << left << "[" << getEpoch() << "]::" << NOW << "::UWAPPLICATION::OPEN_CONNECTION_TCP::SOCKET_CREATION_FAILED" << endl;
        exit(1);
    }
    if (debug_ >= 2) std::cout << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::OPEN_CONNECTION_TCP::SOCKET_CREATED" << endl;
    //setsockopt(servSockDescr, SOL_SOCKET, SO_REUSEADDR, &sockoptval, sizeof (int));
    
    //Fill the members of sockaddr_in structure
    memset(&servAddr,0,sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(servPort);
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    //Bind to the local address       
    if(::bind(servSockDescr, (struct sockaddr *) &servAddr, sizeof (servAddr)) < 0) {
        if (debug_ >= 0) std::cout << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::OPEN_CONNECTION_TCP::BINDING_FAILED_" << strerror(errno) << endl;
        if (logging) out_log << left << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::OPEN_CONNECTION_TCP::BINDING_FAILED_" << strerror(errno) << endl;
        exit(1);
    }
    
    //Listen for incoming connections    
    if(listen(servSockDescr,1)) {
        if (debug_ >= 0) std::cout << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::OPEN_CONNECTION_TCP::LISTEN_FAILED" << endl;
        if (logging) out_log << left << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::OPEN_CONNECTION_TCP::LISTEN_FAILED" << endl;
        exit(1);
    }
    if (debug_ >= 2) std::cout << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::OPEN_CONNECTION_TCP::SERVER_READY" << endl;
    
    chkTimerPeriod.resched(getPeriod());   
    pthread_t pth;
    if (pthread_create(&pth, NULL, read_process_TCP, (void*) this) != 0)
    {
        if (debug_ >= 0) std::cout << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::OPEN_CONNECTION_TCP::CANNOT_CREATE_PARRALEL_THREAD" << endl;
        if (logging) out_log << left << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::OPEN_CONNECTION_TCP::CANNOT_CREATE_PARRALEL_THREAD" << endl;
        exit(1);
    }
    
    return servSockDescr;
}//end openConnectionTCP() method

void *read_process_TCP(void* arg){
    uwApplicationModule* obj = (uwApplicationModule*) arg; 
    int debug_=1;
    //struct sockaddr_in clnAddr;
    
    socklen_t clnLen = sizeof(sockaddr_in);
    //int clnSockDescr;

    clnLen = sizeof(obj->clnAddr);


    while(true) {
        if( (obj->clnSockDescr=accept(obj->servSockDescr, (struct sockaddr *)&(obj->clnAddr), (socklen_t*) &clnLen)) < 0 ) {
            if (debug_ >= 0) std::cout << "[" << obj->getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::READ_PROCESS_TCP::CONNECTION_NOT_ACCEPTED" << endl;
        }
        if (debug_ >= 1) std::cout << "[" << obj->getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::READ_PROCESS_TCP::NEW_CLIENT_IP_" << inet_ntoa(obj->clnAddr.sin_addr)<<std::endl; 
        if (obj->logging) obj->out_log << left << "[" << obj->getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::READ_PROCESS_TCP::NEW_CLIENT_IP_" << inet_ntoa(obj->clnAddr.sin_addr)<<std::endl; 
        obj->handleTCPclient(obj->clnSockDescr);
    }
    
}//end read_process_TCP() method

void uwApplicationModule::handleTCPclient(int clnSock)
{   
    while (true)
    {
        int recvMsgSize = 0;
        char buffer_msg[MAX_LENGTH_PAYLOAD];
        Packet* p = Packet::alloc();
        hdr_DATA_APPLICATION *hdr_Appl = HDR_DATA_APPLICATION(p);
        for(int i = 0; i < MAX_LENGTH_PAYLOAD; i++) {
            buffer_msg[i] = 0;
        }
        if ((recvMsgSize = read(clnSock, buffer_msg, MAX_LENGTH_PAYLOAD)) < 0) {
            if (debug_ >= 0) std::cout << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::READ_PROCESS_TCP::HANDLE_TCP_CLIENT::CONNECTION_NOT_ACCEPTED" << endl;
            break;
        }
        if (recvMsgSize == 0) { //client disconnected
            shutdown(clnSock,2);
            break;
        } else {
            int status = pthread_mutex_lock(&mutex_tcp);
            if (status != 0)
            {
                if (debug_ >= 0) std::cout << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::PTHREAD_MUTEX_LOCK_FAILED " << endl;
            }
            if (debug_ >= 0) 
            {
                std::cout << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::READ_PROCESS_TCP::PAYLOAD_MESSAGE--> ";
                for(int i = 0; i < recvMsgSize; i++)
                {
                    cout << buffer_msg[i];
                }
            }
            if (logging) out_log << left << "[" << getEpoch() << "]::" << NOW << "::UWAPPLICATION::READ_PROCESS_UDP::NEW_PACKET_CREATED"<< endl;
            for (int i = 0; i < MAX_LENGTH_PAYLOAD; i++) {
                hdr_Appl->payload_msg[i] = buffer_msg[i];
            }
            hdr_cmn *ch = HDR_CMN(p);
            ch->size() = recvMsgSize;
            hdr_Appl->payload_size() = recvMsgSize;
            queuePckReadTCP.push(p);
            incrPktsPushQueue();
            status = pthread_mutex_unlock(&mutex_tcp);
            if (status != 0)
            {
                if (debug_ >= 0) std::cout << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::PTHREAD_MUTEX_UNLOCK_FAILED " << endl;
            }
        }
    }
}


void uwApplicationModule::init_Packet_TCP(){
    if( queuePckReadTCP.empty() ) {
    } else {
        Packet* ptmp = queuePckReadTCP.front();
        queuePckReadTCP.pop();
        hdr_cmn *ch = HDR_CMN(ptmp);
        hdr_uwudp *uwudph = hdr_uwudp::access(ptmp);
        hdr_uwip *uwiph = hdr_uwip::access(ptmp);
        hdr_DATA_APPLICATION* uwApph = HDR_DATA_APPLICATION(ptmp);
        cout << endl;
        //Common header fields
        ch->uid_ = uidcnt++;
        ch->ptype_ = PT_DATA_APPLICATION;
        ch->direction_ = hdr_cmn::DOWN; 
        ch->timestamp() = Scheduler::instance().clock();
        
        //Transport header fields
        uwudph->dport() = port_num; 
        
        //IP header fields
        uwiph->daddr() = dst_addr; 
        
        //uwApplication packet header fields
        uwApph->sn_ = txsn++; //Sequence number to the data packet
        if (rftt >= 0) {
                uwApph->rftt_ = (int) ( rftt * 10000); //Forward Trip Time
                uwApph->rftt_valid_ = true;
        } else {
                uwApph->rftt_valid_ = false;
        }
        uwApph->priority_ = 0; //Priority of the message
        if (debug_ >= 2) std::cout << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::INIT_PACKET_TCP::UID_" << ch->uid_ << endl;
        if (debug_ >= 0) std::cout << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::INIT_PACKET_TCP::DEST_" << (int)uwiph->daddr() << endl;
        if (debug_ >= 0) std::cout << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::INIT_PACKET_TCP::SIZE_" << (int)uwApph->payload_size() << endl;
        if (debug_ >= 0) std::cout << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::INIT_PACKET_TCP::SN_" << (int)uwApph->sn_ << endl;
        if (debug_ >= 0) std::cout << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::INIT_PACKET_TCP::INIT_PACKET_TCP::SEND_DOWN_PACKET" << endl;

        if (logging) out_log << left << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::INIT_PACKET_TCP::UID_" << ch->uid_ << endl;
        if (logging) out_log << left << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::INIT_PACKET_TCP::DEST_" << (int)uwiph->daddr() << endl;
        if (logging) out_log << left << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::INIT_PACKET_TCP::SIZE_" << (int)uwApph->payload_size() << endl;
        if (logging) out_log << left << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::INIT_PACKET_TCP::SN_" << (int)uwApph->sn_ << endl;
        if (logging) out_log << left << "[" << getEpoch() << "]::" << NOW <<  "::UWAPPLICATION::INIT_PACKET_TCP::INIT_PACKET_TCP::SEND_DOWN_PACKET" << endl;
        sendDown(ptmp);
    }
}
