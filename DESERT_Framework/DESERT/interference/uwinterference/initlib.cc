#include <tclcl.h>

extern EmbeddedTcl UwInterferenceInitTclCode;

extern "C" int Uwinterference_Init()
{
    UwInterferenceInitTclCode.load();
    return 0;
}
