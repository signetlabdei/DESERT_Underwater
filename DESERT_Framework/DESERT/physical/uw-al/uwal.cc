//
// Copyright (c) 2013 Regents of the SIGNET lab, University of Padova.
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
 * @file uwal.cc
 * @author Riccardo Masiero
 * \version 1.0.0
 * \brief Implementation of the main class that implements the adaptation layer between ns2/NS-Miracle and binary data packets. 
 */

#include "uwal.h"

/**
 * The size, in bytes, of the default Physical Service Data Unit (i.e., the maximum length of a packet coded into a stream of bits to be sent to the modem).
 */
#define DEFAULT_PSDU 32

using namespace std;

/**
 * Class to create the Otcl shadow object for an object of the class Uwal.
 */
static class UwalClass : public TclClass
{
public:
	UwalClass() : TclClass("Module/UW/AL") {}
	TclObject* create(int, const char*const*) {
		return (new Uwal());
	}
} class_module_uwal;

Uwal::Uwal() : InterframeTmr(this) {
    nodeID = 0;
    pkt_counter = 0;
    pPacker = NULL;
    PSDU = DEFAULT_PSDU;
    dummyStr = "";
    while (dummyStr.size() < PSDU) {
        dummyStr += " DEFAULT DUMMY STRING ";
    }
    debug_ = 0;
    interframe_period = 0;
    frame_set_validity = 0;
    frame_padding = 0;

    bind("nodeID", (int*) &nodeID);
    bind("PSDU", (int*) &PSDU);
    bind("debug_", (int*) &debug_);

    bind("interframe_period", &interframe_period);
    bind("frame_set_validity", &frame_set_validity);
    bind("frame_padding", &frame_padding);
}

Uwal::~Uwal() {
    InterframeTmr.force_cancel();
}

int Uwal::command(int argc, const char*const* argv) {
    if (argc == 2)
    {
        if (strcmp(argv[1],"Reset_PER_List") == 0)
        {
            PERList.clear();
            return TCL_OK;
        }
    }
    if (argc == 3) {
        if (strcmp(argv[1], "linkPacker") == 0) { // tcl command to link to this Uwal object a packer object shadowed as an OTcl object.
            pPacker = (packer*) TclObject::lookup(argv[2]);
            return TCL_OK;
        }

        if (strcmp(argv[1], "setDummyStr") == 0) { // tcl command to set a customized dummy string.
            dummyStr = "";
            while (dummyStr.size() < PSDU) {
                dummyStr += argv[2];
            }
            return TCL_OK;
        }
    }
    if (argc >= 3) 
    {
        if (strcmp(argv[1],"Set_PER_List") == 0)
        {
            for(int i = 2; i < argc-1; i=i+2)
            {
                int id = atoi(argv[i]);
                double per = atof(argv[i+1]);
                pl_element pl;
                pl.node_ID = id;
                pl.per = per;
                for(list<pl_element>::iterator it = PERList.begin(); it != PERList.end(); ++it)
                {
                    if (it->node_ID == pl.node_ID)
                    {
                        it = PERList.erase(it);
                    }
                }
                PERList.insert(PERList.begin(),pl);
            }
            return TCL_OK;
        }
        if (strcmp(argv[1],"Clear_PER_List") == 0)
        {
            for (int i = 2; i < argc; i++)
            {
                int id = atoi(argv[i]);
                for(list<pl_element>::iterator it = PERList.begin(); it != PERList.end(); ++it)
                {
                    if (it->node_ID == id)
                    {
                        it = PERList.erase(it);
                    }
                }
            }
            return TCL_OK;
        }
    }
    return Module::command(argc, argv);
}

bool Uwal::isInPERList(int mac_addr) {
    bool flag = false;
    
    for (list<PERListElement>::iterator it = PERList.begin(); it != PERList.end(); ++it)
    {
        if (it->node_ID == mac_addr) {
            flag = true;
            break;
        }
    }
    return flag;
}

double Uwal::getPERfromID(int mac_addr) {
    double per;
    for (list<PERListElement>::iterator it = PERList.begin(); it != PERList.end(); ++it)
    {
        if (it->node_ID == mac_addr) {
            per = it->per;
            cout << "getPER! " << it->per << endl;
            break;
        }
    }
    return per;
}

