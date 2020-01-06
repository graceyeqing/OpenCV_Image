#include <jni.h>
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <vector>
#include <android/bitmap.h>
#include <android/log.h>
#include <opencv2/opencv.hpp>
using namespace cv;

#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "error", __VA_ARGS__))
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "debug", __VA_ARGS__))

#define ASSERT(status, ret)     if (!(status)) { return ret; }
#define ASSERT_FALSE(status)    ASSERT(status, false)

bool BitmapToMat(JNIEnv * env, jobject obj_bitmap, cv::Mat & matrix) {
    void * bitmapPixels;                                            // 保存图片像素数据
    AndroidBitmapInfo bitmapInfo;                                   // 保存图片参数

    ASSERT_FALSE( AndroidBitmap_getInfo(env, obj_bitmap, &bitmapInfo) >= 0);        // 获取图片参数
    ASSERT_FALSE( bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGBA_8888
                  || bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGB_565 );          // 只支持 ARGB_8888 和 RGB_565
    ASSERT_FALSE( AndroidBitmap_lockPixels(env, obj_bitmap, &bitmapPixels) >= 0 );  // 获取图片像素（锁定内存块）
    ASSERT_FALSE( bitmapPixels );

    if (bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
        cv::Mat tmp(bitmapInfo.height, bitmapInfo.width, CV_8UC4, bitmapPixels);    // 建立临时 mat
        tmp.copyTo(matrix);                                                         // 拷贝到目标 matrix
    } else {
        cv::Mat tmp(bitmapInfo.height, bitmapInfo.width, CV_8UC2, bitmapPixels);
        cv::cvtColor(tmp, matrix, cv::COLOR_BGR5652RGB);
    }

    AndroidBitmap_unlockPixels(env, obj_bitmap);            // 解锁
    return true;
}

bool MatToBitmap(JNIEnv * env, cv::Mat & matrix, jobject obj_bitmap) {
    void * bitmapPixels;                                            // 保存图片像素数据
    AndroidBitmapInfo bitmapInfo;                                   // 保存图片参数

    ASSERT_FALSE( AndroidBitmap_getInfo(env, obj_bitmap, &bitmapInfo) >= 0);        // 获取图片参数
    ASSERT_FALSE( bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGBA_8888
                  || bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGB_565 );          // 只支持 ARGB_8888 和 RGB_565
    ASSERT_FALSE( matrix.dims == 2
                  && bitmapInfo.height == (uint32_t)matrix.rows
                  && bitmapInfo.width == (uint32_t)matrix.cols );                   // 必须是 2 维矩阵，长宽一致
    ASSERT_FALSE( matrix.type() == CV_8UC1 || matrix.type() == CV_8UC3 || matrix.type() == CV_8UC4 );
    ASSERT_FALSE( AndroidBitmap_lockPixels(env, obj_bitmap, &bitmapPixels) >= 0 );  // 获取图片像素（锁定内存块）
    ASSERT_FALSE( bitmapPixels );

    if (bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
        cv::Mat tmp(bitmapInfo.height, bitmapInfo.width, CV_8UC4, bitmapPixels);
        switch (matrix.type()) {
            case CV_8UC1:   cv::cvtColor(matrix, tmp, cv::COLOR_GRAY2RGBA);     break;
            case CV_8UC3:   cv::cvtColor(matrix, tmp, cv::COLOR_RGB2RGBA);      break;
            case CV_8UC4:   matrix.copyTo(tmp);                                 break;
            default:        AndroidBitmap_unlockPixels(env, obj_bitmap);        return false;
        }
    } else {
        cv::Mat tmp(bitmapInfo.height, bitmapInfo.width, CV_8UC2, bitmapPixels);
        switch (matrix.type()) {
            case CV_8UC1:   cv::cvtColor(matrix, tmp, cv::COLOR_GRAY2BGR565);   break;
            case CV_8UC3:   cv::cvtColor(matrix, tmp, cv::COLOR_RGB2BGR565);    break;
            case CV_8UC4:   cv::cvtColor(matrix, tmp, cv::COLOR_RGBA2BGR565);   break;
            default:        AndroidBitmap_unlockPixels(env, obj_bitmap);        return false;
        }
    }
    AndroidBitmap_unlockPixels(env, obj_bitmap);                // 解锁
    return true;
}

extern "C"
JNIEXPORT jintArray JNICALL
Java_com_yq_opencv_1image_OpencvImageUtil_ImageBlur(JNIEnv *env, jclass type, jintArray buf,
                                                    jint w, jint h) {
    //读取int数组并转为Mat类型
    jint *cbuf = env->GetIntArrayElements(buf, JNI_FALSE);
    if (NULL == cbuf) {
        return 0;
    }
    Mat imgData(h, w, CV_8UC4, (unsigned char *) cbuf);
    cv::cvtColor(imgData,imgData,COLOR_BGRA2BGR);

    //这里进行图像相关操作
    blur(imgData, imgData, Size(20, 20));

   //对图像相关操作完毕
    cv::cvtColor(imgData,imgData,COLOR_BGR2BGRA);
    //这里传回int数组。
    uchar *ptr = imgData.data;
    int size = w * h;
    jintArray result = env->NewIntArray(size);
    env->SetIntArrayRegion(result, 0, size, (const jint *) ptr);
    env->ReleaseIntArrayElements(buf, cbuf, 0);
    return result;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_yq_opencv_1image_OpencvImageUtil_mosaic(JNIEnv *env, jclass type, jobject bitmap) {
    Mat src;
    BitmapToMat(env, bitmap, src);
    LOGD("nBitmapToMat: BitmapToMat finish");
    int size = 8; //分成8 * 8 的小块 填充相同的颜色
    Mat mosaic = src.clone();
    for (int row = 0; row < src.rows - size; row += size) {
        for (int col = 0; col < src.cols - size; col += size) {
            Vec4b src_pix = src.at<Vec4b>(row, col);
            for (int row_i = 0; row_i < size; ++row_i) {
                for (int col_i = 0; col_i < size; ++col_i) {
                    mosaic.at<Vec4b>(row + row_i, col + col_i) = src_pix;
                }
            }
        }
    }
    LOGD("nBitmapToMat:mosaic");
    MatToBitmap(env, mosaic, bitmap);
    return bitmap;
}

//图像模糊
extern "C"
JNIEXPORT jobject JNICALL
Java_com_yq_opencv_1image_OpencvImageUtil_blur(JNIEnv *env, jclass type, jobject bitmap) {
    Mat src ;
    BitmapToMat(env, bitmap, src);
    //这里进行图像相关操作
    blur(src, src, Size(51, 51));
    MatToBitmap(env, src, bitmap);
    return bitmap;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_yq_opencv_1image_OpencvImageUtil_gray(JNIEnv *env, jclass type, jobject bitmap) {

    Mat src ;
    BitmapToMat(env, bitmap, src);
    //这里进行图像相关操作
    cv::cvtColor(src, src, COLOR_BGR2BGRA);
    MatToBitmap(env, src, bitmap);
    return bitmap;

}