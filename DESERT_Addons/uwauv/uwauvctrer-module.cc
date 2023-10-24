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
* @file uwauvctrer-module.cc
* @author Alessia Ortile
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


UwAUVCtrErModule::UwAUVCtrErModule(UWSMWPPosition* p) 
	: UwCbrModule()
	, last_sn_confirmed(0)
	, sn(0)
	, drop_old_waypoints(1)
	, log_on_file(0)
	, period(60)
	, speed(1.5)
	, accuracy(0.001)
{
	posit=p;
	x_sorg = posit->getX();
	y_sorg = posit->getY();

	bind("drop_old_waypoints_", (int*) &drop_old_waypoints);
	bind("log_on_file_", (int*) &log_on_file );
	bind("period_", (int*) &period );
	bind("sigma_", (double*) &sigma);
	bind("th_ne_", (double*) &th_ne );
	bind("accuracy_ne_", (double*) &accuracy );

}

UwAUVCtrErModule::UwAUVCtrErModule() 
	: UwCbrModule()
	, last_sn_confirmed(0)
	, sn(0)
	, drop_old_waypoints(1)
	, log_on_file(0)
	, period(60)
	, speed(1.5)
	, accuracy(0.001)

{
	posit = new  UWSMWPPosition();
	x_sorg = posit->getX();
	y_sorg = posit->getY();

	bind("drop_old_waypoints_", (int*) &drop_old_waypoints);
	bind("log_on_file_", (int*) &log_on_file );
	bind("period_", (int*) &period );
	bind("sigma_", (double*) &sigma);
	bind("th_ne_", (double*) &th_ne );
	bind("accuracy_ne_", (double*) &accuracy );
	
}

UwAUVCtrErModule::~UwAUVCtrErModule() {}

int UwAUVCtrErModule::command(int argc, const char*const* argv) {
	Tcl& tcl = Tcl::instance();
	if(argc == 2){
		if (strcasecmp(argv[1], "getAUVMonheadersize") == 0) {
			tcl.resultf("%d", this->getAUVMonHeaderSize());
			return TCL_OK;
		} else if(strcasecmp(argv[1], "getAUVctrheadersize") == 0) {
			tcl.resultf("%d", this->getAUVCTRHeaderSize());
			return TCL_OK;
		} else if(strcasecmp(argv[1], "getAUVErrorheadersize") == 0) {
			tcl.resultf("%d", this->getAUVErrorHeaderSize());
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
		} else if (strcasecmp(argv[1], "setSpeed") == 0) {
			speed = atof(argv[2]);
			return TCL_OK;
		} 
	}

	return UwCbrModule::command(argc,argv);
}


void UwAUVCtrErModule::setPosition(UWSMWPPosition* p){

	posit = p;
	x_sorg = posit->getX();
	y_sorg = posit->getY();

	if (log_on_file == 1) {
		pos_log.open("log/position_log.csv",std::ios_base::app);
		pos_log << NOW << "," << posit->getX() << ","<< posit->getY() 
			<< ","<< posit->getZ() << std::endl;
		pos_log.close();
	}

}

void UwAUVCtrErModule::transmit() {

	sendPkt();

	if (debug_) {
		std::cout << NOW << " UwAUVCtrErModule::Sending pkt with period: " << period 
			<< std::endl;
	}
	
	sendTmr_.resched(period);
}

float UwAUVCtrErModule::getDistance(float x_s,float y_s,float x_d,float y_d){

	float dx = x_s - x_d;
	float dy = y_s - y_d;
	return std::sqrt(dx*dx + dy*dy); 

}

float UwAUVCtrErModule::getDistance(float x_s,float y_s,float z_s,float x_d,float y_d,float z_d){

	float dx = x_s - x_d;
	float dy = y_s - y_d;
	float dz = z_s - z_d;

	return std::sqrt(dx*dx + dy*dy + dz*dz); 

}