void Uwal::recv(Packet* p) {
    hdr_cmn *ch = HDR_CMN(p);

    if (ch->direction() == hdr_cmn::UP) {
        if (debug_) {
            std::cout << NOW << "  UW-AL(" << nodeID << ") : received pkt UP ****" << endl;
        }
        if (pPacker != NULL) {
            pPacker -> unpackHdr(p);
        }
        sendUpFrames.push(p);
    } else {
        if (debug_) {
            std::cout << NOW << "  UW-AL(" << nodeID << ") : received pkt DOWN ****" << endl;
        }
        initializeHdr(p, ++pkt_counter);
//        initializeHdr(p, ch->uid()); // the uid is also used to identify the packet at the AL
        if (pPacker != NULL) {
            hdr_cmn* ch = HDR_CMN(p);
            pPacker -> packHdr(p);
            pPacker -> packPayload(p);
            //ch->size_ = pPacker->getHdrBytesLength() + pPacker->getPayloadBytesLength();
        }
        sendDownPkts.push(p);
    }

    ALqueueManager();
}

void Uwal::ALqueueManager() {

    if (!sendDownFrames.empty()) {
        startTx(sendDownFrames.front());
        sendDownFrames.pop();
    } else if (!sendDownPkts.empty()) {
        fragmentPkt(sendDownPkts.front());
        sendDownPkts.pop();
        startTx(sendDownFrames.front());
        sendDownFrames.pop();
    } else {
        while (!sendUpFrames.empty()) {
            reassembleFrames(sendUpFrames.front());
            sendUpFrames.pop();
        }

        checkRxFrameSet();

        if (!sendUpPkts.empty()) {
            startRx(sendUpPkts.front());
            //if (pPacker != NULL) { // NOTE: with this if, it becomes useless to pack the size_ field of the common header...
                //hdr_cmn* ch = HDR_CMN(sendUpPkts.front());
                //ch->size_ = pPacker->getHdrBytesLength() + pPacker->getPayloadBytesLength();
            //}
            endRx(sendUpPkts.front());
            sendUpPkts.pop();
        }

        if (!sendUpPkts.empty()) {
            ALqueueManager();
        }
    }
}

void Uwal::initializeHdr(Packet* p, unsigned int pkt_counter_){
  
      hdr_uwal* hal = HDR_UWAL(p);
      
      hal->srcID() = nodeID;
      hal->pktID() = pkt_counter_;
      hal->framePayloadOffset() = 0;
      hal->Mbit() = 0;
      memset(hal->dummyStr(),'\0',MAX_DUMMY_STRING_LENGTH);
      memcpy(hal->dummyStr(),dummyStr.c_str(),dummyStr.size());
      
      memset(hal->binPkt(),'\0',MAX_BIN_PKT_ARRAY_LENGTH);
      hal->binPktLength()= 0;
      hal->binHdrLength()= 0;  
}

