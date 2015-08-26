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
 * @file   uw-phy-WakeUp.cc
 * @author Federico Guerra
 * @version 1.0.0
 * 
 * \brief Provides the implementation of WakeUp PHY 
 *
 */


/* #include "bpsk.h" */

#include <iostream>

#include "uw-phy-WakeUp.h"
#include <rng.h>
#include <underwater-mpropagation.h>
#include <uwlib.h>

enum {
  WKUP_DISCARD = -100, WKUP_BELOW_THRESHOLD = -200, WKUP_DROPPED_REASON_NOT_SET = -10, 
  WKUP_DROPPED_REASON_WRONG_MODID = 10, WKUP_DROPPED_REASON_NOISE, WKUP_DROPPED_REASON_DEAFNESS,
  WKUP_DROPPED_REASON_CHAIN_SYNC, WKUP_DROPPED_REASON_TX_PENDING

};

/**
 * Class that represents the binding with the tcl configuration script 
 */
static class MPhy_WakeUpClass : public TclClass {
public:
	/**
	 * Constructor of the class
	 */
  MPhy_WakeUpClass() : TclClass("Module/MPhy/Underwater/WKUP") {}
	/**
	 * Creates the TCL object needed for the tcl language interpretation
	 * @return Pointer to an TclObject
	 */
	
  TclObject* create(int, const char*const*) {
    return (new MPhy_WakeUp);
  }
} class_MPhy_WakeUp;


int hdr_wkup::offset_;
/**
 * Class that describes the PacketHeader for WakeUp Tone class
 */
static class WkUpPktClass : public PacketHeaderClass {
public:
	/**
	 * Constructor of the class
	 */
  WkUpPktClass() : PacketHeaderClass("PacketHeader/WKUP", sizeof(hdr_wkup)) {
    bind_offset(&hdr_wkup::offset_);
    bind();
  }
} class_wkup_pkt; 

bool MPhy_WakeUp::initialized = false;
int MPhy_WakeUp::modid = -1;

MPhy_WakeUp::MPhy_WakeUp()
  : PktRx(0), txActive(false), droppedPktsTxPending(0)
{
  if (!initialized) 
    {
      modid = MPhy::registerModulationType(UW_WAKEUP_MODNAME);
      initialized = true;
    }
  bind("AcquisitionThreshold_dB_", (double*)&AcquisitionThreshold_dB_);
  bind("ToneDuration_", (double*)&ToneDuration_);
  bind("MaxTxRange_", (double*)&MaxTxRange_);
}

MPhy_WakeUp::~MPhy_WakeUp()
{

}

int MPhy_WakeUp::command(int argc, const char*const* argv)
{
  //printf("MPhy::command -- %s (%d)\n", argv[1], argc);
  Tcl& tcl = Tcl::instance();

  if(argc == 2)
    {
      if(strcasecmp(argv[1], "getDroppedPktsTxPending")==0)
	{
	  tcl.resultf("%d",getDroppedPktsTxPending());
	  return TCL_OK;
        }
    }
  return(MPhy::command(argc, argv));
}

int MPhy_WakeUp::getModulationType(Packet*)
{
  assert(initialized);
  return modid;
}


double MPhy_WakeUp::getNoisePower(Packet* p)
{
  hdr_MPhy* ph = HDR_MPHY(p);

  assert(propagation_);

  // This assumes that the noise SPD is flat within the signal
  // bandwidth. Not really a very accurate model, of course you can do
  // better than this.

  double freq = ph->srcSpectralMask->getFreq();
  double bw = ph->srcSpectralMask->getBandwidth();

//   std::cerr << " txbw " << ph->srcSpectralMask->getBandwidth()
// 	    << " rxbw " << ph->dstSpectralMask->getBandwidth()
// 	    << " txf " << ph->srcSpectralMask->getFreq()
// 	    << " rxf " << ph->dstSpectralMask->getFreq()
// 	    << std::endl;

  UnderwaterMPropagation* uwmp = dynamic_cast<UnderwaterMPropagation*>(propagation_);
  assert(uwmp);
  double noiseSPDdBperHz = uwmp->uw.getNoise(freq/1000.0);
  double noisepow = bw * pow(10, noiseSPDdBperHz/10.0);
  
  return (noisepow);

}

MSpectralMask* MPhy_WakeUp::getTxSpectralMask(Packet* p)
{  
  return spectralmask_;
}

