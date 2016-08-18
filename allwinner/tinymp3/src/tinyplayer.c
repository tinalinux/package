#include <stdio.h>
#include <mp3player.h>

/*
 * argv[1]: Mp3 file path
 * argv[2]: Speaker Function only for sunxicodec
 * argv[3]: headphone volume control volume only for sunxicodec
 */
int main(int argc, char *argv[])
{
    if(argc == 3){
        char* set1[30]={
            "name=Speaker Function",
            "1"
        };
        cset("default",2,set1,0,0);
        char* argv1[30]={
            "name=headphone volume control",
            argv[2]
        };
        cset("default",2,argv1,0,0);
    }

	tinymp3_play(argv[1]);

	return 0;
}
