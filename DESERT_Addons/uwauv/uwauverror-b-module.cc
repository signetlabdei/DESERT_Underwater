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

#include "uwauverror-b-module.h"
#include <iostream>
#include <rng.h>
#include <random>
#include <stdint.h>
extern packet_t PT_UWCBR;
extern packet_t PT_UWAUV;
extern packet_t PT_UWAUV_CTR;
extern packet_t PT_UWAUV_ERROR;
/**
* Adds the header for <i>hdr_uwAUVError</i> packets in ns2.
*/
/**
* Adds the module for UwAUVModuleClass in ns2.
*/

/**
 * Class that represents the binding with the tcl configuration script 
 */
static class UwAUVErrorBModuleClass : public TclClass {
public: 

	/**
   * Constructor of the class
   */
	UwAUVErrorBModuleClass() : TclClass("Module/UW/AUV/ERB") {
	}

	/**
   * Creates the TCL object needed for the tcl language interpretation
   * @return Pointer to an TclObject
   */
	TclObject* create(int, const char*const*) {
		return (new UwAUVErrorBModule());
	}
} class_module_uwAUV;



UwAUVErrorBModule::UwAUVErrorBModule() 
	: UwCbrModule()
	, last_sn_confirmed(0)
	, sn(0)
	, ack(0)
	, ackPolicy(ACK_PIGGYBACK)
	, ackTimeout(10)
	, ackNotPgbk(0)
	, drop_old_waypoints(1)
	, log_flag(0)
	, period(60)
	, error_p(0.001)
	, alarm_mode(0)
	, speed(1)
	, accuracy(0.01)
{
	UWSMEPosition p = UWSMEPosition();
	posit=&p;
    bind("ackTimeout_", (int*) &ackTimeout);
    bind("drop_old_waypoints_", (int*) &drop_old_waypoints);
    bind("log_flag_", (int*) &log_flag );
	bind("period_", (int*) &period );
	bind("error_p_", (int*) &error_p );
	bind("sigma_", (double*) &sigma);
	bind("accuracy_",(double*) &accuracy);
    if (ackTimeout < 0) {
    	cerr << NOW << " Invalide ACK timeout < 0, timeout set to 10 by defaults"
    		<< std::endl;
    	ackTimeout = 10;
    }


}

UwAUVErrorBModule::UwAUVErrorBModule(UWSMEPosition* p) 
	: UwCbrModule()
	, last_sn_confirmed(0)
	, sn(0)
	, ack(0)
	, ackPolicy(ACK_PIGGYBACK)
	, ackTimeout(10)
	, ackNotPgbk(0)
	, drop_old_waypoints(1)
	, log_flag(0)
	, period(60)
	, error_p(0.01)
	, alarm_mode(0)
	, speed(1)	
	, accuracy(0.01)
{
	posit = p;
    bind("ackTimeout_", (int*) &ackTimeout);
    bind("drop_old_waypoints_", (int*) &drop_old_waypoints);
    bind("log_flag_", (int*) &log_flag );
	bind("period_", (int*) &period );
	bind("error_p_", (int*) &error_p );
	bind("sigma_", (double*) &sigma);
	bind("accuracy_",(double*) &accuracy);

    if (ackTimeout < 0) {
    	cerr << NOW << " Invalide ACK timeout < 0, timeout set to 10 by defaults"
    		<< std::endl;
    	ackTimeout = 10;
    }
	


}

UwAUVErrorBModule::~UwAUVErrorBModule() {}

void UwAUVErrorBModule::setPosition(UWSMEPosition* p){
	posit = p;
}

int UwAUVErrorBModule::command(int argc, const char*const* argv) {
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
			UWSMEPosition* p = dynamic_cast<UWSMEPosition*> (tcl.lookup(argv[2]));
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
		}else if (strcasecmp(argv[1], "adddest") == 0) {
			posit->adddest(atof(argv[2]),atof(argv[3]),atof(argv[4]));
			return TCL_OK;
		}
	}
	else if(argc == 6){
	if (strcasecmp(argv[1], "setdest") == 0) {
		posit->setdest(atof(argv[2]),atof(argv[3]),atof(argv[4]),atof(argv[5]));
		return TCL_OK;
		}else if (strcasecmp(argv[1], "adddest") == 0) {
		posit->adddest(atof(argv[2]),atof(argv[3]),atof(argv[4]),atof(argv[5]));
		return TCL_OK;
		}

	}
	return UwCbrModule::command(argc,argv);
}

void UwAUVErrorBModule::transmit() {
	sendPkt();

	if (debug_) {
		std::cout << NOW << " UwAUVErrorBModule::Sending pkt with period:  " << period 
			<< std::endl;
	}

	
	sendTmr_.resched(period);
}

