#include<tclcl.h>

extern EmbeddedTcl UwalTclCode;

extern "C" int Uwal_Init() {
	UwalTclCode.load();
	return 0;
}
