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
* @file uwauverror-module.cc
* @author Alessia Ortile
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
	, drop_old_waypoints(1)
	, log_on_file(0)
	, period(60)
	, error_m(0)
	, alarm_mode(0)
	, speed(0.5)
	, accuracy(0.001)
{

	posit= new UWSMWPPosition();
	bind("drop_old_waypoints_", (int*) &drop_old_waypoints);
	bind("log_on_file_", (int*) &log_on_file );
	bind("period_", (int*) &period );
	bind("sigma_", (double*) &sigma );
	bind("th_ne_", (double*) &th_ne );
	bind("accuracy_ne_", (double*) &accuracy );

}

UwAUVErrorModule::UwAUVErrorModule(UWSMWPPosition* p) 
	: UwCbrModule()
	, last_sn_confirmed(0)
	, sn(0)
	, drop_old_waypoints(1)
	, log_on_file(0)
	, period(60)
	, error_m(0)
	, alarm_mode(0)
	, speed(0.5)
	, accuracy(0.001)
{
	posit = p;
	bind("drop_old_waypoints_", (int*) &drop_old_waypoints);
	bind("log_on_file_", (int*) &log_on_file );
	bind("period_", (int*) &period );
	bind("sigma_", (double*) &sigma);
	bind("th_ne_", (double*) &th_ne );
	bind("accuracy_ne_", (double*) &accuracy );

}

UwAUVErrorModule::~UwAUVErrorModule() {}

void UwAUVErrorModule::setPosition(UWSMWPPosition* p){
	posit = p;
}