void MPhy_WakeUp::startTx(Packet* p)
{
  if (debug_) cout << NOW << "  MPhy_WakeUp::startTx() tx a TONE";

  // we abort any ongoing rx activity
  if (PktRx != 0) {
    hdr_MPhy* ph = HDR_MPHY(PktRx);
    hdr_wkup* wkuph = HDR_WKUP(PktRx);

//     drp_rsn_map[ph->txtime][ph->rxtime] = WKUP_DROPPED_REASON_TX_PENDING;

    wkuph->startRx_time = WKUP_DISCARD;
    wkuph->endRx_time = WKUP_DISCARD; 

    if (debug_) cout << ", aborting ongoing rx at time = " << ph->rxtime;      
  }
  if (debug_) cout << endl;

  txActive = true;
  PktRx = 0;
  sendDown(p);

}


void MPhy_WakeUp::endTx(Packet* p)
{
  // Notify the MAC
  txActive = false;
  Phy2MacEndTx(p);
}


void MPhy_WakeUp::startRx(Packet* p)
{
  hdr_MPhy* ph = HDR_MPHY(p);
  double rx_time = ph->rxtime;
  double tx_time = ph->txtime;

  if (ph->modulationType == modid) {
     hdr_wkup* wkuph = HDR_WKUP(p);
     double snr_dB = 10*log10(ph->Pr / ph->Pn);

     if ((PktRx == 0) && (txActive == false)) {

         wkuph->startRx_time = ph->rxtime;
         wkuph->endRx_time = ph->rxtime + getTxDuration(p);

         //std::cerr << "snr_dB: " << snr_dB << " AcquisitionThreshold_dB_: " << AcquisitionThreshold_dB_ ;

         if(snr_dB > AcquisitionThreshold_dB_) {

            if (debug_) cout << NOW << "  MPhy_WakeUp::startRx() rx a TONE" << endl;

            // ideal synchronization
            PktRx = p;

            // Notify the MAC
            Phy2MacStartRx(p);
            return;
         }
         else { // below threshold
            if (debug_) cout << NOW << "  MPhy_WakeUp::startRx() rx a TONE but below threshold" << endl;

            wkuph->startRx_time = WKUP_DISCARD;
            wkuph->endRx_time = WKUP_BELOW_THRESHOLD;       

//             drp_rsn_map[tx_time][rx_time] = WKUP_DROPPED_REASON_NOISE;
	        return;
         }
     }
     else if ( (PktRx != 0) && (txActive == false) ){ 

         if (snr_dB > AcquisitionThreshold_dB_ ) {
	        // This is a wakeup tone packet so we chain-sync on it

            hdr_MPhy* phrx = HDR_MPHY(PktRx);
//             drp_rsn_map[phrx->txtime][phrx->rxtime] = WKUP_DROPPED_REASON_CHAIN_SYNC;

	        PktRx = p;

            if (debug_) cout << NOW << "  MPhy_WakeUp::startRx() rx a chain-synched TONE" << endl;

            wkuph->startRx_time = ph->rxtime;
            wkuph->endRx_time = ph->rxtime + getTxDuration(p);
	        return;
	     }
         else { // below threshold
            if (debug_) cout << NOW << "  MPhy_WakeUp::startRx() rx a TONE but below threshold" << endl;

            wkuph->startRx_time = WKUP_DISCARD;
            wkuph->endRx_time = WKUP_BELOW_THRESHOLD;       

//             drp_rsn_map[tx_time][rx_time] = WKUP_DROPPED_REASON_NOISE;
	        return;
         }    
      }
      else if (txActive == true) { // we have to discard this pkt 
         if (debug_) cout << NOW << "  MPhy_WakeUp::startRx() rx a TONE but while tx" << endl;

         wkuph->startRx_time = WKUP_DISCARD;
         wkuph->endRx_time = WKUP_DISCARD;

//          drp_rsn_map[tx_time][rx_time] = WKUP_DROPPED_REASON_TX_PENDING;
	     return;
      }
    }
    else {
       if (debug_) cout << NOW << "  MPhy_WakeUp::startRx() rx a PKT but wrong MODID" << endl;

//        drp_rsn_map[tx_time][rx_time] = WKUP_DROPPED_REASON_WRONG_MODID;
       // wrong modid we have to discard this pkt
    }
}


