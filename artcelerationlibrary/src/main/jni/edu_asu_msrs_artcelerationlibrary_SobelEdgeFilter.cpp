/**
 * This is native(C++) code for Sobel Edge filter image transform.
 * This filter highlights edges in an image.
 * It has two filters: horizontal and vertical.
 */
#include <edu_asu_msrs_artcelerationlibrary_SobelEdgeFilter.h>
#include <time.h>
#include <android/log.h>
#include <android/bitmap.h>
#include <arm_neon.h>

#include <stdlib.h>
#include <math.h>

#define  LOG_TAG    "libimageprocessing"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)

using namespace std;
/**
 * This function performs the Sobel edge filter operation operation.
 * @param info : info is a pointer to AndroidBitmapInfo which provides height and width of the bitmap.
 * @param input : Pointer to input Bitmap.
 * @param gray : Pointer to gray Bitmap.
 * @param output : Pointer to resultant output transformed bitmap.
 * @param a0 : integer parameter as as intArgs[0] to select type of gradient.
 */
static void process(AndroidBitmapInfo* info, void* input, void* gray, void* output,int a0){
            uint32_t* line;
            uint32_t* line1;

            void* pxi = input;
            void* pxg = gray;
            void* pxo = output;

            //vertical edge filter
            int sx[3][3] = {{-1,0,1}, {-2,0,2}, {-1,0,1}} ;

            // horizontal edge filter
            int sy[3][3] = {{-1,-2,-1}, {0,0,0}, {1,2,1}} ;


            // Image represented by 4-bytes (4 channels as A=255,R, G, B)
            int r, g, b;
            int pix;
            int i, j, k;
            int w = info->width;
            int h = info->height;

            // Dynamic allocation for utilizing heap.
            int** gg = new int*[info->width];
            //int gg[w][h];
            for (i=0;i<info->width;i++){
                    gg[i]=new int [info->height];
            }

            /* ARM NEON SIMD implementation (for grayscale image)

                        uint8x8_t ascale = vdup_n_u8(255);
                        uint8x8_t rscale = vdup_n_u8(77);
                        uint8x8_t gscale = vdup_n_u8(151);
                        uint8x8_t bscale = vdup_n_u8(28);

                        w = w/8;
                        for(i=0; i< h; i++){
                            line = (uint8_t*)pxg;
                            line1 = (uint8_t*)pxi;
                            for(j=0; j< w; j++){
                                uint16x8_t temp;
                                uint8x8x4_t values = vld4_u8(line1);
                                uint8x8_t res;
                                //temp = vmull_u8(values.val[0], ascale);
                                temp = vmull_u8(values.val[1], rscale);
                                temp = vmlal_u8(temp, values.val[2], gscale);
                                temp = vmlal_u8(temp, values.val[3], bscale);

                                res = vshrn_n_u16(temp, 8);
                                vst1_u8(line, res);
                                line += 8;
                                line1 += 8*4;
                            }
                            pxg = (char*) pxg + info->stride;
                            pxi = (char*) pxi + info->stride;
                        }

            */

            // Creating grayscale image.
            int value;
            for(j=0; j<h; j++){
                line = (uint32_t*)pxg;
                line1 = (uint32_t*)pxi;
                        for(i=0; i< w; i++){
                            r = ((int)((line1[i] & 0x00FF0000)* 0.2989) >> 16);
                            g = ((int)((line1[i] & 0x0000FF00)* 0.5870) >> 8);
                            b = (int)((line1[i] & 0x00000FF)* 0.1140);
                            value = r+g+b;

                            // Setting output pixel
                            line[i] =
                            (((value << 16) & 0x00FF0000) |
                            ((value << 8) & 0x0000FF00) |
                            (value & 0x000000FF)|
                            0xFF000000);

                            // R=G=B. Hence, storing only 1 value for gradient calculation.
                            gg[i][j] = (line1[i] & 0x0000FF00) >> 8;
                        }
                        pxg = (char*) pxg + info->stride; // Moving to next row of gray bitmap after processing current row.
                        pxi = (char*) pxi + info->stride; // Moving to next row of input bitmap after processing current row.
                    }



            // Output image channel contains Grx.
            if (a0==0){
                int Grx[w][h];
                for (j = 1; j < h-1; j++) {
                    line = (uint32_t*)pxo;
                    for (i=1;i<w-1;i++) {
                        // Gradient calculation.
                        Grx[i][j] = sx[0][0]*gg[i-1][j-1] + sx[1][0]*gg[i][j-1]+ sx[2][0]*gg[i+1][j-1]
                                + sx[0][1]*gg[i-1][j]+ sx[1][1]*gg[i][j]+ sx[2][1]*gg[i+1][j]
                                + sx[0][2]*gg[i-1][j+1] + sx[1][2]*gg[i][j+1]+ sx[2][2]*gg[i+1][j+1];
                         Grx[i][j] = abs(Grx[i][j]);
                        // Setting output pixel
                        line[i] =
                        (((Grx[i][j] << 16) & 0x00FF0000) |
                        ((Grx[i][j] << 8) & 0x0000FF00) |
                        (Grx[i][j] & 0x000000FF)|
                         0xFF000000);
                    }
                    pxo = (char*) pxo + info->stride; // Moving to next row of output bitmap after processing current row.
                }

                // Output image channel contains Gry.
            }else if(a0==1){
                int Gry[w][h];
                for (j = 1; j < h-1; j++) {
                        line = (uint32_t*)pxo;
                        for (i=1;i<w-1;i++) {

                            // Gradient calculation.
                        Gry[i][j] =sy[0][0]*gg[i-1][j-1] + sy[1][0]*gg[i][j-1]+ sy[2][0]*gg[i+1][j-1]
                                + sy[0][1]*gg[i-1][j]+ sy[1][1]*gg[i][j]+ sy[2][1]*gg[i+1][j]
                                + sy[0][2]*gg[i-1][j+1] + sy[1][2]*gg[i][j+1]+ sy[2][2]*gg[i+1][j+1];

                        Gry[i][j] = abs(Gry[i][j]);
                            // Setting output pixel
                        line[i] =
                        (((Gry[i][j] << 16) & 0x00FF0000) |
                        ((Gry[i][j] << 8) & 0x0000FF00) |
                        (Gry[i][j] & 0x000000FF)|
                        0xFF000000);
                        }
                        pxo = (char*) pxo + info->stride; // Moving to next row of output bitmap after processing current row.
                }
                // Output image channel contains overall gradient magnitude.
            }else if (a0 == 2){
                int Grx[w][h];
                int Gry[w][h];
                for (j = 1; j < h-1; j++) {
                        line = (uint32_t*)pxo;
                        for (i=1;i<w-1;i++) {

                            // Gradient calculation. (Grx)
                        Grx[i][j] = sx[0][0]*gg[i-1][j-1] + sx[1][0]*gg[i][j-1]+ sx[2][0]*gg[i+1][j-1]
                                + sx[0][1]*gg[i-1][j]+ sx[1][1]*gg[i][j]+ sx[2][1]*gg[i+1][j]
                                + sx[0][2]*gg[i-1][j+1] + sx[1][2]*gg[i][j+1]+ sx[2][2]*gg[i+1][j+1];

                            // Gradient calculation. (Gry)
                        Gry[i][j] =sy[0][0]*gg[i-1][j-1] + sy[1][0]*gg[i][j-1]+ sy[2][0]*gg[i+1][j-1]
                                + sy[0][1]*gg[i-1][j]+ sy[1][1]*gg[i][j]+ sy[2][1]*gg[i+1][j]
                                + sy[0][2]*gg[i-1][j+1] + sy[1][2]*gg[i][j+1]+ sy[2][2]*gg[i+1][j+1];

                            // overall gradient
                        int Gr = (int) sqrt(Grx[i][j]*Grx[i][j] + Gry[i][j]*Gry[i][j]);

                            // Setting output pixel
                        line[i] =
                        (((Gr << 16) & 0x00FF0000) |
                        ((Gr << 8) & 0x0000FF00) |
                        (Gr & 0x000000FF)|
                        0xFF000000);
                    }
                    pxo = (char*) pxo + info->stride; // Moving to next row of output bitmap after processing current row.
                }
            }

            for (i=0;i<info->width;i++){
                    delete [] gg[i];  // Deleting dynamically allocated memory.
            }
            delete [] gg;
}

