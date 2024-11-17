#include<tclcl.h>

extern EmbeddedTcl PackerUwRangingTdoaTclCode;

extern "C" int Packeruwrangingtdoa_Init() {
	PackerUwRangingTdoaTclCode.load();
	return 0;
}
