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
* @file uwauvctrer-b-module.cc
* @author Filippo Campagnaro, Alessia Ortile
* @version 1.0.0
*
* \brief Provides the <i>UWAUVCtrEr</i> class implementation.
*
* Provides the <i>UWAUVCtrEr</i> class implementation.
*/

#include "uwauvctrer-b-module.h"
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
static class UwAUVCtrErBModuleClass : public TclClass {
public:

	/**
   * Constructor of the class
   */
	UwAUVCtrErBModuleClass() : TclClass("Module/UW/AUV/CEB") {
	}

	/**
   * Creates the TCL object needed for the tcl language interpretation
   * @return Pointer to an TclObject
   */
	TclObject* create(int, const char*const*) {
		return (new UwAUVCtrErBModule());
	}
} class_module_uwAUV_error;


UwAUVCtrErBModule::UwAUVCtrErBModule(UWSMEPosition* p) 
	: UwCbrModule()
	, ack(0)
	, last_sn_confirmed(0)
	, sn(0)
	, ackTimeout(10)
	, drop_old_waypoints(1)
	, log_flag(0)
	, period(60)
	, speed(4)
{
	posit=p;
	x_sorg = posit->getX();
	y_sorg = posit->getY();

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

UwAUVCtrErBModule::UwAUVCtrErBModule() 
	: UwCbrModule()
	, ack(0)
	, last_sn_confirmed(0)
	, sn(0)
	, ackTimeout(10)
	, drop_old_waypoints(1)
	, log_flag(0)
	, period(60)
	, speed(4)

{
	p = NULL;
	UWSMEPosition p = UWSMEPosition();
	posit=&p;
	x_sorg = posit->getX();
	y_sorg = posit->getY();

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

UwAUVCtrErBModule::~UwAUVCtrErBModule() {}

int UwAUVCtrErBModule::command(int argc, const char*const* argv) {
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
			UWSMEPosition* p = dynamic_cast<UWSMEPosition*> (tcl.lookup(argv[2]));
			posit = p;
			return TCL_OK;
		} else
		if (strcasecmp(argv[1], "setSpeed") == 0) {
			speed = atof(argv[2]);
			return TCL_OK;
		} 
	}

	return UwCbrModule::command(argc,argv);
}

void UwAUVCtrErBModule::start() {}

void UwAUVCtrErBModule::setPosition(UWSMEPosition* p){

	posit = p;
	x_sorg = posit->getX();
	y_sorg = posit->getY();

}

void UwAUVCtrErBModule::transmit() {

	sendPkt();

	if (debug_) {
		std::cout << NOW << " UwAUVCtrErBModule::Sending pkt with period:  " << period 
			<< std::endl;
	}
	
	sendTmr_.resched(period);
}

float UwAUVCtrErBModule::getDistance(float x_s,float y_s,float x_d,float y_d){

	float dx = x_s - x_d;
	float dy = y_s - y_d;
	return std::sqrt(dx*dx + dy*dy); 

}

void UwAUVCtrErBModule::initPkt(Packet* p) {

	hdr_uwAUV_error* uwAUVh = HDR_UWAUV_ERROR(p);
	hdr_uwcbr *uwcbrh = HDR_UWCBR(p);
	bool found = false;

	uwAUVh->ack() = ack;  
	ack = 0;
	uwAUVh->error() = 0;



	if (alarm_mode){

		if ((getDistance(posit->getX(),posit->getY(),x_err,y_err) == 0.0)){ //If in the right position
			
			found = true;
			x_s = x_err;
			y_s = y_err;
			
			if (debug_) 
				std::cout << NOW << " UwAUVCtrErrModule::InitPkt(Packet *p) SV reached the destination"<< std::endl;
				

		}else if((getDistance(x_sorg,y_sorg,x_err,y_err) < getDistance(posit->getX(),posit->getY(),x_sorg,y_sorg))){ //if the right position																													// has been already passed
			
			found = true;
			x_s = x_err;
			y_s = y_err;


			if (debug_) 
				std::cout << NOW << " UwAUVCtrErrModule::InitPkt(Packet *p) SV has gone too far "<< std::endl;
				
		}

		if (found){
			
			uwAUVh->error() = -1;
			alarm_mode = false;
			uwAUVh->sn() = ++sn;
			uwAUVh->x() = x_s;
			uwAUVh->y() = y_s;
			this->p = p;

			if (log_flag == 1) {

				pos_log.open("log/position_log.csv",std::ios_base::app);
				pos_log << NOW << "," << posit->getX() << ","<< posit->getY() 
					<< ","<< posit->getZ() << std::endl;
				pos_log.close();

			}

			if (debug_) {
				std::cout << NOW << " UwAUVCtrErBModule::initPkt(Packet *p)  ERROR ("<< x_err << "," << y_err << ") SOLVED" 
				<< std::endl;
			}

			if (!alarm_queue.empty()){ //take care of the next error

				x_err = alarm_queue[0][0];
				y_err = alarm_queue[0][1];

				posit->setdest(x_err,y_err,posit->getZ(),speed);
				alarm_mode = true;

				alarm_queue.erase(alarm_queue.begin());

				x_sorg = posit->getX();
				y_sorg = posit->getY();
				
				if (debug_) {
					std::cout << NOW << " UwAUVCtrErrModule::initPkt(Packet *p) SV picked a new "
					"error from the queue: X = " << x_err << ", Y = " << y_err<< std::endl;
				}
			}

		}else{

			uwAUVh->error() = 1;
			uwAUVh->sn() = sn;
			uwAUVh->x() = x_err;
			uwAUVh->y() = y_err;
			this->p = p;

			if (debug_) {
				std::cout << NOW << " UwAUVCtrErBModule::initPkt(Packet *p)  ERROR ("<< x_err << "," << y_err << ") still to solve" 
				<< std::endl;
			}

		}

	}

	if (log_flag == 1) {

		pos_log.open("log/position_log.csv",std::ios_base::app);
		pos_log << NOW << "," << posit->getX() << ","<< posit->getY() 
			<< ","<< posit->getZ() << std::endl;
		pos_log.close();

	}
	

	if (debug_) {
	std::cout << NOW << " UwAUVCtrErBModule::initPkt(Packet *p)  ACK recv" 
		<< std::endl;
	}

	UwCbrModule::initPkt(p);

	if (debug_) {
		std::cout << NOW << " UwAUVCtrErBModule::initPkt(Packet *p)  setting last ack" 
			<< std::endl;
	}

}