void Uwal::fragmentPkt(Packet* p) {

    hdr_uwal* hal = HDR_UWAL(p);
    hdr_cmn* ch = HDR_CMN(p);

    if (pPacker == NULL) {
        if (debug_) {
            printf("\033[0;0;31m WARNING: \033[0m ");
            cout << "in Uwal::fragmentPkt -> pPacker set to NULL (i.e., no UW AL packer added from tcl with linkPacker). No serialization neither fragmentation can be performed! (i.e., packet just sent to lower layers)." << endl;
        }
        sendDownFrames.push(p);
    } else {
        size_t hdr_length = pPacker->getHdrBytesLength();
        //size_t payload_length = pPacker->getPayloadBytesLength();
       // size_t pkt_length = hdr_length + payload_length;
        size_t pkt_length = hal->binPktLength(); 
        size_t payload_length = pkt_length - hdr_length; // pPacker->getPayloadBytesLength();        

        if (PSDU < hdr_length) {
            printf("\033[0;0;31m ERROR: \033[0m ");
            cout << "in packer::ALfragmenter PSDU: " << PSDU << " but hdr_length; " << hdr_length << ". Packet fragmentation is not possible since the hdr cannot fit in the PSDU." << endl;
            return;
        }

        if (pkt_length <= PSDU){// !(PSDU < pkt_length)) {
            if (frame_padding) {
                hal->binPktLength() = PSDU;
                ch->size_ = PSDU;
            }
            sendDownFrames.push(p);
        } else {
            Packet* f_tmp;
            hdr_uwal* hal_tmp;
            hdr_cmn*  ch_tmp;

            size_t framePayloadLength = PSDU - hdr_length;
            size_t frameNumber = payload_length / framePayloadLength;
            //size_t frameNumber = HDR_UWAL(p)->binPktLength() / framePayloadLength;
            
            size_t lastFramePayloadLength = hal->binPktLength() - hal->binHdrLength() - frameNumber*framePayloadLength;

            if (debug_) {
                std::cout << "------ ch->size_: " << ch->size() << " hdr_length: " << hdr_length << ", payload_length: " << payload_length << ", pkt_length: " << pkt_length << ", framePayloadLength: " << framePayloadLength << ", frameNumber: " << frameNumber << " lastFramePayloadLength: " << lastFramePayloadLength << std::endl;
            }

            for (int i = 0; i < frameNumber; i++) {
                f_tmp = Packet::alloc();
                initializeHdr(f_tmp, hal->pktID());

                hal_tmp = HDR_UWAL(f_tmp);
                ch_tmp  = HDR_CMN(f_tmp);

                //hal_tmp->framePayloadOffset() = i*framePayloadLength;
                hal_tmp->framePayloadOffset() = i;
                if (!(i == (frameNumber - 1) && lastFramePayloadLength == 0))
                    hal_tmp->Mbit() = 1;

                pPacker -> packHdr(f_tmp);

                //memcpy(hal_tmp->binPkt() + hal_tmp->binHdrLength(), hal->binPkt() + hal->binHdrLength() + hal_tmp->framePayloadOffset(), framePayloadLength);
                memcpy(hal_tmp->binPkt() + hal_tmp->binHdrLength(), hal->binPkt() + hal->binHdrLength() + hal_tmp->framePayloadOffset()*framePayloadLength, framePayloadLength);
                hal_tmp->binPktLength() += framePayloadLength;

                if (debug_) {
                    if (i == (frameNumber - 1) && lastFramePayloadLength == 0)
                        std::cout << "TX frame (last) num: " << i << endl;
                    else
                        std::cout << "TX frame num: " << i << endl;
                    std::cout << "Header: " << pPacker->hexdump(hal_tmp->binPkt(), hal_tmp->binHdrLength()) << endl;
                    std::cout << "Payload: " << pPacker->hexdump(hal_tmp->binPkt() + hal_tmp->binHdrLength(), hal_tmp->binPktLength() - hal_tmp->binHdrLength()) << endl;
                }

                ch_tmp->size_ = PSDU;
                sendDownFrames.push(f_tmp);
            }

            if (lastFramePayloadLength > 0) {

                f_tmp = Packet::alloc();
                initializeHdr(f_tmp, hal->pktID());

                hal_tmp = HDR_UWAL(f_tmp);
                ch_tmp =  HDR_CMN(f_tmp);
                //hal_tmp->framePayloadOffset() = frameNumber*framePayloadLength;
                hal_tmp->framePayloadOffset() = frameNumber;

                pPacker -> packHdr(f_tmp);

                if (frame_padding) {
                    memcpy(hal_tmp->binPkt() + hal_tmp->binHdrLength(), hal->binPkt() + hal->binHdrLength() + hal_tmp->framePayloadOffset()*framePayloadLength, PSDU - hal_tmp->binHdrLength());
                    hal_tmp->binPktLength() += (PSDU - hal_tmp->binHdrLength());
                    ch_tmp->size_ = PSDU;
                } else {
                    memcpy(hal_tmp->binPkt() + hal_tmp->binHdrLength(), hal->binPkt() + hal->binHdrLength() + hal_tmp->framePayloadOffset()*framePayloadLength, lastFramePayloadLength);
                    hal_tmp->binPktLength() += lastFramePayloadLength;
                    ch_tmp->size_ = hal_tmp->binHdrLength() + lastFramePayloadLength;
                }

                if (debug_) {
                    std::cout << "TX (last) frame num: " << frameNumber << endl;
                    std::cout << "Header: " << pPacker->hexdump(hal_tmp->binPkt(), hal_tmp->binHdrLength()) << endl;
                    std::cout << "Payload: " << pPacker->hexdump(hal_tmp->binPkt() + hal_tmp->binHdrLength(), hal_tmp->binPktLength() - hal_tmp->binHdrLength()) << endl;
                }

                sendDownFrames.push(f_tmp);
            }

            Packet::free(p);
        }
    }
}

