/*
Copyright (c) 2015 Sagar Gubbi (sagar.writeme@gmail.com)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/*
SGV_OBJ
=======

SGV_OBJ is a simple and incomplete parser for reading Wavefront OBJ files.
All valid OBJ files are not parsed by this lib. Faces are assumed to be 
triangulated, and each vertex is assumed to have position, texture, and normal.

For each vertex, (posX, posY, posZ, normalX, normalY, normalZ, U, V)
are assumed to be present.

HOW TO USE
==========

Include this .h file in all the source files where you want use the provided
library functions. Only in one of the files, #define SGV_OBJ_IMPLEMENTATION
before including this .h file.

For example, if your project has 'main.c' and 'src.c'. Then,

In 'src.c':
#include "sgv_obj.h"
...

In 'main.c':
#define SGV_OBJ_IMPLEMENTATION
#include "sgv_obj.h"
...
*/

// Uncomment following file if you want all library functions to be 'static'
//#define SGV_OBJ_STATIC

//////////////////////////////////////////////////////////////////////////////
#ifndef SGV_OBJ_H
#define SGV_OBJ_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef SGV_OBJ_STATIC
#define SGV_OBJ_DEF static
#else
#define SGV_OBJ_DEF extern
#endif

#define sgv_obj_mem_malloc(x) malloc(x)
#define sgv_obj_mem_free(x) free(x)

typedef struct {
    char* materialFileName;
    char* materialName;
    int vertexBufferOffset; // == start of elements in this mesh
    int vertexBufferLen; // == 8 * num of elements in vertexBuffer for this mesh
} sgv_obj_mesh;

typedef struct {
    char* objectName;
    sgv_obj_mesh* meshes;
    int meshesLen;
} sgv_obj_object;

typedef struct {
    sgv_obj_object* objects;
    int objectsLen;
    float* vertexBuffer;
} sgv_obj;

// returns an sgv_obj
SGV_OBJ_DEF sgv_obj* sgv_obj_readobj(const char* filename);

// free the sgv_obj read using sgv_obj_read(filename);
SGV_OBJ_DEF void sgv_obj_freeobj(sgv_obj* obj);

// print out sgv_obj
SGV_OBJ_DEF void sgv_obj_dispobj(sgv_obj* obj);

typedef struct {
    char* materialName;
    float KaR, KaG, KaB;
    float KdR, KdG, KdB;
    float KsR, KsG, KsB;
    int illum;
    char* map_Ka;
    char* map_Kd;
    char* map_Ks;
} sgv_obj_material;

typedef struct {
    sgv_obj_material* materials;
    int materialsLen;
} sgv_obj_mtl;

// read an sgv_obj_mtl from file
SGV_OBJ_DEF sgv_obj_mtl* sgv_obj_readmtl(const char* filename);

// destroy sgv_obj_mtl
SGV_OBJ_DEF void sgv_obj_freemtl(sgv_obj_mtl* mtl);

// print out sgv_obj_mtl
SGV_OBJ_DEF void sgv_obj_dispmtl(sgv_obj_mtl* mtl);

#ifdef __cplusplus
}
#endif

#endif

#ifdef SGV_OBJ_IMPLEMENTATION

