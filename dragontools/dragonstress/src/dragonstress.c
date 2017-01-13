#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include "dragonstress.h"
#include "ds_html.h"

static struct dragon_stress_data data;
extern char gDSpath[];
extern char gDSdir[];

void handle_result(struct ds_html *ds)
{
    struct dragon_stress_data *dt = &data;
    struct testcase_base_info *info = dt->info;
    FILE *from_child;
    char *exdata;
    int exdata_len; char buffer[128],buf[64];
    char *test_case_id_s;
    int test_case_id;
    char *result_s;
    int result;
	struct hnode *root = ds->root;
	static unsigned int ROUNDS = 0;

    /* listening to child process */
    ds_msg("listening to child process, starting...");

	ROUNDS++;
	from_child = fopen(CMD_PIPE_NAME, "w+e");
	ds_dump("fopen return %p\n", from_child);
	if (from_child == NULL){
		ds_error("finish cmd pipe open failed");
		return;
	}
    while (1) {
		ds_dump("In handle_result loop!\n");
		memset(buffer, 0, sizeof(buffer));
        if (fgets(buffer, sizeof(buffer), from_child) == NULL) {
            //fclose(from_child);
            continue;
        }

        ds_dump("command from child: %s, len=%d", buffer, strlen(buffer));

        /* test completion */
        test_case_id_s = strtok(buffer, "' '\n");
        ds_dump("test case id #%s", test_case_id_s);
        if (strcmp(buffer, TEST_COMPLETION) == 0) {
			printf("done!\n");
            //fclose(from_child);
            break;
        }

        if (test_case_id_s == NULL)
            continue;
        test_case_id = atoi(test_case_id_s);

        result_s = strtok(NULL, "' '\n");
        ds_dump("result: %s", result_s);
        if (result_s == NULL) {
            //fclose(from_child);
            continue;
        }
        result = atoi(result_s);

        exdata = strtok(NULL, "\n");

        if (exdata != NULL) {
            ds_msg("exdata: %s", exdata);
            //strcpy(dt->exdata[test_case_id], exdata);
        }
        ds_dump("%s TEST %s", info[test_case_id].name,
                (result == 1) ? "OK" : "Fail");
        dt->test_status[test_case_id] = (result == 1 ? TEST_STATUS_PASS : TEST_STATUS_FAIL);


		if(root) {
			snprintf(buf, sizeof(buf), "\"%sTABLE\"", info[test_case_id].display_name);
			ds_html_module_table_add(root, buf, ROUNDS, exdata, (result == 1) ? "OK" : "Fail");
		}

    }
	fclose(from_child);
	ds_dump("close read pipe!");

}
void *running_testcases(void *data)
{
	struct dragon_stress_data *dt = (struct dragon_stress_data *)data;
	struct testcase_base_info *info = dt->info;
	for (int i = 0; i < dt->test_count; i++) {
		if (info[i].binary[0] == '\0') {
			ds_msg("can not exec testcase: %s", info[i].display_name);
			continue;
		}
		run_one_testcase(DRAGONSTRESS_VERSION, NULL,  dt->shm, &info[i]);
		dt->test_status[i] = TEST_STATUS_WAIT;
	}
	wait_for_test_completion();
}
#define DEFAULT_ROUND 10
#define DEFAULT_RESULTPATH "/mnt/UDISK"

int main()
{
	struct ds_html ds;
	int i,j;
	int round;
	char buf[256];
	char result_path[256];

	data.shm = init_script("/etc/dragonstress_config.fex");
	data.info = init_testcases();
	data.test_manual = get_manual_testcases_number();
	data.test_auto = get_auto_testcases_number();
	data.test_count = get_testcases_number();
	data.test_status = (int *)malloc(sizeof(int) * data.test_count);
	data.fail_module = (int *)malloc(sizeof(int) * (data.test_count+sizeof(int)-1)/sizeof(int));

	ds_debug("manual:%d, auto:%d, count%d\n", data.test_manual, data.test_auto, data.test_count);
	if(script_fetch("DRAGONSTRESS", "round", &round, 1) < 0)
		round = DEFAULT_ROUND;
	if(script_fetch("DRAGONSTRESS", "result_path", (int *)result_path, sizeof(result_path)/sizeof(int)) < 0)
		strcpy(result_path, DEFAULT_RESULTPATH);

	if (data.test_status == NULL) {
		ds_error("data->test_status malloc failed");
		exit(0);
	}

	if(!ds_html_open(&ds, "/etc/dragonstress.html")) {
        printf("ds html open error!\n");
		exit(0);
    }

	ds_html_init(&ds, &data, result_path);
	for(j=0; j<round; j++) {
		printf("======%d round======\n", j+1);
		pthread_create(&data.tid_running, NULL, running_testcases, (void *)&data);
		handle_result(&ds);
		for(i=0; i<data.test_count; i++) {
			memset(result_path, 0, sizeof(result_path));
			if(0 == script_fetch(data.info[i].name, "outlogname", (int *)result_path, sizeof(result_path)/sizeof(int))) {
				snprintf(buf, sizeof(buf), "echo ==========%d round========== >> %s/%s", j+1, gDSpath, result_path);
				system(buf);
				snprintf(buf, sizeof(buf), "cat /tmp/%s >> %s/%s", result_path, gDSpath, result_path);
				system(buf);
				snprintf(buf, sizeof(buf), "rm /tmp/%s", result_path);
				system(buf);
			}
			if(0 == script_fetch(data.info[i].name, "errlogname", (int *)result_path, sizeof(result_path)/sizeof(int))) {
				snprintf(buf, sizeof(buf), "echo ==========%d round========== >> %s/%s", j+1, gDSpath, result_path);
				system(buf);
				snprintf(buf, sizeof(buf), "cat /tmp/%s >> %s/%s", result_path, gDSpath, result_path);
				system(buf);
				snprintf(buf, sizeof(buf), "rm /tmp/%s", result_path);
				system(buf);
			}
			if(data.test_status[i] != TEST_STATUS_PASS &&
				(data.fail_module[(i+sizeof(int)-1)/sizeof(int)]&
				(1<<(i%32))) == 0) {
				//add fail module
				ds_html_set_test_result(ds.root, data.test_status[i], data.info[i].display_name);
				data.fail_module[(i+sizeof(int)-1)/sizeof(int)] |= 1<<(i%32);
				break;
			}
		}
		snprintf(buf, sizeof(buf), "%u", j+1);
		ds_modify_chars(ds.root, "ROUNDS", buf);
	}
	for(i=0; i<data.test_count; i++) {
		if(0 == script_fetch(data.info[i].name, "outlogname", (int *)result_path, sizeof(result_path)/sizeof(int))) {
			snprintf(buf, sizeof(buf), "\"%sTABLE\"", data.info[i].display_name);
			ds_html_module_table_add_log(ds.root, buf, result_path);
		}
		if(0 == script_fetch(data.info[i].name, "errlogname", (int *)result_path, sizeof(result_path)/sizeof(int))) {
			snprintf(buf, sizeof(buf), "\"%sTABLE\"", data.info[i].display_name);
			ds_html_module_table_add_log(ds.root, buf, result_path);
		}
	}
	printf("exit dragonstress!\n");
	ds_html_write(&ds);
	ds_html_close(&ds);
	snprintf(result_path, sizeof(result_path), "tar cf %s/../DragonStress.tar -C %s/.. %s", gDSpath, gDSpath, gDSdir);
	system(result_path);
}
