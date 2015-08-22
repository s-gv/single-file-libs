#include <stdio.h>
#include <assert.h>

#define SGV_OBJ_IMPLEMENTATION
#include "sgv_obj.h"

int main(int argc, const char *argv[])
{
    sgv_obj* obj;
    sgv_obj_mtl* mtl;

    printf("Testing test1.obj ... ");
    obj = sgv_obj_readobj("test1.obj");
    assert(obj->objectsLen == 2);
    assert(obj->objects[0].meshesLen == 1);
    assert(obj->objects[0].meshes[0].vertexBufferLen == 6*2*3*8);
    //sgv_obj_dispobj(obj);
    sgv_obj_freeobj(obj);
    printf("OK\n");

    printf("Testing test1.mtl ... ");
    mtl = sgv_obj_readmtl("test1.mtl");
    assert(mtl->materialsLen == 2);
    //sgv_obj_dispmtl(mtl);
    sgv_obj_freemtl(mtl);
    printf("OK\n");

    return 0;
}
