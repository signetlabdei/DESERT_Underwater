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
 * @file uwtls-module.cpp
 * @author Davide Costa
 * \version 1.0.0
 * \brief  Implementation of the class responsible for the Transport Layer Security.
 */

#include "uwtls-module.h"

// Declaration of headers
int hdr_tls_record::offset_;
int hdr_handshake::offset_;
int hdr_changecipherspec::offset_;
int hdr_clienthello::offset_;
int hdr_serverhello::offset_;
int hdr_clientkeyexchange::offset_;

static class UwTlsRecordHdrClass : public PacketHeaderClass
{
public:
	UwTlsRecordHdrClass()
		: PacketHeaderClass("PacketHeader/UWTLSRecord", sizeof(hdr_tls_record))
	{
		this->bind();
		bind_offset(&hdr_tls_record::offset_);
	}
} class_tls_record_hdr;

static class UwTlsHandshakeHdrClass : public PacketHeaderClass
{
public:
	UwTlsHandshakeHdrClass()
		: PacketHeaderClass("PacketHeader/UWTLSHandshake", sizeof(hdr_handshake))
	{
		this->bind();
		bind_offset(&hdr_handshake::offset_);
	}
} class_tls_handshake_hdr;

static class UwTlsClienthelloHdrClass : public PacketHeaderClass
{
public:
	UwTlsClienthelloHdrClass()
		: PacketHeaderClass("PacketHeader/UWTLSClienthello", sizeof(hdr_clienthello))
	{
		this->bind();
		bind_offset(&hdr_clienthello::offset_);
	}
} class_tls_clienthello_hdr;

static class UwTlsServerhelloHdrClass : public PacketHeaderClass
{
public:
	UwTlsServerhelloHdrClass()
		: PacketHeaderClass("PacketHeader/UWTLSServerhello", sizeof(hdr_serverhello))
	{
		this->bind();
		bind_offset(&hdr_serverhello::offset_);
	}
} class_tls_serverhello_hdr;

static class UwTlsClientKeyExchangeHdrClass : public PacketHeaderClass
{
public:
	UwTlsClientKeyExchangeHdrClass()
		: PacketHeaderClass("PacketHeader/UWTLSClientKeyExchange", sizeof(hdr_clientkeyexchange))
	{
		this->bind();
		bind_offset(&hdr_clientkeyexchange::offset_);
	}
} class_tls_clientkeyexchange_hdr;

static class UwTlsChangeCipherSpecHdrClass : public PacketHeaderClass
{
public:
	UwTlsChangeCipherSpecHdrClass()
		: PacketHeaderClass("PacketHeader/UWTLSChangeCipherSpec", sizeof(hdr_changecipherspec))
	{
		this->bind();
		bind_offset(&hdr_changecipherspec::offset_);
	}
} class_tls_changecipherspec_hdr;
// Declaration of headers finished

static class UwTlsClass : public TclClass
{
public:
	UwTlsClass()
		: TclClass("Module/UW/TLS")
	{
	}

	TclObject *
	create(int, const char *const *)
	{
		return (new UwTls());
	}
} class_module_uwtls;


void
UwTlsRetransmissionTimer::expire(Event *e)
{
	module_->sendCurrentFlight();
}


UwTls::UwTls()
	: state(HandshakeState::NOT_INITIALIZED)
	, client_random{0}
	, server_random{0}
	, role(Role::UNKNOWN)
	, retransmit_timeout(0)
	, max_retransmissions(0)
	, retransmissions_done(0)
	, current_timeout(0)
	, flight_active(false)
	, retransmission_timer_(this)
{
	bind("debug_", (int *) &debug_);
	bind("retransmit_timeout_", (double *) &retransmit_timeout);
	bind("max_retransmissions_", (int *) &max_retransmissions);
}

UwTls::~UwTls()
{
	if (remote_ec_pubkey != nullptr) {
		EVP_PKEY_free(remote_ec_pubkey);
		remote_ec_pubkey = nullptr;
	}

	if (local_ec_privkey != nullptr) {
		EVP_PKEY_free(local_ec_privkey);
		local_ec_privkey = nullptr;
	}
}

int
UwTls::command(int argc, const char *const *argv)
{
	Tcl &tcl = Tcl::instance();

	if (argc == 3) {
		if (strcasecmp(argv[1], "loadPrivateKey") == 0) {
			local_ec_privkey = loadPrivateKeyFromPemFile(argv[2]);
			
			if (local_ec_privkey == nullptr) {
				tcl.result("Failed to load private key");
				return TCL_ERROR;
			}
			
			return TCL_OK;
		} else if (strcasecmp(argv[1], "loadPublicKey") == 0) {
			remote_ec_pubkey = loadPublicKeyFromPemFile(argv[2]);
			
			if (remote_ec_pubkey == nullptr) {
				tcl.result("Failed to load public key");
				return TCL_ERROR;
			}
			
			return TCL_OK;
		}
	}

	return Module::command(argc, argv);
}

