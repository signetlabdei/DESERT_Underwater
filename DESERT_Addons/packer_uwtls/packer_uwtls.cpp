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

/**
 * @file       packer_uwtls.cpp
 * @author     Davide Costa
 */

#include "packer_uwtls.h"

static class PackerUwTlsClass : public TclClass
{
public:
	PackerUwTlsClass()
		: TclClass("UW/TLS/Packer")
	{
	}

	TclObject *
	create(int, const char *const *)
	{
		return (new packer_uwtls());
	}
} class_module_packeruwtls;

packer_uwtls::packer_uwtls()
	: packer(false)
{
	this->init();
}

packer_uwtls::~packer_uwtls()
{
}

void
packer_uwtls::init()
{
	n_bits.clear();
	n_bits.assign(0, 0);

}

size_t
packer_uwtls::packMyHdr(Packet *p, unsigned char *buffer, size_t offset)
{
	hdr_cmn *hcmn = HDR_CMN(p);
	
	if (hcmn->ptype() == PT_TLS_RECORD) {
		// Packet to be serialized is TLS RECORD packet
		return packRecord(p, buffer, offset);
	}
	return offset;
}


size_t
packer_uwtls::packRecord(Packet *p, unsigned char *buffer, size_t offset)
{
	hdr_tls_record *tr = HDR_TLS_RECORD(p);
	
	offset += put(buffer, offset, &(tr->contentType), 8);
	offset += put(buffer, offset, &(tr->version), 16);
	offset += put(buffer, offset, &(tr->length), 16);
	
	switch (tr->contentType) {
		case TLS_HANDSHAKE:
			return packHandshake(p, buffer, offset);
		case TLS_CHANGECIPHERSPEC:
			return packChangeCipherSpec(p, buffer, offset);
		default:
			// Received an unrecognized record type
			return offset;
	}
}

size_t
packer_uwtls::packHandshake(Packet *p, unsigned char *buffer, size_t offset)
{
	hdr_handshake *hh = HDR_HANDSHAKE(p);
	
	offset += put(buffer, offset, &(hh->type), 8);
	offset += put(buffer, offset, &(hh->length), 24);
	
	switch (hh->type) {
		case (CLIENT_HELLO):
			return packClientHello(p, buffer, offset);
		case (SERVER_HELLO):
			return packServerHello(p, buffer, offset);
		case (CLIENT_KEY_EXCHANGE):
			return packClientKeyExchange(p, buffer, offset);
		default:
			// Received an unrecognized handshake type
			return offset;
	}
}

size_t
packer_uwtls::packClientHello(Packet *p, unsigned char *buffer, size_t offset)
{
	hdr_clienthello *ch = HDR_CLIENTHELLO(p);
	
	offset += put(buffer, offset, &(ch->version), 16);
	offset += put(buffer, offset, &(ch->random), 32 * 8);
	
	offset += put(buffer, offset, &(ch->sessionIdLen), 8);
	int session_size_bits = ch->sessionIdLen * 8;
	offset += put(buffer, offset, &(ch->sessionId), session_size_bits);
	
	offset += put(buffer, offset, &(ch->cipherSuitesLen), 16);
	int cipher_len_bits = ch->cipherSuitesLen * 16;
	offset += put(buffer, offset, &(ch->cipherSuites), cipher_len_bits);
	
	offset += put(buffer, offset, &(ch->compressionLen), 8);
	int compression_len_bits = ch->compressionLen * 8;
	offset += put(buffer, offset, &(ch->compression), compression_len_bits);
	
	offset += put(buffer, offset, &(ch->extensionsLen), 16);
	int extensions_len_bits = ch->extensionsLen * 8;
	offset += put(buffer, offset, &(ch->extensions), extensions_len_bits);
	
	return offset;
}

size_t
packer_uwtls::packServerHello(Packet *p, unsigned char *buffer, size_t offset)
{
	hdr_serverhello *sh = HDR_SERVERHELLO(p);
	
	offset += put(buffer, offset, &(sh->version), 16);
	offset += put(buffer, offset, &(sh->random), 32 * 8);
	
	offset += put(buffer, offset, &(sh->sessionIdLen), 8);
	int session_size_bits = sh->sessionIdLen * 8;
	offset += put(buffer, offset, &(sh->sessionId), session_size_bits);
	
	offset += put(buffer, offset, &(sh->selectedCipherSuite), 16);
	
	offset += put(buffer, offset, &(sh->selectedCompression), 8);
	
	offset += put(buffer, offset, &(sh->extensionsLen), 16);
	int extensions_len_bits = sh->extensionsLen * 8;
	offset += put(buffer, offset, &(sh->extensions), extensions_len_bits);
	
	return offset;
}

