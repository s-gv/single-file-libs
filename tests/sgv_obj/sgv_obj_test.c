#include <stdio.h>
#include <assert.h>

#define SGV_OBJ_IMPLEMENTATION
#include "sgv_obj.h"

int main(int argc, const char *argv[])
{
    sgv_obj* obj;

    printf("Testing test1.obj ... ");
    obj = sgv_obj_read("test1.obj");
    assert(obj->objectsLen == 2);
    assert(obj->objects[0].meshesLen == 1);
    assert(obj->objects[0].meshes[0].vertexBufferLen == 6*2*3*8);
    //sgv_obj_disp(obj);
    sgv_obj_free(obj);
    printf("OK\n");

    return 0;
}
