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
    char* material;
    float* vertexBuffer; // an array of (X, Y, Z, normalX, normalY, normalZ, U, V) tuples.
    int vertexBufferLen; // == 8 * num of elements in vertexBuffer
} sgv_obj_mesh;

typedef struct {
    char* object;
    sgv_obj_mesh* meshes;
    int meshesLen;
} sgv_obj_object;

// returns an array of type sgv_obj_object. 
// 'objectsLen' is the number of elements in objects
SGV_OBJ_DEF sgv_obj_object* sgv_obj_read(const char* filename, int* objectsLen);

// free the vertex array read using sgv_obj_read(filename);
SGV_OBJ_DEF void sgv_obj_free(sgv_obj_object* objects);

#ifdef __cplusplus
}
#endif

#endif

#ifdef SGV_OBJ_IMPLEMENTATION

#define SGV_OBJ_MAX_OBJECTS 10000
#define SGV_OBJ_MAX_MESHES_PER_OBJECT 10

SGV_OBJ_DEF sgv_obj_object* sgv_obj_read(const char* filename, int* objectsLen)
{
    int i;

    *objectsLen = 0;
    sgv_obj_object* result = NULL;

    char* lineBuf = sgv_obj_mem_malloc(1024*sizeof(char));
    if (lineBuf == NULL)
        goto error_0;
    
    int* nTriangles = sgv_obj_mem_malloc(SGV_OBJ_MAX_OBJECTS*SGV_OBJ_MAX_MESHES_PER_OBJECT*sizeof(int));
    if (nTriangles == NULL)
        goto error_1;

    char* materialFileName = sgv_obj_mem_malloc(250*sizeof(char));
    if (materialFileName == NULL)
        goto error_2;

    char* material = sgv_obj_mem_malloc(250*sizeof(char));
    if (material == NULL)
        goto error_3;

    int objectIdx = 0, meshIdx = 0;
    int nMaterialFiles = 0;
    int nVertices = 0, nTexCoords = 0, nNormals = 0, nTotalTris = 0, nObjects = 0, nMeshes = 0;
    for (i = 0; i < SGV_OBJ_MAX_OBJECTS*SGV_OBJ_MAX_MESHES_PER_OBJECT; i++)
        nTriangles[i] = 0;

    ///////////////////////////////////////////////////////////////////////////
    // First pass: Figure out the size of the arrays
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) // Error. Could not open file
        goto error_4;

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
            nTriangles[(objectIdx-1) * SGV_OBJ_MAX_MESHES_PER_OBJECT + (meshIdx-1)]++;
        }
        if (lineBuf[0] == 'v' && lineBuf[1] == 't' && lineBuf[2] == ' ')
        {
            nTexCoords++;
        }
        if (lineBuf[0] == 'o' && lineBuf[1] == ' ')
        {
            objectIdx++;
            meshIdx = 0;
        }
        if (lineBuf[0] == 'u' && lineBuf[1] == 's')
        {
            meshIdx++;
            nMeshes++;
        }
        if (lineBuf[0] == 'm' && lineBuf[1] == 't' && lineBuf[2] == 'l')
        {
            nMaterialFiles++;
        }
    }
    nObjects = objectIdx;
    fclose(fp);
    //////////////////////////////////////////////////////////////////////////////

    float* vertices = sgv_obj_mem_malloc(nVertices*3*sizeof(float)); 
    if (vertices == NULL)
        goto error_4;

    float* normals = sgv_obj_mem_malloc(nNormals*3*sizeof(float));
    if (normals == NULL)
        goto error_5;

    float* texcoords = sgv_obj_mem_malloc(nTexCoords*2*sizeof(float));
    if (texcoords == NULL)
        goto error_6;

    float* vertexBuffer = sgv_obj_mem_malloc(nTotalTris*3*8*sizeof(float));
    if (vertexBuffer == NULL)
        goto error_7;

    sgv_obj_mesh* meshes = sgv_obj_mem_malloc(nMeshes*sizeof(sgv_obj_mesh));
    if (meshes == NULL)
        goto error_8;

    sgv_obj_object* objects = sgv_obj_mem_malloc(nObjects*sizeof(sgv_obj_object));
    if (objects == NULL)
        goto error_9;

    char* materialFileNames = sgv_obj_mem_malloc(nMaterialFiles*50*sizeof(char));
    if (materialFileNames == NULL)
        goto error_10;

    char* objectNames = sgv_obj_mem_malloc(nObjects*50*sizeof(char));
    if (objectNames == NULL)
        goto error_11;

    char* materialNames = sgv_obj_mem_malloc(nMeshes*50*sizeof(char));
    if (materialNames == NULL)
        goto error_12;

    //////////////////////////////////////////////////////////////////////////////
    // Second pass: Fill in the array
    int vertexIdx = 0, normalIdx = 0, vertexBufferIdx = 0, texcoordIdx = 0;
    int matFileNameIdx = 0, matNameIdx = 0, objectNameIdx = 0;
    int lastMatFileNameIdx = 0, lastMatNameIdx = 0, lastObjectNameIdx = 0;
    meshIdx = 0;
    objectIdx = 0;

    fp = fopen(filename, "r");
    if (fp == NULL) // Error. Could not open file
        goto error_13;
    
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

                vertexBuffer[8*vertexBufferIdx+3] = normals[3*(n[i]-1) + 1]; // normalX
                vertexBuffer[8*vertexBufferIdx+4] = normals[3*(n[i]-1) + 2]; // normalY
                vertexBuffer[8*vertexBufferIdx+5] = normals[3*(n[i]-1) + 1]; // normalZ

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
            meshes[meshIdx].material = &materialNames[lastMatNameIdx];
            meshes[meshIdx].vertexBuffer = &vertexBuffer[8*vertexBufferIdx];
            meshes[meshIdx].vertexBufferLen = 0;
            meshIdx++;
        }
        if (lineBuf[0] == 'o')
        {
            sscanf(lineBuf, "o %s", &objectNames[objectNameIdx]);
            lastObjectNameIdx = objectNameIdx;
            objectNameIdx += strlen(&objectNames[objectNameIdx]) + 1;

            objectIdx++;
            objects[objectIdx-1].object = &objectNames[lastObjectNameIdx];
            objects[objectIdx-1].meshes = &meshes[meshIdx];
            objects[objectIdx-1].meshesLen = 0;
        }
    }
    fclose(fp);
    //////////////////////////////////////////////////////////////////////////////

    *objectsLen = nObjects;
    result = objects;
    goto error_7;