size_t
packer_uwtls::packClientKeyExchange(Packet *p, unsigned char *buffer, size_t offset)
{
	hdr_clientkeyexchange *cke = HDR_CLIENTKEYEXCHANGE(p);
	
	offset += put(buffer, offset, &(cke->encrypted_premaster_len), 16);
	int encrypted_premaster_bits = cke->encrypted_premaster_len * 8;
	offset += put(buffer, offset, &(cke->encrypted_premaster), encrypted_premaster_bits);
	
	return offset;
}


size_t
packer_uwtls::packChangeCipherSpec(Packet *p, unsigned char *buffer, size_t offset)
{
	hdr_changecipherspec *ccs = HDR_CHANGECIPHERSPEC(p);
	
	offset += put(buffer, offset, &(ccs->type), 8);
	
	return offset;
}

size_t
packer_uwtls::unpackMyHdr(unsigned char *buffer, size_t offset, Packet *p)
{

	hdr_cmn *hcmn = HDR_CMN(p);

	if (hcmn->ptype() == PT_TLS_RECORD) {
		// Packet to be serialized is a PT_TLS_RECORD packet
		unpackRecord(buffer, offset, p);
	}
	return offset;
}


size_t
packer_uwtls::unpackRecord(unsigned char *buffer, size_t offset, Packet *p)
{
	hdr_tls_record *tr = HDR_TLS_RECORD(p);
	
	memset(&(tr->contentType), 0, sizeof(tr->contentType));
	offset += get(buffer, offset, &(tr->contentType), 8);
	
	memset(&(tr->version), 0, sizeof(tr->version));
	offset += get(buffer, offset, &(tr->version), 16);
	
	memset(&(tr->length), 0, sizeof(tr->length));
	offset += get(buffer, offset, &(tr->length), 16);
	
	switch (tr->contentType) {
		case TLS_HANDSHAKE:
			return unpackHandshake(buffer, offset, p);
		case TLS_CHANGECIPHERSPEC:
			return unpackChangeCipherSpec(buffer, offset, p);
		default:
			// Received an unrecognized record type
			return offset;
	}
}

size_t
packer_uwtls::unpackHandshake(unsigned char *buffer, size_t offset, Packet *p)
{
	hdr_handshake *hh = HDR_HANDSHAKE(p);

	memset(&(hh->type), 0, sizeof(hh->type));
	offset += get(buffer, offset, &(hh->type), 8);

	memset(&(hh->length), 0, sizeof(hh->length));
	offset += get(buffer, offset, &(hh->length), 24);

	switch (hh->type) {
		case CLIENT_HELLO:
			return unpackClientHello(buffer, offset, p);
		case SERVER_HELLO:
			return unpackServerHello(buffer, offset, p);
		case CLIENT_KEY_EXCHANGE:
			return unpackClientKeyExchange(buffer, offset, p);
		default:
			// Unrecognized handshake type, skip body according to hh->length if needed
			return offset;
	}
}

