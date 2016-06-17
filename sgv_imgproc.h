/* sgv_imgproc - public domain lib of image processing routines

Authored in 2016 by Sagar Gubbi (sagar.writeme@gmail.com).

Before #including,
    #define SGV_IMGP_IMPLEMENTATION
in the file where you want to have the implementation.

OPTIONS
-------

When #including this file along with the implementation,

- If you want all functions in this lib to be static,
  #define SGV_IMGP_STATIC before #including this file.
- If you dont want <assert.h> (or you want different behviour),
  #define SGV_IMGP_ASSERT(x) before #including this file.
- If you dont want <math.h>, #define SGV_IMGP_FABS(x) and SGV_IMGP_EXP(x)
  before #including this file.

LICENSE
-------

This software is dual-licensed to the public domain and under the following
license: you are granted a perpetual, irrevocable license to copy, modify,
publish, and distribute this file as you see fit.

*/

#ifndef SGV_IMGP_H
#define SGV_IMGP_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SGV_IMGP_STATIC
#define SGVIMGP_DEF static
#else
#define SGVIMGP_DEF extern
#endif

/*****************************************************************************
****************************** Public API ***********************************/
typedef struct {
    unsigned char* data; /* in HWC format */
    int w, h, d; /* width, height, no. of channels */
} sgv_img;

typedef struct {
    float* data; /* in HWC format */
    int w, h, d;
} sgv_fimg;

typedef struct {
    float* data; /*[filter_height, filter_width, in_channels, out_channels]*/
    int w, h, ind, outd;
} sgv_filt;

typedef struct {
    int x, y;
} sgv_imgp_i2;

typedef struct {
    int x, y, z;
} sgv_imgp_i3;

typedef struct {
    int x, y, z, w;
} sgv_imgp_i4;

/* Copy in to out. [0-255] in 'in' will be [-1.0, 1.0] in 'out' */
SGVIMGP_DEF void sgv_make_fimg(sgv_img in, sgv_fimg out);

/* Draw line between 'p1' and 'p2' in 'img' */
SGVIMGP_DEF void sgv_draw_line(sgv_img img,
                               sgv_imgp_i2 p1, sgv_imgp_i2 p2,
                               sgv_imgp_i3 color, int thickness);


/* Draw quadrilateral defined by (p1, p2, p3, p4) on img */
SGVIMGP_DEF void sgv_draw_quadrilateral(sgv_img img,
                                        sgv_imgp_i2 p1, sgv_imgp_i2 p2,
                                        sgv_imgp_i2 p3, sgv_imgp_i2 p4,
                                        sgv_imgp_i3 color, int thickness);

/* 2D convolution */
SGVIMGP_DEF void sgv_conv2d_valid(sgv_fimg img, sgv_filt filt, sgv_fimg out);

/* Add a bias to all pixels each channel. len(biases) == img.d */
SGVIMGP_DEF void sgv_add_bias(sgv_fimg img, float* biases);

/* Rectifies the image */
SGVIMGP_DEF void sgv_relu(sgv_fimg in, sgv_fimg out);

/* Max pool 2x2. */
SGVIMGP_DEF void sgv_maxpool2(sgv_fimg in, sgv_fimg out);

/* convert scores to probabilities */
SGVIMGP_DEF void sgv_softmax(float* scores_in, int n, float* probs_out);

/* Apply affine transform. in_offset is the offset in the input image for the
   the transform operation. So, out[0, 0] == in[in_offset.x, in_offset.y].
   theta is the 2x2 transform matrix */
SGVIMGP_DEF void sgv_imgp_affine_transform(sgv_img in, sgv_imgp_i2 in_offset,
                                           float* theta, sgv_img out);

/* Crop and rescale the image */
SGVIMGP_DEF void sgv_imgp_crop_rescale(sgv_img in, sgv_imgp_i2 in_left_top,
                                      sgv_imgp_i2 crop_size, sgv_img out);

/* Finds the threshold that minimizes intra-class variance (Otsu threshold) */
SGVIMGP_DEF unsigned char sgv_imgp_otsu(sgv_img img);

/* Enhance contrast with histogram equalization */
SGVIMGP_DEF void sgv_imgp_enhance_contrast(sgv_img in, sgv_img out);

#ifdef __cplusplus
}
#endif

#endif

/*****************************************************************************
****************************** Implementation********************************/
#ifdef SGV_IMGP_IMPLEMENTATION

#define SGV_IMGP_ABS(x) ((x > 0) ? x : -(x))

#ifndef SGV_IMGP_FABS
#include <math.h>
#define SGV_IMGP_FABS(x) fabs(x)
#define SGV_IMGP_EXP(x) exp(x)
#endif

