#include <sys/stat.h>
#include <sys/types.h>

#include <assert.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ds_html.h"

#ifndef BOARD
#define BOARD "Unknow"
#endif

char gDSpath[128];
char gDSdir[128];

void module_table_script_init(struct hnode *root, unsigned int num, struct testcase_base_info *info)
{
    struct hnode *head,*tmp;
	char buf[256];
    unsigned int i;

	head = ds_get_hnode(root, "HEAD");
	if(!head)
		return;
    tmp = hnode_alloc_elem(HELEM_SCRIPT, 1, 1);
    hnode_addchild(tmp, hnode_alloc_chars("window.onload=function(){\n", 1, 1));
    for(i=0;i<num;i++) {
		snprintf(buf, sizeof(buf), "AltRowsColor('%sTABLE');\n", info[i].display_name);
        hnode_addchild(tmp, hnode_alloc_chars(buf, 1, 1));
    }
    hnode_addchild(tmp, hnode_alloc_chars("}\n", 1, 1));
    hnode_addchild(head, tmp);
}

static inline struct hnode *ds_detect_id(struct hnode *node, const char *id)
{
	struct hattr *attr;
	if(!node || !id)
		return NULL;
	attr = hnode_attr(node);
	//printf("attr:  type-%d,value:%s\n", hattr_type(attr), hattr_value(attr));
	//printf("type:%d,id:%s\n", HATTR_ID, id);
	if(attr!=NULL && HATTR_ID == hattr_type(attr) &&
	!strncmp((char *)id, hattr_value(attr), strlen(id))) {
		//printf("found! %p, type-%d,value:%s[node line:%d]\n", node, hattr_type(attr), hattr_value(attr), hnode_line(node));
		return node;
		}
	return NULL;
}

static inline struct hnode *ds_search_child_node(struct hnode *node, const char *id);
static inline struct hnode *ds_search_sibling_node(struct hnode *node, const char *id)
{
	struct hnode *next, *tmp;
	next = hnode_sibling(node);
	while(next) {
		//printf("sibling=%p, line:%d\n", tmp, hnode_line(tmp));
		if(ds_detect_id(next, id))
			return next;
		else {
			tmp = ds_search_child_node(next, id);
			if(tmp)
				return tmp;
			else
				next = hnode_sibling(next);
		}
	}
	return NULL;

}
static inline struct hnode *ds_search_child_node(struct hnode *node, const char *id)
{
	struct hnode *next,*tmp;
	next = hnode_child(node);
	while(next) {
		//printf("node=%p, line:%d\n", tmp, hnode_line(tmp));
		if(ds_detect_id(next, id))
			return next;
		else {
			tmp = ds_search_sibling_node(next, id);
			if(tmp)
				return tmp;
			else
				next = hnode_child(next);
		}
	}
	return NULL;
}

struct hnode *ds_get_hnode(struct hnode *root, const char *id)
{
	if(!root || !id)
		return NULL;
	return ds_search_child_node(root, id);
}



//Warpper
#if 0
int ds_modify_attr(struct hnode *root, struct hattr *attr, char *id)
{
	struct hnode *node;
	if(!root || !id || !attr)
		return -1;
	node = ds_get_hnode(root, id);
	if(!node){
		printf("can't not find id:[%s]\n", id);
		return -1;
	}


}
#endif
int ds_modify_chars(struct hnode *root, const char *id, const char *text)
{
	struct hnode *node,*tmp,*next;
	if(!root || !id || !text)
		return -1;
	node = ds_get_hnode(root, id);
	if(!node){
		printf("can't not find id:[%s]\n", id);
		return -1;
	}
	//hnode_delete(hnode_child(node));
	//hnode_addchild(node, hnode_alloc_chars(text, 1, 1));
	tmp = hnode_child(node);
	//printf("tmp=%p,line:%dtype:%d,text:%s\n", tmp,hnode_line(tmp),hnode_type(tmp), hnode_text(tmp));
	next = hnode_sibling(tmp);
	if(next) {
		struct hnode *clone;
		//printf("has next=%p!,line:%dtype:%d\n", next,hnode_line(next),hnode_type(next));
		clone = hnode_clone(next);
		hnode_delete(tmp);
		hnode_delete(next);
		tmp = hnode_alloc_chars(text, 1, 1);
		hnode_addchild(node, tmp);
		hnode_addchild(node, clone);
	}else {
		hnode_delete(tmp);
		hnode_addchild(node, hnode_alloc_chars(text, 1, 1));
	}
	return 0;
}

