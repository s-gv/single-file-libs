/*  sgv_glmath.h - Public domain lib for 3d matrix transforms like scaling,
    perspective, etc.

Authored in 2015 by Sagar Gubbi (sagar.writeme@gmail.com).

Do this:
    #define SGV_GLMATH_IMPLEMENTATION
before you include this file in *one* C or C++ file to create the
implementation.

NOTES
-----

- The comments next to the function declarations below serve as documentation.
- All angles are expected to be in radians.
- All matrices are expected to be 4x4 matrices in row-major order.
- OpenGL likes column major MVP matrices. Don't forget to transpose!

EXAMPLE
-------

float mvp_matrix[16];
sgv_glm_eye(mvp_matrix); // make mvp_matrix a 4x4 identity matrix
sgv_glm_scale(mvp_matrix, 2.0f, 3.0f, 1.0f); // scale mvp_matrix
sgv_glm_translate(mvp_matrix, 10.0f, 2.0f, 5.0f); // Translate the scaled obj
sgv_glm_transpose(mvp_matrix);
glUniformMatrix4fv(mvp_uniform, 1, GL_FALSE, mvp_matrix);

OPTIONS
-------

- If you want all functions in this lib to be static,
  #define SGV_GLMATH_STATIC
  before including this file.
- If you don't want to use the C math library,
  #define SGV_GLMATH_NO_MATH_LIBC
  before including this file in the C file where you have
  defined SGV_GLMATH_IMPLEMENTATION. You must then provide implementations of
  sgv_glm_sin(x), sgv_glm_cos(x), and sgv_glm_tan(x).

LICENSE
-------

This software is in the public domain. Where that dedication is not
recognized, redistribution and use in source and binary forms, with
or without modification, are permitted. No warranty for any purpose
is expressed or implied.

*/

/*****************************************************************************
****************************** Public API ***********************************/
#ifndef SGV_GLMATH_H
#define SGV_GLMATH_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SGV_GLMATH_STATIC
#define SGVGLM_DEF static
#else
#define SGVGLM_DEF extern
#endif

/* res = I (4x4 identity matrix) */
SGVGLM_DEF void sgv_glm_eye(float* res);

/* res = res' */
SGVGLM_DEF void sgv_glm_transpose(float* res);

/* res = a*b */
SGVGLM_DEF void sgv_glm_mul(float* res, float* a, float* b);

/* res = m * res */
SGVGLM_DEF void sgv_glm_premul(float* res, float* m);

/* dest = src */
SGVGLM_DEF void sgv_glm_cpy(float* dest, float* src);

/* Roll; res = Rz * res */
SGVGLM_DEF void sgv_glm_rotate_z(float* res, float theta);

/* Pitch; res = Ry * res */
SGVGLM_DEF void sgv_glm_rotate_y(float* res, float theta);

/* Yaw; res = Rx * res */
SGVGLM_DEF void sgv_glm_rotate_x(float* res, float theta);

/* res = T * res */
SGVGLM_DEF void sgv_glm_translate(float* res, float x, float y, float z);

/* res = S * res */
SGVGLM_DEF void sgv_glm_scale(float* res, float x, float y, float z);

/* res = ViewMat * res. Eye is the location of the camera and
   Center is what the camera is looking at. */
SGVGLM_DEF void sgv_glm_look_at(float* res,
                                float eye_x, float eye_y, float eye_z,
                                float center_x, float center_y, float center_z,
                                float up_x, float up_y, float up_z);

/* res = perspectiveMat * res */
SGVGLM_DEF void sgv_glm_perspective(float* res, float fov_y, float aspect,
                                    float near_z, float far_z);

#ifdef __cplusplus
}
#endif

#endif

/*****************************************************************************
****************************** Implementation********************************/
#ifdef SGV_GLMATH_IMPLEMENTATION

#ifndef SGV_GLMATH_NO_MATH_LIBC

#include <math.h>
#define sgv_glm_sin(x) sin(x)
#define sgv_glm_cos(x) cos(x)
#define sgv_glm_tan(x) tan(x)

#endif

SGVGLM_DEF void sgv_glm_eye(float* res)
{
    int i;
    for(i = 0; i < 16; i++)
        res[i] = 0.0f;
    res[0] = res[5] = res[10] = res[15] = 1.0f;
}

SGVGLM_DEF void sgv_glm_cpy(float* dest, float* src)
{
    int i;
    for(i=0; i < 16; i++)
    {
        dest[i] = src[i];
    }
}

SGVGLM_DEF void sgv_glm_transpose(float* res)
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

SGVGLM_DEF void sgv_glm_mul(float* res, float* a, float* b)
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

SGVGLM_DEF void sgv_glm_premul(float* res, float* m)
{
    float temp[16];
    sgv_glm_mul(temp, m, res);
    sgv_glm_cpy(res, temp);
}