void
UwTls::recv(Packet *p)
{
	printOnLog(Logger::LogLevel::ERROR,
			"UWTLS",
			"recv::packet sent without source module");
	Packet::free(p);
}

void
UwTls::recv(Packet *p, int idSrc)
{
	hdr_cmn *cmn = HDR_CMN(p);
	hdr_uwudp *uwudp = HDR_UWUDP(p);
	hdr_uwip *uwiph = HDR_UWIP(p);
	
	if (!cmn->error()) {
		if (cmn->direction() == hdr_cmn::UP) {
			if (cmn->ptype() == PT_TLS_RECORD) {
				// https://datatracker.ietf.org/doc/html/rfc5246#page-36
				// https://datatracker.ietf.org/doc/html/rfc5288#section-3
				
				handleRecord(p);
			} else if (state == HandshakeState::ESTABLISHED) {
				if (role == Role::SERVER) {
					// Flight S2 was "ACKed" by an encrypted message
					stopRepeatedMessage();
				}
				symmetricDecrypt(p);
				sendUp(p);
			} else {
				// The transmitting node is sending a packet but TLS is not yet estabilished
				// Enqueue the packet until the handshake is completed
				waitingQueue.push(p);
			}
		} else {
			if (state == HandshakeState::NOT_INITIALIZED) {
				// Enqueue the packet until the handshake is completed
				waitingQueue.push(p);
				// CLIENT
				role = Role::CLIENT;
				state = HandshakeState::WAIT_SERVER_HELLO;
				sendRepeatedMessage(&UwTls::sendClientHello, uwudp->dport(), uwiph->daddr());
			} else if (state == HandshakeState::ESTABLISHED) {
				symmetricEncrypt(p);
				sendDown(p);
			} else {
				// Enqueue the packet until the handshake is completed
				waitingQueue.push(p);
			}
		}
	}
}

EVP_PKEY *
UwTls::loadPrivateKeyFromPemFile(const char *path)
{
	FILE *fp = fopen(path, "r");
	if (fp == nullptr) {
		return nullptr;
	}

	EVP_PKEY *pkey = PEM_read_PrivateKey(fp, nullptr, nullptr, nullptr);
	fclose(fp);
	return pkey;
}

EVP_PKEY *
UwTls::loadPublicKeyFromPemFile(const char *path)
{
	FILE *fp = fopen(path, "r");
	if (fp == nullptr) {
		return nullptr;
	}

	EVP_PKEY *pkey = PEM_read_PUBKEY(fp, nullptr, nullptr, nullptr);
	fclose(fp);
	return pkey;
}

void
UwTls::handleRecord(Packet *p)
{
	hdr_tls_record *tr = HDR_TLS_RECORD(p);
	
	switch (tr->contentType) {
		case TLS_HANDSHAKE:
			handleHandshake(p);
			break;
		case TLS_CHANGECIPHERSPEC:
			handleChangeCipherSpec(p);
			break;
		default:
			printOnLog(Logger::LogLevel::ERROR,
					"UWTLS",
					"handleHandshake::Received an unrecognized record type");
			break;
	}
}

void
UwTls::handleHandshake(Packet *p)
{
	hdr_uwudp *uwudp = HDR_UWUDP(p);
	hdr_uwip *uwiph = HDR_UWIP(p);
	
	hdr_handshake *hh = HDR_HANDSHAKE(p);
	
	switch (hh->type) {
		case (CLIENT_HELLO):
			// SERVER
			if (flight_active) {
				printOnLog(Logger::LogLevel::ERROR,
						"UWTLS",
						"handleHandshake::A sender was running while a ClientHello was received. Ignoring...");
				break;
			}
			
			printOnLog(Logger::LogLevel::INFO,
					"UWTLS",
					"handleHandshake::Received ClientHello");
			
			role = Role::SERVER;
			interpretClientHello(p);
			state = HandshakeState::WAIT_CLIENT_KEY_EXCHANGE;
			
			sendRepeatedMessage(&UwTls::sendServerHello, uwudp->sport(), uwiph->saddr());
			
			break;
		case (SERVER_HELLO):
			// CLIENT
			printOnLog(Logger::LogLevel::INFO,
					"UWTLS",
					"handleHandshake::Received ServerHello");
			
			interpretServerHello(p);
			if (role == Role::CLIENT) {
				// Flight C1 was "ACKed" by S1
				stopRepeatedMessage();
			} else {
				break;
			}
			
			state = HandshakeState::WAIT_CHANGE_CIPHER_SPEC;
			
			// Start flight C2: ClientKeyExchange + ChangeCipherSpec
			sendRepeatedMessage(&UwTls::sendFlightC2, uwudp->sport(), uwiph->saddr());
			
			break;
		case (CLIENT_KEY_EXCHANGE):
			// SERVER
			printOnLog(Logger::LogLevel::INFO,
					"UWTLS",
					"handleHandshake::Received ClientKeyExchange");
			
			if (role == Role::SERVER) {
				// Flight S1 was "ACKed" by C2
				stopRepeatedMessage();
			} else {
				break;
			}
			
			state = HandshakeState::WAIT_CHANGE_CIPHER_SPEC;
			interpretClientKeyExchange(p);
			
			if (bufferedChangeCipherSpec) {
				printOnLog(Logger::LogLevel::INFO,
						"UWTLS",
						"handleHandshake::A premaure ChangeCipherSpec was buffered, applying it");
				handleChangeCipherSpec(*bufferedChangeCipherSpec);
				bufferedChangeCipherSpec.reset();
			}
			
			break;
		default:
			printOnLog(Logger::LogLevel::ERROR,
					"UWTLS",
					"handleHandshake::Received an unrecognized handshake type");
			break;
	}
}