static int add_navigation(struct hnode *node, const char *name)
{
	struct hnode *tmp;
	char buf[256];

	if(!node || !name)
		return -1;
	snprintf(buf, sizeof(buf), "\"#%s_MODULE\"", name);
	tmp = hnode_alloc_elem(HELEM_A, 1, 1);
	hnode_addattr(tmp,hattr_alloc_chars(HATTR_ID, "\"link\"", 0, 1, 1));
	hnode_addattr(tmp,hattr_alloc_chars(HATTR_CLASS, "\"nav_item\"", 0, 1, 1));
	hnode_addattr(tmp,hattr_alloc_chars(HATTR_HREF, buf, 0, 1, 1));
	hnode_addchild(tmp, hnode_alloc_chars(name, 1, 1));
	hnode_addchild(node, tmp);
	return 0;
}

static int ds_html_navigation_init(struct hnode *root, unsigned int num, struct testcase_base_info *info)
{
	struct hnode *node;
	unsigned int i;

	node = ds_get_hnode(root, "navigation_list");
	if(!node)
		return -1;
	add_navigation(node, "TOP");
	hnode_addchild(node, hnode_alloc_chars("\n", 1, 1));
	for(i=0; i<num; i++){
		add_navigation(node, info[i].display_name);
		hnode_addchild(node, hnode_alloc_chars("\n", 1, 1));
	}
}
static int add_module_table(struct hnode *node, const char *name)
{
	struct hnode *div,*tmp,*table,*tr,*th;
	char buf[256];

	snprintf(buf, sizeof(buf), "\"%s_MODULE\"", name);
	div = hnode_alloc_elem(HELEM_DIV, 1, 1);
	hnode_addattr(div,hattr_alloc_chars(HATTR_CLASS, "\"module\"", 0, 1, 1));
	//link
	tmp = hnode_alloc_elem(HELEM_A, 1, 1);
	hnode_addattr(tmp,hattr_alloc_chars(HATTR_CLASS, "\"module_name\"", 0, 1, 1));
	hnode_addattr(tmp,hattr_alloc_chars(HATTR_NAME, buf, 0, 1, 1));
	hnode_addchild(tmp, hnode_alloc_chars(buf, 1, 1));
	hnode_addchild(div, tmp);
	hnode_addchild(div, hnode_alloc_chars("\n", 1, 1));
	//table
	snprintf(buf, sizeof(buf), "\"%sTABLE\"", name);
	table = hnode_alloc_elem(HELEM_TABLE, 1, 1);
	hnode_addattr(table, hattr_alloc_chars(HATTR_ID, buf, 0, 1, 1));
	hnode_addchild(table, hnode_alloc_chars("\n", 1, 1));
	//th
	tr = hnode_alloc_elem(HELEM_TR, 1, 1);
	hnode_addchild(tr, hnode_alloc_chars("\n", 1, 1));
	th = hnode_alloc_elem(HELEM_TH, 1, 1);
	hnode_addchild(th, hnode_alloc_chars("Round(No.)", 1, 1));
	hnode_addchild(tr, th);
	th = hnode_alloc_elem(HELEM_TH, 1, 1);
	hnode_addchild(th, hnode_alloc_chars("Test Information", 1, 1));
	hnode_addchild(tr, th);
	th = hnode_alloc_elem(HELEM_TH, 1, 1);
	hnode_addchild(th, hnode_alloc_chars("Test Result", 1, 1));
	hnode_addchild(tr, th);
	hnode_addchild(tr, hnode_alloc_chars("\n", 1, 1));
	hnode_addchild(table, tr);
	hnode_addchild(table, hnode_alloc_chars("\n", 1, 1));
	hnode_addchild(div, table);
	hnode_addchild(node, div);
	return 0;
}
static int ds_html_module_table_init(struct hnode *root, unsigned int num, struct testcase_base_info *info)
{
	struct hnode *node;
	unsigned int i;

	node = ds_get_hnode(root, "BODY");
	if(!node)
		return -1;
	for(i=0; i<num; i++){
		add_module_table(node, info[i].display_name);
		hnode_addchild(node, hnode_alloc_chars("\n", 1, 1));
	}
}

int ds_html_module_table_add_log(struct hnode *root, const char *table_id, const char *log_path)
{
	struct hnode *node, *link,*br;
	char buf[128];

	if(!root || !table_id || !log_path)
		return -1;
	node = ds_get_hnode(root, table_id);
	if(!node)
		return -1;
	br = hnode_alloc_elem(HELEM_BR, 1, 1);
	link = hnode_alloc_elem(HELEM_A, 1, 1);
	hnode_addattr(link, hattr_alloc_chars(HATTR_HREF, log_path, 0, 1, 1));
	hnode_addattr(link, hattr_alloc_chars(HATTR_TARGET, "_blank", 0, 1, 1));
	snprintf(buf, sizeof(buf), "Detail message.(%s)", log_path);
	hnode_addchild(link, hnode_alloc_chars(buf, 1, 1));
	hnode_addchild(br, link);
	hnode_addchild(node, br);
	return 0;
}

