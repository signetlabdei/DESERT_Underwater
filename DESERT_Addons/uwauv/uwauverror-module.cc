//
// Copyright (c) 2017 Regents of the SIGNET lab, University of Padova.
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
* @author Filippo Campagnaro, Alessia Ortile
* @version 1.0.0
*
* \brief Provides the <i>UWAUVError</i> class implementation.
*
*/

#include "uwauverror-module.h"
#include <iostream>
#include <rng.h>
#include <random>
#include <stdint.h>
extern packet_t PT_UWCBR;
extern packet_t PT_UWAUV;
extern packet_t PT_UWAUV_CTR;
extern packet_t PT_UWAUV_ERROR;
int hdr_uwAUV_error::offset_; /**< Offset used to access in 
									<i>hdr_uwAUVError</i> packets header. */
/**
* Adds the header for <i>hdr_uwAUVError</i> packets in ns2.
*/
/**
* Adds the module for UwAUVModuleClass in ns2.
*/

/**
 * Class that represents the binding with the tcl configuration script 
 */
static class UwAUVErrorModuleClass : public TclClass {
public: 

	/**
   * Constructor of the class
   */
	UwAUVErrorModuleClass() : TclClass("Module/UW/AUV/ERR") {
	}

	/**
   * Creates the TCL object needed for the tcl language interpretation
   * @return Pointer to an TclObject
   */
	TclObject* create(int, const char*const*) {
		return (new UwAUVErrorModule());
	}
} class_module_uwAUV;


UwAUVErrorModule::UwAUVErrorModule() 
	: UwCbrModule()
	, last_sn_confirmed(0)
	, sn(0)
	, ackPolicy(ACK_PIGGYBACK)
	, ackTimeout(10)
	, ackNotPgbk(0)
	, drop_old_waypoints(0)
	, log_flag(0)
	, out_file_stats(0)
{
	UWSMPosition p = UWSMPosition();
	speed = 5;
	posit=&p;
    bind("ackTimeout_", (int*) &ackTimeout);
    bind("drop_old_waypoints_", (int*) &drop_old_waypoints);
    bind("log_flag_", (int*) &log_flag );
    if (ackTimeout < 0) {
    	cerr << NOW << " Invalide ACK timeout < 0, timeout set to 10 by defaults"
    		<< std::endl;
    	ackTimeout = 10;
    }

}

UwAUVErrorModule::UwAUVErrorModule(UWSMPosition* p) 
	: UwCbrModule()
	, last_sn_confirmed(0)
	, sn(0)
	, ackPolicy(ACK_PIGGYBACK)
	, ackTimeout(10)
	, ackNotPgbk(0)
	, drop_old_waypoints(0)
	, log_flag(0)
	, out_file_stats(0)
{
	posit = p;
	speed = 5;
    bind("ackTimeout_", (int*) &ackTimeout);
    bind("drop_old_waypoints_", (int*) &drop_old_waypoints);
    bind("log_flag_", (int*) &log_flag );
    if (ackTimeout < 0) {
    	cerr << NOW << " Invalide ACK timeout < 0, timeout set to 10 by defaults"
    		<< std::endl;
    	ackTimeout = 10;
    }


}

UwAUVErrorModule::~UwAUVErrorModule() {}

void UwAUVErrorModule::setPosition(UWSMPosition* p){
	posit = p;
}