void
UwTls::handleChangeCipherSpec(Packet *p)
{
	hdr_uwudp *uwudp = HDR_UWUDP(p);
	hdr_uwip *uwiph = HDR_UWIP(p);
	
	if (state == HandshakeState::WAIT_CHANGE_CIPHER_SPEC) {
		printOnLog(Logger::LogLevel::INFO,
				"UWTLS",
				"handleChangeCipherSpec::Received ChangeCipherSpec at correct time");
			
		if (role == Role::SERVER) {
			// Flight S2: ChangeCipherSpec only
			sendRepeatedMessage(&UwTls::sendChangeCipherSpec, uwudp->sport(), uwiph->saddr());
		} else if (role == Role::CLIENT) {
			// Flight C2 was "ACKed" by S2
			stopRepeatedMessage();
		}
		
		applyChangeCipherSpec(p);
	} else if (state == HandshakeState::WAIT_CLIENT_KEY_EXCHANGE) {
		printOnLog(Logger::LogLevel::INFO,
				"UWTLS",
				"handleChangeCipherSpec::Received an early ChangeCipherSpec, bufferizing it");
		bufferedChangeCipherSpec = p;
	} else {
		printOnLog(Logger::LogLevel::ERROR,
				"UWTLS",
				"handleChangeCipherSpec::Received an out of order ChangeCipherSpec, ignoring");
	}
}

void
UwTls::applyChangeCipherSpec(Packet *p)
{
	hdr_changecipherspec *ccs = HDR_CHANGECIPHERSPEC(p);
	
	if (ccs->type != 0x01) {
		printOnLog(Logger::LogLevel::ERROR,
				"UWTLS",
				"applyChangeCipherSpec::Invalid ChangeCipherSpec");
		state = HandshakeState::NOT_INITIALIZED;
		return;
	}
	

	if (ecdh_premaster.size() == 0) {
		printOnLog(Logger::LogLevel::ERROR,
				"UWTLS",
				"applyChangeCipherSpec::ECDH premaster not available");
		state = HandshakeState::ERROR;
		return;
	}

	// TLS 1.2
	//
	// master_secret = PRF(pre_master_secret,
	//			"master secret",
	//			ClientHello.random + ServerHello.random)
	//
	// key_block = PRF(master_secret,
	//		   "key expansion",
	//		   ServerHello.random + ClientHello.random)
	//
	// TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256 requires:
	// client_write_key = 16 bytes
	// server_write_key = 16 bytes
	// client_write_IV  = 4 bytes
	// server_write_IV  = 4 bytes
	//
	// Total key_block = 40 bytes

	std::vector<uint8_t> key_block;

	try {
		derive_tls12_key_block(key_block, ecdh_premaster.data(), ecdh_premaster.size(), client_random, server_random, 40);
	} catch (...) {
		printOnLog(Logger::LogLevel::ERROR,
				"UWTLS",
				"applyChangeCipherSpec::Key block derivation failed");
		state = HandshakeState::ERROR;
		return;
	}

	if (key_block.size() < 40) {
		printOnLog(Logger::LogLevel::ERROR,
				"UWTLS",
				"applyChangeCipherSpec::Key block too short for AES_128_GCM");
		state = HandshakeState::ERROR;
		return;
	}

	// Layout TLS 1.2 AEAD GCM
	memcpy(client_write_key, key_block.data() + 0, 16);
	memcpy(server_write_key, key_block.data() + 16, 16);
	memcpy(client_write_iv,  key_block.data() + 32, 4);
	memcpy(server_write_iv,  key_block.data() + 36, 4);
	
	state = HandshakeState::ESTABLISHED;
	
	printOnLog(Logger::LogLevel::INFO,
			"UWTLS",
			"applyChangeCipherSpec::Switching to encrypted mode");
	
	// Expect Finished message next (already encrypted)
	
	sendBufferedPackets();
}

