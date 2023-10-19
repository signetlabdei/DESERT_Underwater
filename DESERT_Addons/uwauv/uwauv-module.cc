//
// Copyright (c) 2023 Regents of the SIGNET lab, University of Padova.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
// 3. Neither the name of the University of Padova (SIGNET lab) nor the
// names of its contributors may be used to endorse or promote products
// derived from this software without specific prior written permission.
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
* @file uwauv-module.cc
* @author Alessia Ortile
* @version 1.0.0
*
* \brief Provides the <i>UWAUV</i> class implementation.
*
*/

#include "uwauv-module.h"
#include <iostream>
#include <rng.h>
#include <stdint.h>
extern packet_t PT_UWCBR;
extern packet_t PT_UWAUV;
extern packet_t PT_UWAUV_CTR;
int hdr_uwAUV_monitoring::offset_; /**< Offset used to access in 
									<i>hdr_uwAUV</i> packets header. */
int hdr_uwAUV_ctr::offset_; /**< Offset used to access in <i>hdr_uwAUV</i> 
									packets header. */

/**
* Adds the header for <i>hdr_uwAUV</i> packets in ns2.
*/
/**
* Adds the module for UwAUVModuleClass in ns2.
*/

/**
 * Class that represents the binding with the tcl configuration script 
 */
static class UwAUVModuleClass : public TclClass {
public:

	/**
   * Constructor of the class
   */
	UwAUVModuleClass() : TclClass("Module/UW/AUV") {
	}

	/**
   * Creates the TCL object needed for the tcl language interpretation
   * @return Pointer to an TclObject
   */
	TclObject* create(int, const char*const*) {
		return (new UwAUVModule());
	}
} class_module_uwAUV;

void UwAUVSendAckTimer::expire(Event *e) {
	module->sendAck();
}

UwAUVModule::UwAUVModule() 
	: UwCbrModule()
	, last_sn_confirmed(0)
	, ack(0)
	, ackPolicy(ACK_PIGGYBACK)
	, ackTimeout(10)
	, ackTimer_(this)
	, ackPriority(0)
	, ackNotPgbk(0)
	, drop_old_waypoints(0)
	, log_on_file(0)
	, out_file_stats(0)
{
	posit= new UWSMWPPosition();
    bind("ackTimeout_", (double*) &ackTimeout);
    bind("ackPriority_", (int*) &ackPriority);
    bind("drop_old_waypoints_", (int*) &drop_old_waypoints);
    bind("log_on_file_", (int*) &log_on_file );
    if (ackTimeout < 0) {
    	cerr << NOW << " Invalide ACK timeout < 0, timeout set to 10 by defaults"
    		<< std::endl;
    	ackTimeout = 10;
    }

}

UwAUVModule::UwAUVModule(UWSMWPPosition* p) 
	: UwCbrModule()
	, last_sn_confirmed(0)
	, ack(0)
	, ackPolicy(ACK_PIGGYBACK)
	, ackTimeout(10)
	, ackTimer_(this)
	, ackPriority(0)
	, ackNotPgbk(0)
	, drop_old_waypoints(0)
	, log_on_file(0)
	, out_file_stats(0)
{
	posit = p;
    bind("ackTimeout_", (double*) &ackTimeout);
    bind("ackPriority_", (int*) &ackPriority);
    bind("drop_old_waypoints_", (int*) &drop_old_waypoints);
    bind("log_on_file_", (int*) &log_on_file );
    if (ackTimeout < 0) {
    	cerr << NOW << " Invalide ACK timout < 0, timeout set to 10 by defaults"
    		<< std::endl;
    	ackTimeout = 10;
    }


}

UwAUVModule::~UwAUVModule() {}

void UwAUVModule::setPosition(UWSMWPPosition* p){
	posit = p;
}

