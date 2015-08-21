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

SGV_OBJ is a simple but incomplete parser for reading Wavefront OBJ files.
It reads OBJ files with a single object that is triangulated.

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

#ifdef SGV_OBJ_STATIC
#define SGV_OBJ_DEF static
#else
#define SGV_OBJ_DEF extern
#endif


// read an array of (X, Y, Z, normalX, normalY, normalZ, U, V) tuples.
// 'len' is the size of the returned array. 
// For example, if two tuples are read, 'len' is set to 16.
SGV_OBJ_DEF float* sgv_obj_read(const char* filename, int* len);

// free the vertex array read using sgv_obj_read(filename);
SGV_OBJ_DEF void sgv_obj_free(float* data);

#ifdef __cplusplus
}
#endif

#endif

#ifdef SGV_OBJ_IMPLEMENTATION
SGV_OBJ_DEF float* sgv_obj_read(const char* filename, int* len)
{
    char lineBuf[1024];
    
    *len = 0;

    ///////////////////////////////////////////////////////////////////////////
    // First pass: Figure out the size of the arrays
    int nVertices = 0, nTexCoords = 0, nNormals = 0, nTriangles = 0;

    FILE* fp = fopen(filename, "r");
    if (fp == NULL) // Error. Could not open file
    {
        return NULL;
    }
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
            nTriangles++;
        }
        if (lineBuf[0] == 'v' && lineBuf[1] == 't' && lineBuf[2] == ' ')
        {
            nTexCoords++;
        }
    }
    fclose(fp);
    //////////////////////////////////////////////////////////////////////////////

    float* vertices = malloc(nVertices*3*sizeof(float)); 
    float* normals = malloc(nNormals*3*sizeof(float));
    float* texcoords = malloc(nTexCoords*2*sizeof(float));
    float* vertexBuffer = malloc(nTriangles*3*8*sizeof(float));

    int vertexPtr = 0, normalPtr = 0, texcoordPtr = 0, vertexBufferPtr = 0;

    if (vertices == NULL || normals == NULL 
            || texcoords == NULL || vertexBuffer == NULL)
    {
        free(vertexBuffer); 
        free(normals);
        free(vertices);
        free(texcoords);
        return NULL;
    }

    //////////////////////////////////////////////////////////////////////////////
    // Second pass: Fill in the array
    fp = fopen(filename, "r");
    if (fp == NULL) // Error. Could not open file
    {
        free(vertexBuffer); 
        free(normals);
        free(vertices);
        free(texcoords);
        return NULL;
    }
    while(fgets(lineBuf, 1024, fp) != NULL)
    {
        if (lineBuf[0] == 'v' && lineBuf[1] == ' ')
        {
            sscanf(lineBuf, "v %f %f %f", 
                    &vertices[3*vertexPtr], 
                    &vertices[3*vertexPtr+1],
                    &vertices[3*vertexPtr+2]);
            vertexPtr++;
        }
        if (lineBuf[0] == 'v' && lineBuf[1] == 'n' && lineBuf[2] == ' ')
        {
            sscanf(lineBuf, "vn %f %f %f",
                    &normals[3*normalPtr],
                    &normals[3*normalPtr+1],
                    &normals[3*normalPtr+2]);
            normalPtr++;
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
                vertexBuffer[8*vertexBufferPtr] = vertices[3*(v[i]-1)]; // X
                vertexBuffer[8*vertexBufferPtr+1] = vertices[3*(v[i]-1) + 1]; // Y
                vertexBuffer[8*vertexBufferPtr+2] = vertices[3*(v[i]-1) + 2]; // Z

                vertexBuffer[8*vertexBufferPtr+3] = normals[3*(n[i]-1) + 1]; // normalX
                vertexBuffer[8*vertexBufferPtr+4] = normals[3*(n[i]-1) + 2]; // normalY
                vertexBuffer[8*vertexBufferPtr+5] = normals[3*(n[i]-1) + 1]; // normalZ

                vertexBuffer[8*vertexBufferPtr+6] = texcoords[3*(t[i]-1) + 1]; // Y
                vertexBuffer[8*vertexBufferPtr+7] = texcoords[3*(t[i]-1) + 2]; // Z

                vertexBufferPtr++;
            }
        }
        if (lineBuf[0] == 'v' && lineBuf[1] == 't' && lineBuf[2] == ' ')
        {
            sscanf(lineBuf, "vt %f %f", 
                    &texcoords[texcoordPtr*3], 
                    &texcoords[texcoordPtr*3+1]);
            texcoordPtr++;
        }
    }
    fclose(fp);
    //////////////////////////////////////////////////////////////////////////////
    *len = nTriangles*3*8;

    return NULL;
}
SGV_OBJ_DEF void sgv_obj_free(float* data)
{
    free(data);
}

#endif