void
UwTls::symmetricEncrypt(Packet *p)
{
	hdr_DATA_APPLICATION *app = HDR_DATA_APPLICATION(p);

	const uint64_t seq_num = app->sn();
	const int plaintext_len = app->payload_size();

	if (plaintext_len < 0) {
		printOnLog(Logger::LogLevel::ERROR,
				"UWTLS",
				"symmetricEncrypt::Invalid plaintext length");
		return;
	}

	const unsigned char *key =
		(role == Role::CLIENT) ? client_write_key : server_write_key;

	const unsigned char *implicit_iv =
		(role == Role::CLIENT) ? client_write_iv : server_write_iv;

	unsigned char nonce_explicit[8];
	unsigned char nonce[12];
	unsigned char aad[13];
	unsigned char tag[16];

	writeUint64BE(nonce_explicit, seq_num);

	memset(nonce, 0, sizeof(nonce));
	memcpy(nonce, implicit_iv, 4);
	memcpy(nonce + 4, nonce_explicit, 8);

	memset(aad, 0, sizeof(aad));
	memcpy(aad, nonce_explicit, 8);		// sequence number
	aad[8]  = 23;				// application_data
	aad[9]  = 0x03;				// TLS 1.2
	aad[10] = 0x03;
	aad[11] = static_cast<unsigned char>((plaintext_len >> 8) & 0xFF);
	aad[12] = static_cast<unsigned char>(plaintext_len & 0xFF);

	std::vector<unsigned char> plaintext(plaintext_len);
	memcpy(plaintext.data(), app->payload_msg, plaintext_len);

	std::vector<unsigned char> ciphertext(plaintext_len);

	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
	if (!ctx) {
		printOnLog(Logger::LogLevel::ERROR,
				"UWTLS",
				"symmetricEncrypt::EVP_CIPHER_CTX_new failed");
		return;
	}

	int len = 0;
	int ciphertext_len = 0;
	bool ok = true;

	ok = ok && (EVP_EncryptInit_ex(ctx, EVP_aes_128_gcm(), nullptr, nullptr, nullptr) == 1);
	ok = ok && (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 12, nullptr) == 1);
	ok = ok && (EVP_EncryptInit_ex(ctx, nullptr, nullptr, key, nonce) == 1);
	ok = ok && (EVP_EncryptUpdate(ctx, nullptr, &len, aad, sizeof(aad)) == 1);
	ok = ok && (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext.data(), plaintext_len) == 1);
	ciphertext_len = len;
	ok = ok && (EVP_EncryptFinal_ex(ctx, ciphertext.data() + ciphertext_len, &len) == 1);
	ciphertext_len += len;
	ok = ok && (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag) == 1);

	EVP_CIPHER_CTX_free(ctx);

	if (!ok || ciphertext_len != plaintext_len) {
		printOnLog(Logger::LogLevel::ERROR,
				"UWTLS",
				"symmetricEncrypt::AES-128-GCM encryption failed");
		return;
	}

	const int protected_len = app->payload_size() + 16;
	
	memcpy(app->payload_msg, ciphertext.data(), ciphertext_len);
	memcpy(app->payload_msg + ciphertext_len, tag, 16);

	app->payload_size_ = protected_len;
	
	printOnLog(Logger::LogLevel::INFO,
			"UWTLS",
			"symmetricEncrypt::Protected payload size updated from " +
			std::to_string(plaintext_len) + " to " + std::to_string(protected_len));
}

void
UwTls::symmetricDecrypt(Packet *p)
{
	hdr_DATA_APPLICATION *app = HDR_DATA_APPLICATION(p);

	const int protected_len = app->payload_size();

	if (protected_len < 16) {
		printOnLog(Logger::LogLevel::ERROR,
				"UWTLS",
				"symmetricDecrypt::Protected payload too short");
		return;
	}

	const int ciphertext_len = protected_len - 16;
	const uint64_t seq_num = app->sn();

	const unsigned char *key =
		(role == Role::CLIENT) ? server_write_key : client_write_key;

	const unsigned char *implicit_iv =
		(role == Role::CLIENT) ? server_write_iv : client_write_iv;

	unsigned char nonce_explicit[8];
	unsigned char nonce[12];
	unsigned char aad[13];
	unsigned char tag[16];

	writeUint64BE(nonce_explicit, seq_num);

	memset(nonce, 0, sizeof(nonce));
	memcpy(nonce, implicit_iv, 4);
	memcpy(nonce + 4, nonce_explicit, 8);

	memset(aad, 0, sizeof(aad));
	memcpy(aad, nonce_explicit, 8);		// sequence number
	aad[8]  = 23;				// application_data
	aad[9]  = 0x03;				// TLS 1.2
	aad[10] = 0x03;
	aad[11] = static_cast<unsigned char>((ciphertext_len >> 8) & 0xFF);
	aad[12] = static_cast<unsigned char>(ciphertext_len & 0xFF);

	std::vector<unsigned char> ciphertext(ciphertext_len);
	std::vector<unsigned char> plaintext(ciphertext_len);

	memcpy(ciphertext.data(), app->payload_msg, ciphertext_len);
	memcpy(tag, app->payload_msg + ciphertext_len, 16);

	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
	if (!ctx) {
		printOnLog(Logger::LogLevel::ERROR,
				"UWTLS",
				"symmetricDecrypt::EVP_CIPHER_CTX_new failed");
		return;
	}

	int len = 0;
	int plaintext_len = 0;
	bool ok = true;

	ok = ok && (EVP_DecryptInit_ex(ctx, EVP_aes_128_gcm(), nullptr, nullptr, nullptr) == 1);
	ok = ok && (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 12, nullptr) == 1);
	ok = ok && (EVP_DecryptInit_ex(ctx, nullptr, nullptr, key, nonce) == 1);
	ok = ok && (EVP_DecryptUpdate(ctx, nullptr, &len, aad, sizeof(aad)) == 1);
	ok = ok && (EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext.data(), ciphertext_len) == 1);
	plaintext_len = len;
	ok = ok && (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag) == 1);

	int final_ok = EVP_DecryptFinal_ex(ctx, plaintext.data() + plaintext_len, &len);
	plaintext_len += len;

	EVP_CIPHER_CTX_free(ctx);

	if (!ok || final_ok <= 0 || plaintext_len != ciphertext_len) {
		printOnLog(Logger::LogLevel::ERROR,
				"UWTLS",
				"symmetricDecrypt::AES-128-GCM authentication failed");
		return;
	}

	memcpy(app->payload_msg, plaintext.data(), plaintext_len);

	app->payload_size_ = plaintext_len;

	printOnLog(Logger::LogLevel::INFO,
			"UWTLS",
			"symmetricDecrypt::Recovered plaintext size updated from " +
			std::to_string(protected_len) + " to " + std::to_string(plaintext_len));
}

