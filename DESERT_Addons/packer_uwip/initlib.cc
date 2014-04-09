#include<tclcl.h>

extern EmbeddedTcl PackerUwipTclCode;

extern "C" int Packeruwip_Init() {
	PackerUwipTclCode.load();
	return 0;
}