int UwAUVErrorModule::command(int argc, const char*const* argv) {
	Tcl& tcl = Tcl::instance();
	if(argc == 2){
		if (strcasecmp(argv[1], "getAUVMonheadersize") == 0) {
			tcl.resultf("%d", getAUVMonHeaderSize());
			return TCL_OK;
		}
		else if(strcasecmp(argv[1], "getAUVctrheadersize") == 0) {
			tcl.resultf("%d", getAUVCTRHeaderSize());
			return TCL_OK;
		}else if(strcasecmp(argv[1], "getAUVErrorheadersize") == 0) {
			tcl.resultf("%d", getAUVErrorHeaderSize());
			return TCL_OK;
		}
		else if(strcasecmp(argv[1], "getX") == 0) {
			tcl.resultf("%f", posit->getX());
			return TCL_OK;
		}
		else if(strcasecmp(argv[1], "getY") == 0) {
			tcl.resultf("%f", posit->getY());
			return TCL_OK;
		}
		else if(strcasecmp(argv[1], "getZ") == 0) {
			tcl.resultf("%f", posit->getZ());
			return TCL_OK;
		}
		else if(strcasecmp(argv[1], "getAckNotPgbk") == 0) {
			tcl.resultf("%d", ackNotPgbk);
			return TCL_OK;
		}
	}
	else if(argc == 3){
		if (strcasecmp(argv[1], "setPosition") == 0) {
			UWSMPosition* p = dynamic_cast<UWSMPosition*> (tcl.lookup(argv[2]));
			posit=p;
			tcl.resultf("%s", "position Setted\n");
			return TCL_OK;
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
			}
		}
		if (strcasecmp(argv[1], "setAckTimeout") == 0) {
			ackTimeout = atof(argv[2]);
			return TCL_OK;
		}
	}
	else if(argc == 5){
		if (strcasecmp(argv[1], "setdest") == 0) {
			posit->setdest(atof(argv[2]),atof(argv[3]),atof(argv[4]));
			return TCL_OK;
		}
	}
	else if(argc == 6){
	if (strcasecmp(argv[1], "setdest") == 0) {
		posit->setdest(atof(argv[2]),atof(argv[3]),atof(argv[4]),atof(argv[5]));
		return TCL_OK;
		}
	}
	return UwCbrModule::command(argc,argv);
}

void UwAUVErrorModule::initPkt(Packet* p) {
	
	//hdr_uwAUV_ctr* uwAUVh = HDR_UWAUV_CTR(p);
	hdr_uwcbr *uwcbrh = HDR_UWCBR(p);


	if(this->p == NULL){
		random_device rd;
		mt19937 generator(rd());
		uniform_real_distribution<double> distrib(0.0, 1.0);
		hdr_uwAUV_error* uwAUVh = HDR_UWAUV_ERROR(p);

		double randomValue = distrib(generator) ;

		std::cout << NOW << " UwAUVErrorModule::r.v " 
				<< randomValue<< std::endl;

		if(randomValue < 0.75){
			uwAUVh->x() = posit->getX();
			uwAUVh->y() = posit->getY();
			uwAUVh->z() = posit->getZ();
			uwAUVh->speed() = speed;
			uwAUVh->sn() = ++sn;
			this->p = p;
			if (debug_) { 
			std::cout << NOW << " UwAUVErroModule::initPkt(Packet *p) " 
				<< "ERROR!"<< std::endl;
			}
		}
	}
	else{
		hdr_uwAUV_error* uwAUVh = HDR_UWAUV_ERROR(p);
		uwAUVh->x() = posit->getX();
		uwAUVh->y() = posit->getY();
		uwAUVh->z() = posit->getZ();
		uwAUVh->speed() = speed;
		uwAUVh->sn() = sn;
		if (debug_) { 
			std::cout << NOW << " UwAUVErroModule::initPkt(Packet *p) " 
				<< "Retransmitting"<< std::endl;
		}
	}

	if (debug_){
		hdr_uwAUV_error* uwAUVh = HDR_UWAUV_ERROR(p);
		std::cout << NOW << " UwAUVErrorModule::initPkt(Packet *p) AUV current "
			<< "position: X = " << uwAUVh->x() << ", Y = " << uwAUVh->y() 
			<< ", Z = " << uwAUVh->z()<< std::endl;
	}

	//ackTimer_.force_cancel();
	UwCbrModule::initPkt(p);

	if (debug_) {
		std::cout << NOW << " UwAUVErrorModule::sending packet with priority " 
			<< (int)uwcbrh->priority() << std::endl;
	}
	//priority_ = 0;
}

void UwAUVErrorModule::recv(Packet* p, Handler* h) {
	recv(p);
}

void UwAUVErrorModule::recv(Packet* p) {

	hdr_uwAUV_error* uwAUVh = HDR_UWAUV_ERROR(p);

	if(uwAUVh->ack() == sn + 1) {
		sendTmr_.force_cancel();
		this->p = NULL;
	}
	

	if(uwAUVh->ack() > 0 && debug_) 
		std::cout << NOW << " UwAUVErrorModule::recv(Packet *p) error ACK "
			<< "received " << uwAUVh->ack()<< std::endl;
	else if((uwAUVh->ack())<0 && debug_)
		std::cout << NOW << " UwAUVErrorModule::recv(Packet *p) error NACK " 
			<<"received"<< std::endl;
		
}