size_t
packer_uwtls::unpackClientHello(unsigned char *buffer, size_t offset, Packet *p)
{
	hdr_clienthello *ch = HDR_CLIENTHELLO(p);

	memset(&(ch->version), 0, sizeof(ch->version));
	offset += get(buffer, offset, &(ch->version), 16);

	memset(&(ch->random), 0, sizeof(ch->random));
	offset += get(buffer, offset, &(ch->random), 32 * 8);

	memset(&(ch->sessionIdLen), 0, sizeof(ch->sessionIdLen));
	offset += get(buffer, offset, &(ch->sessionIdLen), 8);
	int session_size_bits = ch->sessionIdLen * 8;
	if (session_size_bits > 0) {
		memset(&(ch->sessionId), 0, sizeof(ch->sessionId));
		offset += get(buffer, offset, &(ch->sessionId), session_size_bits);
	}

	memset(&(ch->cipherSuitesLen), 0, sizeof(ch->cipherSuitesLen));
	offset += get(buffer, offset, &(ch->cipherSuitesLen), 16);
	int cipher_len_bits = ch->cipherSuitesLen * 16;
	if (cipher_len_bits > 0) {
		memset(&(ch->cipherSuites), 0, sizeof(ch->cipherSuites));
		offset += get(buffer, offset, &(ch->cipherSuites), cipher_len_bits);
	}

	memset(&(ch->compressionLen), 0, sizeof(ch->compressionLen));
	offset += get(buffer, offset, &(ch->compressionLen), 8);
	int compression_len_bits = ch->compressionLen * 8;
	if (compression_len_bits > 0) {
		memset(&(ch->compression), 0, sizeof(ch->compression));
		offset += get(buffer, offset, &(ch->compression), compression_len_bits);
	}

	memset(&(ch->extensionsLen), 0, sizeof(ch->extensionsLen));
	offset += get(buffer, offset, &(ch->extensionsLen), 16);
	int extensions_len_bits = ch->extensionsLen * 8;
	if (extensions_len_bits > 0) {
		memset(&(ch->extensions), 0, sizeof(ch->extensions));
		offset += get(buffer, offset, &(ch->extensions), extensions_len_bits);
	}

	return offset;
}

size_t
packer_uwtls::unpackServerHello(unsigned char *buffer, size_t offset, Packet *p)
{
	hdr_serverhello *sh = HDR_SERVERHELLO(p);

	memset(&(sh->version), 0, sizeof(sh->version));
	offset += get(buffer, offset, &(sh->version), 16);

	memset(&(sh->random), 0, sizeof(sh->random));
	offset += get(buffer, offset, &(sh->random), 32 * 8);

	memset(&(sh->sessionIdLen), 0, sizeof(sh->sessionIdLen));
	offset += get(buffer, offset, &(sh->sessionIdLen), 8);
	int session_size_bits = sh->sessionIdLen * 8;
	if (session_size_bits > 0) {
		memset(&(sh->sessionId), 0, sizeof(sh->sessionId));
		offset += get(buffer, offset, &(sh->sessionId), session_size_bits);
	}

	memset(&(sh->selectedCipherSuite), 0, sizeof(sh->selectedCipherSuite));
	offset += get(buffer, offset, &(sh->selectedCipherSuite), 16);

	memset(&(sh->selectedCompression), 0, sizeof(sh->selectedCompression));
	offset += get(buffer, offset, &(sh->selectedCompression), 8);

	memset(&(sh->extensionsLen), 0, sizeof(sh->extensionsLen));
	offset += get(buffer, offset, &(sh->extensionsLen), 16);
	int extensions_len_bits = sh->extensionsLen * 8;
	if (extensions_len_bits > 0) {
		memset(&(sh->extensions), 0, sizeof(sh->extensions));
		offset += get(buffer, offset, &(sh->extensions), extensions_len_bits);
	}

	return offset;
}

size_t
packer_uwtls::unpackClientKeyExchange(unsigned char *buffer, size_t offset, Packet *p)
{
	hdr_clientkeyexchange *cke = HDR_CLIENTKEYEXCHANGE(p);

	memset(&(cke->encrypted_premaster_len), 0, sizeof(cke->encrypted_premaster_len));
	offset += get(buffer, offset, &(cke->encrypted_premaster_len), 16);
	int encrypted_premaster_bits = cke->encrypted_premaster_len * 8;
	if (encrypted_premaster_bits > 0) {
		memset(&(cke->encrypted_premaster), 0, sizeof(cke->encrypted_premaster));
		offset += get(buffer, offset, &(cke->encrypted_premaster), encrypted_premaster_bits);
	}

	return offset;
}

size_t
packer_uwtls::unpackChangeCipherSpec(unsigned char *buffer, size_t offset, Packet *p)
{
	hdr_changecipherspec *ccs = HDR_CHANGECIPHERSPEC(p);

	memset(&(ccs->type), 0, sizeof(ccs->type));
	offset += get(buffer, offset, &(ccs->type), 8);

	return offset;
}

void
packer_uwtls::printMyHdrMap()
{
}

void
packer_uwtls::printMyHdrFields(Packet *p)
{
}