int ds_html_module_table_add(struct hnode *root, const char *table_id, unsigned int round, const char *info, const char *result)
{
	struct hnode *node,*tr,*td;
	char round_buf[8];

	if(!root || !table_id || !info || !result)
		return -1;
	node = ds_get_hnode(root, table_id);
	if(!node)
		return -1;
	tr = hnode_alloc_elem(HELEM_TR, 1, 1);
	hnode_addattr(tr, hattr_alloc_chars(HATTR_ONMOUSEOVER, "\"this.style.backgroundColor='#FFFF66'\"", 0, 1, 1));
	hnode_addattr(tr, hattr_alloc_chars(HATTR_ONMOUSEOUT, "\"SetTableSrcColor(this)\"", 0, 1, 1));
	hnode_addchild(tr, hnode_alloc_chars("\n", 1, 1));
	snprintf(round_buf, sizeof(round_buf), "%u", round);
	td = hnode_alloc_elem(HELEM_TD, 1, 1);
	hnode_addchild(td, hnode_alloc_chars(round_buf, 1, 1));
	hnode_addchild(tr, td);
	td = hnode_alloc_elem(HELEM_TD, 1, 1);
	hnode_addchild(td, hnode_alloc_chars(info, 1, 1));
	hnode_addchild(tr, td);
	td = hnode_alloc_elem(HELEM_TD, 1, 1);
	hnode_addchild(td, hnode_alloc_chars(result, 1, 1));
	hnode_addchild(tr, td);
	hnode_addchild(tr, hnode_alloc_chars("\n", 1, 1));
	hnode_addchild(node, tr);
	hnode_addchild(node, hnode_alloc_chars("\n", 1, 1));
}

int ds_html_set_test_result(struct hnode *root, int result, const char *name)
{
	if(!root)
		return -1;
	if(result == TEST_STATUS_PASS) {
		ds_modify_chars(root, "TEST_RESULT", "TEST PASS");
	}else {
		struct hnode *node,*div,*a;
		char buf[256];
		snprintf(buf, sizeof(buf), "\"#%s_MODULE\"", name);
		ds_modify_chars(root, "TEST_RESULT", "TEST FAIL");
		node = ds_get_hnode(root, "FAIL_MODULE");
		if(!node)
			return -1;
		//printf("node:%p,line:%d\n", node, hnode_line(node));
		hnode_addchild(node, hnode_alloc_chars("\n", 1, 1));
		div = hnode_alloc_elem(HELEM_DIV, 1, 1);
		hnode_addattr(div, hattr_alloc_chars(HATTR_CLASS, "\"fail_module\"", 0, 1, 1));
		hnode_addchild(div, hnode_alloc_chars("\n", 1, 1));
		a = hnode_alloc_elem(HELEM_A, 1, 1);
		hnode_addattr(a, hattr_alloc_chars(HATTR_HREF, buf, 0, 1, 1));
		hnode_addchild(a, hnode_alloc_chars(name, 1, 1));
		hnode_addchild(div, a);
		hnode_addchild(div, hnode_alloc_chars("\n", 1, 1));
		hnode_addchild(node, div);
	}
	return 0;
}

static int out_open(void *arg)
{
	int *fd = (int *)arg;
	char buf[256];

	snprintf(buf, sizeof(buf), "%s/DragonStress.html", gDSpath);
	truncate(buf, 0);
	*fd = open(buf, O_RDWR|O_CREAT, 0666);
	if(*fd < 0) {
		printf("open error\n");
		return 0;
	}
	return 1;
}
static int out_close(void *arg)
{
	int *fd = (int *)arg;
	if(*fd > 0){
		fsync(*fd);
		close(*fd);
	}
	return 1;
}
static int out_puts(const char *msg, void *arg)
{
	int *fd = (int *)arg;
	ssize_t size,len;

	len = strlen(msg);
	size = write(*fd, msg, len);
	if(len != size) {
		printf("write error.len=%u,size=%u\n", len, size);
		return 0;
	}
	return 1;
}
static int out_putchar(char c, void *arg)
{
	int *fd = (int *)arg;
	ssize_t size;

	size = write(*fd, &c, 1);
	if(size != 1) {
		printf("write c error.\n");
		return 0;
	}
	return 1;

}

struct hnode *ds_html_open(struct ds_html *ds, const char *conf_file)
{
	static int out_fd;
	static struct iofd  fd;
	static struct ioctx     ctx;