bool
UwTls::randomBytes(uint8_t *data, size_t size)
{
	uint8_t random_value = 0;
	ifstream urandom("/dev/urandom", ios::in|ios::binary);
	if (urandom) {
		for (size_t i=0; i < size; i++) {
			urandom.read(reinterpret_cast<char*>(&random_value), 1);
			if (urandom) {
				data[i] = random_value;
			} else {
				return false;
			}
		}
		urandom.close();
	} else {
		return false;
	}
	
	return true;
}

void
UwTls::sendRepeatedMessage(void (UwTls::*func)(int, uint8_t), int portnum, uint8_t ipaddr)
{
	flight_func = func;
	flight_port = portnum;
	flight_ip = ipaddr;
	
	retransmissions_done = 0;
	current_timeout = retransmit_timeout;
	flight_active = true;

	if (retransmission_timer_.status() == TimerHandler::TIMER_PENDING) {
		retransmission_timer_.cancel();
	}

	sendCurrentFlight();
}

void
UwTls::sendCurrentFlight()
{
	if (!flight_active || flight_func == nullptr) {
		return;
	}

	(this->*flight_func)(flight_port, flight_ip);
	retransmissions_done++;

	if (retransmissions_done > max_retransmissions) {
		printOnLog(Logger::LogLevel::ERROR,
				   "UWTLS",
				   "sendCurrentFlight::handshake flight max retransmissions reached, aborting");
		state = HandshakeState::ERROR;
		flight_active = false;
		return;
	}

	retransmission_timer_.resched(current_timeout);
	current_timeout *= 2.0;
}

void
UwTls::stopRepeatedMessage()
{
	flight_active = false;

	if (retransmission_timer_.status() == TimerHandler::TIMER_PENDING) {
		retransmission_timer_.cancel();
	}
}

