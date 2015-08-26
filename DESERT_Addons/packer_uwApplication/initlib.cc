#include<tclcl.h>

extern EmbeddedTcl PackerUwApplicationTclCode;

extern "C" int Packeruwapplication_Init() {
	PackerUwApplicationTclCode.load();
	return 0;
}
