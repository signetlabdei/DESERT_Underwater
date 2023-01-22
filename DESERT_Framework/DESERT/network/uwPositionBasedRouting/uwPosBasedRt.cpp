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
 * @file   uwPosBasedRt.cpp
 * @author Alberto Signori
 * @version 1.0.0
 *
 * \brief Routing protocol for static node based on ROV position
 *
 */

#include "uwPosBasedRt.h"
#include <iostream>

extern packet_t PT_UWPOSITIONBASEDROUTING;



static class UwPosBasedRtModuleClass : public TclClass
{
public:
	UwPosBasedRtModuleClass()
		: TclClass("Module/UW/PosBasedRt")
	{
	}

	TclObject *
	create(int, const char *const *)
	{
		return (new UwPosBasedRt());
	}
} class_module_uwposbasedrt;



UwPosBasedRt::UwPosBasedRt()
	: ipAddr(0)
	, timestamp(0)
	, ROV_speed(1)
	, maxTxRange(3000)
	, node_pos()
{
	static_routing.clear();
	bind("debug_", &debug_);
	bind("maxTxRange_",(double *) &maxTxRange);
	bind("ROV_speed_", (double *) &ROV_speed);
	if (maxTxRange <= 0) {
		std::cout << " UwPosBasedRt::maxTxRange value not valid, value set by "
					<< "default to 3000 m" << std::endl;
		maxTxRange = 3000;
	}
	if (ROV_speed <= 0) {
		std::cout << " UwPosBasedRt::ROV_speed value not valid, value set by "
					<< "defaul to 1 m/s" << std::endl;
		ROV_speed = 1;
	}
}

UwPosBasedRt::~UwPosBasedRt()
{
}



int UwPosBasedRt::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();

	if(argc == 3) {
		if (strcasecmp(argv[1],"setMaxTxRange") == 0) {
			if (atof(argv[2]) <= 0) {
				std::cerr << " UwPosBasedRt(IP=" <<(int)ipAddr << ")::invalid "
					<< "value for max transmission range, use a value >= 0";
				return TCL_ERROR;
			}
			setMaxTxRange(atof(argv[2]));
			return TCL_OK;
		}
		if (strcasecmp(argv[1], "addr") == 0) {
			ipAddr = static_cast<uint8_t>(atoi(argv[2]));
			if (ipAddr == 0) {
				std::cerr << "UwPosBasedRt::0 is not a valid IP address"
					<< std::endl;
				return TCL_ERROR;
			}
			return TCL_OK;
		}
		if (strcasecmp(argv[1],"setNodePosition") == 0) {
			Position *p = dynamic_cast<Position*> (tcl.lookup(argv[2]));
			node_pos = *p;
			std::cout << NOW << " UwPosBasedRt(IP=" <<(int)ipAddr 
					<< ")::node position, x: " <<node_pos.getX() << ", y: " 
					<< node_pos.getY()<< ", z: " <<node_pos.getZ() << std::endl;
			return TCL_OK;
		}
	} else if (argc == 5) {
		if (strcasecmp(argv[1], "addRoute") == 0) {
			if (strcasecmp(argv[4], "toMovingNode") == 0) {
				addRoute(static_cast<uint8_t>(atoi(argv[2])),
						static_cast<uint8_t>(atoi(argv[3])),0);
				return TCL_OK;				
			} else if (strcasecmp(argv[4], "toFixedNode") == 0) {
				addRoute(static_cast<uint8_t>(atoi(argv[2])),
						static_cast<uint8_t>(atoi(argv[3])),1);
				return TCL_OK;	
			} else {
				std::cerr << "UwPosBasedRt::invalid command, set "
						<< "\"toFixedNode\" or \"toMovingNode\" "<< std::endl;
				return TCL_ERROR;
			}
		}
	}
	return Module::command(argc, argv);
}


