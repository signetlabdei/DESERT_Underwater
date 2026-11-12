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
 * @file uwtls-module.h
 * @author Davide Costa
 * \version 1.0.0
 * \brief  Header of the class responsible for the Transport Layer Security.
 */

#ifndef UWTLS_MODULE_H
#define UWTLS_MODULE_H

#include "packer.h"
#include "uwip-module.h"
#include "uwudp-module.h"
#include "uwApplication_module.h"

#include "tls-record-header.h"
#include "handshake-header.h"
#include "clienthello-header.h"
#include "serverhello-header.h"
#include "clientkeyexchange-header.h"
#include "changecipherspec-header.h"

#include "tls-prf.h"

#include <module.h>

#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bio.h>

#include <timer-handler.h>

#define TLS_HANDSHAKE         22
#define TLS_CHANGECIPHERSPEC  20

#define CLIENT_HELLO          1
#define SERVER_HELLO          2
#define CLIENT_KEY_EXCHANGE   16

/**
 * Flight C1 (client): ClientHello
 * Flight S1 (server): ServerHello
 * Flight C2 (client): ClientKeyExchange + ChangeCipherSpec
 * Flight S2 (server): ChangeCipherSpec
 */

enum class HandshakeState {
    NOT_INITIALIZED,
    WAIT_SERVER_HELLO,
    WAIT_CLIENT_KEY_EXCHANGE,
    WAIT_CHANGE_CIPHER_SPEC,
    WAIT_FINISHED,
    ESTABLISHED,
    ERROR
};

enum class Role {
    UNKNOWN,
    CLIENT,
    SERVER
};

class UwTls;

class UwTlsRetransmissionTimer : public TimerHandler
{
public:
	explicit UwTlsRetransmissionTimer(UwTls *m) : TimerHandler(), module_(m) {}
protected:
	void expire(Event *e) override;
private:
	UwTls *module_;
};


class UwTls : public Module
{
public:
	/**
	 * Constructor of UwTls class.
	 */
	UwTls();

	/**
	 * Destructor of UwTls class.
	 */
	virtual ~UwTls();

	/**
	 * TCL command interpreter. It implements the following OTCL methods:
	 *
	 * @param argc Number of arguments in <i>argv</i>.
	 * @param argv Array of strings which are the command parameters
	 * @return TCL_OK or TCL_ERROR whether the command has been dispatched
	 *successfully or not.
	 *
	 **/
	virtual int command(int argc, const char *const *argv) override;

	/**
	 * Performs the reception of packets from upper and lower layers.
	 *
	 * @param Packet* Pointer to the packet will be received.
	 */
	virtual void recv(Packet *) override;

	/**
	 * Performs the reception of packets from upper and lower layers.
	 *
	 * @param Packet* Pointer to the packet will be received.
	 * @param Handler* Handler.
	 */
	virtual void recv(Packet *, int) override;

	/**
	 * Sends the current flight loaded in the module instance
	 */
	void sendCurrentFlight();

protected:
	/**
	 * Loads a private key
	 *
	 * @param const char* String containing a path to the PEM key file.
	 * @return Pointer to OpenSSL key.
	 */
	EVP_PKEY * loadPrivateKeyFromPemFile(const char *path);

	/**
	 * Loads a public key
	 *
	 * @param const char* String containing a path to the PEM key file.
	 * @return Pointer to OpenSSL key.
	 */
	EVP_PKEY * loadPublicKeyFromPemFile(const char *path);

	/**
	 * Handles the TLS Record packet type
	 *
	 * @param Packet* Pointer to the packet to analyze.
	 */
	void handleRecord(Packet *p);

	/**
	 * Handles the TLS Handshake packet type
	 *
	 * @param Packet* Pointer to the packet to analyze.
	 */
	void handleHandshake(Packet *p);

	/**
	 * Handles the TLS ChangeCipherSpec message
	 *
	 * @param Packet* Pointer to the packet to interpret.
	 */
	void handleChangeCipherSpec(Packet *p);

	/**
	 * Applies the TLS ChangeCipherSpec received
	 *
	 * @param Packet* Pointer to the packet to interpret.
	 */
	void applyChangeCipherSpec(Packet *p);

	/**
	 * Encrypt the application payload of a message
	 *
	 * @param Packet* Pointer to the packet to encrypt.
	 */
	void symmetricEncrypt(Packet *p);

	/**
	 * Dencrypt the application payload of a message
	 *
	 * @param Packet* Pointer to the packet to decrypt.
	 */
	void symmetricDecrypt(Packet *p);

