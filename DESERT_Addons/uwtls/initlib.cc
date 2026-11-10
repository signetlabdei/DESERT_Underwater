#include <module.h>
#include <tclcl.h>

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
