/* sgv_glmath - Lib for 3d matrix transforms like scaling, perspective, etc.

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
Do this:
    #define SGV_GLMATH_IMPLEMENTATION
    before you include this file in *one* C or C++ file to create the implementation.

NOTES
-----

- The comments next to the function declarations below serve as documentation.
- All angles are expected to be in radians.
- All matrices are expected to be 4x4 matrices in row-major order.
- Warning: OpenGL likes column major MVP matrices. Don't forget to transpose!
- Uses C standard math lib for trigonometric functions.

EXAMPLE
-------

float mvpMatrix[16];
sgv_glm_eye(mvpMatrix); // make mvpMatrix a 4x4 identity matrix
sgv_glm_scale(mvpMatrix, 2.0f, 3.0f, 1.0f); // scale mvpMatrix
sgv_glm_translate(mvpMatrix, 10.0f, 2.0f, 5.0f); // Then translate the scaled obj
sgv_glm_transpose(mvpMatrix);
glUniformMatrix4fv(mvpUniform, 1, GL_FALSE, mvpMatrix); // Transpose arg has to be GL_FALSE!

*/

////////////////////////////////////////////////////////////////////////////////
////////////////////////////// Public API //////////////////////////////////////
#ifndef SGV_GLMATH_H
#define SGV_GLMATH_H

#ifdef __cplusplus
extern "C" {
#endif

void sgv_glm_eye(float* res); // res = I (4x4 identity matrix)
void sgv_glm_transpose(float* res); // res = res'
void sgv_glm_mul(float* res, float* a, float* b); // res = a*b
void sgv_glm_premul(float* res, float* m); // res = m * res
void sgv_glm_cpy(float* dest, float* src); // dest = src
void sgv_glm_rotateZ(float* res, float theta); // Roll; res = Rz * res
void sgv_glm_rotateY(float* res, float theta); // Pitch; res = Ry * res
void sgv_glm_rotateX(float* res, float theta); // Yaw; res = Rx * res
void sgv_glm_translate(float* res, float x, float y, float z); // res = T * res
void sgv_glm_scale(float* res, float x, float y, float z); // res = S * res
void sgv_glm_lookAt(float* res,
                    float eyeX, float eyeY, float eyeZ,
                    float targetX, float targetY, float targetZ,
                    float upX, float upY, float upZ); // res = ViewMat * res.
                    // Eye is the location of the camera and
                    // Center is what the camera is looking at.
void sgv_glm_perspective(float* res, float fovY, float aspect, float nearZ, float farZ); // res = perspective * res

#ifdef __cplusplus
}
#endif

#endif

////////////////////////////////////////////////////////////////////////////////
///////////////////////////// Implementation ///////////////////////////////////
#ifdef SGV_GLMATH_IMPLEMENTATION

#include <math.h>