int UwAUVModule::command(int argc, const char*const* argv) {
	Tcl& tcl = Tcl::instance();
	if(argc == 2){
		if (strcasecmp(argv[1], "getAUVMonheadersize") == 0) {
			tcl.resultf("%d", getAUVMonHeaderSize());
			return TCL_OK;
		} else if(strcasecmp(argv[1], "getAUVctrheadersize") == 0) {
			tcl.resultf("%d", getAUVCTRHeaderSize());
			return TCL_OK;
		} else if(strcasecmp(argv[1], "getX") == 0) {
			tcl.resultf("%f", posit->getX());
			return TCL_OK;
		} else if(strcasecmp(argv[1], "getY") == 0) {
			tcl.resultf("%f", posit->getY());
			return TCL_OK;
		} else if(strcasecmp(argv[1], "getZ") == 0) {
			tcl.resultf("%f", posit->getZ());
			return TCL_OK;
		} else if(strcasecmp(argv[1], "getAckNotPgbk") == 0) {
			tcl.resultf("%d", ackNotPgbk);
			return TCL_OK;
		}
	} else if(argc == 3){
		if (strcasecmp(argv[1], "setPosition") == 0) {

			UWSMWPPosition* p = dynamic_cast<UWSMWPPosition*> (tcl.lookup(argv[2]));

			if(p){
				posit=p;
				tcl.resultf("%s", "position Setted\n");
				return TCL_OK;
			} else {
				tcl.resultf("%s", "Invalid position\n");
				return TCL_ERROR;
			}
			
		}
		if (strcasecmp(argv[1], "setAckPolicy") == 0) {
			if (atof(argv[2]) == 1) {
				ackPolicy = ACK_PIGGYBACK;
				return TCL_OK;
			}
			if (atof(argv[2]) == 2) {
				ackPolicy = ACK_IMMEDIATELY;
				return TCL_OK;
			}
			if (atof(argv[2]) == 3) {
				ackPolicy = ACK_PGBK_OR_TO;
				return TCL_OK;
			} else {
				cerr<<"Policy not supported" << std::endl;
				return TCL_ERROR;
			}
		}
		if (strcasecmp(argv[1], "setAckTimeout") == 0) {
			ackTimeout = atof(argv[2]);
			return TCL_OK;
		}
		if (strcasecmp(argv[1], "setAckPriority") == 0) {
			ackPriority = atof(argv[2]);
			return TCL_OK;
		}
	} else if(argc == 5){
		if (strcasecmp(argv[1], "setDest") == 0) {
			posit->setDest(atof(argv[2]),atof(argv[3]),atof(argv[4]));
			return TCL_OK;
		} else if (strcasecmp(argv[1], "addDest") == 0) {
			posit->addDest(atof(argv[2]),atof(argv[3]),atof(argv[4]));
			return TCL_OK;
		}
	} else if(argc == 6){
	if (strcasecmp(argv[1], "setDest") == 0) {
		posit->setDest(atof(argv[2]),atof(argv[3]),atof(argv[4]),atof(argv[5]));
		return TCL_OK;
		} else if (strcasecmp(argv[1], "addDest") == 0) {
		posit->addDest(atof(argv[2]),atof(argv[3]),atof(argv[4]),atof(argv[5]));
		return TCL_OK;
		}
	}
	return UwCbrModule::command(argc,argv);
}

void UwAUVModule::initPkt(Packet* p) {

	hdr_uwAUV_monitoring* uwAUVh = hdr_uwAUV_monitoring::access(p);

	hdr_uwcbr *uwcbrh = HDR_UWCBR(p);
	uwAUVh->x() = posit->getX();
	uwAUVh->y() = posit->getY();
	uwAUVh->z() = posit->getZ();
	
	ack=0;

	if (debug_)
		std::cout << NOW << " UwAUVModule::initPkt(Packet *p) AUV current "
			<< "position: X = " << uwAUVh->x() << ", Y = " << uwAUVh->y() 
			<< ", Z = " << uwAUVh->z()<< std::endl;

	ackTimer_.force_cancel();
	UwCbrModule::initPkt(p);

	if (debug_) {
		std::cout << NOW << " UwAUVModule::sending packet with priority " 
			<< (int)uwcbrh->priority() << std::endl;
	}

	priority_ = 0;
}

void UwAUVModule::recv(Packet* p) {

	hdr_uwAUV_ctr* uwAUVh = hdr_uwAUV_ctr::access(p);

	//obsolete packets
	if (drop_old_waypoints == 1 && uwAUVh->sn() <= last_sn_confirmed) { 

		if (debug_) {
			std::cout << NOW << " UwAUVModule::old waypoint with sn " 
				<< uwAUVh->sn() << " dropped " << std::endl;
		}

	} else { //packet in order

		posit->addDest(uwAUVh->x(),uwAUVh->y(),uwAUVh->z(),uwAUVh->speed());
		last_sn_confirmed = uwAUVh->sn();

	}

	ack = last_sn_confirmed+1;
	priority_ = (char) ackPriority;

	if (log_on_file == 1) {
		out_file_stats.open("my_log_file.csv",std::ios_base::app);
		out_file_stats << left << "time: " << NOW << ", positions AUV: x = " 
			<< posit->getX() << ", y = " << posit->getY() 
			<< ", z = " << posit->getZ() << std::endl;
		out_file_stats.close();
	}


	if (debug_)
		std::cout << NOW << " UwAUVModule::recv(Packet *p) AUV received new "
			<< "way point: X = " << uwAUVh->x() << ", Y = " << uwAUVh->y() 
			<< ", Z = " << uwAUVh->z()<< std::endl;
	
	UwCbrModule::recv(p);

	if (ackPolicy == ACK_IMMEDIATELY) {			
		sendAck();
	}

	if (ackPolicy == ACK_PGBK_OR_TO) {
		ackTimer_.resched(ackTimeout);
	}
}

void UwAUVModule::sendAck() {

	ackNotPgbk++;
	if (ackPriority == 0) {
		UwCbrModule::sendPkt();
		if (debug_){
			if (ackPolicy == ACK_IMMEDIATELY) {
				cout << NOW << " ACK sent immediately with standard priority " 
					<< std::endl;
			} else {
				cout << NOW << " ACK timeout expired, ACK sent with standard "
				<< "priority " << std::endl;
			}			
		}
	} else {
		UwCbrModule::sendPktHighPriority();
		if (debug_){
			if (ackPolicy == ACK_IMMEDIATELY) {
				cout << NOW << " ACK sent immediately with high priority " 
					<< std::endl;
			} else {
				cout << NOW << " ACK timeout expired, ACK sent with high priority" 
					<< std::endl;
			}
		}
	}
	
}