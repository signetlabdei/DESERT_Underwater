//
// Copyright (c) 2017 Regents of the SIGNET lab, University of Padova.
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
// 3. Neither the name of the University of Padova (SIGNET \) nor the
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
 * @file   uwPosBasedRtROV.cpp
 * @author Alberto Signori
 * @version 1.0.0
 *
 * \brief Routing protocol for vehicles based on ROV position
 *
 */

#include "uwPosBasedRtROV.h"
#include <iostream>

extern packet_t PT_UWPOSITIONBASEDROUTING;


static class UwPosBasedRtROVModuleClass : public TclClass
{
public:
	UwPosBasedRtROVModuleClass()
		: TclClass("Module/UW/PosBasedRt/ROV")
	{
	}

	TclObject *
	create(int, const char *const *)
	{
		return (new UwPosBasedRtROV());
	}
} class_module_uwposbasedrt_rov;



UwPosBasedRtROV::UwPosBasedRtROV()
	: ipAddr(0)
	, maxTxRange(3000)
	, ROV_pos()
	, list_posIP()
{
	bind("debug_", &debug_);
	bind("maxTxRange_",(double *) &maxTxRange);
	if (maxTxRange <= 0) {
		std::cout << " UwPosBasedRtROV::maxTxRange value not valid, value set "
			<< " by default to 3000 m" << std::endl;
		maxTxRange = 3000;
	}
}

UwPosBasedRtROV::~UwPosBasedRtROV()
{
}



int UwPosBasedRtROV::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();

	if(argc == 3) {
		if (strcasecmp(argv[1],"setMaxTxRange") == 0) {
			if (atof(argv[2]) <= 0) {
				std::cerr << " UwPosBasedRtROV(IP=" <<(int)ipAddr 
					<< ")::invalid value for max transmission range, "
					<< "use a value >= 0";
				return TCL_ERROR;
			}
			setMaxTxRange(atof(argv[2]));
			return TCL_OK;
		}
		if (strcasecmp(argv[1], "addr") == 0) {
			ipAddr = static_cast<uint8_t>(atoi(argv[2]));
			if (ipAddr == 0) {
				std::cerr << "UwPosBasedRtROV::0 is not a valid IP address"
					<< std::endl;
				return TCL_ERROR;
			}
			return TCL_OK;
		}
		if (strcasecmp(argv[1],"setROVPosition") == 0) {
			UWSMPosition *p = dynamic_cast<UWSMPosition*> (tcl.lookup(argv[2])); 
			ROV_pos = p;
			std::cout << NOW << " UwPosBasedRtROV(IP=" <<(int)ipAddr 
				<< ")::ROV position, x: " <<ROV_pos->getX() << ", y: " 
				<< ROV_pos->getY()<< ", z: " <<ROV_pos->getZ() << std::endl;
			return TCL_OK;
		}
	} else if (argc == 4) {
		if (strcasecmp(argv[1], "addPosition_IPotherNodes") == 0) { 
			//add node position with its IP address
			Position *p = dynamic_cast<Position*> (tcl.lookup(argv[2]));
			uint8_t ip = static_cast<uint8_t>(atoi(argv[3]));
			if (ip == 0) {
				std::cerr << "UwPosBasedRtROV::0 is not a valid IP address"
					<< std::endl;
				return TCL_ERROR;
			}
			std::cout << NOW << " UwPosBasedRtROV(IP=" <<(int)ipAddr
				<< ")::add node position, x:" << p->getX() << " y: "<< p->getY()
				<< " z: " << p->getZ() << " with IP " << (int)ip << std::endl;
			pair_posIP provv = std::pair<Position,uint8_t>(*p,ip);
			list_posIP.push_back(provv);

			return TCL_OK;
		}
	}
	return Module::command(argc, argv);
}


void UwPosBasedRtROV::recv(Packet* p) 
{

	if (ipAddr == 0) {
		std::cout << NOW << " UwPosBasedRtROV(IP=" <<(int)ipAddr 
			<< ")::node IP addres not set, drop packet" << std::endl;
		drop(p, 1, DROP_IP_NOT_SET);
	}

	hdr_cmn *ch = HDR_CMN(p);
	hdr_uwip *iph = HDR_UWIP(p);

	if (ch->direction() == hdr_cmn::UP) {

		if (iph->daddr() == ipAddr) {//I'm the intended node

			sendUp(p);
			return;
		} else { //find next hop
			ch->next_hop() = (nsaddr_t)findNextHop(p);
			if (ch->next_hop() == 0) {
				std::cout << NOW << " UwPosBasedRtROV(IP=" <<(int)ipAddr 
					<< ")::next hop not found, drop packet" << std::endl;
				drop(p, 1, DROP_NEXT_HOP_NOT_FOUND);
			}
			if (debug_) {
				std::cout << NOW << " UwPosBasedRtROV(IP=" <<(int)ipAddr 
					<< ")::next hop: " << (int)ch->next_hop() << std::endl;
			}
			sendDown(p);
			return;
		}

	} else {//DOWN
		
		initPkt(p);

		ch->next_hop() = (nsaddr_t)findNextHop(p);
		if (ch->next_hop() == 0) {
			std::cout << NOW << " UwPosBasedRtROV(IP=" <<(int)ipAddr 
				<< ")::next hop not found, drop packet" << std::endl;
			drop(p, 1, DROP_NEXT_HOP_NOT_FOUND);
		}
		if (debug_) {
			std::cout << NOW << " UwPosBasedRtROV(IP=" <<(int)ipAddr 
				<< ")::next hop: " << (int)ch->next_hop() << std::endl;
		}
		sendDown(p);
		return;
	}
}

