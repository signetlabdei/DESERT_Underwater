#include<tclcl.h>

extern EmbeddedTcl PackerCommonTclCode;

extern "C" int Packercommon_Init() {
	PackerCommonTclCode.load();
	return 0;
}
