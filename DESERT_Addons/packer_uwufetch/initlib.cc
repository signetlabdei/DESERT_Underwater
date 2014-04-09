#include<tclcl.h>

extern EmbeddedTcl PackerUwUFetchTclCode;

extern "C" int Packeruwufetch_Init() {
	PackerUwUFetchTclCode.load();
	return 0;
}