void UwPosBasedRt::recv(Packet* p) 
{
	//packet arrives only if i'm the intended node, the next hop or the source
	if (ipAddr == 0) {
		std::cout << NOW << " UwPosBasedRt(IP=" <<(int)ipAddr 
				<< ")::node IP addres not set, drop packet" << std::endl;
		drop(p, 1, DROP_IP_NOT_SET);
	}

	hdr_cmn *ch = HDR_CMN(p);
	hdr_uwip *iph = HDR_UWIP(p);

	if (ch->direction() == hdr_cmn::UP) {

		updatePosInfo(p);

		if (iph->daddr() == ipAddr) //I'm the intended node
		{
			if (debug_) 
				std::cout << NOW << " UwPosBasedRt(IP=" 
					<<(int)ipAddr << ")::packet for me, send up" << std::endl;
			
			sendUp(p);
			return;
		} else { //find next hop
			ch->next_hop() = (nsaddr_t)findNextHop(p);
			if (ch->next_hop() == 0) {
				std::cout << NOW << " UwPosBasedRt(IP=" <<(int)ipAddr 
					<< ")::next hop not found, drop packet" << std::endl;
				drop(p, 1, DROP_NEXT_HOP_NOT_FOUND);
			}
			if (debug_) {
				std::cout << NOW << " UwPosBasedRt(IP=" <<(int)ipAddr 
					<< ")::next hop: " << (int)ch->next_hop() << std::endl;
			}
			sendDown(p);
			return;
		}

	} else {//DOWN
		
		initPkt(p);
		ch->next_hop() = (nsaddr_t)findNextHop(p);
		if (ch->next_hop() == 0) {
			if (debug_) 
				std::cout << NOW << " UwPosBasedRt(IP=" <<(int)ipAddr 
					<< ")::next hop not found, drop packet" << std::endl;
			
			drop(p, 1, DROP_NEXT_HOP_NOT_FOUND);
		}
		if (debug_) {
			std::cout << NOW << " UwPosBasedRt(IP=" <<(int)ipAddr 
				<< ")::next hop: " << (int)ch->next_hop() << std::endl;
		}
		sendDown(p);
		return;
	}
}

void UwPosBasedRt::initPkt(Packet* p)
{
	
	hdr_uwpos_based_rt *pbrh = HDR_UWPOS_BASED_RT(p);
	hdr_uwip *iph = HDR_UWIP(p);

	//If iph->daddr() is an address in the ROV_routing table, i.e., 
	//destination is a vehicles, initialize packet with information about ROV

	std::map<uint8_t, UwPosEstimation>::iterator itROV = 
												ROV_routing.find(iph->daddr());
	if (itROV != ROV_routing.end()) {

		Position tempLastROVpos = (itROV->second).getInitPos();
		Position tempLastWp = (itROV->second).getDest();


		pbrh->x_ROV() = tempLastROVpos.getX();
		pbrh->y_ROV() = tempLastROVpos.getY();
		pbrh->z_ROV() = tempLastROVpos.getZ();
		pbrh->x_waypoint() = tempLastWp.getX();
		pbrh->y_waypoint() = tempLastWp.getY();
		pbrh->z_waypoint() = tempLastWp.getZ();
		pbrh->timestamp() = (itROV->second).getTimestamp();
		pbrh->ROV_speed() = (itROV->second).getSpeed();
		pbrh->IP_ROV() = iph->daddr();

		if (debug_) 
			std::cout << NOW << " UwPosBasedRt(IP=" <<(int)ipAddr 
				<< ")::information inserted" 
				<< ", x waypoint: " << pbrh->x_waypoint() 
				<< " ,y waypoint: " << pbrh->y_waypoint() 
				<< " ,z waypoint: " << pbrh->z_waypoint() 
				<< " ,x ROV: " << pbrh->x_ROV() 
				<< " ,y ROV: " << pbrh->y_ROV()
				<< " ,z ROV: " << pbrh->z_ROV() 
				<< " ,timestamp: " << pbrh->timestamp() 
				<< " ,ROV speed: " << pbrh->ROV_speed() 
				<< " ,ROV IP: " << (int)pbrh->IP_ROV() << std::endl;
	} else {
		if (debug_) 
			std::cout << NOW << " UwPosBasedRt(IP=" <<(int)ipAddr 
				<< ")::initPacket, dest addr not found " 
				<< " in ROV routing table" << std::endl;
		pbrh->IP_ROV() = 0;
	}

}

