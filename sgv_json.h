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
- Tokens contain pointers to the parsed JSON string (so don't de-allocate it)

EXAMPLE
-------

OPTIONS
-------

Define these constants/macros before including this lib to change the behavior.

- If you want all functions in this lib to be static,
    #define SGV_JSON_STATIC
- To enable debug messages,
    #define SGV_JSON_DEBUG
- If you prefer to use your own logging functions instead of printf,
  define the SGV_JSON_PRINT macro like this:
    #define SGV_JSON_PRINT(x)  print x
  This gets used as SGV_JSON_PRINT(("Error code = %d\n", 2));

LICENSE
-------

This software is in the public domain. Where that dedication is not
recognized, redistribution and use in source and binary forms, with
or without modification, are permitted. No warranty for any purpose
is expressed or implied.

*/

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

/*****************************************************************************
****************************** Public API ***********************************/

typedef struct sgv_json_token sgv_json_token;
typedef enum {
    SGV_JSON_TOKEN_ARR,
    SGV_JSON_TOKEN_ELEMENT,
    SGV_JSON_TOKEN_OBJ,
    SGV_JSON_TOKEN_PAIR,
    SGV_JSON_TOKEN_KEY,
    SGV_JSON_TOKEN_VAL,
    SGV_JSON_TOKEN_VAL_STR,
    SGV_JSON_TOKEN_VAL_NUM,
    SGV_JSON_TOKEN_VAL_BOOL,
    SGV_JSON_TOKEN_VAL_NULL
} sgv_json_token_type;

/* returns 0 on success, negative if error, and number of tokens (positive)
   if the size of scratch_pad is in-sufficient. The root JSON obj will be in
   tokens[0]. */
SGVJSON_DEF int sgv_json_parse_object(char* json_str,
                                      int json_str_len,
                                      sgv_json_token* tokens,
                                      int max_tokens);

/* returns 0 on success, negative if error, and number of tokens (positive)
   if the size of scratch_pad is in-sufficient. The root JSON array will be in
   tokens[0]. */
SGVJSON_DEF int sgv_json_parse_array(char* json_str,
                                     int json_str_len,
                                     sgv_json_token* tokens,
                                     int max_tokens);

/* Returns first pair in the obj, NULL if the JSON object is empty/on error */
SGVJSON_DEF sgv_json_token* sgv_json_first_pair(sgv_json_token* obj);

/* Returns the next pair in the JSON object, NULL if there are none left */
SGVJSON_DEF sgv_json_token* sgv_json_next_pair(sgv_json_token* current_pair);

/* Returns first element in array, NULL if the JSON array is empty/on error */
SGVJSON_DEF sgv_json_token* sgv_json_first_element(sgv_json_token* arr);

/* Returns next element in array, NULL if there are none left/on error */
SGVJSON_DEF sgv_json_token* sgv_json_next_element(sgv_json_token* arr);

/* Returns the key corresponding to 'pair'. NULL if error */
SGVJSON_DEF sgv_json_token* sgv_json_pair_key(sgv_json_token* pair);

/* Returns the value corresponding to 'pair'. NULL if error */
SGVJSON_DEF sgv_json_token* sgv_json_pair_value(sgv_json_token* pair);

/* Returns the value corresponding to 'element'. NULL if error */
SGVJSON_DEF sgv_json_token* sgv_json_element_value(sgv_json_token* element);

/* Scans the JSON object for a pair with matching key and retrieves
   the *value* of the pair. Returns NULL if no such pair exists / on error */
SGVJSON_DEF sgv_json_token* sgv_json_obj_value(sgv_json_token* obj, char* key);

/* Scans the JSON array and retrieves the *value* at index 'idx'.
   Returns NULL if no such element exists / on error */
SGVJSON_DEF sgv_json_token* sgv_json_arr_value(sgv_json_token* arr, int idx);

/* returns 0 on success. negative on error */
SGVJSON_DEF int sgv_json_key_string(sgv_json_token* pair_key,
                                    char **str, int *str_len);

/* returns the type of the token */
SGVJSON_DEF sgv_json_token_type sgv_json_type(sgv_json_token* token);

/* returns JSON obj on success, NULL if error */
SGVJSON_DEF sgv_json_token* sgv_json_value_obj(sgv_json_token* value);

/* returns JSON array on success, NULL if error */
SGVJSON_DEF sgv_json_token* sgv_json_value_array(sgv_json_token* value);

/* returns 0 on success. negative on error */
SGVJSON_DEF int sgv_json_value_string(sgv_json_token* value,
                                      char **str, int *str_len);

/* returns 0 on success. negative on error */
SGVJSON_DEF int sgv_json_value_int(sgv_json_token* value, int* number);

/* returns 0 on success. negative on error */
SGVJSON_DEF int sgv_json_value_double(sgv_json_token* value, double* number);

/* returns 0 on success. negative on error. bool_val = 1 if True, else 0 */
SGVJSON_DEF int sgv_json_value_bool(sgv_json_token* value, int* bool_val);

/* returns 0 on success. negative on error. is_null = 1 if Null, else 0 */
SGVJSON_DEF int sgv_json_value_null(sgv_json_token* value, int* is_null);


/*****************************************************************************
*****************************************************************************/

/* WARNING: Do *not* access members of this struct directly.
   This is a private struct whose definition has been exposed to facilitate
   allocation of tokens on the stack (or static allocation) */
struct sgv_json_token {
    sgv_json_token_type type;
    union token_data {
        struct {
            sgv_json_token* next;
            sgv_json_token* key;
            sgv_json_token* value;
        } node;
        struct {
            char* str;
            int str_len;
            int integer_num;
            double decimal_num;
        } value;
    } data;
};

#ifdef __cplusplus
}
#endif

#endif

/*****************************************************************************
****************************** Implementation********************************/
#ifdef SGV_JSON_IMPLEMENTATION

#ifdef SGV_JSON_DEBUG
    #ifndef SGV_JSON_LOG
        #include <stdio.h>
        #define SGV_JSON_LOG(x) printf(x)
    #endif
#else
    static void sgv_json_print(const char *str)
    {
        str = str;
    }
    #define SGV_JSON_LOG(x) do { if (0) sgv_json_print(x); } while (0)
#endif

SGVJSON_DEF sgv_json_token_type sgv_json_type(sgv_json_token* token)
{
    return token->type;
}

SGVJSON_DEF int sgv_json_parse_object(char* json_str,
                                      int json_str_len,
                                      sgv_json_token* tokens,
                                      int max_tokens)
{
    json_str = json_str;
    json_str_len = json_str_len;
    tokens = tokens;
    max_tokens = max_tokens;
    SGV_JSON_LOG(("Hi %d\n", max_tokens));
    return 0;
}

#endif