void UwAUVErrorBModule::initPkt(Packet* p) {
		
	hdr_uwcbr *uwcbrh = HDR_UWCBR(p);
	hdr_uwAUV_error* uwAUVh = HDR_UWAUV_ERROR(p);

	uwAUVh->ack() = ack;
	ack = 0;
	uwAUVh->error() = 0;

	if (!alarm_mode){

		std::random_device rd;
		std::mt19937 generator(rd());
		std::uniform_real_distribution<double> distrib(0.0, 1.0);

		double t_e = distrib(generator) ;

		std::normal_distribution<> n_dis(0.0, sigma);  // Adjust the standard deviation using the variance

		// Generate a random value from the Gaussian distribution
		double noise = n_dis(generator);

		double m = t_e + noise;

		double p_e = std::erfc((((1 - error_p) - m) / std::sqrt(2.0)) / sigma); // prob of true error (t_e) greater than th_ne

		if (t_e > (1-error_p)){
			if (log_flag == 1) {
				t_err_log.open("log/true_error_log.csv",std::ios_base::app);
				t_err_log << NOW << "," << x_e <<","<< y_e <<",e" << std::endl;
				t_err_log.close();
			}
		}
	
		if (p_e > accuracy){ //if p_e is small enough --> no error, otherwise gray zone

			x_e = posit->getX();
			y_e = posit->getY();

			posit->setdest(posit->getXdest(),posit->getYdest(),posit->getZdest(),0);
			posit->setAlarm(true);
			alarm_mode = true;

			uwAUVh->x() = x_e;
			uwAUVh->y() = y_e;
			uwAUVh->error() = 1;
			uwAUVh->sn() = ++sn;

			if (log_flag == 1) {
					err_log.open("log/error_log.csv",std::ios_base::app);
					err_log << "ON,"<< NOW << "," << x_e <<","<< y_e << std::endl;
					err_log.close();
			}

			if (log_flag == 1) {
				if(t_e > (1-error_p)){
					t_err_log.open("log/true_error_log.csv",std::ios_base::app);
					t_err_log << NOW << "," << x_e <<","<< y_e <<",tp" << std::endl;
					t_err_log.close();
				}else{
					t_err_log.open("log/true_error_log.csv",std::ios_base::app);
					t_err_log << NOW << "," << x_e <<","<< y_e <<",fp" << std::endl;
					t_err_log.close();
				}
			}
		}else{

			if (log_flag == 1) {
				if(t_e <= (1-error_p)){
					t_err_log.open("log/true_error_log.csv",std::ios_base::app);
					t_err_log << NOW << "," << x_e <<","<< y_e <<",tn" << std::endl;
					t_err_log.close();
				}else{
					t_err_log.open("log/true_error_log.csv",std::ios_base::app);
					t_err_log << NOW << "," << x_e <<","<< y_e <<",fn" << std::endl;
					t_err_log.close();
				}
			}
		}
		

	}else{

		uwAUVh->x() = x_e;                      
		uwAUVh->y() = y_e;
		uwAUVh->error() = 1;
		this->p = p;
		uwAUVh->sn() = sn;
	}

	

	UwCbrModule::initPkt(p);

	if (log_flag == 1) {
		out_file_stats.open("log/position_log_a.csv",std::ios_base::app);
		out_file_stats << NOW << "," << posit->getX() << ","<< posit->getY() 
			<< "," << posit->getZ() << std::endl;
		out_file_stats.close();
	}

}

void UwAUVErrorBModule::recv(Packet* p, Handler* h) {
	recv(p);
}

void UwAUVErrorBModule::recv(Packet* p) {

	hdr_uwAUV_error* uwAUVh = HDR_UWAUV_ERROR(p);

	if(uwAUVh->ack() == sn + 1) {
		this->p = NULL;	
	}
	
	if (drop_old_waypoints == 1 && uwAUVh->sn() <= last_sn_confirmed) { //obsolete packets

		if (debug_) {
			std::cout << NOW << " UwAUVErrModule::old error with sn " 
				<< uwAUVh->sn() << " dropped " << std::endl;
		}

	} else { //packet in order

		if (alarm_mode && uwAUVh->x() == x_e && uwAUVh->y() == y_e){  //Valid pkt refering to my error

			if (uwAUVh->error() < 0 ){

				posit->setAlarm(false);
				alarm_mode = 0;
				posit->setdest(posit->getXdest(),posit->getYdest(),posit->getZdest(),speed);
				sendTmr_.force_cancel();
				sendTmr_.resched(period);

				if (log_flag == 1) {
					err_log.open("log/error_log.csv",std::ios_base::app);
					err_log << "OFF,"<< NOW << "," << x_e <<","<< y_e << std::endl;
					err_log.close();
				}

				if (debug_) {
					std::cout << NOW << " UwAUVErrModule::recv(Packet *p) error ("<< x_e <<","<< y_e <<") solved "
					"AUV can move again with speed=" << posit->getSpeed()<< std::endl;
				}

			} else if (uwAUVh->error() == 1 ){

				alarm_mode = true;

				if (debug_)
					std::cout << NOW << " UwAUVErrModule::recv(Packet *p) for SURE there is an error ("<< x_e <<","<< y_e <<")"
					"STOP until ctr arrival"<< std::endl;

			}
		}

		last_sn_confirmed = uwAUVh->sn();
	}

	ack = last_sn_confirmed+1;

	UwCbrModule::recv(p);
	UwCbrModule::sendPkt();
	
	if(uwAUVh->ack() > 0 && debug_) 
		std::cout << NOW << " UwAUVErrorBModule::recv(Packet *p) error ACK "
			<< "received " << uwAUVh->ack()<< std::endl;
	else if((uwAUVh->ack())<0 && debug_)
		std::cout << NOW << " UwAUVErrorBModule::recv(Packet *p) error NACK " 
			<<"received"<< std::endl;
		
	if (log_flag == 1) {
		out_file_stats.open("log/position_log_a.csv",std::ios_base::app);
		out_file_stats << NOW << "," << posit->getX() << ","<< posit->getY() 
			<< "," << posit->getZ() << std::endl;
		out_file_stats.close();
	}

}