SGVGLM_DEF void sgv_glm_scale(float* res, float x, float y, float z)
{
    float s[16];
    sgv_glm_eye(s);
    s[0] = x;
    s[5] = y;
    s[10] = z;

    sgv_glm_premul(res, s);
}

SGVGLM_DEF void sgv_glm_translate(float* res, float x, float y, float z)
{
    float t[16];
    sgv_glm_eye(t);
    t[3] = x;
    t[7] = y;
    t[11] = z;

    sgv_glm_premul(res, t);
}

SGVGLM_DEF void sgv_glm_rotate_z(float* res, float theta)
{
    float r[16];
    sgv_glm_eye(r);
    r[0] = r[5] = sgv_glm_cos(theta);
    r[1] = -sgv_glm_sin(theta);
    r[4] = sgv_glm_sin(theta);

    sgv_glm_premul(res, r);
}

SGVGLM_DEF void sgv_glm_rotate_x(float* res, float theta)
{
    float r[16];
    sgv_glm_eye(r);
    r[5] = r[10] = sgv_glm_cos(theta);
    r[6] = -sgv_glm_sin(theta);
    r[9] = sgv_glm_sin(theta);

    sgv_glm_premul(res, r);
}

SGVGLM_DEF void sgv_glm_rotate_y(float* res, float theta)
{
    float r[16];
    sgv_glm_eye(r);
    r[0] = r[10] = sgv_glm_cos(theta);
    r[2] = -sgv_glm_sin(theta);
    r[8] = sgv_glm_sin(theta);

    sgv_glm_premul(res, r);
}

static void sgvp_norm3(float* res)
{
    float norm = sqrt(res[0]*res[0] + res[1]*res[1] + res[2]*res[2]);
    res[0] /= norm; res[1] /= norm; res[2] /= norm;
}

static void sgvp_cross3(float* res, float* a, float* b)
{
    /* res = a x b */
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
SGVGLM_DEF void sgv_glm_look_at(float* res,
                                float eye_x, float eye_y, float eye_z,
                                float target_x, float target_y, float target_z,
                                float up_x, float up_y, float up_z)
{
    float v[16];
    /* v = (scale -1 on z-axis) * (change of basis matrix) * (translate matrix)
       Change of basis matrix transforms:
            ex (1, 0, 0), ey (0, 1, 0) and ez (0, 0, 1)
         TO
            s,            u,           and f
         where f = norm(centre - eye), s = f x up, and u = f x l

              | 1  0  0  0 |   |  s[0]   s[1]   s[2]   0  |   | 1  0  0  -ex |
         v  = | 0  1  0  0 | * |  u[0]   u[1]   u[2]   0  | * | 0  1  0  -ey |
              | 0  0 -1  0 |   |  f[0]   f[1]   f[2]   0  |   | 0  0  1  -ez |
              | 0  0  0  1 |   |  0      0      0      1  |   | 0  0  0   1  |
    */

    float s[3], u[3], f[3];

    f[0] = target_x - eye_x; f[1] = target_y - eye_y; f[2] = target_z - eye_z;
    sgvp_norm3(f);

    u[0] = up_x; u[1] = up_y; u[2] = up_z;
    sgvp_cross3(s, u, f); sgvp_norm3(s);
    sgvp_cross3(u, f, s); sgvp_norm3(u);

    v[0] =  s[0];
    v[1] =  s[1];
    v[2] =  s[2];
    v[3] = -(s[0]*eye_x + s[1]*eye_y + s[2]*eye_z);

    v[4] =  u[0];
    v[5] =  u[1];
    v[6] =  u[2];
    v[7] = -(u[0]*eye_x + u[1]*eye_y + u[2]*eye_z);

    v[8] = -f[0];
    v[9] = -f[1];
    v[10] = -f[2];
    v[11] = (f[0]*eye_x + f[1]*eye_y + f[2]*eye_z);

    v[12] =  0;
    v[13] =  0;
    v[14] =  0;
    v[15] =  1;

    sgv_glm_premul(res, v);
}

SGVGLM_DEF void sgv_glm_perspective(float* res, float fov_y, float aspect,
                                    float near_z, float far_z)
{
    float p[16];
    float height = near_z * tan(fov_y/2); /* tan(fovy/2) = (height) / zNear */
    float width = aspect * height; /* aspect = (width / height) */

    sgv_glm_eye(p);
    p[0] = near_z/width;
    p[5] = near_z/height;
    p[10] = -(far_z + near_z) / (far_z - near_z);
    p[11] = -2.0f * near_z * far_z / (far_z - near_z);
    p[14] = -1.0f;
    p[15] = 0.0f;

    sgv_glm_premul(res, p);
}

#endif