#ifndef SGV_IMGP_ASSERT
#include <assert.h>
#define SGV_IMGP_ASSERT(x) assert(x)
#endif

SGVIMGP_DEF void sgv_make_fimg(sgv_img in, sgv_fimg out)
{
    int x, y, d;
    SGV_IMGP_ASSERT(in.w == out.w && in.h == out.h && in.d == out.d);
    for(y = 0; y < in.h; y++) {
        for(x = 0; x < in.w; x++) {
            for(d = 0; d < in.d; d++) {
                float v = (in.data[in.w*in.d*y + in.d*x + d] - 127.0f)/128.0f;
                out.data[in.w*in.d*y + in.d*x + d] = v;
            }
        }
    }
}

SGVIMGP_DEF void sgvp_draw_line_prim(sgv_img img,
                                     int x0, int y0, int x1, int y1,
                                     sgv_imgp_i3 color)
{
    float dx, dy, x_inc, y_inc, x, y;
    int i, px, py, steps;

    dx = x1 - x0;
    dy = y1 - y0;

    steps = SGV_IMGP_ABS(x1-x0);
    if(SGV_IMGP_FABS(dx) <  SGV_IMGP_FABS(dy)) {
        steps =  SGV_IMGP_ABS(y1-y0);
    }

    x_inc = dx / steps;
    y_inc = dy / steps;

    x = x0;
    y = y0;

    for(i = 0; i < steps; i++) {
        px = (int)x, py = (int)y;
        if(px >= 0 && py >= 0 && px < img.w && py < img.h) {
            img.data[img.d*img.w*py + img.d*px + 0] = color.x;

            if(img.d > 1)
                img.data[img.d*img.w*py + img.d*px + 1] = color.y;
            else
                img.data[img.d*img.w*py + img.d*px + 1] = 0;

            if(img.d > 2)
                img.data[img.d*img.w*py + img.d*px + 2] = color.z;
            else
                img.data[img.d*img.w*py + img.d*px + 2] = 0;
        }
        x += x_inc;
        y += y_inc;
    }
}

SGVIMGP_DEF void sgv_draw_line(sgv_img img,
                               sgv_imgp_i2 p1, sgv_imgp_i2 p2,
                               sgv_imgp_i3 color, int t)
{
    int d;
    for(d = -t/2; d <= t/2; d++) {
        sgvp_draw_line_prim(img, p1.x+d, p1.y, p2.x+d, p2.y, color);
        sgvp_draw_line_prim(img, p1.x, p1.y+d, p2.x, p2.y+d, color);
    }
}

SGVIMGP_DEF void sgv_draw_quadrilateral(sgv_img img,
                                        sgv_imgp_i2 p1, sgv_imgp_i2 p2,
                                        sgv_imgp_i2 p3, sgv_imgp_i2 p4,
                                        sgv_imgp_i3 color, int t)
{
    sgv_draw_line(img, p1, p2, color, t);
    sgv_draw_line(img, p2, p3, color, t);
    sgv_draw_line(img, p3, p4, color, t);
    sgv_draw_line(img, p4, p1, color, t);
}

SGVIMGP_DEF void sgv_imgp_affine_transform(sgv_img in, sgv_imgp_i2 in_offset,
                                           float* theta, sgv_img out)
{
    float x, y, xt, yt, alpha, beta, temp;
    int ix, iy, c, xo_a, yo_a, xo_b, yo_b;

    for(iy = 0; iy < out.h; iy++) {
        for(ix = 0; ix < out.w; ix++) {
            x = (ix + 0.5f) / out.w;
            y = (iy + 0.5f) / out.h;

            xt = theta[0]*x + theta[1]*y + (in_offset.x + 0.5f)/in.w;
            yt = theta[2]*x + theta[3]*y + (in_offset.y + 0.5f)/in.h;

            xo_a = xt*in.w - 0.5f, xo_b = xo_a+1; alpha = xo_a + 1.5f - xt*in.w;
            yo_a = yt*in.h - 0.5f, yo_b = yo_a+1; beta = yo_a + 1.5f - yt*in.h;

            for(c = 0; c < in.d; c++) {
                temp = 0;

                if(xo_a >= 0 && yo_a >= 0 && xo_a < in.w && yo_a < in.h)
                    temp += alpha * beta * in.data[in.w*in.d*yo_a + in.d*xo_a + c];
                if(xo_b >= 0 && yo_a >= 0 && xo_b < in.w && yo_a < in.h)
                    temp += (1-alpha) * beta * in.data[in.w*in.d*yo_a + in.d*xo_b + c];
                if(xo_a >= 0 && yo_b >= 0 && xo_a < in.w && yo_b < in.h)
                    temp += alpha * (1-beta) * in.data[in.w*in.d*yo_b + in.d*xo_a + c];
                if(xo_b >= 0 && yo_b >= 0 && xo_b < in.w && yo_b < in.h)
                    temp += (1-alpha) * (1-beta) * in.data[in.w*in.d*yo_b + in.d*xo_b + c];

                out.data[out.d*(out.w*iy + ix) + c] = temp;
            }
        }
    }
}