	struct ioout     *out = &ds->out;
#ifdef VALID
#endif
	enum hmode   inmode, outmode;

	inmode = outmode = HTML_4_01;
	/* Initialise output context. */
	out_fd = -1;
	memset(out, 0, sizeof(struct ioout));
	out->open = out_open;
	out->close = out_close;
	out->puts = out_puts;
	out->putchar = out_putchar;
	out->arg = &out_fd;
	//out->puts = iostdout_puts;
	//out->putchar = iostdout_putchar;
	/* Initialise input context. */
	memset(&ctx, 0, sizeof(struct ioctx));

	memset(&fd, 0, sizeof(fd));
	ctx.open = iofd_open;
	ctx.getchar = iofd_getchar;
	ctx.close = iofd_close;
	ctx.rew = iofd_rew;
	//ctx.error = fd_error;
	//ctx.syntax = fd_syntax;ctx.open = iofd_open;
	//ctx.warn = fd_warn;
	ctx.arg = &fd;

	/* Initialise validation. */
#if VALID
    memset(&valid, 0, sizeof(struct iovalid));

    valid.valid_attr = val_attr;
    valid.valid_battr = val_battr;
    valid.valid_ident = val_ident;
    valid.valid_node = val_node;
    valid.valid_bnode = val_bnode;
    valid.arg = &fd;
#endif

	/* Initialise parser. */
    ds->parse = hparse_alloc(inmode);
    if (NULL == ds->parse) {
        perror(NULL);
		goto Err;
    }
	/* Initialise writer. */
    ds->writer = hwrite_alloc(outmode);
    if (NULL == ds->writer) {
        perror(NULL);
		goto Err;
    }
#if VALID
    /* Initialise validator. */
    validator = hvalid_alloc(inmode);
    if (NULL == validator) {
        perror(NULL);
		goto Err;
    }
#endif

	fd.file = conf_file;
	fd.fd = -1;
	//fd.perr = fd_perror;

	/* Parse, validate, write! */
	printf("prepare hparse_tree!\n");
	if ( ! hparse_tree(ds->parse, &ctx, &ds->root))
		goto Err;
	printf("after parse!\n");
	assert(ds->root);

	//ds_html_write(ds);
	return ds->root;
Err:
	ds_html_close(ds);
	return NULL;
}

int ds_html_write(struct ds_html *ds)
{
	printf("prepare write!\n");
	if (!ds || !hwrite_tree(ds->writer, &ds->out, ds->root))
		return -1;
	return 0;
}

void ds_html_close(struct ds_html *ds)
{
	if (ds->root)
        hnode_delete(ds->root);
#ifdef VALID
    hvalid_delete(ds->validator);
#endif
    hwrite_delete(ds->writer);
    hparse_delete(ds->parse);
	memset(ds, 0, sizeof(struct ds_html));
}

int ds_html_init(struct ds_html *ds, struct dragon_stress_data *data, const char *result_path)
{
	struct dragon_stress_data *dt = (struct dragon_stress_data *)data;
	struct testcase_base_info *info = dt->info;
	unsigned int i;
	time_t timep;
	struct tm *p;
	char date_time_buf[128];
	struct hnode *root = ds->root;

	time(&timep);
	p = localtime(&timep);
	printf("%04d-%02d-%02d, %02d:%02d:%02d\n", p->tm_year+1900, p->tm_mon+1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
	snprintf(gDSdir, sizeof(gDSdir), "DragonStressReport_%04d-%02d-%02d_%02d:%02d:%02d", p->tm_year+1900, p->tm_mon+1, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
	snprintf(gDSpath, sizeof(gDSpath), "%s/%s", result_path, gDSdir);
	mkdir(gDSpath, 0777);
	//Set Platform
	ds_modify_chars(root, "PLATFORM", BOARD);
	//Set Date
	snprintf(date_time_buf, sizeof(date_time_buf), "%04d-%02d-%02d", p->tm_year+1900, p->tm_mon+1, p->tm_mday);
	ds_modify_chars(root, "DATE", date_time_buf);
	//Set Time
	snprintf(date_time_buf, sizeof(date_time_buf), "%02d:%02d:%02d", p->tm_hour, p->tm_min, p->tm_sec);
	ds_modify_chars(root, "TIME", date_time_buf);
	//Set Rounds
	ds_modify_chars(root, "ROUNDS", "0");
	//Set Test Result
	ds_modify_chars(root, "TEST_RESULT", "TEST PASS");
	//Set Fail Module
	//Set Navigation
	ds_html_navigation_init(root, data->test_count, data->info);
	//Set Module Table
	module_table_script_init(root, data->test_count, data->info);
	ds_html_module_table_init(root, data->test_count, data->info);

}