int UwAUVErrorModule::command(int argc, const char*const* argv) {
	Tcl& tcl = Tcl::instance();
	if(argc == 2){
		if (strcasecmp(argv[1], "getAUVMonheadersize") == 0) {
			tcl.resultf("%d", getAUVMonHeaderSize());
			return TCL_OK;
		} else if(strcasecmp(argv[1], "getAUVctrheadersize") == 0) {
			tcl.resultf("%d", getAUVCTRHeaderSize());
			return TCL_OK;
		} else if(strcasecmp(argv[1], "getAUVErrorheadersize") == 0) {
			tcl.resultf("%d", getAUVErrorHeaderSize());
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

void UwAUVErrorModule::transmit() {
	sendPkt();

	if (debug_) {
		std::cout << NOW << " UwAUVErrorModule::Sending pkt with period: " << 
			period << std::endl;
	}
	sendTmr_.resched(period);
}

void UwAUVErrorModule::initPkt(Packet* p) {
		 
	hdr_uwcbr *uwcbrh = HDR_UWCBR(p);
	hdr_uwAUV_error* uwAUVh = hdr_uwAUV_error::access(p);

	uwAUVh->error() = 0;

	if (alarm_mode != 2 ){ //if not in alarm mode

		if (alarm_mode == 1){ //gray zone

			error_m = getErrorMeasure(t_e);

			if (debug_) { 
				std::cout << NOW << " UwAUVErroModule::initPkt(Packet *p) " 
					<< "Gray Zone error, new measure: "<< error_m <<", true error: "
					<< t_e << std::endl;
			}
			
		} else {

			error_m = getErrorMeasure();

			if (debug_) { 
				std::cout << NOW << " UwAUVErroModule::initPkt(Packet *p) " 
					<< "New error, measure: "<< error_m <<", true error: "<< t_e 
					<< std::endl;
			}

			if(alarm_mode == 1){ // start error timer
				
				if (log_on_file == 1) {

					err_log.open("log/error_log_t.csv",std::ios_base::app);
					err_log << "G,"<< NOW << "," << posit->getX() <<","<< 
						posit->getY() <<", ON"<< std::endl;
					err_log.close();

					err_log.open("log/error_log.csv",std::ios_base::app);
					err_log << "ON,"<< NOW << "," << x_e <<","<< y_e << std::endl;
					err_log.close();
				}

			}
		}

		if (alarm_mode == 1){

			x_e = posit->getX(); // Save error position
			y_e = posit->getY();

			//STOP
			posit->setDest(posit->getXdest(),posit->getYdest(),posit->getZdest(),0); 
			posit->setAlarm(true);
			
			uwAUVh->x() = x_e;                      
			uwAUVh->y() = y_e;
			uwAUVh->error() = error_m;
			this->p = p;	

			if (debug_) { 
				std::cout << NOW << " UwAUVErroModule::initPkt(Packet *p) " 
					<< "ERROR ("<< x_e <<","<< y_e << "," << error_m <<")," 
					<<"alarm_mode = "<< alarm_mode<<", true error= "<< t_e 
					<< std::endl;
			}

		} else {

			if (debug_) { 
				std::cout << NOW << " UwAUVErroModule::initPkt(Packet *p) " 
					<< "no error "<<" alarm_mode = " << alarm_mode << std::endl;
			}

		}

		uwAUVh->sn() = ++sn;
		
	} else {

		uwAUVh->x() = x_e;
		uwAUVh->y() = y_e;
		uwAUVh->error() = error_m;
		this->p = p;
		uwAUVh->sn() = ++sn;
	}

	

	UwCbrModule::initPkt(p);

	if (log_on_file == 1) {
			out_file_stats.open("log/position_log_a.csv",std::ios_base::app);
			out_file_stats << NOW << "," << posit->getX() << ","<< posit->getY() 
				<< "," << posit->getZ() << ", " << posit->getSpeed()<< std::endl;
			out_file_stats.close();
	}

}

void UwAUVErrorModule::recv(Packet* p) {

	hdr_uwAUV_error* uwAUVh = hdr_uwAUV_error::access(p);

	//obsolete packets
	if (drop_old_waypoints == 1 && uwAUVh->sn() <= last_sn_confirmed) { 
			if (debug_) {
				std::cout << NOW << " UwAUVErrModule::old error with sn " 
					<< uwAUVh->sn() << " dropped " << std::endl;
			}

		} else { //packet in order

		/** 
		 * error > 0 tx more data
		 * error > 1 stop tx, Ctr is coming
		 * error < 0 stop tx, there is no error
		*/

		//Valid pkt refering to my error
		if (alarm_mode && uwAUVh->x() == x_e && uwAUVh->y() == y_e){  

			if (uwAUVh->error() < 0 ){

				posit->setAlarm(false);
				posit->setDest(posit->getXdest(),posit->getYdest(),posit->getZdest(),speed);
				
				sendTmr_.force_cancel();
				sendTmr_.resched(period);

				if (log_on_file == 1) {

					err_log.open("log/error_log_t.csv",std::ios_base::app);
					err_log << "W,"<< NOW << "," << x_e <<","<< y_e <<
						",ON"<< std::endl;
					err_log.close();

					err_log.open("log/error_log.csv",std::ios_base::app);
					err_log << "OFF,"<< NOW << "," << x_e <<","<< y_e 
						<< std::endl;
					err_log.close();
					
					if (uwAUVh->error() == -1){
						
						if(t_e <= th_ne){

							t_err_log.open("log/true_error_log.csv",std::ios_base::app);
							t_err_log << NOW << "," << x_e <<","<< y_e <<",tn" 
								<< std::endl;
							t_err_log.close();

						} else {

							t_err_log.open("log/true_error_log.csv",std::ios_base::app);
							t_err_log << NOW << "," << x_e <<","<< y_e <<",fn" 
								<< std::endl;
							t_err_log.close();

						}
					}
					
				}

				alarm_mode = 0;
				

				if (debug_) {
					std::cout << NOW << " UwAUVErrModule::recv(Packet *p) error("
						<< x_e <<","<< y_e <<") solved AUV can move again" 
						<< "with speed=" << posit->getSpeed()<< std::endl;
				}

			} else if (uwAUVh->error() >= 1 ){

				alarm_mode = 2;

				if (log_on_file == 1) {

					if(t_e > th_ne){
						t_err_log.open("log/true_error_log.csv",std::ios_base::app);
						t_err_log << NOW << "," << x_e <<","<< y_e <<",tp" 
							<< std::endl;
						t_err_log.close();

					} else {

						t_err_log.open("log/true_error_log.csv",std::ios_base::app);
						t_err_log << NOW << "," << x_e <<","<< y_e <<",fp" 
							<< std::endl;
						t_err_log.close();

					}

				}

				if (debug_)
					std::cout << NOW << " UwAUVErrModule::recv(Packet *p) for"
						<< "SURE there is an error ("<< x_e <<","<< y_e <<")"
						<< "STOP until ctr arrival"<< std::endl;

			} else {// ->error=0

				alarm_mode = 1;

				if (debug_)
					std::cout << NOW << " UwAUVErrModule::recv(Packet *p)"
						<< "MAYBE there is an error ("<< x_e <<","<< y_e <<")"<<
						"continue tx"<< std::endl;

			}
		}

			last_sn_confirmed = uwAUVh->sn();
	}

	UwCbrModule::recv(p);
		
	if (log_on_file == 1) {
		out_file_stats.open("log/position_log_a.csv",std::ios_base::app);
		out_file_stats << NOW << "," << posit->getX() << ","<< posit->getY() 
			<< "," << posit->getZ() << ',' << posit->getSpeed() << std::endl;
		out_file_stats.close();
	}

}

double UwAUVErrorModule::getErrorMeasure(){

	std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> u_dis(0.0, 1.0);

    // Generate a random value from the uniform distribution
    t_e = u_dis(gen);

	std::normal_distribution<> n_dis(0.0, sigma); 

    // Generate a random value from the Gaussian distribution
    double noise = n_dis(gen);
	double m = t_e + noise;

	// Calculate the error probability
	// prob of true error (t_e) greater than th_ne
    double p_e = std::erfc(((th_ne - m) / std::sqrt(2.0)) / sigma)/2;
	//if p_e is small enough --> no error, otherwise gray zone 
	if (p_e > accuracy){ 
		alarm_mode = 1;
	} else {
		if (t_e > th_ne){
			if (log_on_file == 1) {
				t_err_log.open("log/true_error_log.csv",std::ios_base::app);
				t_err_log << NOW << "," << x_e <<","<< y_e <<",fn"<< std::endl;
				t_err_log.close();
			}
		}
	}

	if (t_e > th_ne){

		if (log_on_file == 1) {
			t_err_log.open("log/true_error_log.csv",std::ios_base::app);
			t_err_log << NOW << "," << x_e <<","<< y_e <<",e"<< std::endl;
			t_err_log.close();
		}
	}
	

	return m;
	

}


double UwAUVErrorModule::getErrorMeasure(double t_e){

	std::random_device rd;
    std::mt19937 gen(rd());

	std::normal_distribution<> n_dis(0.0, sigma);  
    // Generate a random value from the Gaussian distribution
    double noise = n_dis(gen);

	double m = t_e + noise;

	return m;	

}