void UwAUVCtrErModule::initPkt(Packet* p) {

	hdr_uwAUV_error* uwAUVh = hdr_uwAUV_error::access(p);
	hdr_uwcbr *uwcbrh = HDR_UWCBR(p);

	bool found = false;
	uwAUVh->error() = 0;

	if (error_released){

		uwAUVh->error() = -1;
		uwAUVh->sn() = ++sn;
		uwAUVh->x() = x_s;
		uwAUVh->y() = y_s;
		error_released = false;
		this->p = p;

		if (debug_) 
			std::cout << NOW << " UwAUVCtrErrModule::initPkt(Packet *p) Error" 
				<< "released"<< std::endl;


	} else if (alarm_mode == 2 && active_alarm){ //I need to go there

		if (log_on_file == 1) {

			pos_log.open("log/position_log.csv",std::ios_base::app);
			pos_log << NOW << "," << posit->getX() << ","<< posit->getY() 
				<< ","<< posit->getZ() << std::endl;
			pos_log.close();

		}

		//If in the right position
		if (getDistance(posit->getX(),posit->getY(),x_err,y_err) == 0.0){ 
			found = true;
			//x_s = x_err;
			//y_s = y_err;

			
			alarm_mode = 3;
			posit->setDest(x_err,y_err,-90,speed);

			if (debug_){ 
				std::cout << NOW << " UwAUVCtrErrModule::InitPkt(Packet *p) SV" 
					<< "reached the surface destination"<< std::endl;
				std::cout << NOW << " UwAUVCtrErrModule::InitPkt(Packet *p) start to dive"
					<< std::endl;
			}

				
		//if the right position
		} else if(getDistance(x_sorg,y_sorg,x_err,y_err) < 
			getDistance(posit->getX(),posit->getY(),x_sorg,y_sorg)){ 
			
			found = true;
			//x_s = x_err;
			//y_s = y_err;

			alarm_mode = 3;
			posit->setDest(x_err,y_err,-90,speed);


			if (debug_){ 
				std::cout << NOW << " UwAUVCtrErrModule::InitPkt(Packet *p) ASV has gone"
					<< "too far "<< std::endl;
				std::cout << NOW << " UwAUVCtrErrModule::InitPkt(Packet *p) start to dive"
					<< std::endl;
			}	
		}

		if(!found){
			uwAUVh->error() = 1;
			uwAUVh->sn() = ++sn; 
			uwAUVh->x() = x_err;
			uwAUVh->y() = y_err;
			this->p = p;

			if (debug_) {
				std::cout << NOW << " UwAUVCtrErModule::initPkt(Packet *p)  ERROR ("<< x_err 
					<< "," << y_err << ") still to solve" << std::endl;
			}
		}
	}

	if(alarm_mode == 3 && active_alarm){
		
		found = false;

		if (getDistance(posit->getX(),posit->getY(),posit->getZ(),x_err,y_err,-90) == 0.0){
			found = true;
			x_s = x_err;
			y_s = y_err;
			
			if (debug_) 
				std::cout << NOW << " UwAUVCtrErrModule::InitPkt(Packet *p) SV" 
						<< "in range"<< std::endl;

		}else if(getDistance(x_sorg,y_sorg,-1,x_err,y_err,-90) < 
			getDistance(posit->getX(),posit->getY(),posit->getZ(),x_sorg,y_sorg,-1)){ 

			found = true;
			x_s = x_err;
			y_s = y_err;
			
			if (debug_) 
				std::cout << NOW << " UwAUVCtrErrModule::InitPkt(Packet *p) SV" 
						<< "in range (too far)"<< std::endl;

		}
		
		if (found){
			
			uwAUVh->error() = -2;
			alarm_mode = 0;
			uwAUVh->sn() = ++sn;
			uwAUVh->x() = x_s;
			uwAUVh->y() = y_s;
			this->p = p;
			active_alarm = false;

			posit->setDest(x_err,y_err,-1,speed);
			

			if (log_on_file == 1) {

				err_log.open("log/error_log_t.csv",std::ios_base::app);
				err_log << "R,"<< NOW << "," << x_s<<","<<y_s<<", OFF"<< std::endl;
				err_log.close();

			}

			if (debug_) {
				std::cout << NOW << " UwAUVCtrErModule::initPkt(Packet *p)  ERROR ("<< x_err 
					<< "," << y_err << ") SOLVED" << std::endl;
			}

			x_err = -4000;
			y_err = -4000;

		} 

	} 
	
	UwCbrModule::initPkt(p);

}

