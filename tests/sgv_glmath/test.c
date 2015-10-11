#include <stdio.h>
#include <assert.h>
#include <math.h>

#define SGV_GLMATH_IMPLEMENTATION
#include "sgv_glmath.h"

int main() {
    float mat[16];
    float EPS = 1e-2f;

    sgv_glm_eye(mat);
    assert(fabs(mat[0] - 1.0f) < EPS);
    printf("Test 1 passed . . .\n");

    sgv_glm_eye(mat);
    sgv_glm_scale(mat, 2.0f, 3.0f, 4.0f);
    assert(fabs(mat[5] - 3.0f) < EPS);
    printf("Test 2 passed . . .\n");

    printf("All tests done . . .\n");
    return 0;
}