void UwPosBasedRt::updatePosInfo(Packet* p)
{


	hdr_uwpos_based_rt *pbrh = HDR_UWPOS_BASED_RT(p);
	
	std::map<uint8_t, UwPosEstimation>::iterator itROV = 
											ROV_routing.find(pbrh->IP_ROV());
	if (itROV != ROV_routing.end()) { //there is an entry with this IP ROV

		if ((itROV->second).getTimestamp() < pbrh->timestamp()) { 
			//info in the packet is more recent
			Position tempLastROVpos;
			tempLastROVpos.setX(pbrh->x_ROV());
			tempLastROVpos.setY(pbrh->y_ROV());
			tempLastROVpos.setZ(pbrh->z_ROV());
			Position tempLastWp;
			tempLastWp.setX(pbrh->x_waypoint());
			tempLastWp.setY(pbrh->y_waypoint());
			tempLastWp.setZ(pbrh->z_waypoint());
			(itROV->second).update(tempLastROVpos,tempLastWp,
									pbrh->timestamp(),pbrh->ROV_speed());

			Position provvInitPos = (itROV->second).getInitPos();
			Position provvDest = (itROV->second).getDest();
			if (debug_) {
				std::cout << NOW << " UwPosBasedRt(IP=" <<(int)ipAddr 
					<< ")::update node info" << std::endl;
				std::cout << NOW << " UwPosBasedRt(IP=" <<(int)ipAddr 
					<< ")::information setted in the node" 
					<< ", x ROV: " << provvInitPos.getX() 
					<< " ,y ROV: " << provvInitPos.getY() 
					<< " ,z ROV: " << provvInitPos.getZ()
					<< " ,x wp: " << provvDest.getX()
					<< " ,y wp: " << provvDest.getY()
					<< " ,z wp: " << provvDest.getZ()
					<< " ,timestamp: " << (itROV->second).getTimestamp()
					<< " ,ROV speed: " << (itROV->second).getSpeed() 
					<< std::endl;
			}
		} else { //info in the node is more recent
			
			Position tempLastROVpos = (itROV->second).getInitPos();
			Position tempLastWp = (itROV->second).getDest();
			pbrh->x_ROV() = tempLastROVpos.getX();
			pbrh->y_ROV() = tempLastROVpos.getY();
			pbrh->z_ROV() = tempLastROVpos.getZ();
			pbrh->x_waypoint() = tempLastWp.getX();
			pbrh->y_waypoint() = tempLastWp.getY();
			pbrh->z_waypoint() = tempLastWp.getZ();
			pbrh->timestamp() = (itROV->second).getTimestamp();
			pbrh->ROV_speed() = (itROV->second).getSpeed();

			if (debug_) {
				std::cout << NOW << " UwPosBasedRt(IP=" <<(int)ipAddr 
					<< ")::update packet info" << std::endl;
				std::cout << NOW << " UwPosBasedRt(IP=" <<(int)ipAddr 
					<< ")::information setted in the packet" 
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
	}


}

uint8_t UwPosBasedRt::findNextHop(const Packet* p)
{
	hdr_uwip *iph = HDR_UWIP(p);

	std::map<uint8_t, UwPosEstimation>::iterator itROV = 
												ROV_routing.find(iph->daddr());
	if (itROV != ROV_routing.end()) {
		Position tempEstim = (itROV->second).getEstimatePos(NOW);

		if (debug_) 
			std::cout << NOW << " UwPosBasedRt(IP=" <<(int)ipAddr 
				<< ")::estimated ROV position, x ROV: " << tempEstim.getX() 
				<< " ,y ROV: " << tempEstim.getY() 
				<< " ,z ROV: " << tempEstim.getZ() << std::endl;

		double dist = nodesDistance(tempEstim,node_pos);
		if (debug_) 
			std::cout << NOW << " UwPosBasedRt(IP=" <<(int)ipAddr << ")::" << 
				"estimated distance between node and ROV " << dist << std::endl;
		
		if (dist < maxTxRange) {
			if (debug_) 
				std::cout << NOW << " UwPosBasedRt(IP=" <<(int)ipAddr 
					<< ")::forward directly to ROV" <<std::endl;
			return iph->daddr();
		} else {
			if (debug_) 
				std::cout << NOW << " UwPosBasedRt(IP=" <<(int)ipAddr 
					<< ")::ROV is not in the tx range" <<std::endl;
		}
	}

	//ROV is not in the tx range or packet is intendet for a static node
	std::map<uint8_t, uint8_t>::const_iterator it = 
											static_routing.find(iph->daddr());
	if (it != static_routing.end()) {
		if (debug_) 
			std::cout << NOW << " UwPosBasedRt(IP=" <<(int)ipAddr 
				<< ")::routing table, entry found (dest,next hop)=("
				<< (int)it->first<<"," << (int)it->second<<")" << std::endl;
		return it->second; 
	} else {
		return 0;
	}


}


double UwPosBasedRt::nodesDistance(Position& p1, Position& p2)
{
	double x1 = p1.getX();
	double y1 = p1.getY();
	double z1 = p1.getZ();
	double x2 = p2.getX();
	double y2 = p2.getY();
	double z2 = p2.getZ();

	return sqrt(pow(x2-x1,2) + pow(y2-y1,2) + pow(z2-z1,2));
}

void UwPosBasedRt::addRoute(
		const uint8_t &dst, const uint8_t &next, const int toFixedNode)
{
	if (dst == 0) {
		std::cout << NOW << " UwPosBasedRt(IP=" <<(int)ipAddr 
			<< ")::invalid IP address equal to 0" << std::endl;
		return;
	}
	if (toFixedNode == 0) {
		
		std::map<uint8_t, UwPosEstimation>::iterator itROV = 
														ROV_routing.find(dst);
		if (itROV != ROV_routing.end()) {
			if (debug_) 
				std::cout << NOW << " UwPosBasedRt(IP=" <<(int)ipAddr 
					<< ")::entry for ROV routing yet present" << std::endl;

		} else {
			if (debug_) 
				std::cout << NOW << " UwPosBasedRt(IP=" <<(int)ipAddr 
					<< ")::inserted entry for ROV routing" << std::endl;
			UwPosEstimation tempEstimateROVPos;
			ROV_routing.insert(std::pair<uint8_t,
								UwPosEstimation>(dst,tempEstimateROVPos));
		}

	} 
	std::map<uint8_t, uint8_t>::iterator it = static_routing.find(dst);
	if (it != static_routing.end()) { //entry alredy exist
		it->second = next;
		return;
	} else {
		static_routing.insert(std::pair<uint8_t,uint8_t>(dst,next));
	}
	std::cout << NOW << " UwPosBasedRt(IP=" <<(int)ipAddr 
		<< ")::addRoute, inserted entry (dest=" << (int)dst 
		<< ",next="<< (int)next <<")" <<std::endl;

	
}


void UwPosBasedRt::setMaxTxRange(double newRange)
{
	if (newRange == 0) {
		std::cout << NOW << " UwPosBasedRt(IP=" <<(int)ipAddr 
			<< ")::value for max transmission range not valid" << std::endl;
		return;
	}
	maxTxRange = newRange;
	if (debug_) 		
		std::cout << NOW << " UwPosBasedRt(IP=" <<(int)ipAddr 
			<< ")::max transmission range set to " << maxTxRange << std::endl;
}