SGVIMGP_DEF void sgv_imgp_crop_rescale(sgv_img in, sgv_imgp_i2 in_left_top,
                                       sgv_imgp_i2 crop_size, sgv_img out)
{
    int x, y, c, temp, x1, y1, enlarged_w, enlarged_h;
    int dsf; /* down scale factor */
    int ds_factor_w, ds_factor_h;
    float xi, yi, alpha, beta;
    int xo_a, yo_a, xo_b, yo_b;

    SGV_IMGP_ASSERT(in.d == out.d);

    ds_factor_w = (crop_size.x+out.w-1)/out.w;
    ds_factor_h = (crop_size.y+out.h-1)/out.h;
    dsf = ds_factor_w > ds_factor_h ? ds_factor_w : ds_factor_h;

    enlarged_w = out.w * dsf;
    enlarged_h = out.h * dsf;

    for(y = 0; y < out.h; y++) {
        for(x = 0; x < out.w; x++) {
            for(c = 0; c < out.d; c++) {
                temp = 0;
                for(y1 = dsf*y; y1 < dsf*(y+1); y1++) {
                    for(x1 = dsf*x; x1 < dsf*(x+1); x1++) {
                        xi = (x1 + 0.5f)/enlarged_w;
                        yi = (y1 + 0.5f)/enlarged_h;

                        xo_a = xi*crop_size.x - 0.5f + in_left_top.x, xo_b = xo_a+1;
                        alpha = xo_a - in_left_top.x + 1.5f - xi*crop_size.x;

                        yo_a = yi*crop_size.y - 0.5f + in_left_top.y, yo_b = yo_a+1;
                        beta = yo_a - in_left_top.y + 1.5f - yi*crop_size.y;

                        if(xo_a >= 0 && yo_a >= 0 && xo_a < in.w && yo_a < in.h)
                            temp += alpha * beta * in.data[in.w*in.d*yo_a + in.d*xo_a + c];
                        if(xo_b >= 0 && yo_a >= 0 && xo_b < in.w && yo_a < in.h)
                            temp += (1-alpha) * beta * in.data[in.w*in.d*yo_a + in.d*xo_b + c];
                        if(xo_a >= 0 && yo_b >= 0 && xo_a < in.w && yo_b < in.h)
                            temp += alpha * (1-beta) * in.data[in.w*in.d*yo_b + in.d*xo_a + c];
                        if(xo_b >= 0 && yo_b >= 0 && xo_b < in.w && yo_b < in.h)
                            temp += (1-alpha) * (1-beta) * in.data[in.w*in.d*yo_b + in.d*xo_b + c];
                    }
                }
                out.data[out.w*out.d*y + out.d*x + c] = temp/(dsf*dsf);
            }
        }
    }
}

SGVIMGP_DEF void sgv_conv2d_valid(sgv_fimg in, sgv_filt filt, sgv_fimg out)
{
    float tmp, fv;
    int xi, yi, ci, xo, yo, co, xf, yf;

    SGV_IMGP_ASSERT(filt.ind == in.d && filt.outd == out.d);
    SGV_IMGP_ASSERT(out.w == (in.w - filt.w + 1));
    SGV_IMGP_ASSERT(out.h == (in.h - filt.h + 1));

    for(yo = 0; yo < out.h; yo++) {
        for(xo = 0; xo < out.w; xo++) {
            for(co = 0; co < out.d; co++) {
                tmp = 0;
                for(yi = yo, yf = 0; yi < yo + filt.h; yi++, yf++) {
                    for(xi = xo, xf = 0; xi < xo + filt.w; xi++, xf++) {
                        for(ci = 0; ci < in.d; ci++) {
                            fv = filt.data[((yf*filt.w + xf)*in.d + ci)*out.d + co];
                            tmp += in.data[in.w*in.d*yi + in.d*xi + ci] * fv;
                        }
                    }
                }
                out.data[(yo*out.w + xo)*out.d + co] = tmp;
            }
        }
    }
}

