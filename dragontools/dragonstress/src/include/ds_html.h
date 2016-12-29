#ifndef _DS_HTML_H_
#define _DE_HTML_H_
#ifdef __cplusplus
extern "C"{
#endif
#include "html.h"
#ifdef __cplusplus
}
#endif
#include "dragonstress.h"



struct ds_html {
	struct hnode *root;
	struct hparse *parse;
	struct hwrite *writer;
	struct hvalid *validator;
	struct ioout out;
};


struct hnode *ds_get_hnode(struct hnode *root, const char *id);
void module_table_script_init(struct hnode *head, unsigned int num, char *list[]);
void module_table_add_tr(struct hnode *table, unsigned int num, char *td_list[]);

struct hnode *ds_html_open(struct ds_html *ds, const char *conf_file);
int ds_html_init(struct ds_html *ds, struct dragon_stress_data *data, const char *result_path);
int ds_html_write(struct ds_html *ds);
void ds_html_close(struct ds_html *ds);

int ds_modify_chars(struct hnode *root, const char *id, const char *text);
int ds_html_module_table_add(struct hnode *root, const char *table_id, unsigned int round, const char *info, const char *result);
int ds_html_module_table_add_log(struct hnode *root, const char *table_id, const char *log_path);
int ds_html_set_test_result(struct hnode *root, int result, const char *name);

#endif //_DS_HTML_H_
