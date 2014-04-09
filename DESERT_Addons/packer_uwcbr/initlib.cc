#include<tclcl.h>

extern EmbeddedTcl PackerUwcbrTclCode;

extern "C" int Packeruwcbr_Init() {
	PackerUwcbrTclCode.load();
	return 0;
}