void
UwTls::sendClientHello(int portnum, uint8_t ipaddr)
{
	Packet *p = Packet::alloc();
	
	hdr_cmn *cmn = HDR_CMN(p);
	hdr_uwudp *uwudp = HDR_UWUDP(p);
	hdr_uwip *uwiph = HDR_UWIP(p);

	cmn->uid() = 1;
	cmn->ptype() = PT_TLS_RECORD;
	cmn->direction() = hdr_cmn::DOWN;
	cmn->timestamp() = NOW;
	
	uwudp->dport() = portnum;
	uwiph->daddr() = ipaddr;
	
	hdr_tls_record *tr = HDR_TLS_RECORD(p);
	hdr_handshake *hh = HDR_HANDSHAKE(p);
	hdr_clienthello *ch = HDR_CLIENTHELLO(p);
	
	// TLS 1.2
	ch->version = 0x0303;
	
	// Client random
	if (!randomBytes(client_random, 32)) {
		printOnLog(Logger::LogLevel::ERROR,
				"UWTLS",
				"sendClientHello::Failed to read from /dev/urand");
		state = HandshakeState::NOT_INITIALIZED;
		return;
	}
	memcpy(ch->random, client_random, 32);
	
	// Session ID: length (empty example)
	ch->sessionIdLen = 0;
	
	// TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256 = 0xC02D
	ch->cipherSuitesLen = 1;
	ch->cipherSuites[0] = 0xC02D;
	
	// Compression Methods: length + list (null compression only)
	ch->compressionLen = 1;
	ch->compression[0] = 0x00;
	
	uint8_t *ext = ch->extensions;
	uint16_t *ext16 = (uint16_t*)ext;
	
	// RFC 7250: ONLY RawPublicKey (NO X.509)
	*ext16++ = 19;	// Extension type 19: client_certificate_type
	*ext16++ = 2;	// Total extension length: 2 bytes
	ext = (uint8_t*)ext16;
	*ext++ = 1;	// Certificate types length: 1 type
	*ext++ = 2;	// Type 2 = RawPublicKey ONLY [RFC 7250]
	
	ext16 = (uint16_t*)ext;
	
	*ext16++ = 20;	// Extension type 20: server_certificate_type
	*ext16++ = 2;	// Total extension length: 2 bytes
	ext = (uint8_t*)ext16;
	*ext++ = 1;	// Certificate types length: 1 type
	*ext++ = 2;	// Type 2 = RawPublicKey ONLY [RFC 7250]
	
	ch->extensionsLen = ext - ch->extensions;
	
	// Length calculations
	uint32_t clientHelloLen = 2 + 32 + 1 + 0 + 2 + 2 + 1 + 1 + 2 + ch->extensionsLen;
	hh->type = CLIENT_HELLO;
	hh->length[0] = (clientHelloLen >> 16) & 0xFF;
	hh->length[1] = (clientHelloLen >> 8) & 0xFF;
	hh->length[2] = clientHelloLen & 0xFF;
	
	// Handshake
	tr->contentType = TLS_HANDSHAKE;
	// TLS 1.2
	tr->version = 0x0303;
	
	tr->length = 4 + clientHelloLen;
	
	cmn->size() = 1 + 2 + 2 + tr->length;
	
	printOnLog(Logger::LogLevel::INFO,
			"UWTLS",
			"sendClientHello::TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256 + RFC7250 RawPublicKey");
	sendDown(p);
}

void
UwTls::sendServerHello(int portnum, uint8_t ipaddr)
{
	Packet *p = Packet::alloc();
	
	hdr_cmn *cmn = HDR_CMN(p);
	hdr_uwudp *uwudp = HDR_UWUDP(p);
	hdr_uwip *uwiph = HDR_UWIP(p);

	cmn->uid() = 2;
	cmn->ptype() = PT_TLS_RECORD;
	cmn->direction() = hdr_cmn::UP;
	cmn->timestamp() = NOW;
	
	uwudp->dport() = portnum;
	uwiph->daddr() = ipaddr;
	
	hdr_tls_record *tr = HDR_TLS_RECORD(p);
	hdr_handshake *hh = HDR_HANDSHAKE(p);
	hdr_serverhello *sh = HDR_SERVERHELLO(p);
	
	// TLS 1.2
	sh->version = 0x0303;
	
	// Server random
	if (!randomBytes(server_random, 32)) {
		printOnLog(Logger::LogLevel::ERROR,
				"UWTLS",
				"sendServerHello::Failed to read from /dev/urand");
		state = HandshakeState::NOT_INITIALIZED;
		return;
	}
	memcpy(sh->random, server_random, 32);
	
	// Session ID: length (empty example)
	sh->sessionIdLen = 0;
	
	// Selected: TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256
	sh->selectedCipherSuite = 0xC02D;
	
	// Selected: null compression
	sh->selectedCompression = 0x00;
	
	uint8_t *ext = sh->extensions;
	uint16_t *ext16 = (uint16_t*)ext;
	
	// RFC 7250: RawPublicKey (NO X.509)
	*ext16++ = 19;	// Extension type 19: client_certificate_type
	*ext16++ = 2;	// Total extension length: 2 bytes
	ext = (uint8_t*)ext16;
	*ext++ = 1;	// Certificate types length: 1 type
	*ext++ = 2;	// Type 2 = RawPublicKey ONLY [RFC 7250]
	
	ext16 = (uint16_t*)ext;
	
	*ext16++ = 20;	// Extension type 20: server_certificate_type
	*ext16++ = 2;	// Total extension length: 2 bytes
	ext = (uint8_t*)ext16;
	*ext++ = 1;	// Certificate types length: 1 type
	*ext++ = 2;	// Type 2 = RawPublicKey ONLY [RFC 7250]
	
	sh->extensionsLen = ext - sh->extensions;
	
	// Length calculations (ServerHello: version+random+session+1cipher+1comp+extensions)
	uint32_t serverHelloLen = 2 + 32 + 1 + 0 + 2 + 1 + 2 + sh->extensionsLen;
	hh->type = SERVER_HELLO;
	hh->length[0] = (serverHelloLen >> 16) & 0xFF;
	hh->length[1] = (serverHelloLen >> 8) & 0xFF;
	hh->length[2] = serverHelloLen & 0xFF;
	
	// Handshake
	tr->contentType = TLS_HANDSHAKE;
	// TLS 1.2
	tr->version = 0x0303;
	
	tr->length = 4 + serverHelloLen;
	
	cmn->size() = 1 + 2 + 2 + tr->length;
	
	printOnLog(Logger::LogLevel::INFO,
			"UWTLS",
			"sendServerHello::TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256 + RFC7250 RawPublicKey");
	sendDown(p);
}

