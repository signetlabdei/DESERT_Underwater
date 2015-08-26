//
// Copyright (c) 2014 Regents of the SIGNET lab, University of Padova.
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
* @file uwrov-module.cc
* @author Filippo Campagnaro
* @version 1.0.0
*
* \brief Provides the <i>UWROV</i> class implementation.
*
*/

#include "uwrov-module.h"
#include <iostream>
#include <rng.h>
#include <stdint.h>
extern packet_t PT_UWCBR;
int hdr_uwROV_monitoring::offset_; /**< Offset used to access in <i>hdr_uwROV</i> packets header. */
int hdr_uwROV_ctr::offset_; /**< Offset used to access in <i>hdr_uwROV</i> packets header. */

/**
* Adds the header for <i>hdr_uwROV</i> packets in ns2.
*/
/**
* Adds the module for UwROVModuleClass in ns2.
*/

/**
 * Class that represents the binding with the tcl configuration script 
 */
static class UwROVModuleClass : public TclClass {
public:

	/**
   * Constructor of the class
   */
	UwROVModuleClass() : TclClass("Module/UW/ROV") {
	}

	/**
   * Creates the TCL object needed for the tcl language interpretation
   * @return Pointer to an TclObject
   */
	TclObject* create(int, const char*const*) {
		return (new UwROVModule());
	}
} class_module_uwROV;

UwROVModule::UwROVModule() : UwCbrModule(), last_sn_confirmed(0), ack(0), send_ack_immediately(0){
	UWSMPosition p = UWSMPosition();
	posit=&p;
    bind("send_ack_immediately", (int*) &send_ack_immediately);
}

UwROVModule::UwROVModule(UWSMPosition* p) : UwCbrModule(), last_sn_confirmed(0), ack(0), send_ack_immediately(0){
	posit = p;
    bind("send_ack_immediately", (int*) &send_ack_immediately);
}

UwROVModule::~UwROVModule() {}

void UwROVModule::setPosition(UWSMPosition* p){
	posit = p;
}

int UwROVModule::command(int argc, const char*const* argv) {
	Tcl& tcl = Tcl::instance();
	if(argc == 2){
		if (strcasecmp(argv[1], "getROVMonheadersize") == 0) {
			tcl.resultf("%d", getROVMonHeaderSize());
			return TCL_OK;
		}
		else if(strcasecmp(argv[1], "getROVctrheadersize") == 0) {
			tcl.resultf("%d", getROVCTRHeaderSize());
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
			UWSMPosition* p = dynamic_cast<UWSMPosition*> (tcl.lookup(argv[2]));
			posit=p;
			tcl.resultf("%s", "position Setted\n");
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

void UwROVModule::initPkt(Packet* p) {
	hdr_uwROV_monitoring* uwROVh = HDR_UWROV_MONITORING(p);
	uwROVh->x() = posit->getX();
	uwROVh->y() = posit->getY();
	uwROVh->z() = posit->getZ();
	uwROVh->ack() = ack;
	ack=0;
	if (debug_)
		std::cout << NOW << " UwROVModule::initPkt(Packet *p) ROV current position: X = "
			<< uwROVh->x() << ", Y = " << uwROVh->y() << ", Z = " << uwROVh->z()<< std::endl;
	UwCbrModule::initPkt(p);
}

void UwROVModule::recv(Packet* p, Handler* h) {
	recv(p);
}

void UwROVModule::recv(Packet* p) {
	hdr_uwROV_ctr* uwROVh = HDR_UWROV_CTR(p);
	posit->setdest(uwROVh->x(),uwROVh->y(),uwROVh->z(),uwROVh->speed());
	last_sn_confirmed = uwROVh->sn();
	ack=last_sn_confirmed+1;
	if (debug_)
		std::cout << NOW << " UwROVModule::recv(Packet *p) ROV received new way point: X = "
			<< uwROVh->x() << ", Y = " << uwROVh->y() << ", Z = " << uwROVh->z()<< std::endl;
	UwCbrModule::recv(p);
	if (send_ack_immediately > 0)
		UwCbrModule::sendPkt();
}
