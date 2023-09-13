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
* @file uwauvctr-module.cc
* @author Alessia Ortile
* @version 1.0.0
*
* \brief Provides the <i>UWAUVCtr</i> class implementation.
*
* Provides the <i>UWAUVCtr</i> class implementation.
*/

#include "uwauvctr-module.h"
#include <iostream>
#include <rng.h>
#include <stdint.h>
extern packet_t PT_UWCBR;
extern packet_t PT_UWAUV;
extern packet_t PT_UWAUV_CTR;

/**
* Adds the module for UwAUVModuleClass in ns2.
*/

/**
* Class that represents the binding with the tcl configuration script 
*/
static class UwAUVCtrModuleClass : public TclClass {
public:

	/**
   * Constructor of the class
   */
	UwAUVCtrModuleClass() : TclClass("Module/UW/AUV/CTR") {
	}

	/**
   * Creates the TCL object needed for the tcl language interpretation
   * @return Pointer to an TclObject
   */
	TclObject* create(int, const char*const*) {
		return (new UwAUVCtrModule());
	}
} class_module_uwAUV_ctr;

UwAUVCtrModule::UwAUVCtrModule(UWSMWPPosition* p) 
	: UwCbrModule()
	, sn(0)
	, adaptiveRTO(0)
	, adaptiveRTO_parameter(0.5)
{
	posit=p;
	speed=0.5;
	bind("adaptiveRTO_", (int *) &adaptiveRTO);
	bind("adaptiveRTO_parameter_", (double *) &adaptiveRTO_parameter);

	if (adaptiveRTO_parameter < 0) {
		cerr << NOW << "Invalid adaptive RTO parameter < 0, set to 0.5 " 
			<< "by default " << std::endl;
		adaptiveRTO_parameter = 0.5;
	}
}

UwAUVCtrModule::UwAUVCtrModule() 
	: UwCbrModule()
	, sn(0) 
	, adaptiveRTO(0)
	, adaptiveRTO_parameter(0.5)
{

	posit= new UWSMWPPosition();
	speed = 0.5;
	bind("adaptiveRTO_", (int *) &adaptiveRTO);
	bind("adaptiveRTO_parameter_", (double *) &adaptiveRTO_parameter);	

	if (adaptiveRTO_parameter < 0) {
		cerr << NOW << "Invalide adaptive RTO parameter < 0, set to 0.5 "
			<< "by default " << std::endl;
		adaptiveRTO_parameter = 0.5;

	}
	
}

UwAUVCtrModule::~UwAUVCtrModule() {}