void
UwTls::sendClientKeyExchange(int portnum, uint8_t ipaddr)
{
	Packet *p = Packet::alloc();
	
	hdr_cmn *cmn = HDR_CMN(p);
	hdr_uwudp *uwudp = HDR_UWUDP(p);
	hdr_uwip *uwiph = HDR_UWIP(p);
	
	cmn->uid() = 3;
	cmn->ptype() = PT_TLS_RECORD;
	cmn->direction() = hdr_cmn::DOWN;
	cmn->timestamp() = NOW;
	
	uwudp->dport() = portnum;
	uwiph->daddr() = ipaddr;
	
	hdr_tls_record *tr = HDR_TLS_RECORD(p);
	hdr_handshake *hh = HDR_HANDSHAKE(p);
	// hdr_clientkeyexchange *cke = HDR_CLIENTKEYEXCHANGE(p);
	
	uint32_t ckeLen = 0;
	
	hh->type = CLIENT_KEY_EXCHANGE;
	hh->length[0] = (ckeLen >> 16) & 0xFF;
	hh->length[1] = (ckeLen >> 8) & 0xFF;
	hh->length[2] = ckeLen & 0xFF;
	
	tr->contentType = TLS_HANDSHAKE;
	tr->version = 0x0303;
	tr->length = 4 + ckeLen;
	
	cmn->size() = 1 + 2 + 2 + tr->length;

	if (!deriveStaticEcdhPremaster()) {
		printOnLog(Logger::LogLevel::ERROR,
				"UWTLS",
				"sendClientKeyExchange::Static ECDH premaster derivation failed");
		state = HandshakeState::ERROR;
		return;
	}
	
	printOnLog(Logger::LogLevel::INFO,
			"UWTLS",
			"sendClientKeyExchange::Empty message sent");
	
	sendDown(p);
}

void
UwTls::sendChangeCipherSpec(int portnum, uint8_t ipaddr)
{
	if (state == HandshakeState::ERROR) {
		stopRepeatedMessage();
		return;
	}
	
	Packet *p = Packet::alloc();
	
	hdr_cmn *cmn = HDR_CMN(p);
	hdr_uwudp *uwudp = HDR_UWUDP(p);
	hdr_uwip *uwiph = HDR_UWIP(p);

	cmn->uid() = (role == Role::CLIENT ? 4 : 5);
	cmn->ptype() = PT_TLS_RECORD;
	cmn->direction() = hdr_cmn::DOWN;
	cmn->timestamp() = NOW;
	
	uwudp->dport() = portnum;
	uwiph->daddr() = ipaddr;
	
	hdr_tls_record *tr = HDR_TLS_RECORD(p);
	hdr_changecipherspec *ccs = HDR_CHANGECIPHERSPEC(p);
	
	// ChangeCipherSpec: single byte 0x01
	ccs->type = 0x01;
	
	// Record layer
	tr->contentType = TLS_CHANGECIPHERSPEC;	// ChangeCipherSpec: 20
	tr->version = 0x0303;
	tr->length = 1;				// Single byte payload
	
	cmn->size() = 1 + 2 + 2 + tr->length;	// ContentType(1)+Version(2)+Length(2)+Payload(1)
	
	printOnLog(Logger::LogLevel::INFO,
			"UWTLS",
			"sendChangeCipherSpec::ChangeCipherSpec sent");
	sendDown(p);
}

void
UwTls::interpretClientHello(Packet *p)
{
	hdr_clienthello *ch = HDR_CLIENTHELLO(p);
	
	memcpy(client_random, ch->random, 32);
	
	int supportedEncryptions = 0;
	for (int i=0; i < ch->cipherSuitesLen; i++) {
		// TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256
		if (ch->cipherSuites[i] == 0xC02D) {
			supportedEncryptions++;
		}
	}
	
	if (supportedEncryptions == 0) {
		printOnLog(Logger::LogLevel::ERROR,
				"UWTLS",
				"interpretClientHello::No supported encryptions found");
		state = HandshakeState::NOT_INITIALIZED;
		return;
	}
	
	int supportedCompressions = 0;
	for (int i=0; i < ch->compressionLen; i++) {
		// Null compression
		if (ch->compression[i] == 0x00) {
			supportedCompressions++;
		}
	}
	
	if (supportedEncryptions == 0) {
		printOnLog(Logger::LogLevel::ERROR,
				"UWTLS",
				"interpretClientHello::No supported compressions found");
		state = HandshakeState::NOT_INITIALIZED;
		return;
	}
}