SGV_OBJ_DEF sgv_obj* sgv_obj_readobj(const char* filename)
{
    sgv_obj* result = NULL;

    char* lineBuf = sgv_obj_mem_malloc(1024*sizeof(char));
    if (lineBuf == NULL)
        goto error_0;
    
    int nMaterialFiles = 0, nMeshes = 0, nObjects = 0;
    int nVertices = 0, nTexCoords = 0, nNormals = 0, nTotalTris = 0;

    ///////////////////////////////////////////////////////////////////////////
    // First pass: Figure out the size of the arrays
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) // Error. Could not open file
        goto error_1;

    while(fgets(lineBuf, 1024, fp) != NULL)
    {
        if (lineBuf[0] == 'v' && lineBuf[1] == ' ')
        {
            nVertices++;
        }
        if (lineBuf[0] == 'v' && lineBuf[1] == 'n' && lineBuf[2] == ' ')
        {
            nNormals++;
        }
        if (lineBuf[0] == 'f' && lineBuf[1] == ' ')
        {
            nTotalTris++;
        }
        if (lineBuf[0] == 'v' && lineBuf[1] == 't' && lineBuf[2] == ' ')
        {
            nTexCoords++;
        }
        if (lineBuf[0] == 'o' && lineBuf[1] == ' ')
        {
            nObjects++;
        }
        if (lineBuf[0] == 'u' && lineBuf[1] == 's')
        {
            nMeshes++;
        }
        if (lineBuf[0] == 'm' && lineBuf[1] == 't' && lineBuf[2] == 'l')
        {
            nMaterialFiles++;
        }
    }
    fclose(fp);
    //////////////////////////////////////////////////////////////////////////////

    float* vertices = sgv_obj_mem_malloc(nVertices*3*sizeof(float)); 
    if (vertices == NULL)
        goto error_1;

    float* normals = sgv_obj_mem_malloc(nNormals*3*sizeof(float));
    if (normals == NULL)
        goto error_2;

    float* texcoords = sgv_obj_mem_malloc(nTexCoords*2*sizeof(float));
    if (texcoords == NULL)
        goto error_3;

    float* vertexBuffer = sgv_obj_mem_malloc(nTotalTris*3*8*sizeof(float));
    if (vertexBuffer == NULL)
        goto error_4;

    sgv_obj_mesh* meshes = sgv_obj_mem_malloc(nMeshes*sizeof(sgv_obj_mesh));
    if (meshes == NULL)
        goto error_5;

    sgv_obj_object* objects = sgv_obj_mem_malloc(nObjects*sizeof(sgv_obj_object));
    if (objects == NULL)
        goto error_6;

    char* materialFileNames = sgv_obj_mem_malloc(nMaterialFiles*50*sizeof(char));
    if (materialFileNames == NULL)
        goto error_7;

    char* objectNames = sgv_obj_mem_malloc(nObjects*50*sizeof(char));
    if (objectNames == NULL)
        goto error_8;

    char* materialNames = sgv_obj_mem_malloc(nMeshes*50*sizeof(char));
    if (materialNames == NULL)
        goto error_9;

    result = sgv_obj_mem_malloc(sizeof(sgv_obj));
    if (result == NULL)
        goto error_10;

    //////////////////////////////////////////////////////////////////////////////
    // Second pass: Fill in the array
    int vertexIdx = 0, normalIdx = 0, vertexBufferIdx = 0, texcoordIdx = 0;
    int matFileNameIdx = 0, matNameIdx = 0, objectNameIdx = 0;
    int lastMatFileNameIdx = 0, lastMatNameIdx = 0, lastObjectNameIdx = 0;
    int meshIdx = 0, objectIdx = 0;

    fp = fopen(filename, "r");
    if (fp == NULL) // Error. Could not open file
        goto error_11;
    
    while(fgets(lineBuf, 1024, fp) != NULL)
    {
        if (lineBuf[0] == 'v' && lineBuf[1] == ' ')
        {
            sscanf(lineBuf, "v %f %f %f", 
                    &vertices[3*vertexIdx], 
                    &vertices[3*vertexIdx+1],
                    &vertices[3*vertexIdx+2]);
            vertexIdx++;
        } 
        if (lineBuf[0] == 'v' && lineBuf[1] == 'n' && lineBuf[2] == ' ')
        {
            sscanf(lineBuf, "vn %f %f %f",
                    &normals[3*normalIdx],
                    &normals[3*normalIdx+1],
                    &normals[3*normalIdx+2]);
            normalIdx++;
        } 
        if (lineBuf[0] == 'f' && lineBuf[1] == ' ')
        {
            int v[3], t[3], n[3];
            sscanf(lineBuf, "f %d/%d/%d %d/%d/%d %d/%d/%d",
                    &v[0], &t[0], &n[0], 
                    &v[1], &t[1], &n[1], 
                    &v[2], &t[2], &n[2]);
            int i;
            for (i = 0; i < 3; i++)
            {
                vertexBuffer[8*vertexBufferIdx] = vertices[3*(v[i]-1)]; // X
                vertexBuffer[8*vertexBufferIdx+1] = vertices[3*(v[i]-1) + 1]; // Y
                vertexBuffer[8*vertexBufferIdx+2] = vertices[3*(v[i]-1) + 2]; // Z

                vertexBuffer[8*vertexBufferIdx+3] = normals[3*(n[i]-1)]; // normalX
                vertexBuffer[8*vertexBufferIdx+4] = normals[3*(n[i]-1) + 1]; // normalY
                vertexBuffer[8*vertexBufferIdx+5] = normals[3*(n[i]-1) + 2]; // normalZ

                vertexBuffer[8*vertexBufferIdx+6] = texcoords[3*(t[i]-1) + 1]; // Y
                vertexBuffer[8*vertexBufferIdx+7] = texcoords[3*(t[i]-1) + 2]; // Z

                vertexBufferIdx++;
                meshes[meshIdx-1].vertexBufferLen += 8;
            }
        } 
        if (lineBuf[0] == 'v' && lineBuf[1] == 't' && lineBuf[2] == ' ')
        {
            sscanf(lineBuf, "vt %f %f", 
                    &texcoords[texcoordIdx*2], 
                    &texcoords[texcoordIdx*2+1]);
            texcoordIdx++;
        }
        if (lineBuf[0] == 'm' && lineBuf[1] == 't' && lineBuf[2] == 'l')
        {
            sscanf(lineBuf, "mtllib %s", &materialFileNames[matFileNameIdx]);
            lastMatFileNameIdx = matFileNameIdx;
            matFileNameIdx += strlen(&materialFileNames[matFileNameIdx]) + 1;
        }
        if (lineBuf[0] == 'u' && lineBuf[1] == 's' && lineBuf[2] == 'e')
        {
            sscanf(lineBuf, "usemtl %s", &materialNames[matNameIdx]);
            lastMatNameIdx = matNameIdx;
            matNameIdx += strlen(&materialNames[matNameIdx]) + 1;

            objects[objectIdx-1].meshesLen = objects[objectIdx-1].meshesLen + 1;
            meshes[meshIdx].materialFileName = &materialFileNames[lastMatFileNameIdx];
            meshes[meshIdx].materialName = &materialNames[lastMatNameIdx];
            meshes[meshIdx].vertexBufferOffset = vertexBufferIdx*8;
            meshes[meshIdx].vertexBufferLen = 0;
            meshIdx++;
        }
        if (lineBuf[0] == 'o')
        {
            sscanf(lineBuf, "o %s", &objectNames[objectNameIdx]);
            lastObjectNameIdx = objectNameIdx;
            objectNameIdx += strlen(&objectNames[objectNameIdx]) + 1;

            objectIdx++;
            objects[objectIdx-1].objectName = &objectNames[lastObjectNameIdx];
            objects[objectIdx-1].meshes = &meshes[meshIdx];
            objects[objectIdx-1].meshesLen = 0;
        }
    }
    fclose(fp);
    //////////////////////////////////////////////////////////////////////////////
    result->vertexBuffer = vertexBuffer;
    result->objects = objects;
    result->objectsLen = nObjects;
    goto error_4;

