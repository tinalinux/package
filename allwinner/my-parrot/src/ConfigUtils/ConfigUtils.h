#ifndef __CONFIG_UITLS_H__
#define __CONFIG_UITLS_H__

namespace Parrot{

class ConfigUtils
{
public:
    static const int SmartlinkdRespBroadcastPort = 20000;
    static const int DecoveryBroadcastPort = 20000;
    static void init();
    static char *getMac();
    static char *getUUID();
    static char *getBTMacConfig();
    static char *getBTNameConfig();

};
}
#endif /*__CONFIG_UITLS_H__*/