#!/bin/sh

source send_cmd_pipe.sh
source script_parser_stress.sh

platform=`script_fetch "EXAMPLESH" "platform"`


SEND_CMD_PIPE_OK_EX $3 "example print platform:$platform"
#SEND_CMD_PIPE_FAIL_EX $3 "example get platform error"

