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

#ifdef SGV_OBJ_STATIC
#define SGV_OBJ_DEF static
#else
#define SGV_OBJ_DEF extern
#endif


// read the number of vertices and triangles in an obj file
SGV_OBJ_DEF void sgv_obj_readsize(const char* filename, 
                                  int* nVertices, 
                                  int* nTriangles); 



// read the OBJ file. 
// vBuffer is filled with (X, Y, Z, normalX, normalY, normalZ, U, V) tuples
// vBufferCount should be 8*nVertices
// iBuffer is filled with (v1, v2, v3) tuples of the triangles
// iBufferCount should be 3*nTriangles
SGV_OBJ_DEF void sgv_obj_read(const char* filename,
                              float* vBuffer, int vBufferCount,
                              float* iBuffer, int iBufferCount);


#ifdef __cplusplus
}
#endif

#endif

#ifdef SGV_OBJ_IMPLEMENTATION
SGV_OBJ_DEF void sgv_obj_readsize(const char* filename, 
                                  int* nVertices, 
                                  int* nTriangles)
{
    FILE* fp = fopen(filename, "r");

    if (fp == NULL) // Error. Could not open file
    {
        nVertices = -1;
        nTriangles = -1;
        return;
    }

    

    fclose(fp);
}
#endif