error_13:
    sgv_obj_mem_free(materialNames);
error_12:
    sgv_obj_mem_free(objectNames);
error_11:
    sgv_obj_mem_free(materialFileNames);
error_10:
    sgv_obj_mem_free(objects);
error_9:
    sgv_obj_mem_free(meshes);
error_8:
    sgv_obj_mem_free(vertexBuffer);
error_7:
    sgv_obj_mem_free(texcoords);
error_6:
    sgv_obj_mem_free(normals);
error_5:
    sgv_obj_mem_free(vertices);
error_4:
    sgv_obj_mem_free(material);
error_3:
    sgv_obj_mem_free(materialFileName);
error_2:
    sgv_obj_mem_free(nTriangles);
error_1:
    sgv_obj_mem_free(lineBuf);
error_0:
    return result;
}

SGV_OBJ_DEF void sgv_obj_free(sgv_obj_object* objects)
{
    sgv_obj_mem_free(objects[0].meshes[0].vertexBuffer);
    sgv_obj_mem_free(objects[0].meshes[0].materialFileName);
    sgv_obj_mem_free(objects[0].meshes[0].material);
    sgv_obj_mem_free(objects[0].object);
    sgv_obj_mem_free(objects[0].meshes);
    sgv_obj_mem_free(objects);
}

#endif
