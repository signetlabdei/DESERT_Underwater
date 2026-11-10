PacketHeaderManager set tab_(PacketHeader/UWTLSRecord) 1
PacketHeaderManager set tab_(PacketHeader/UWTLSHandshake) 1
PacketHeaderManager set tab_(PacketHeader/UWTLSClienthello) 1
PacketHeaderManager set tab_(PacketHeader/UWTLSServerhello) 1
PacketHeaderManager set tab_(PacketHeader/UWTLSClientKeyExchange) 1
PacketHeaderManager set tab_(PacketHeader/UWTLSChangeCipherSpec) 1

Module/UW/TLS set debug_ 0
Module/UW/TLS set retransmit_timeout_ 10.0
Module/UW/TLS set max_retransmissions_ 4
