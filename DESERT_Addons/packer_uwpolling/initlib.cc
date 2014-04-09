#include<tclcl.h>

extern EmbeddedTcl PackerUwpollingTclCode;

extern "C" int Packeruwpolling_Init() {
	PackerUwpollingTclCode.load();
	return 0;
}