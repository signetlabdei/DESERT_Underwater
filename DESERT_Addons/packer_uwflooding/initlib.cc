#include <tclcl.h>

extern EmbeddedTcl PackerUwfloodingTclCode;

extern "C" int Packeruwflooding_Init() {
    PackerUwfloodingTclCode.load();
    return 0;
}

extern "C" int Cyguwip_Init() {
    Packeruwflooding_Init();
    return 0;
}
