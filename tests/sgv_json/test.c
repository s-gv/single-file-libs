#include <stdio.h>
#include <string.h>

#define SGV_JSON_IMPLEMENTATION
#include "sgv_json.h"

int main()
{
    char json_str1[] = "{}";
    sgv_json_token tokens[100];

    printf("Test 1 ...\n");
    sgv_json_parse(json_str1, strlen(json_str1), tokens, 100);

    printf("All tests done.\n");
    return 0;
}