	/**
	 * Fill a variable with cryptographically secure random bytes
	 *
	 * @param uint8_t *data Pointer to the data variable.
	 * @param size_t size Size of the data in bytes.
	 * @return True if generation succeded, false otherwise.
	 */
	bool randomBytes(uint8_t *data, size_t size);
	
	/**
	 * Fill a variable with cryptographically secure random bytes
	 *
	 * @param uint8_t rsa_premaster[48] Decrypted RSA premaster.
	 * @param uint8_t* psk The Pre-Shared Key.
	 * @param size_t psk_len The Pre-Shared Key length.
	 */
	void buildRsaPskPremaster(const uint8_t rsa_premaster[48], const uint8_t* psk, size_t psk_len);

	/**
	 * Sends a periodic generic message
	 *
	 * @param func Function to invoke.
	 * @param int Port number of the destination.
	 * @param uint8_t Address of the destination.
	 */
	void sendRepeatedMessage(void (UwTls::*func)(int, uint8_t), int portnum, uint8_t ipaddr);

	/**
	 * Stops the current flight loaded in the module instance
	 */
	void stopRepeatedMessage();

	/**
	 * Sends the client hello TLS message
	 *
	 * @param int Port number of the destination.
	 * @param uint8_t Address of the destination.
	 */
	void sendClientHello(int portnum, uint8_t ipaddr);

	/**
	 * Sends the server hello TLS message
	 *
	 * @param int Port number of the destination.
	 * @param uint8_t Address of the destination.
	 */
	void sendServerHello(int portnum, uint8_t ipaddr);

	/**
	 * Sends the ClientKeyExchange TLS message
	 *
	 * @param int Port number of the destination.
	 * @param uint8_t Address of the destination.
	 */
	void sendClientKeyExchange(int portnum, uint8_t ipaddr);

	/**
	 * Sends the ChangeCipherSpec TLS message
	 *
	 * @param int Port number of the destination.
	 * @param uint8_t Address of the destination.
	 */
	void sendChangeCipherSpec(int portnum, uint8_t ipaddr);

	/**
	 * Interprets the client hello TLS message
	 *
	 * @param Packet* Pointer to the packet to interpret.
	 */
	void interpretClientHello(Packet *p);

	/**
	 * Interprets the client key exchange TLS message
	 *
	 * @param Packet* Pointer to the packet to interpret.
	 */
	void interpretClientKeyExchange(Packet *p);

	/**
	 * Interprets the server hello TLS message
	 *
	 * @param Packet* Pointer to the packet to interpret.
	 */
	void interpretServerHello(Packet *p);
	
	/**
	 * Derives the premaster secret from the public-private key combination
	 */
	bool deriveStaticEcdhPremaster();

	/**
	 * Send the packets generated before TLS handshake completion
	 */
	void sendBufferedPackets();
	
	/**
	 * Helper function to send Flight C2
	 */
	inline void sendFlightC2(int portnum, uint8_t ipaddr)
	{
		sendClientKeyExchange(portnum, ipaddr);
		sendChangeCipherSpec(portnum, ipaddr);
	}
	
	/**
	 * Helper function to prevend endian mismatches
	 */
	inline void writeUint64BE(unsigned char out[8], uint64_t v)
	{
		out[0] = (v >> 56) & 0xFF;
		out[1] = (v >> 48) & 0xFF;
		out[2] = (v >> 40) & 0xFF;
		out[3] = (v >> 32) & 0xFF;
		out[4] = (v >> 24) & 0xFF;
		out[5] = (v >> 16) & 0xFF;
		out[6] = (v >> 8) & 0xFF;
		out[7] = v & 0xFF;
	}

	HandshakeState state;
	uint8_t tls_status;
	
	std::queue<Packet *> waitingQueue;
	std::optional<Packet *> bufferedChangeCipherSpec;
	
	// Asymmetric material
	uint8_t client_random[32];
	uint8_t server_random[32];
	
	std::vector<uint8_t> ecdh_premaster;
	
	EVP_PKEY *local_ec_privkey;
	EVP_PKEY *remote_ec_pubkey;
	
	// Symmetric material
	uint8_t client_write_key[16];
	uint8_t server_write_key[16];
	uint8_t client_write_iv[4];
	uint8_t server_write_iv[4];
	
	Role role;
	
	double retransmit_timeout;     // In seconds
	int max_retransmissions;
	int retransmissions_done;
	
	// Info to resend the current flight
	int flight_port;
	uint8_t flight_ip;
	void (UwTls::*flight_func)(int, uint8_t);

	double current_timeout;
	bool flight_active;
	
	UwTlsRetransmissionTimer retransmission_timer_;
	
	// Unused variable - here to prevent a warning
	int debug_;
};

#endif // UWTLS_MODULE_H
