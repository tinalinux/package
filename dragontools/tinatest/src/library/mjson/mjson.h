#ifndef __MJSON_H
#define __MJSON_H

#define DEFAULT_JSON_PATH "/etc/tinatest.json"

#include <stdio.h>
#include <json-c/json_object.h>
#include <json-c/json_object_private.h>
#include <json-c/json_tokener.h>
#include <json-c/arraylist.h>
#include <json-c/linkhash.h>

#define ARRAY_MAX 30
#define TURE 1
#define FAILED 0

enum mjson_type {
    mjson_type_null = 0,
    mjson_type_boolean,
    mjson_type_double,
    mjson_type_int,
    mjson_type_object, //user can ignore it
    mjson_type_array,
    mjson_type_string,
    mjson_type_error //no this key or array not string
};

struct mjson_value {
    union mdata {
        int m_boolean;
        double m_double;
        int m_int;
        char *m_string;
        char **m_array;
    } val;
    enum mjson_type type;

    char *keyname;
    const char *keypath;
    struct json_object *obj;
};

extern int mjson_load(const char *fpath);
extern void mjson_draw_tree(const char *keypath);
extern enum mjson_type mjson_get_type(const char *keypath);
//fetch family
extern   struct mjson_value mjson_fetch(const char *keypath);
extern                  int mjson_fetch_int(const char *keypath);
extern                  int mjson_fetch_boolean(const char *keypath);
extern               double mjson_fetch_double(const char *keypath);
extern               char * mjson_fetch_string(const char *keypath);
extern              char ** mjson_fetch_array(const char *keypath); // array[0] = $len; array[1] = $first_str; ...

//this funcion is used for macros mjson_foreach, not user
extern void mjson_get_value_from_obj(json_object *obj, struct mjson_value *val);
//mjson_foreach(char *keypath, char *key, union mdata val, enum mjson_type mtype);
//make sure the existed for keypath
#define mjson_foreach(kpath, key, _val, mtype) \
    char *key __attribute__((__unused__)); \
    union mdata _val __attribute__((__unused__)); \
    enum mjson_type mtype __attribute__((__unused__)); \
    struct mjson_value entry ## _val; \
    for( struct lh_entry *entry_next ## key = NULL, \
            *entry ## key = json_object_get_object(mjson_fetch(kpath).obj)->head; \
        ({ if(entry ## key) { \
            key = (char *)entry ## key->k; \
            mjson_get_value_from_obj((struct json_object *)entry ## key->v, &entry ## _val); \
            _val = entry ## _val.val; \
            mtype = entry ## _val.type; \
            entry_next ## key = entry ## key->next; \
        } ; entry ## key; }); \
        entry ## key = entry_next ## key)

#endif