error_11:
    sgv_obj_mem_free(result);
    result = NULL;
error_10:
    sgv_obj_mem_free(materialNames);
error_9:
    sgv_obj_mem_free(objectNames);
error_8:
    sgv_obj_mem_free(materialFileNames);
error_7:
    sgv_obj_mem_free(objects);
error_6:
    sgv_obj_mem_free(meshes);
error_5:
    sgv_obj_mem_free(vertexBuffer);
error_4:
    sgv_obj_mem_free(texcoords);
error_3:
    sgv_obj_mem_free(normals);
error_2:
    sgv_obj_mem_free(vertices);
error_1:
    sgv_obj_mem_free(lineBuf);
error_0:
    return result;
}

SGV_OBJ_DEF void sgv_obj_freeobj(sgv_obj* obj)
{
    sgv_obj_mem_free(obj->vertexBuffer);
    sgv_obj_mem_free(obj->objects[0].objectName);
    sgv_obj_mem_free(obj->objects[0].meshes[0].materialFileName);
    sgv_obj_mem_free(obj->objects[0].meshes[0].materialName);
    sgv_obj_mem_free(obj->objects[0].meshes);
    sgv_obj_mem_free(obj->objects);
    sgv_obj_mem_free(obj);
}

SGV_OBJ_DEF void sgv_obj_dispobj(sgv_obj* obj)
{
    int i, j, k;
    for (i = 0; i < obj->objectsLen; i++)
    {
        printf("\nObject: %s\n", obj->objects[i].objectName);
        for (j = 0; j < obj->objects[i].meshesLen; j++) 
        {
            printf("  MaterialFileName: %s\n", obj->objects[i].meshes[j].materialFileName);
            printf("  MaterialName: %s\n", obj->objects[i].meshes[j].materialName);
            int offset = obj->objects[i].meshes[j].vertexBufferOffset;
            for (k = 0; k < obj->objects[i].meshes[j].vertexBufferLen; k++)
            {
                if (k % 8 == 0)
                    printf("\n");
                printf("%8.2f, ", obj->vertexBuffer[offset+k]);
            }
        }
    }
    printf("\n");
}

