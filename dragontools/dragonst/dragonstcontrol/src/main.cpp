#include <string.h>
#include "awstcrl.h"
#include "GenSerialPortStream.h"

#define CFG_XML_FILE_PATH "/home/awtestcontrol_cfg.xml"

static AwsNP::AwsOStream g_out;

//       0          1      2        3        4          5           6
// awtestcontrol -msgid msgidnum [Option] [Param] [Sub Option] [Sub Param]...
int main(int argc, char* argv[])
{
    if (argc < 4)
    {
        g_out << "Missing arguments\n\n";
        return 1;
    }

    AwsNP::Awtestcontrol awsTestControl;

    if (!awsTestControl.loadCfg(CFG_XML_FILE_PATH))
    {
        g_out << "Failed to load " << CFG_XML_FILE_PATH << "\n\n";
        //return 2;

        g_out << "Set default cfg...\n\n";
    }

    AwsNP::AwsOStream* pOStream;
    AwsNP::GenSerialPortStream gsp;

    if (awsTestControl.getCfg()->strSerialPortName == "__std")
    {
        pOStream = &g_out;
    }
    else
    {
        if (!gsp.init(awsTestControl.getCfg()->strSerialPortName.c_str()))
        {
            g_out << "Failed to open "
                 << awsTestControl.getCfg()->strSerialPortName
                 << "\n\n";

            return 3;
        }

        pOStream = &gsp;
    }

    CppString strError;
    int iRet;

    g_out << "begin command End\n\n";

    awsTestControl.setOstream(*pOStream);
    iRet = awsTestControl.command(argv, argc, &strError);

    if (iRet != 0)
    {
        g_out << strError << "  " << iRet << "\n\n";
    }

    g_out << "awstestcontrol End\n\n";

    return 0;
}
