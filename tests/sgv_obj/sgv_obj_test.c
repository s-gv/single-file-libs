#include <stdio.h>
#include <assert.h>

#define SGV_OBJ_IMPLEMENTATION
#include "sgv_obj.h"

int main(int argc, const char *argv[])
{
    sgv_obj_object* objects; int objectsLen;

    printf("Testing test1.obj ... ");
    objects = sgv_obj_read("test1.obj", &objectsLen);
    assert(objectsLen == 2);
    assert(objects[0].meshesLen == 1);
    assert(objects[1].meshes[0].vertexBufferLen == 6*2*3*8);
    sgv_obj_free(objects);
    printf("OK\n");

    return 0;
}
