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
* @file uwauvctrer-module.cc
* @author Filippo Campagnaro, Alessia Ortile
* @version 1.0.0
*
* \brief Provides the <i>UWAUVCtrEr</i> class implementation.
*
* Provides the <i>UWAUVCtrEr</i> class implementation.
*/

#include "uwauvctrer-module.h"
#include <iostream>
#include <rng.h>
#include <stdint.h>
extern packet_t PT_UWCBR;
extern packet_t PT_UWAUV;
extern packet_t PT_UWAUV_CTR;
extern packet_t PT_UWAUV_ERROR;
/**
* Adds the module for UwAUVModuleClass in ns2.
*/

/**
* Class that represents the binding with the tcl configuration script 
*/
static class UwAUVCtrErModuleClass : public TclClass {
public:

	/**
   * Constructor of the class
   */
	UwAUVCtrErModuleClass() : TclClass("Module/UW/AUV/CER") {
	}

	/**
   * Creates the TCL object needed for the tcl language interpretation
   * @return Pointer to an TclObject
   */
	TclObject* create(int, const char*const*) {
		return (new UwAUVCtrErModule());
	}
} class_module_uwAUV_error;


UwAUVCtrErModule::UwAUVCtrErModule(UWSMPosition* p) 
	: UwCbrModule()
	, ack(0)
	, last_sn_confirmed(0)
	, sn(0)
	, ackTimeout(10)
	, drop_old_waypoints(1)
	, log_flag(0)
	, pos_log(0)
	, alarm_mode(false)
	, period(60)
{
	posit=p;
	speed=5;
	bind("ackTimeout_", (int*) &ackTimeout);
    bind("drop_old_waypoints_", (int*) &drop_old_waypoints);
    bind("log_flag_", (int*) &log_flag );
	bind("period_", (int*) &period );
    if (ackTimeout < 0) {
    	cerr << NOW << " Invalide ACK timeout < 0, timeout set to 10 by defaults"
    		<< std::endl;
    	ackTimeout = 10;
    }
}

UwAUVCtrErModule::UwAUVCtrErModule() 
	: UwCbrModule()
	, ack(0)
	, last_sn_confirmed(0)
	, sn(0)
	, ackTimeout(10)
	, drop_old_waypoints(1)
	, log_flag(0)
	//, out_file_stats(0)
	, alarm_mode(false) 
	, period(60)

{
	p = NULL;
	UWSMPosition p = UWSMPosition();
	posit=&p;
	//posit = Position();
	speed = 5;
	bind("ackTimeout_", (int*) &ackTimeout);
    bind("drop_old_waypoints_", (int*) &drop_old_waypoints);
    bind("log_flag_", (int*) &log_flag );
	bind("period_", (int*) &period );
    if (ackTimeout < 0) {
    	cerr << NOW << " Invalide ACK timeout < 0, timeout set to 10 by defaults"
    		<< std::endl;
    	ackTimeout = 10;
    }
	
}

UwAUVCtrErModule::~UwAUVCtrErModule() {}

int UwAUVCtrErModule::command(int argc, const char*const* argv) {
	Tcl& tcl = Tcl::instance();
	if(argc == 2){
		if (strcasecmp(argv[1], "getAUVMonheadersize") == 0) {
			tcl.resultf("%d", this->getAUVMonHeaderSize());
			return TCL_OK;
		}
		else if(strcasecmp(argv[1], "getAUVctrheadersize") == 0) {
			tcl.resultf("%d", this->getAUVCTRHeaderSize());
			return TCL_OK;
		}else if(strcasecmp(argv[1], "getAUVErrorheadersize") == 0) {
			tcl.resultf("%d", this->getAUVErrorHeaderSize());
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
			posit = p;
			return TCL_OK;
		} else
		if (strcasecmp(argv[1], "setSpeed") == 0) {
			speed = atof(argv[2]);
			return TCL_OK;
		} 
	}
	/*else if(argc == 4){
		/**if (strcasecmp(argv[1], "sendPosition") == 0) {
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
	}*/

	return UwCbrModule::command(argc,argv);
}

void UwAUVCtrErModule::start() {}

void UwAUVCtrErModule::setPosition(UWSMPosition* p){
	posit = p;

}

void UwAUVCtrErModule::transmit() {
	sendPkt();

	if (debug_) {
		std::cout << NOW << " UwAUVCtrErModule::Sending pkt with period:  " << period 
			<< std::endl;
	}

	
	sendTmr_.resched(period);
}

