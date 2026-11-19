#include <tclcl.h>

extern EmbeddedTcl PackerUwTlsTclCode;

extern "C" int
Packeruwtls_Init()
{
	PackerUwTlsTclCode.load();
	return 0;
}