SGV_OBJ_DEF sgv_obj_mtl* sgv_obj_readmtl(const char* filename)
{
    int nAllChars = 0, nMaterials = 0;
    sgv_obj_mtl* result = NULL;

    char* lineBuf = sgv_obj_mem_malloc(1024*sizeof(char));
    if (lineBuf == NULL)
        goto error_0;

    /////////////////////////////////////////////////////////////
    // First pass
    FILE* fp = fopen(filename, "r");
    if (fp == NULL)
        goto error_1;

    while(fgets(lineBuf, 1024, fp) != NULL)
    {
        if (lineBuf[0] == 'n' && lineBuf[1] == 'e' && lineBuf[2] == 'w')
        {
            nAllChars += strlen(lineBuf);
            nMaterials++;
        }
        if (lineBuf[0] == 'm' && lineBuf[1] == 'a' && lineBuf[2] == 'p')
        {
            nAllChars += strlen(lineBuf);
        }
    }
    fclose(fp);
    /////////////////////////////////////////////////////////////
    char* charBuf = sgv_obj_mem_malloc(nAllChars*sizeof(char));
    if (charBuf == NULL)
        goto error_1;

    sgv_obj_material* materials = sgv_obj_mem_malloc(nMaterials*sizeof(sgv_obj_material));
    if (materials == NULL)
        goto error_2;
    
    result = sgv_obj_mem_malloc(sizeof(sgv_obj_mtl));
    if (result == NULL)
        goto error_3;

    int materialIdx = 0, charIdx = 0;

    /////////////////////////////////////////////////////////////
    // Second pass
    fp = fopen(filename, "r");
    if (fp == NULL)
        goto error_4;
    
    while(fgets(lineBuf, 1024, fp) != NULL)
    {
        if (lineBuf[0] == 'n' && lineBuf[1] == 'e' && lineBuf[2] == 'w')
        {
            sscanf(lineBuf, "newmtl %s", &charBuf[charIdx]);
            materialIdx++;
            materials[materialIdx-1].materialName = &charBuf[charIdx];
            materials[materialIdx-1].map_Ka = NULL;
            materials[materialIdx-1].map_Kd = NULL;
            materials[materialIdx-1].map_Ks = NULL;
            charIdx += strlen(&charBuf[charIdx]) + 1;
        }
        if (lineBuf[0] == 'm' && lineBuf[1] == 'a' && lineBuf[2] == 'p')
        {
            if (lineBuf[4] == 'K' && lineBuf[5] == 'a')
            {
                sscanf(lineBuf, "map_Ka %s", &charBuf[charIdx]);
                materials[materialIdx-1].map_Ka = &charBuf[charIdx];
                charIdx += strlen(&charBuf[charIdx]) + 1;
            }
            if (lineBuf[4] == 'K' && lineBuf[5] == 'd')
            {
                sscanf(lineBuf, "map_Kd %s", &charBuf[charIdx]);
                materials[materialIdx-1].map_Kd = &charBuf[charIdx];
                charIdx += strlen(&charBuf[charIdx]) + 1;
            }
            if (lineBuf[4] == 'K' && lineBuf[5] == 's')
            {
                sscanf(lineBuf, "map_Ks %s", &charBuf[charIdx]);
                materials[materialIdx-1].map_Ks = &charBuf[charIdx];
                charIdx += strlen(&charBuf[charIdx]) + 1;
            }
        }

        if (lineBuf[0] == 'K' && lineBuf[1] == 'a')
        {
            sscanf(lineBuf, "Ka %f %f %f", 
                    &materials[materialIdx-1].KaR,
                    &materials[materialIdx-1].KaG,
                    &materials[materialIdx-1].KaB);
        }
        if (lineBuf[0] == 'K' && lineBuf[1] == 'd')
        {
            sscanf(lineBuf, "Kd %f %f %f", 
                    &materials[materialIdx-1].KdR,
                    &materials[materialIdx-1].KdG,
                    &materials[materialIdx-1].KdB);
        }
        if (lineBuf[0] == 'K' && lineBuf[1] == 's')
        {
            sscanf(lineBuf, "Ks %f %f %f", 
                    &materials[materialIdx-1].KsR,
                    &materials[materialIdx-1].KsG,
                    &materials[materialIdx-1].KsB);
        }

        if (lineBuf[0] == 'i' && lineBuf[1] == 'l' && lineBuf[2] == 'l')
        {
            sscanf(lineBuf, "illum %d", &materials[materialIdx-1].illum);
        }
    }
    fclose(fp);
    /////////////////////////////////////////////////////////////
    result->materialsLen = nMaterials;
    result->materials = materials;
    goto error_1;

error_4:
    sgv_obj_mem_free(result);
    result = NULL;
error_3:
    sgv_obj_mem_free(materials);
error_2:
    sgv_obj_mem_free(charBuf);
error_1:
    sgv_obj_mem_free(lineBuf);
error_0:
    return result;
}

SGV_OBJ_DEF void sgv_obj_freemtl(sgv_obj_mtl* mtl)
{
    sgv_obj_mem_free(mtl->materials[0].materialName);
    sgv_obj_mem_free(mtl->materials);
    sgv_obj_mem_free(mtl);
}

SGV_OBJ_DEF void sgv_obj_dispmtl(sgv_obj_mtl* mtl)
{
    int i;
    printf("\nNumber of materials in this file = %d\n", mtl->materialsLen);
    for (i = 0; i < mtl->materialsLen; i++)
    {
        printf("  Material name: %s\n", mtl->materials[i].materialName);
        if (mtl->materials[i].map_Kd != NULL)
            printf("    map_Kd = %s\n", mtl->materials[i].map_Kd);
    }
}

#endif