void Uwal::reassembleFrames(Packet* p) {
    hdr_uwal* hal = HDR_UWAL(p);
    hdr_cmn *ch   = HDR_CMN(p);
    
    if (pPacker == NULL) {
        if (debug_) {
            printf("\033[0;0;31m WARNING: \033[0m ");
            cout << "in Uwal::fragmentPkt -> pPacker set to NULL (i.e., no UW AL packer added from tcl with linkPacker). No serialization neither reassembling of fragmented packets can be performed! (i.e., frame just sent to upper layers)." << endl;
        }
        sendUpPkts.push(p);
    } else {
        RxFrameSetKey newKey(hal->srcID(), hal->pktID());
        std::map<RxFrameSetKey, RxFrameSet>::iterator it = sendUpFrameSet.find(newKey);
        
        size_t framePayloadOffset = hal->framePayloadOffset()*(PSDU - hal->binHdrLength());
        //size_t framePayloadOffset = hal->framePayloadOffset()*(ch->size() - hal->binHdrLength());

        if (it != sendUpFrameSet.end()) {
            if (hal -> Mbit()) { // not the last pkt frame
                //(it->second).UpdateRxFrameSet(hal->binPkt() + hal->binHdrLength(), hal->framePayloadOffset(), hal->binPktLength() - hal->binHdrLength(), -1, Scheduler::instance().clock());
                (it->second).UpdateRxFrameSet(hal->binPkt() + hal->binHdrLength(), framePayloadOffset, hal->binPktLength() - hal->binHdrLength(), -1, Scheduler::instance().clock());
            } else {
                //(it->second).UpdateRxFrameSet(hal->binPkt() + hal->binHdrLength(), hal->framePayloadOffset(), hal->binPktLength() - hal->binHdrLength(), (hal->framePayloadOffset() + hal->binPktLength() - hal->binHdrLength()), Scheduler::instance().clock());
                (it->second).UpdateRxFrameSet(hal->binPkt() + hal->binHdrLength(), framePayloadOffset, hal->binPktLength() - hal->binHdrLength(), (framePayloadOffset + hal->binPktLength() - hal->binHdrLength()), Scheduler::instance().clock());
            }

            if (ch->error()){
                if (debug_){
                    std::cout << NOW << "  UW-AL(" << nodeID << ") - Received frame in error" << std::endl;
                }
                (it->second).setError();
            }

        } else {
            RxFrameSet newSet;

            if (hal -> Mbit()) { // not the unique or last pkt frame
                //newSet.UpdateRxFrameSet(hal->binPkt() + hal->binHdrLength(), hal->framePayloadOffset(), hal->binPktLength() - hal->binHdrLength(), -1, Scheduler::instance().clock());
                newSet.UpdateRxFrameSet(hal->binPkt() + hal->binHdrLength(), framePayloadOffset, hal->binPktLength() - hal->binHdrLength(), -1, Scheduler::instance().clock());
            } else {
               // newSet.UpdateRxFrameSet(hal->binPkt() + hal->binHdrLength(), hal->framePayloadOffset(), hal->binPktLength() - hal->binHdrLength(), (hal->framePayloadOffset() + hal->binPktLength() - hal->binHdrLength()), Scheduler::instance().clock());
                newSet.UpdateRxFrameSet(hal->binPkt() + hal->binHdrLength(), framePayloadOffset, hal->binPktLength() - hal->binHdrLength(), (hal->framePayloadOffset() + hal->binPktLength() - hal->binHdrLength()), Scheduler::instance().clock());
            }

            if (ch->error()){
                if (debug_){
                    std::cout << NOW << "  UW-AL(" << nodeID << ") - Received frame in error" << std::endl;
                }
                newSet.setError();
            }

            sendUpFrameSet.insert(std::pair<RxFrameSetKey, RxFrameSet > (newKey, newSet));
        }

        Packet::free(p);

        if (debug_) {
            std::cout << NOW << "  UW-AL(" << nodeID << ") Generated map of RxFrameSets. Number of elements: " << sendUpFrameSet.size() << endl;
            int i = 1;
            for (std::map<RxFrameSetKey, RxFrameSet>::iterator it = sendUpFrameSet.begin(); it != sendUpFrameSet.end(); it++) {
                std::cout << "Element num: " << i++ << endl;
                std::cout << "Key: " << it->first.displayKey() << endl;
                if (debug_>5){
                    std::cout << "Set: " << it->second.displaySet() << endl;
                }
            }
        }
    }
}