int UwAUVCtrModule::command(int argc, const char*const* argv) {
	Tcl& tcl = Tcl::instance();
	if(argc == 2){
		if (strcasecmp(argv[1], "getAUVMonheadersize") == 0) {
			tcl.resultf("%d", this->getAUVMonHeaderSize());
			return TCL_OK;
		}
		else if(strcasecmp(argv[1], "getAUVctrheadersize") == 0) {
			tcl.resultf("%d", this->getAUVCTRHeaderSize());
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
	}
	else if(argc == 3){
		if (strcasecmp(argv[1], "setPosition") == 0) {
			UWSMWPPosition* p = dynamic_cast<UWSMWPPosition*> (tcl.lookup(argv[2]));
			if(p){
				posit=p;
				tcl.resultf("%s", "position Setted\n");
				return TCL_OK;
			}else{
				tcl.resultf("%s", "Invalid position\n");
				return TCL_ERROR;
			}
			
		} else if (strcasecmp(argv[1], "setSpeed") == 0) {
			speed = atof(argv[2]);
			return TCL_OK;
		} else if (strcasecmp(argv[1], "setAdaptiveRTOparameter") == 0) {
			adaptiveRTO_parameter = atof(argv[2]);
			return TCL_OK;
		}
	}
	else if(argc == 5){
		if (strcasecmp(argv[1], "sendPosition") == 0) {
			newX = atof(argv[2]);
			newY = atof(argv[3]);
			newZ = atof(argv[4]);
			this->reset_retx();
			this->transmit();
			tcl.resultf("%s", "position Setted");
			return TCL_OK;
		}
	}else if(argc == 6){
		if (strcasecmp(argv[1], "sendPosition") == 0) {
			newX = atof(argv[2]);
			newY = atof(argv[3]);
			newZ = atof(argv[4]);
			speed = atof(argv[5]);
			this->reset_retx();
			this->transmit();
			tcl.resultf("%s", "position Setted");
			return TCL_OK;
		}
	}
	return UwCbrModule::command(argc,argv);
}

void UwAUVCtrModule::transmit() {
	sendPkt();

	if (adaptiveRTO == 1 && rttsamples > 0) {
		period_ =  sumrtt/rttsamples + adaptiveRTO_parameter* sumrtt/rttsamples;
		if (debug_) {
			std::cout << NOW << " UwAUVCtrModule::RTO set to " << period_ 
				<< std::endl;
		}
	}
	
	sendTmr_.resched(period_);
}

void UwAUVCtrModule::start() {}

void UwAUVCtrModule::setPosition(UWSMWPPosition* p){
	posit = p;
}

void UwAUVCtrModule::initPkt(Packet* p) {
	if(this->p == NULL){
		hdr_uwAUV_ctr* uwAUVh = HDR_UWAUV_CTR(p);
		uwAUVh -> x() = newX;
		uwAUVh->y() = newY;
		uwAUVh->z() = newZ;
		uwAUVh->speed() = speed;
		uwAUVh->sn() = ++sn;
		this->p = p;
	}
	else{
		hdr_uwAUV_ctr* uwAUVh = HDR_UWAUV_CTR(p);
		uwAUVh->x() = newX;
		uwAUVh->y() = newY;
		uwAUVh->z() = newZ;
		uwAUVh->speed() = speed;
		uwAUVh->sn() = sn;
		if (debug_) { 
			std::cout << NOW << " UwAUVCtrModule::initPkt(Packet *p) " 
				<< "Retransmitting"<< std::endl;
		}
	}

	if (debug_) { 
		if (rttsamples > 0) {
			std::cout << NOW << " RTT UwAUVCtr: " << sumrtt/rttsamples 
				<< " s" << std::endl;
		}
	}

	UwCbrModule::initPkt(p);
	if (debug_) {
		hdr_uwAUV_ctr* uwAUVh = HDR_UWAUV_CTR(p);
		std::cout << NOW << " UwAUVCtrModule::initPkt(Packet *p)  setting new" 
			<< " AUV way-point: X = "<< uwAUVh->x() <<", Y = " 
			<< uwAUVh->y() << ", Z = " << uwAUVh->z()<< std::endl;
	}
}

void UwAUVCtrModule::recv(Packet* p) {
	
	hdr_uwAUV_monitoring* monitoring = HDR_UWAUV_MONITORING(p);
	x_auv = monitoring->x();
	y_auv = monitoring->y();
	z_auv = monitoring->z();

	
	hdr_uwcbr *uwcbrh = HDR_UWCBR(p);
	hdr_cmn *ch = hdr_cmn::access(p);
	
	if(monitoring->ack() == sn + 1) {
		sendTmr_.force_cancel();
		this->p = NULL;
	}

	if(monitoring->ack() > 0 && debug_) 
		std::cout << NOW << " UwAUVCtrModule::recv(Packet *p) control ACK "
			<< "received " << monitoring->ack()<< std::endl;
	else if((monitoring->ack())<0 && debug_)
		std::cout << NOW << " UwAUVCtrModule::recv(Packet *p) control error " 
			<<"received"<< std::endl;
	if (debug_ > 10)
		std::cout << NOW << " UwAUVCtrModule::recv(Packet *p) AUV monitoring "
			<< "position: X = " << x_auv << ", Y = " << y_auv 
			<< ", Z = " << z_auv << std::endl;

	UwCbrModule::recv(p);
}
