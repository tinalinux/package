#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libdragonst/awserdev.h>
#include <libdragonst/cppstring.h>
#include <libdragonst/GeneralFuncs.h>

#include <libdragon/dragonboard.h>
#include <libdragon/test_case.h>
#include <libdragon/core.h>

int main(int argc, char *argv[])
{
    int ret;
    int shm;
    int test_count;
    char exdate[64];
    struct testcase_base_info *info;

    if (argc < 4) {
        db_warn("Missing param.\n\n");
        return -1;
    }

    char *pMsgId = (char *)argv[1];
    char *pAuthor = (char *)argv[2];

    ret = waiting_for_bootcompeled(10);
    if (ret < 0) {
        db_warn("system booting timeout\n");
        return -1;
    }

    shm = init_script("/boot-res/test_config.fex");
    info = init_testcases();

    //n = get_manual_testcases_number();
    //n = get_auto_testcases_number();
    test_count = get_testcases_number();


    char sz_error[4096];
    AWSERDEVHANDLE hAwsPlugin = AwsInitSerDevLibrary(sz_error, 4096);

    if (hAwsPlugin == NULL)
    {
        db_warn("%s\n\n", sz_error);
        return -1;
    }
    const char *sz_items[64] = {0};
    for (int i = 0; i < test_count; i++) {
        sz_items[i] = info[i].display_name;
    }

    AwsTestBegin(pMsgId, (char**)sz_items, test_count);
    db_msg("test begin\n");

    int iResponse;
    int iPassed;
    for (int i = 0; i < test_count; i++) {
        iPassed = 0;
        AwsTestBeginItem(i, NULL);
        if (info[i].binary[0] == '\0') {
            //test_failed(info[i].id);
            continue;
        } else {
            iPassed = run_one_testcase(DRAGONBOARD_VERSION, exdate, shm, &info[i]);
            if (info[i].category == CATEGORY_MANUAL) {
                AwsTestResponse(i, 120000, info[i].name, &iResponse, sz_error);
                iPassed = iResponse;
            }
        }
        AwsTestFinishItem(i, iPassed, 0, exdate);
    }

    AwsTestFinish(pAuthor, true, 0, NULL);
    AwsReleaseSerDevLibrary(&hAwsPlugin);

    return 0;
}
