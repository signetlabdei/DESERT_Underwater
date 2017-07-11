#include<tclcl.h>

extern EmbeddedTcl PackerUwcsmacaTclCode;

extern "C" int Packeruwcsmaca_Init() {
	PackerUwcsmacaTclCode.load();
	return 0;
}