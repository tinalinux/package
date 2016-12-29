/*
 * \file        example.c
 * \brief       just an example.
 *
 * \version     1.0.0
 * \date        2012年05月31日
 * \author      James Deng <csjamesdeng@allwinnertech.com>
 *
 * Copyright (c) 2012 Allwinner Technology. All Rights Reserved.
 *
 */

/* include system header */
#include <string.h>
#include "script.h"
#include <libdragon/test_case.h>
#include <libdragon/dragonboard.h>

/*
 ** argv[0] name
 ** argv[1] version
 ** argv[2] shmid
 ** argv[3] test id
 **/

/* C entry.
 *
 * \param argc the number of arguments.
 * \param argv the arguments.
 *
 * DO NOT CHANGES THE NAME OF PARAMETERS, otherwise your program will get
 * a compile error if you are using INIT_CMD_PIPE macro.
 */
int main(int argc, char *argv[])
{
	char platform[64];

    /* init cmd pipe after local variable declaration */
	printf("enter example!\n");
    INIT_CMD_PIPE();

    init_script(atoi(argv[2]));
    if(script_fetch("EXAMPLEC", "platform", (int *)platform, sizeof(platform)/sizeof(int)) < 0){
		printf("get example platform error\n");
		SEND_CMD_PIPE_FAIL_EX("EXAMPLE TEST ERROR");
	}else {
	printf("example platform:%s\n", platform);
		SEND_CMD_PIPE_OK_EX("EXAMPLE TEST INFO");
	}
	deinit_script();
	EXIT_CMD_PIPE();
    return 0;
}