void UwAUVCtrErBModule::recv(Packet* p, Handler* h) {
	recv(p);
}

void UwAUVCtrErBModule::recv(Packet* p) {
	
	hdr_uwAUV_error* uwAUVh = HDR_UWAUV_ERROR(p);
	bool exist = false;
	

	if(uwAUVh->ack() == sn + 1) { //ack received
		this->p = NULL;
	}
	

	if(uwAUVh->ack() > 0 && debug_) 
		std::cout << NOW << " UwAUVErrorModule::recv(Packet *p) error ACK "
			<< "received " << uwAUVh->ack()<< std::endl;
	else if((uwAUVh->ack())<0 && debug_)
		std::cout << NOW << " UwAUVErrorModule::recv(Packet *p) error NACK " 
			<<"received"<< std::endl;

	

	if (drop_old_waypoints == 1 && uwAUVh->sn() <= last_sn_confirmed) { //obsolete packets
		if (debug_) {
			std::cout << NOW << " UwAUVCtrErrModule::old error with sn " 
				<< uwAUVh->sn() << " dropped " << std::endl;
		}

	} else { 

		if (uwAUVh->error() == 0){// AUV MARKED IT AS NO ERROR

			if (debug_)
				std::cout << NOW << " UwAUVCtrErrModule:: no error" << std::endl;

		}else{ // error 

			if (!alarm_mode){

				//check in the queue before
				if (alarm_queue.empty()){

					x_err = uwAUVh->x();
					y_err = uwAUVh->y();
					x_sorg = posit->getX();
					y_sorg = posit->getY();

					posit->setdest(x_err,y_err,posit->getZ(),speed);

					alarm_mode = true;

					if (debug_) 
						std::cout << NOW << " UwAUVCtrErrModule::recv(Packet *p) SV received new "
						"error(2): X = " << uwAUVh->x() << ", Y = " << uwAUVh->y() << ", error = " << uwAUVh->error() << std::endl;

					if (log_flag == 1) {

						pos_log.open("log/position_log.csv",std::ios_base::app);
						pos_log << NOW << "," << posit->getX() << ","<< posit->getY() 
							<< ","<< posit->getZ() << std::endl;
						pos_log.close();

					}

				}else{

					x_err = alarm_queue[0][0];
					y_err = alarm_queue[0][1];
					posit->setdest(x_err,y_err,posit->getZ(),speed);

					alarm_mode = true;
					
					x_sorg = posit->getX();
					y_sorg = posit->getY();

					exist = false;

					for (const auto& vec : alarm_queue) {
						// Controlla se le coordinate corrispondono
						if (vec[0] == uwAUVh->x() && vec[1] == uwAUVh->y()) {
							exist = true;
							break;
						}
					}

					if (!exist){
						alarm_queue.push_back({uwAUVh->x(),uwAUVh->y()});
					}
					
					alarm_queue.erase(alarm_queue.begin());

					if (log_flag == 1) {

						pos_log.open("log/position_log.csv",std::ios_base::app);
						pos_log << NOW << "," << posit->getX() << ","<< posit->getY() 
							<< ","<< posit->getZ() << std::endl;
						pos_log.close();

					}


					if (debug_) 
						std::cout << NOW << " UwAUVCtrErrModule::recv(Packet *p) SV add new "
							"error(2) in the queue: X = " << uwAUVh->x() << ", Y = " << uwAUVh->y() << ", error = " <<  uwAUVh->error() << std::endl;

				}

			}else{

				exist = false;

				for (const auto& vec : alarm_queue) {
					// Controlla se le coordinate corrispondono
					if (vec[0] == uwAUVh->x() && vec[1] == uwAUVh->y()) {
						exist = true;
						break;
					}
				}

				if (!exist){
					alarm_queue.push_back({uwAUVh->x(),uwAUVh->y()});
				}

				if (debug_) 
					std::cout << NOW << " UwAUVCtrErrModule::recv(Packet *p) SV add new "
					"error(2) in the queue: X = " << uwAUVh->x() << ", Y = " << uwAUVh->y() << ", error = " <<  uwAUVh->error() << std::endl;
				
			}

		}

		last_sn_confirmed = uwAUVh->sn();
	}

	ack = last_sn_confirmed+1;

	UwCbrModule::recv(p);
	UwCbrModule::sendPkt();
}

