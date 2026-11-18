#include <module.h>
#include <tclcl.h>

#include "tls-record-header.h"
#include "handshake-header.h"
#include "clienthello-header.h"
#include "serverhello-header.h"
#include "clientkeyexchange-header.h"
#include "changecipherspec-header.h"

int hdr_tls_record::offset_;
int hdr_handshake::offset_;
int hdr_changecipherspec::offset_;
int hdr_clienthello::offset_;
int hdr_serverhello::offset_;
int hdr_clientkeyexchange::offset_;

packet_t PT_TLS_RECORD;
packet_t PT_HANDSHAKE;
packet_t PT_CLIENTHELLO;
packet_t PT_SERVERHELLO;
packet_t PT_CLIENTKEYEXCHANGE;
packet_t PT_CHANGECIPHERSPEC;

extern EmbeddedTcl UwTlsTclCode;

extern "C" int
Uwtls_Init()
{
	PT_TLS_RECORD = p_info::addPacket("TLSRecord");
	PT_HANDSHAKE = p_info::addPacket("TLSHandshake");
	PT_CLIENTHELLO = p_info::addPacket("TLSClienthello");
	PT_SERVERHELLO = p_info::addPacket("TLSServerhello");
	PT_CLIENTKEYEXCHANGE = p_info::addPacket("TLSClientKeyExchange");
	PT_CHANGECIPHERSPEC = p_info::addPacket("TLSChangeCipherSpec");
	UwTlsTclCode.load();
	return 0;
}