void UwAUVCtrErModule::recv(Packet* p) {
	
	hdr_uwAUV_error* uwAUVh = hdr_uwAUV_error::access(p);

	//obsolete packets
	if (drop_old_waypoints == 1 && uwAUVh->sn() <= last_sn_confirmed) { 
		if (debug_) {
			std::cout << NOW << " UwAUVCtrErrModule::old error with sn " 
				<< uwAUVh->sn() << " dropped " << std::endl;
		}
	} else { 
		if (uwAUVh->error() == 0){// AUV MARKED IT AS NO ERROR

			if (debug_)
				std::cout << NOW << " UwAUVCtrErrModule:: no error" << std::endl;

		} else { // error of some kind

			rcv_queue.push_back({uwAUVh->x(), uwAUVh->y()});


			//dev_status status = checkError(uwAUVh->error(),1,uwAUVh->x(), uwAUVh->y());
			int status = checkError(uwAUVh->error(),1,uwAUVh->x(), uwAUVh->y());
			
			bool exists = false;

			if (!gray_queue.empty()){

				int i=0;
				for (const auto& vec : gray_queue) {
					if (vec[0] == uwAUVh->x() && vec[1] == uwAUVh->y()) {
						exists = true;
						break;
					}
					i++;
				}

				if(exists){

					if (debug_)
						std::cout << NOW << " UwAUVCtrErModule::recv(Pakct p) gray_queue error"
							<< "value updated, old error("<< (gray_queue[i][2]/gray_queue[i][3]) << "),";

					gray_queue[i][2] += uwAUVh->error();
					gray_queue[i][3] += 1;

					status = checkError(gray_queue[i][2],gray_queue[i][3], uwAUVh->x(), uwAUVh->y());

					if (debug_)
						std::cout << " updated error("<< (gray_queue[i][2]/gray_queue[i][3]) 
							<< ")" << std::endl;
					
				}
			}

			if (status == 1){

				if(!exists){
					gray_queue.push_back({uwAUVh->x(), uwAUVh->y(), uwAUVh->error(),1});

					if (debug_){
						std::cout << NOW << " UwAUVCtrErrModule::recv(Pakct p) new error added" 
							<< "to gray_queue, error = " << uwAUVh->error() << std::endl;
						std::cout << NOW << " UwAUVCtrErrModule::recv(Pakct p) Next gray error"
							<< "= " << (gray_queue[0][2]/gray_queue[0][3]) << std::endl;
					}
				}


			} else if(status == 2){ //status 2

				//delete point in gray_queue
				int i = 0;
				if (!gray_queue.empty()){
					for (auto it = gray_queue.begin(); it != gray_queue.end(); ++it) {
						if ((*it)[0]== uwAUVh->x() &&  (*it)[1] == uwAUVh->y()) {
							gray_queue.erase(it);
							break;
						}
					}
				}

				if (!alarm_mode){

					//check in the queue before
					if (alarm_queue.empty()){

						x_err = uwAUVh->x();
						y_err = uwAUVh->y();
						x_sorg = posit->getX();
						y_sorg = posit->getY();

						posit->addDest(x_err,y_err,-1,speed);
						alarm_mode = 2;
						active_alarm = true;

						if (debug_) 
							std::cout << NOW << " UwAUVCtrErrModule::recv(Packet *p) SV" 
								<< "received new error(2): X = " << uwAUVh->x() << ", Y = " 
								<< uwAUVh->y() << ", error = " << uwAUVh->error() << std::endl;

						if (log_on_file == 1) {
							pos_log.open("log/position_log.csv",std::ios_base::app);
							pos_log << NOW << "," << posit->getX() << ","<< 
								posit->getY() << ","<< posit->getZ() << std::endl;
							pos_log.close();

							err_log.open("log/error_log_t.csv",std::ios_base::app);
							err_log << "R,"<< NOW << "," << x_err <<","<<y_err<<
								",ON"<< std::endl;
							err_log.close();
						}

					} else {

						exists=false;

						for (const auto& vec : rcv_queue) {
							//next error is associated to this app
							if (vec[0] == alarm_queue[0][0] && vec[1] == alarm_queue[0][1]) { 
								exists = true;
								break;
							}
						}

						if(exists){

							x_err = alarm_queue[0][0];
							y_err = alarm_queue[0][1];
							posit->addDest(x_err,y_err,-1,speed);
							alarm_mode = 2;
							active_alarm = true;
							
							x_sorg = posit->getX();
							y_sorg = posit->getY();

							alarm_queue.erase(alarm_queue.begin());

						} else {

							//alarm_mode = 4;
							//wait another app to take care of it
						}

						
						exists = false;
						for (const auto& vec : alarm_queue) {
							if (vec[0] == uwAUVh->x() && vec[1] == uwAUVh->y()) {
								exists = true;
								break;
							}
						}

						if (!exists){
							alarm_queue.push_back({uwAUVh->x(),uwAUVh->y()});
							if (log_on_file == 1) {
								err_log.open("log/error_log_t.csv",std::ios_base::app);
								err_log << "R,"<< NOW << "," << uwAUVh->x() <<","<< 
									uwAUVh->y() << ", ON"<< std::endl;
								err_log.close();
							}
						}
						
						

						if (log_on_file == 1) {

							pos_log.open("log/position_log.csv",std::ios_base::app);
							pos_log << NOW << "," << posit->getX() << ","<< 
								posit->getY() << ","<< posit->getZ() << std::endl;
							pos_log.close();

						}


						if (debug_) 
							std::cout << NOW << " UwAUVCtrErrModule::recv(Packet *p)" 
								<< "SV add new error(2) in the queue: X = " << uwAUVh->x()
								<< ", Y = " << uwAUVh->y() << ", error = " <<  
								uwAUVh->error() << std::endl;

					}

				} else {

					exists = false;

					for (const auto& vec : alarm_queue) {
						if (vec[0] == uwAUVh->x() && vec[1] == uwAUVh->y()) {
							exists = true;
							break;
						}
					}

					if (!exists){
						alarm_queue.push_back({uwAUVh->x(),uwAUVh->y()});
						if (log_on_file == 1) {
							err_log.open("log/error_log_t.csv",std::ios_base::app);
							err_log << "R,"<< NOW << "," << uwAUVh->x() <<","<<
								uwAUVh->y()<<", ON"<< std::endl;
							err_log.close();
						}
					}

					if (debug_) 
						std::cout << NOW << " UwAUVCtrErrModule::recv(Packet *p) SV" 
							<< "add new error(2) in the queue: X = " << uwAUVh->x() <<
							",Y = " << uwAUVh->y() << ", error = " <<  uwAUVh->error() 
							<< std::endl;
					
				}

			} else {

				int i = 0;
				for (auto it = gray_queue.begin(); it != gray_queue.end(); ++it) {
					if ((*it)[0]== uwAUVh->x() &&  (*it)[1] == uwAUVh->y()) {
						gray_queue.erase(it);
						break;
					}
				}

				exists = false;
				
				for (const auto& vec : rcv_queue) {
					//next error is associated to this app
					if (vec[0] == uwAUVh->x() && vec[1] == uwAUVh->y()) { 
						exists = true;
						break;
					}
				}

				if(exists){

					error_released = true;
					x_s = uwAUVh->x();
					y_s = uwAUVh->y();

					if (log_on_file == 1) {
							err_log.open("log/error_log_t.csv",std::ios_base::app);
							err_log << "G,"<< NOW << "," << x_s <<","<< y_s <<", OFF"
								<< std::endl;
							err_log.close();
					}
					
					if (debug_)
						std::cout << NOW << " UwAUVCtrErrModule:: After some tx" 
							<<"converged to no error" << std::endl;
				}

			}

		}

		last_sn_confirmed = uwAUVh->sn();
	}

	UwCbrModule::recv(p);
}

int UwAUVCtrErModule::checkError(double m, int n_pkt, float x, float y){
	
	// prob of true error (t_e) greater than th_ne
    double p_e = std::erfc((((th_ne - (m/n_pkt)) *  std::sqrt(n_pkt)) / std::sqrt(2.0)) / sigma)/2; 
	//dev_status status;
	int status;
	if (p_e < accuracy){ //if p_e is small enough --> no error 
		//status = no_error;
		status=0;
	} else if (p_e > (1-accuracy)){
		// FOR SURE ERROR
		//status = error;
		status = 2;
	} else {
		//I NEED MORE DATA
		//status = undetermined;
		status=1;
	}

	return status;
}