void
UwTls::interpretServerHello(Packet *p)
{
	hdr_serverhello *sh = HDR_SERVERHELLO(p);
	
	memcpy(server_random, sh->random, 32);
	
	// Check selected cipher suite
	if (sh->selectedCipherSuite != 0xC02D) {
		printOnLog(Logger::LogLevel::ERROR,
				"UWTLS",
				"interpretServerHello::Unsupported cipher suite selected: " + std::format("0x{:X}", sh->selectedCipherSuite));
		state = HandshakeState::NOT_INITIALIZED;
		return;
	}
	
	// Check selected compression method
	if (sh->selectedCompression != 0x00) {
		printOnLog(Logger::LogLevel::ERROR,
				"UWTLS",
				"interpretServerHello::Unsupported compression method selected: " + std::format("0x{:X}", sh->selectedCompression));
		state = HandshakeState::NOT_INITIALIZED;
		return;
	}
	
	printOnLog(Logger::LogLevel::INFO,
			"UWTLS",
			"interpretServerHello::Hello accepted with cipher " + std::format("0x{:X}", sh->selectedCipherSuite) + 
			" and compression " + std::format("0x{:X}", sh->selectedCompression));
}

void
UwTls::interpretClientKeyExchange(Packet *p)
{
	hdr_uwip *uwiph = HDR_UWIP(p);

	if (local_ec_privkey == nullptr) {
		printOnLog(Logger::LogLevel::ERROR,
				"UWTLS",
				"interpretClientKeyExchange::Local EC private key not available");
		state = HandshakeState::NOT_INITIALIZED;
		return;
	}

	if (remote_ec_pubkey == nullptr) {
		printOnLog(Logger::LogLevel::ERROR,
				"UWTLS",
				"interpretClientKeyExchange::Remote EC public key not available for peer " +
				std::to_string(uwiph->saddr()));
		state = HandshakeState::NOT_INITIALIZED;
		return;
	}

	if (!deriveStaticEcdhPremaster()) {
		printOnLog(Logger::LogLevel::ERROR,
				"UWTLS",
				"interpretClientKeyExchange::Static ECDH premaster derivation failed");
		state = HandshakeState::ERROR;
		return;
	}

	if (ecdh_premaster.empty()) {
		printOnLog(Logger::LogLevel::ERROR,
				"UWTLS",
				"interpretClientKeyExchange::Derived premaster is empty");
		state = HandshakeState::ERROR;
		return;
	}
}

bool
UwTls::deriveStaticEcdhPremaster()
{
	ecdh_premaster.clear();

	if (local_ec_privkey == nullptr || remote_ec_pubkey == nullptr) {
		printOnLog(Logger::LogLevel::ERROR,
				"UWTLS",
				"deriveStaticEcdhPremaster::Null local or remote key");
		return false;
	}

	EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(local_ec_privkey, nullptr);
	if (ctx == nullptr) {
		printOnLog(Logger::LogLevel::ERROR,
				"UWTLS",
				"deriveStaticEcdhPremaster::EVP_PKEY_CTX_new failed");
		return false;
	}

	bool ok = true;
	size_t premaster_len = 0;

	ok = ok && (EVP_PKEY_derive_init(ctx) == 1);
	ok = ok && (EVP_PKEY_derive_set_peer(ctx, remote_ec_pubkey) == 1);
	ok = ok && (EVP_PKEY_derive(ctx, nullptr, &premaster_len) == 1);

	if (!ok || premaster_len == 0) {
		printOnLog(Logger::LogLevel::ERROR,
				"UWTLS",
				"deriveStaticEcdhPremaster::Failed to determine premaster length");
		EVP_PKEY_CTX_free(ctx);
		return false;
	}

	ecdh_premaster.resize(premaster_len);

	if (EVP_PKEY_derive(ctx, ecdh_premaster.data(), &premaster_len) != 1) {
		printOnLog(Logger::LogLevel::ERROR,
				"UWTLS",
				"deriveStaticEcdhPremaster::EVP_PKEY_derive failed");
		EVP_PKEY_CTX_free(ctx);
		ecdh_premaster.clear();
		return false;
	}

	EVP_PKEY_CTX_free(ctx);

	if (premaster_len == 0) {
		printOnLog(Logger::LogLevel::ERROR,
				"UWTLS",
				"deriveStaticEcdhPremaster::Derived premaster is empty");
		ecdh_premaster.clear();
		return false;
	}

	if (premaster_len != ecdh_premaster.size()) {
		ecdh_premaster.resize(premaster_len);
	}

	printOnLog(Logger::LogLevel::INFO,
			"UWTLS",
			"deriveStaticEcdhPremaster::Static ECDH premaster derived successfully");

	return true;
}

void
UwTls::sendBufferedPackets()
{
	while (!waitingQueue.empty()) {
		Packet *p = waitingQueue.front();
		waitingQueue.pop();
		
		hdr_cmn *cmn = HDR_CMN(p);
		
		if (cmn->direction() == hdr_cmn::DOWN) {
			symmetricEncrypt(p);
			sendDown(p);
		} else {
			symmetricDecrypt(p);
			sendUp(p);
		}
	}
}