void Uwal::checkRxFrameSet() {
    int fs_size = sendUpFrameSet.size();
    std::map<RxFrameSetKey, RxFrameSet>::iterator it = sendUpFrameSet.begin();

    for (int i = 0; i < fs_size; i++) {
        if (it->second.tot_length() == it->second.curr_length()) {
            if (debug_) {
                std::cout << NOW << "  UW-AL(" << nodeID << ")::checkRxFrameSet() - COMPLETE pkt RECEIVED! ****" << endl;
                std::cout << "Number of elements in sendUpFrameSet: " << sendUpFrameSet.size() << endl;
                std::cout << "Key: " << it->first.displayKey() << endl;
                if (debug_>5){
                    std::cout << "Set: " << it->second.displaySet() << endl;
                }
            }
            Packet* p = Packet::alloc();
            initializeHdr(p, it->first.pktID());

            hdr_uwal* hal = HDR_UWAL(p);
            hdr_cmn* ch   = HDR_CMN(p);

            hal->srcID() = it->first.srcID();

            //pPacker -> packHdr(p); // (NOTE: it is not necessary to re-pack the uwal header, since the new allocated packet must be forwarded to the upper layers)

            memcpy(hal->binPkt() + hal->binHdrLength(), (it->second.binPayload()).c_str(), (it->second.binPayload()).size());
            hal->binPktLength() += (it->second.binPayload()).size();

            // check if this is right
//            ch->uid() = hal->pktID();
//            if (debug_){
//                std::cout << "Size: " << ch->size() << endl;
//                std::cout << "Uid : "<< ch->uid() << std::endl;
//            }

	        // Set temporary size as num_frames*payload_lenght
            ch->size() = it->second.tot_length();
            if (debug_){
                std::cout << "Packet size = " << ch->size() << std::endl;    
            }
	    
            pPacker -> unpackPayload(p);
            

            hdr_mac* mach = HDR_MAC(p);
            if ( isInPERList(mach->macSA()))
            {
                double x = RNG::defaultrng()->uniform_double();
                cout << "x = " << x << endl;
                double per = getPERfromID(mach->macSA());
                cout << "PER = " << per << endl;
                bool error = x <= per;
                if (error)
                    ch->error() = 1;
            }
            
            if (it->second.getError()) {
                if (debug_){
                    std::cout << NOW << "  UW-AL(" << nodeID << ") - Packet in error" << std::endl;
                }

                ch->error() = 1;
            }

            sendUpPkts.push(p);
            sendUpFrameSet.erase(it++);
        } else if (Scheduler::instance().clock() - it->second.t_last_rx_frame() > frame_set_validity) {
            if (debug_) {
                printf("\033[0;0;31m WARNING: \033[0m ");
                std::cout << "**** Uwal::checkRxFrameSet() - INCOMPLETE pkt DISCARDED! - frame_set_validity elapsed! ****" << endl;
                std::cout << "Number of elements in sendUpFrameSet: " << sendUpFrameSet.size() << endl;
            }
            sendUpFrameSet.erase(it++);
        } else {
            ++it;
        }
    }
}

void Uwal::startTx(Packet* p) {
    InterframeTmr.resched(interframe_period);// + (double)rand() / (double)RAND_MAX * interframe_period );
    hdr_uwal* hal = HDR_UWAL(p);

    if (!hal->Mbit()) {
        endTx(p);
    }
    sendDown(p);
}

void Uwal::endTx(Packet* p) {
    Phy2MacEndTx(p);
}

void Uwal::startRx(Packet* p) {
    Phy2MacStartRx(p);
}

void Uwal::endRx(Packet* p) {
    sendUp(p);
}

// TxFrameTimer
void TxFrameTimer::expire(Event *e) {
    pUwal_->ALqueueManager();
}