/*
 * JNI call for Sobel edge filter.
 * @param a0: integer parameter as as intArgs[0] to select type of gradient.
 * @param input: native equivalent of input bitmap.
 * @param grayscale: native equivalent of gray bitmap.
 * @param output: native equivalent of output bitmap.
 */

JNIEXPORT void JNICALL Java_edu_asu_msrs_artcelerationlibrary_SobelEdgeFilter_getSobelEdgeFilter
  (JNIEnv* env, jclass jc, jint a0, jobject input, jobject grayscale, jobject output){
          AndroidBitmapInfo  info_input;
          int ret;
          void* pixels_input;
          void* pixels_output;
          void* pixels_gray;

            // Get info for input bitmap.
          if ((ret = AndroidBitmap_getInfo(env, input, &info_input)) < 0) {
                  LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
                  return;
              }

            // Acquire a lock for input pixels.
          if ((ret = AndroidBitmap_lockPixels(env, input, &pixels_input)) < 0) {
              LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
          }

            // Acquire a lock for grayscale pixels.
           if ((ret = AndroidBitmap_lockPixels(env, grayscale, &pixels_gray)) < 0) {
                        LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
                    }

            // Acquire a lock for output pixels.
            if ((ret = AndroidBitmap_lockPixels(env, output, &pixels_output)) < 0) {
                LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
            }

            // Call process function for transform.
          process(&info_input, pixels_input, pixels_gray, pixels_output, a0);

            // Unlock pixels.
          AndroidBitmap_unlockPixels(env, input);
          AndroidBitmap_unlockPixels(env, output);
          AndroidBitmap_unlockPixels(env, grayscale);
  }

