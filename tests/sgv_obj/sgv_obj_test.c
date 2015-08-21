#include <stdio.h>
#include <assert.h>

#define SGV_OBJ_IMPLEMENTATION
#include "sgv_obj.h"

int main(int argc, const char *argv[])
{
    float* vData;
    int vDataCount;

    printf("Testing test1.obj ... ");
    vData = sgv_obj_read("test1.obj", &vDataCount);
    assert(vDataCount == 8*3*12);
    sgv_obj_free(vData);
    printf("OK\n");

    return 0;
}
