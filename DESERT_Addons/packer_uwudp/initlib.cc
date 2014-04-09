#include<tclcl.h>

extern EmbeddedTcl PackerUwudpTclCode;

extern "C" int Packeruwudp_Init() {
	PackerUwudpTclCode.load();
	return 0;
}