void MPhy_WakeUp::endRx(Packet* p)
{
  hdr_cmn* ch = HDR_CMN(p);
  hdr_MPhy* ph = HDR_MPHY(p);

  if (ph->modulationType == modid) {

     hdr_wkup* wkuph = HDR_WKUP(p);

     if  ( wkuph->startRx_time == WKUP_DISCARD ) {
//        checkDropReason(p);
       dropPacket(p);
       return;
     }

     double per_ni; // packet error rate due to noise and/or interference
     double per_n;  // packet error rate due to noise only 
	
     int nbits = 1;
     per_n = getPER(ph->Pr/ph->Pn, nbits);
     per_ni = getPER(ph->Pr/(ph->Pn + ph->Pi), nbits);
//      per_ni = -1;
//      per_n = -1;

     double x = RNG::defaultrng()->uniform_double();
     bool error_ni = x <= per_ni;
     bool error_n = x <= per_n;
	
     ch->error() = error_ni || error_n ;

     if (error_n) {
        incrErrorPktsNoise();
     }
     else if (error_ni) {
        incrErrorPktsInterf();
     }

     if (PktRx == p) {  
        // always send up; the PHY is not responsible for discarding erroneous packets
		sendUp(p);
        PktRx = 0; // We can now sync onto another packet
     }
     else if (PktRx != 0) {
        hdr_wkup* rxwkuph = HDR_WKUP(PktRx);
        hdr_cmn* rxch = HDR_CMN(PktRx);
        // if no errors are found we refresh the time interval
        if (ch->error() == false) {
           double min_start_time = min(wkuph->startRx_time, rxwkuph->startRx_time);
           double max_end_time = max(wkuph->endRx_time, rxwkuph->endRx_time);
           rxwkuph->startRx_time = min_start_time;
           rxwkuph->endRx_time = max_end_time;
        }
        // we must discard this pkt and sendUp the last only
//         checkDropReason(p);
	    dropPacket(p);
     }
     else {
        // anything else, for debug purposes
//        checkDropReason(p);
       dropPacket(p);
     }
  }
  else {
    // wrong mod-id ==> discard pkt
//     checkDropReason(p);
    dropPacket(p);
  }
}


// void MPhy_WakeUp::checkDropReason(Packet* p) {
//   hdr_MPhy* ph = HDR_MPHY(p);
// 
//   cout.precision(16);
// 
//   if (debug_) cout << NOW << "  MPhy_WakeUp::checkDropReason() pkt txtime = " << ph->txtime << "; rxtime = " 
//                    << ph->rxtime << endl;
// 
//   map<double, map<double, int> >::iterator it = drp_rsn_map.find(ph->txtime);
//   map<double,int>::iterator it2;
// 
//   if ( it != drp_rsn_map.end() ) {
//       it2 = it->second.find(ph->rxtime);
// 
//       if ( it2 == it->second.end() ) {
//          cout << NOW << "  MPhy_WakeUp::checkDropReason() ERROR, can't find rx time" << endl;
//          exit(1);
//       }
//   }
//   else {
//       cout << NOW << "  MPhy_WakeUp::checkDropReason() ERROR, can't find tx time" << endl;
//       exit(1);
//   }
// 
//   if (it2->second == WKUP_DROPPED_REASON_CHAIN_SYNC) return;
//   else if (it2->second == WKUP_DROPPED_REASON_WRONG_MODID) {
//       if (debug_) cout << NOW << "  MPhy_WakeUp::checkDropReason() pkt dropped due to wrong mod id" << endl;
// 
//       incrDroppedPktsWrongModId();
//   }
//   else if (it2->second == WKUP_DROPPED_REASON_NOISE) {
//       if (debug_) cout << NOW << "  MPhy_WakeUp::checkDropReason() pkt dropped due to noise" << endl;
// 
//       incrDroppedPktsNoise();
//   }
//   else if (it2->second == WKUP_DROPPED_REASON_DEAFNESS) {
//       if (debug_) cout << NOW << "  MPhy_WakeUp::checkDropReason() pkt dropped due to deafness conditions" << endl;
// 
//       incrDroppedPktsDeaf();
//   } 
//   else if (it2->second == WKUP_DROPPED_REASON_TX_PENDING) {
//       if (debug_) cout << NOW << "  MPhy_WakeUp::checkDropReason() pkt dropped due to tx pending" << endl;
// 
//       incrDroppedPktsTxPending();
//   }
//   else {
//       cout << NOW << "  MPhy_Bpsk::checkDropReason() wrong reason!" << endl;
//       exit(1);
//   }
// }

void MPhy_WakeUp::dropPacket(Packet* p) { 
//   hdr_MPhy* ph = HDR_MPHY(p);
// 
//   map<double, map<double, int> >::iterator it = drp_rsn_map.find(ph->txtime);
// 
//   if ( it != drp_rsn_map.end() ) {
//       it->second.erase(ph->rxtime);
//       
//   }
//   else {
//       cout << NOW << "  MPhy_WakeUp::dropPacket() ERROR, can't find tx time" << endl;
//       exit(1);
//   }
// 
//   if ( it->second.size() == 0 ) drp_rsn_map.erase(ph->txtime);
//  
  Packet::free(p); 
}


double MPhy_WakeUp::getPER(double snr, int nbits)
{
  double ber = 0.5*erfc(sqrt(snr*0.5));  
  double per = 1-pow(1 - ber, nbits );
  return per;
}

void MPhy_WakeUp::waitForUser()
{
  std::string response;
  std::cout << "Press Enter to continue";
  std::getline(std::cin, response);
} 



