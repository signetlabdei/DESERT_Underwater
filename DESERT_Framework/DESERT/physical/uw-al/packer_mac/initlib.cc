#include<tclcl.h>

extern EmbeddedTcl PackerMacTclCode;

extern "C" int Packermac_Init() {
	PackerMacTclCode.load();
	return 0;
}