void sgv_glm_eye(float* res)
{
    int i;
    for(i = 0; i < 16; i++)
        res[i] = 0.0f;
    res[0] = res[5] = res[10] = res[15] = 1.0f;
}
void sgv_glm_cpy(float* dest, float* src)
{
    int i;
    for(i=0; i < 16; i++)
    {
        dest[i] = src[i];
    }
}
void sgv_glm_transpose(float* res)
{
    float tmp;
    int i, j;
    for(i = 0; i < 4; i++)
    {
        for(j = i; j < 4; j++)
        {
            tmp = res[4*i+j];
            res[4*i+j] = res[4*j+i];
            res[4*j+i] = tmp;
        }
    }
}
void sgv_glm_mul(float* res, float* a, float* b)
{
    int i, j, k;
    float sum;
    for(i = 0; i < 4; i++)
    {
        for(j = 0; j < 4; j++)
        {
            sum = 0;
            for(k = 0; k < 4; k++)
            {
                sum += a[4*i+k] * b[4*k + j];
            }
            res[4*i + j] = sum;
        }
    }
}
void sgv_glm_premul(float* res, float* m)
{
    float temp[16];
    sgv_glm_mul(temp, m, res);
    sgv_glm_cpy(res, temp);
}
void sgv_glm_scale(float* res, float x, float y, float z)
{
    float s[16];
    sgv_glm_eye(s);
    s[0] = x;
    s[5] = y;
    s[10] = z;

    sgv_glm_premul(res, s);
}
void sgv_glm_translate(float* res, float x, float y, float z)
{
    float t[16];
    sgv_glm_eye(t);
    t[3] = x;
    t[7] = y;
    t[11] = z;

    sgv_glm_premul(res, t);
}
void sgv_glm_rotateZ(float* res, float theta)
{
    float r[16];
    sgv_glm_eye(r);
    r[0] = r[5] = cos(theta);
    r[1] = -sin(theta);
    r[4] = sin(theta);

    sgv_glm_premul(res, r);
}
void sgv_glm_rotateX(float* res, float theta)
{
    float r[16];
    sgv_glm_eye(r);
    r[5] = r[10] = cos(theta);
    r[6] = -sin(theta);
    r[9] = sin(theta);

    sgv_glm_premul(res, r);
}
void sgv_glm_rotateY(float* res, float theta)
{
    float r[16];
    sgv_glm_eye(r);
    r[0] = r[10] = cos(theta);
    r[2] = -sin(theta);
    r[8] = sin(theta);

    sgv_glm_premul(res, r);
}
static void sgvp_norm3(float* res)
{
    float norm = sqrt(res[0]*res[0] + res[1]*res[1] + res[2]*res[2]);
    res[0] /= norm; res[1] /= norm; res[2] /= norm;
}
static void sgvp_cross3(float* res, float* a, float* b)
{
    // res = a x b
    res[0] = a[1]*b[2] - b[1]*a[2];
    res[1] = b[0]*a[2] - a[0]*b[2];
    res[2] = a[0]*b[1] - b[0]*a[1];
}
/*
static void dispMat(float* mat)
{
    int i, j;
    printf("\n");
    for(i = 0; i < 4; i++)
    {
        printf("|\t");
        for(j = 0; j < 4; j++)
        {
            printf("%.2f\t", mat[4*i + j]);
        }
        printf("|\n");
    }
    printf("\n");
}
*/
void sgv_glm_lookAt(float* res,
                    float eyeX, float eyeY, float eyeZ,
                    float targetX, float targetY, float targetZ,
                    float upX, float upY, float upZ)
{
    float v[16];
    // v = (scale -1 on z-axis) * (change of basis matrix) * (translate matrix)
    // Change of basis matrix transforms:
    //      ex (1, 0, 0), ey (0, 1, 0) and ez (0, 0, 1)
    //   TO
    //      s,            u,           and f
    //   where f = norm(centre - eye), s = f x up, and u = f x l
    //
    //           | 1  0  0  0 |   |  s[0]   s[1]   s[2]   0  |   | 1  0  0  -ex |
    //      v  = | 0  1  0  0 | * |  u[0]   u[1]   u[2]   0  | * | 0  1  0  -ey |
    //           | 0  0 -1  0 |   |  f[0]   f[1]   f[2]   0  |   | 0  0  1  -ez |
    //           | 0  0  0  1 |   |  0      0      0      1  |   | 0  0  0   1  |

    float s[3], u[3], f[3];
    f[0] = targetX - eyeX; f[1] = targetY - eyeY; f[2] = targetZ - eyeZ; sgvp_norm3(f);
    u[0] = upX; u[1] = upY; u[2] = upZ;
    sgvp_cross3(s, u, f); sgvp_norm3(s);
    sgvp_cross3(u, f, s); sgvp_norm3(u);

    v[0] =  s[0]; v[1] =  s[1]; v[2] =  s[2]; v[3] = -(s[0]*eyeX + s[1]*eyeY + s[2]*eyeZ);
    v[4] =  u[0]; v[5] =  u[1]; v[6] =  u[2]; v[7] = -(u[0]*eyeX + u[1]*eyeY + u[2]*eyeZ);
    v[8] = -f[0]; v[9] = -f[1]; v[10]= -f[2]; v[11]= (f[0]*eyeX + f[1]*eyeY + f[2]*eyeZ);
    v[12]=  0   ; v[13]=  0   ; v[14]=  0   ; v[15]=  1;

    sgv_glm_premul(res, v);
}
void sgv_glm_perspective(float* res, float fovY, float aspect, float nearZ, float farZ)
{
    float p[16];
    float height = nearZ * tan(fovY/2); // tan(fovy/2) = (height) / zNear. height is half the screen height
    float width = aspect * height; // aspect = width / height

    sgv_glm_eye(p);
    p[0] = nearZ/width;
    p[5] = nearZ/height;
    p[10] = -(farZ + nearZ) / (farZ - nearZ);
    p[11] = -2.0f * nearZ * farZ / (farZ - nearZ);
    p[14] = -1.0f;
    p[15] = 0.0f;

    sgv_glm_premul(res, p);
}
#endif
