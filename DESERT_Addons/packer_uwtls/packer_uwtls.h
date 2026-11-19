//
// Copyright (c) 2026 Regents of the SIGNET lab, University of Padova.
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
// 3. Neither the name of the University of Padova (SIGNET lab) nor the
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

/*
 * File:   packer_uwtls.h
 * Author: Davide Costa
 */

#ifndef PACKER_UWTLS_H
#define PACKER_UWTLS_H

#include "packer.h"

#include "tls-record-header.h"
#include "handshake-header.h"
#include "clienthello-header.h"
#include "serverhello-header.h"
#include "clientkeyexchange-header.h"
#include "changecipherspec-header.h"

#define TLS_HANDSHAKE         22
#define TLS_CHANGECIPHERSPEC  20

#define CLIENT_HELLO          1
#define SERVER_HELLO          2
#define CLIENT_KEY_EXCHANGE   16


/**
 * Class to map a ns2 mac header into a bit stream, and vice-versa.
 */
class packer_uwtls : public packer
{
public:
	/**
	 * Constructor packer_uwtls class
	 *
	 */
	packer_uwtls();

	/**
	 * Destructor packer_uwtls class
	 *
	 */
	~packer_uwtls();

private:
	/**
	 * Init the packer for uwtls protocol
	 */
	void init();

	/**
	 *  Method to transform the headers of TLS protocol into a stream of bits
	 *
	 * @param Pointer to the packet to serialize
	 * @param Pointer to the buffer
	 * @param Offset from the begin of the buffer
	 *
	 * @return  size_t New offset after packing the headers of the packets
	 */
	size_t packMyHdr(Packet *p, unsigned char *buffer, size_t offset);

	/**
	 *  Method to transform the TLS Record
	 *
	 * @param Pointer to the packet to serialize
	 * @param Pointer to the buffer
	 * @param Offset from the begin of the buffer
	 *
	 * @return  size_t New offset after packing the headers of the packets
	 */
	size_t packRecord(Packet *p, unsigned char *buffer, size_t offset);

	/**
	 *  Method to transform the TLS Handshake
	 *
	 * @param Pointer to the packet to serialize
	 * @param Pointer to the buffer
	 * @param Offset from the begin of the buffer
	 *
	 * @return  size_t New offset after packing the headers of the packets
	 */
	size_t packHandshake(Packet *p, unsigned char *buffer, size_t offset);

	/**
	 *  Method to transform the TLS ClientHello
	 *
	 * @param Pointer to the packet to serialize
	 * @param Pointer to the buffer
	 * @param Offset from the begin of the buffer
	 *
	 * @return  size_t New offset after packing the headers of the packets
	 */
	size_t packClientHello(Packet *p, unsigned char *buffer, size_t offset);

	/**
	 *  Method to transform the TLS ServerHello
	 *
	 * @param Pointer to the packet to serialize
	 * @param Pointer to the buffer
	 * @param Offset from the begin of the buffer
	 *
	 * @return  size_t New offset after packing the headers of the packets
	 */
	size_t packServerHello(Packet *p, unsigned char *buffer, size_t offset);

	/**
	 *  Method to transform the TLS ClientKeyExchange
	 *
	 * @param Pointer to the packet to serialize
	 * @param Pointer to the buffer
	 * @param Offset from the begin of the buffer
	 *
	 * @return  size_t New offset after packing the headers of the packets
	 */
	size_t packClientKeyExchange(Packet *p, unsigned char *buffer, size_t offset);

	/**
	 *  Method to transform the TLS ChangeCipherSpec
	 *
	 * @param Pointer to the packet to serialize
	 * @param Pointer to the buffer
	 * @param Offset from the begin of the buffer
	 *
	 * @return  size_t New offset after packing the headers of the packets
	 */
	size_t packChangeCipherSpec(Packet *p, unsigned char *buffer, size_t offset);

	/**
	 *  Method responsible to take the informations from the received buffer and
	 * store it into the headers of the packet
	 *
	 * @param Pointer to the buffer received
	 * @param Offset from the begin of the buffer
	 * @param Pointer to the new packet
	 *
	 * @return size_t New offset after unpacking the headers
	 */
	size_t unpackMyHdr(unsigned char *buffer, size_t offset, Packet *p);

	/**
	 *  Method to get back the TLS Record
	 *
	 * @param Pointer to the buffer received
	 * @param Offset from the begin of the buffer
	 * @param Pointer to the new packet
	 *
	 * @return  size_t New offset after unpacking the headers
	 */
	size_t unpackRecord(unsigned char *buffer, size_t offset, Packet *p);

	/**
	 *  Method to get back the TLS Handshake
	 *
	 * @param Pointer to the buffer received
	 * @param Offset from the begin of the buffer
	 * @param Pointer to the new packet
	 *
	 * @return  size_t New offset after unpacking the headers
	 */
	size_t unpackHandshake(unsigned char *buffer, size_t offset, Packet *p);

	/**
	 *  Method to get back the TLS ClientHello
	 *
	 * @param Pointer to the buffer received
	 * @param Offset from the begin of the buffer
	 * @param Pointer to the new packet
	 *
	 * @return  size_t New offset after unpacking the headers
	 */
	size_t unpackClientHello(unsigned char *buffer, size_t offset, Packet *p);

	/**
	 *  Method to get back the TLS ServerHello
	 *
	 * @param Pointer to the buffer received
	 * @param Offset from the begin of the buffer
	 * @param Pointer to the new packet
	 *
	 * @return  size_t New offset after unpacking the headers
	 */
	size_t unpackServerHello(unsigned char *buffer, size_t offset, Packet *p);

	/**
	 *  Method to get back the TLS ClientKeyExchange
	 *
	 * @param Pointer to the buffer received
	 * @param Offset from the begin of the buffer
	 * @param Pointer to the new packet
	 *
	 * @return  size_t New offset after unpacking the headers
	 */
	size_t unpackClientKeyExchange(unsigned char *buffer, size_t offset, Packet *p);

	/**
	 *  Method to get back the TLS ChangeCipherSpec
	 *
	 * @param Pointer to the buffer received
	 * @param Offset from the begin of the buffer
	 * @param Pointer to the new packet
	 *
	 * @return  size_t New offset after unpacking the headers
	 */
	size_t unpackChangeCipherSpec(unsigned char *buffer, size_t offset, Packet *p);

	/**
	 * Method used for debug purposes. It prints the number of bits for each
	 * header serialized
	 */
	void printMyHdrMap();

	/**
	 *  Method used for debug purposes. It prints the value of the headers of a packet
	 *
	 * @param Pointer of the packet
	 */
	void printMyHdrFields(Packet *);
};

#endif // PACKER_UWTLS_H