void UwAUVCtrErModule::initPkt(Packet* p) {

	hdr_uwAUV_error* uwAUVh = HDR_UWAUV_ERROR(p);
	hdr_uwcbr *uwcbrh = HDR_UWCBR(p);

	uwAUVh->ack() = ack;
	ack = 0;

	if(this->p == NULL){ 

		if ((abs(posit->getX() - x_auv) < 0.1) && (abs(posit->getY() - y_auv ) < 0.1)){
			
			uwAUVh->speed() = 100;
			alarm_mode = false;
			uwAUVh->sn() = ++sn;
			this->p = p;

			if (debug_) {
				std::cout << NOW << " UwAUVCtrErModule::initPkt(Packet *p)  ERROR SOLVED" 
			<< std::endl;
		}

		}

		if (debug_) {
		std::cout << NOW << " UwAUVCtrErModule::initPkt(Packet *p)  ACK recv" 
			<< std::endl;
		}

	//Retransmission
	}else if ((abs(posit->getX() - x_auv) < 0.1) && (abs(posit->getY() - y_auv ) < 0.1) ){

		uwAUVh->speed() = 100;
		alarm_mode = false;
		uwAUVh->sn() = sn;

		if (debug_) {
		std::cout << NOW << " UwAUVCtrErModule::initPkt(Packet *p)  Retransmission ERROR SOLVED" 
			<< std::endl;
		}
	}
	
	UwCbrModule::initPkt(p);

	if (debug_) {
		std::cout << NOW << " UwAUVCtrErModule::initPkt(Packet *p)  setting last ack" 
			<< std::endl;
	}

	if (log_flag == 1) {

			pos_log.open("position_log.csv",std::ios_base::app);
			pos_log << NOW << "," << posit->getX() << ","<< posit->getY() 
				<< ","<< posit->getZ() << std::endl;
			pos_log.close();

	}
}

void UwAUVCtrErModule::recv(Packet* p, Handler* h) {
	recv(p);
}

void UwAUVCtrErModule::recv(Packet* p) {
	
	hdr_uwAUV_error* uwAUVh = HDR_UWAUV_ERROR(p);
	

	if(uwAUVh->ack() == sn + 1) { //ack received
		//sendTmr_.force_cancel();
		this->p = NULL;
	}
	

	if(uwAUVh->ack() > 0 && debug_) 
		std::cout << NOW << " UwAUVErrorModule::recv(Packet *p) error ACK "
			<< "received " << uwAUVh->ack()<< std::endl;
	else if((uwAUVh->ack())<0 && debug_)
		std::cout << NOW << " UwAUVErrorModule::recv(Packet *p) error NACK " 
			<<"received"<< std::endl;

	if (!alarm_mode){

		if (drop_old_waypoints == 1 && uwAUVh->sn() <= last_sn_confirmed) { //obsolete packets
			if (debug_) {
				std::cout << NOW << " UwAUVCtrErrModule::old error with sn " 
					<< uwAUVh->sn() << " dropped " << std::endl;
			}

		} else { //packet in order
			x_auv = uwAUVh->x();
			y_auv = uwAUVh->y();
			posit->setdest(x_auv,y_auv,posit->getZ(),1);
			last_sn_confirmed = uwAUVh->sn();
			alarm_mode = true;

			if (log_flag == 1) {
				err_log.open("error_log.csv",std::ios_base::app);
				err_log << NOW << "," << x_auv<<","<<y_auv<< std::endl;
				err_log.close();
			}

		}

		ack = last_sn_confirmed+1;

		if (log_flag == 1) {
			pos_log.open("position_log.csv",std::ios_base::app);
			pos_log<< NOW << ","<<posit->getX() << ","<< posit->getY() 
				<< ","<< posit->getZ() << std::endl;
			pos_log.close();
		}


		if (debug_) {
			std::cout << NOW << " UwAUVCtrErrModule::recv(Packet *p) SV received new "
				"error: X = " << uwAUVh->x() << ", Y = " << uwAUVh->y() 
				<< ", Z = " << uwAUVh->z() << " speed "<< uwAUVh->speed()<< std::endl;
		}

		UwCbrModule::recv(p);


		if (debug_)
			cout << NOW << " ACK sent immediately with standard priority " 
				<< std::endl;
	}

	UwCbrModule::sendPkt();

	if (debug_) 
		std::cout << NOW << " UwAUVCtrErrModule::recv(Packet *p) Packet dropped: "
			<< "alarm mode "<< std::endl;
		

}

