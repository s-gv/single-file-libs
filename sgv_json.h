/*  sgv_json.h - Public domain lib for working with JSON

Authored in 2015 by Sagar Gubbi (sagar.writeme@gmail.com).

Do this:
    #define SGV_JSON_IMPLEMENTATION
before you include this file in *one* C or C++ file to create the
implementation.

NOTES
-----

- This library performs no memory allocation.
- See http://www.json.org/ for the JSON spec.
- Notation of pair, value, array, object, etc. are followed from the spec.

EXAMPLE
-------

char* json_str = "{a: 2, b: [3, 6, 9], c: true}";
int json_str_len = strlen(json_str);

sgv_json_object obj;
unsigned char scratch_pad[100];
sgv_json_parse_object(json_str, json_str_len, scratch_pad, 100, &obj);

sgv_json_value val;
sgv_json_get_value(&obj, "a", &val);

int a;
sgv_json_get_int(&val, &a); // a will be 2

sgv_json_get_value(&obj, "b", &val);
sgv_json_array arr;
sgv_json_get_array(&val, &arr);

sgv_json_element ele1, ele2;
sgv_json_get_first_element(&arr, &ele1);
sgv_json_get_next_element(&ele1, &ele2);
sgv_json_get_element_value(&ele2, &val);
sgv_json_get_int(&val, &a); // a will be 6

OPTIONS
-------

- If you want all functions in this lib to be static,
    #define SGV_JSON_STATIC
  before including this file.

LICENSE
-------

This software is in the public domain. Where that dedication is not
recognized, redistribution and use in source and binary forms, with
or without modification, are permitted. No warranty for any purpose
is expressed or implied.

*/

/*****************************************************************************
****************************** Public API ***********************************/
#ifndef SGV_JSON_H
#define SGV_JSON_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SGV_JSON_STATIC
#define SGVJSON_DEF static
#else
#define SGVJSON_DEF extern
#endif

typedef struct sgv_json_object;
typedef struct sgv_json_array;
typedef struct sgv_json_pair;
typedef struct sgv_json_value;
typedef struct sgv_json_string {
    char* str; // May not be null terminated!
    int n_chars;
} sgv_json_string;

typedef enum {
    SGV_JSON_VALUE_STRING,
    SGV_JSON_VALUE_NUMBER,
    SGV_JSON_VALUE_OBJECT,
    SGV_JSON_VALUE_ARRAY,
    SGV_JSON_VALUE_TRUE,
    SGV_JSON_VALUE_FALSE,
    SGV_JSON_VALUE_NULL
} sgv_json_value_type;

typedef enum {
    SGV_JSON_ERROR_PARSE_FAIL = -1,
    SGV_JSON_ERROR_INCORRECT_VALUE_TYPE = -2,
} sgv_json_error_type;

/* returns 0 on success, negative if error, and number of bytes (positive)
   if the size of scratch_pad is in-sufficient */
SGVJSON_DEF int sgv_json_parse_object(char* json_str,
                                      int json_str_len,
                                      unsigned char* scratch_pad,
                                      int scratch_pad_len,
                                      sgv_json_object* obj);

/* looks for pair by key.
   Returns NULL if there is no pair with matching key */
SGVJSON_DEF void sgv_json_get_value(sgv_json_object* obj,
                                    char* key,
                                    sgv_json_value* value);

/* returns the first pair in the JSON object, NULL if the object is empty */
SGVJSON_DEF void sgv_json_get_first_pair(sgv_json_object* obj,
                                         sgv_json_pair* pair);

/* returns the next pair in the JSON object, NULL if no more pairs are left */
SGVJSON_DEF void sgv_json_get_next_pair(sgv_json_pair* current_pair,
                                        sgv_json_pair* next_pair);

/* returns the string corresponding to the key of this pair */
SGVJSON_DEF sgv_json_string sgv_json_get_pair_key(sgv_json_pair* pair);

/* returns the value of this pair */
SGVJSON_DEF sgv_json_value sgv_json_get_pair_value(sgv_json_pair* pair);

/* returns the type of the value */
SGVJSON_DEF int sgv_json_get_value_type(sgv_json_value* value);

/* returns 0 on successfully reading value,
   SGV_JSON_ERROR_INCORRECT_VALUE_TYPE if that value type cant be read.
   Boolean value: 0-false, 1-true */
SGVJSON_DEF int sgv_json_get_int(sgv_json_value* val, int* number);
SGVJSON_DEF int sgv_json_get_float(sgv_json_value* val, float* number);
SGVJSON_DEF int sgv_json_get_double(sgv_json_value* val, double* number);
SGVJSON_DEF int sgv_json_get_bool(sgv_json_value* val, int* bool_val);
SGVJSON_DEF int sgv_json_get_string(sgv_json_value* v, sgv_json_string* str);
SGVJSON_DEF int sgv_json_get_object(sgv_json_value* v, sgv_json_object* obj);
SGVJSON_DEF int sgv_json_get_array(sgv_json_value* val, sgv_json_array* arr);

/* returns 0 on success, negative if error, and number of bytes (positive)
   if the size of scratch_pad is in-sufficient */
SGVJSON_DEF int sgv_json_parse_array(char* json_str,
                                     int json_str_len,
                                     unsigned char* scratch_pad,
                                     int scratch_pad_len,
                                     sgv_json_array* arr);

/* returns the first element in the JSON array, NULL if the array is empty */
SGVJSON_DEF void sgv_json_get_first_element(sgv_json_array* arr,
                                            sgv_json_element* element);

/* returns the next element in the JSON object, NULL if no more are left */
SGVJSON_DEF void sgv_json_get_next_element(sgv_json_element* current_element,
                                           sgv_json_element* next_element);

/* returns the value of this pair */
SGVJSON_DEF sgv_json_value sgv_json_get_element_value(sgv_json_element* ele);

#ifdef __cplusplus
}
#endif

#endif

/*****************************************************************************
****************************** Implementation********************************/
#ifdef SGV_JSON_IMPLEMENTATION

#endif