void UwPosBasedRtROV::initPkt(Packet* p)
{
	if (debug_) 
		std::cout << NOW << " UwPosBasedRtROV(IP=" <<(int)ipAddr 
			<< ")::initPkt, init header fields" << std::endl;
	
	hdr_uwpos_based_rt *pbrh = HDR_UWPOS_BASED_RT(p);

	
	pbrh->x_ROV() = ROV_pos->getX();
	pbrh->y_ROV() = ROV_pos->getY();
	pbrh->z_ROV() = ROV_pos->getZ();
	pbrh->x_waypoint() = ROV_pos->getXdest();
	pbrh->y_waypoint() = ROV_pos->getYdest();
	pbrh->z_waypoint() = ROV_pos->getZdest();
	pbrh->timestamp() = NOW;
	pbrh->ROV_speed() = ROV_pos->getSpeed();
	pbrh->IP_ROV() = ipAddr;

	if (debug_) {
		std::cout << NOW << " UwPosBasedRtROV(IP=" <<(int)ipAddr 
			<< ")::initPkt, init header fields" << std::endl;
		std::cout << NOW << " UwPosBasedRtROV(IP=" <<(int)ipAddr 
			<< ")::Inserted information"
			<< ", x waypoint: " << pbrh->x_waypoint() 
			<< " ,y waypoint: " << pbrh->y_waypoint() 
			<< " ,z waypoint: " << pbrh->z_waypoint() 
			<< " ,x ROV: " << pbrh->x_ROV() 
			<< " ,y ROV: " << pbrh->y_ROV()
			<< " ,z ROV: " << pbrh->z_ROV() 
			<< " ,timestamp: " << pbrh->timestamp() 
			<< " ,ROV speed: " << pbrh->ROV_speed() << std::endl;
	}
	
}


uint8_t UwPosBasedRtROV::findNextHop(const Packet* p)
{
	if (list_posIP.empty()) {
		std::cout << NOW << " UwPosBasedRtROV(IP=" <<(int)ipAddr 
			<< ")::List of position not setted" << std::endl;
		return 0;
	} else {
		double minDist = 10000000;
		double tempDist;
		uint8_t ipCloserNode;
		for (std::list<pair_posIP>::iterator it=list_posIP.begin(); 
										it != list_posIP.end(); ++it) {
			tempDist = nodesDistance(it->first, *ROV_pos);
			if (tempDist < minDist) {
				minDist = tempDist;
				ipCloserNode = it->second;
			}
		}
		if (minDist < maxTxRange) {
			if (debug_) 
				std::cout << NOW << " UwPosBasedRtROV(IP=" <<(int)ipAddr 
					<< ")::findNextHop,the closest node in the tx range " 
					<< "has IP equal to " << (int)ipCloserNode << std::endl;
			return ipCloserNode;
		} else {
			return (uint8_t)0;
		}
	}
}


double UwPosBasedRtROV::nodesDistance(Position& p1, Position& p2)
{
	double x1 = p1.getX();
	double y1 = p1.getY();
	double z1 = p1.getZ();
	double x2 = p2.getX();
	double y2 = p2.getY();
	double z2 = p2.getZ();

	return sqrt(pow(x2-x1,2) + pow(y2-y1,2) + pow(z2-z1,2));
}


void UwPosBasedRtROV::setMaxTxRange(double newRange)
{
	if (newRange == 0) {
		std::cout << NOW << " UwPosBasedRtROV(IP=" <<(int)ipAddr 
			<< ")::value for max transmission range not valid" << std::endl;
		return;
	}
	maxTxRange = newRange;
	if (debug_) 		
		std::cout << NOW << " UwPosBasedRtROV(IP=" <<(int)ipAddr 
			<< ")::max transmission range set to "  << maxTxRange << std::endl;
}