SGVIMGP_DEF void sgv_add_bias(sgv_fimg img, float* biases)
{
    int x, y, c;
    for(y = 0; y < img.h; y++) {
        for(x = 0; x < img.w; x++) {
            for(c = 0; c < img.d; c++) {
                img.data[img.w*img.d*y + img.d*x + c] += biases[c];
            }
        }
    }
}

SGVIMGP_DEF void sgv_relu(sgv_fimg in, sgv_fimg out)
{
    int x, y, c;
    float val;

    SGV_IMGP_ASSERT(in.w == out.w && in.h == out.h && in.d == out.d);

    for(y = 0; y < in.h; y++) {
        for(x = 0; x < in.w; x++) {
            for(c = 0; c < in.d; c++) {
                val = in.data[in.w*in.d*y + in.d*x + c];
                out.data[in.w*in.d*y + in.d*x + c] = (val > 0) ? val : 0;
            }
        }
    }
}

SGVIMGP_DEF void sgv_maxpool2(sgv_fimg in, sgv_fimg out)
{
    int x, y, c, x1, y1;
    float val, new_val;

    SGV_IMGP_ASSERT(in.w/2 == out.w && in.h/2 == out.h && in.d == out.d);

    for(y = 0; y < out.w; y++) {
        for(x = 0; x < out.w; x++) {
            for(c = 0; c < out.d; c++) {
                val = in.data[in.w*in.d*(2*y) + in.d*(2*x) + c];
                for(y1 = 2*y; y1 < 2*(y+1); y1++) {
                    for(x1 = 2*x; x1 < 2*(x+1); x1++) {
                        new_val = in.data[in.w*in.d*y1 + in.d*x1 + c];
                        if(new_val > val) {
                            val = new_val;
                        }
                    }
                }
                out.data[out.w*out.d*y + out.d*x + c] = val;
            }
        }
    }
}

SGVIMGP_DEF void sgv_softmax(float* scores_in, int n, float* probs_out)
{
    int i;
    float max_score, sum;

    max_score = scores_in[0];
    for(i = 0; i < n; i++) {
        if(scores_in[i] > max_score) {
            max_score = scores_in[i];
        }
    }

    sum = 0;
    for(i = 0; i < n; i++) {
        probs_out[i] = SGV_IMGP_EXP(scores_in[i] - max_score);
        sum += probs_out[i];
    }

    for(i = 0; i < n; i++) {
        probs_out[i] /= sum;
    }
}

SGVIMGP_DEF unsigned char sgv_imgp_otsu(sgv_img img)
{
    int x, y, i, sum1, total, wB, wF, sumB, mB, mF;
    int objective, level, max_objective;
    int hist[256] = {0};

    SGV_IMGP_ASSERT(img.d == 1);

    for(y = 0; y < img.h; y++) {
        for(x = 0; x < img.w; x++) {
            hist[img.data[y*img.w + x]] += 1;
        }
    }

    sum1 = 0;
    total = 0;
    for(i = 0; i < 256; i++) {
        sum1 += i*hist[i];
        total += hist[i];
    }

    wB = 0;
    wF = 0;
    sumB = 0;
    max_objective = 0;
    level = 0;
    for(i = 0; i < 256; i++) {
        wB += hist[i];
        if(wB == 0) {
            continue;
        }

        wF = total - wB;
        if(wF == 0) {
            break;
        }

        sumB += i*hist[i];
        mB = sumB / wB;
        mF = (sum1 - sumB) / wF;
        objective = wB * wF * (mB - mF) * (mB - mF);
        if(objective >= max_objective) {
            level = i;
            max_objective = objective;
        }
    }

    return level;
}

SGVIMGP_DEF void sgv_imgp_enhance_contrast(sgv_img in, sgv_img out)
{
    int x, y, i;
    int cdf_min;
    int cdf[256] = {0};

    SGV_IMGP_ASSERT(in.d == 1);
    SGV_IMGP_ASSERT(in.w == out.w && in.h == out.h && in.d == out.d);

    for(y = 0; y < in.h; y++) {
        for(x = 0; x < in.w; x++) {
            int val = in.data[y*in.w + x];
            cdf[val]++;
        }
    }

    cdf_min = cdf[0];
    for(i = 1; i < 256; i++) {
        cdf[i] += cdf[i-1];
        if(cdf[i] < cdf_min) {
            cdf_min = cdf[i];
        }
    }

    for(y = 0; y < in.h; y++) {
        for(x = 0; x < in.w; x++) {
            out.data[y*out.w + x] = (int) ((cdf[in.data[y*in.w + x]] - cdf_min)*255.0f/(in.w*in.h - cdf_min));
        }
    }
}

#endif
