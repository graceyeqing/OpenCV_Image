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

#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "debug", __VA_ARGS__))

#define ASSERT(status, ret)     if (!(status)) { return ret; }
#define ASSERT_FALSE(status)    ASSERT(status, false)

bool BitmapToMat(JNIEnv *env, jobject obj_bitmap, cv::Mat &matrix) {
    void *bitmapPixels;                                            // 保存图片像素数据
    AndroidBitmapInfo bitmapInfo;                                   // 保存图片参数

    ASSERT_FALSE(AndroidBitmap_getInfo(env, obj_bitmap, &bitmapInfo) >= 0);        // 获取图片参数
    ASSERT_FALSE(bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGBA_8888
                 || bitmapInfo.format ==
                    ANDROID_BITMAP_FORMAT_RGB_565);          // 只支持 ARGB_8888 和 RGB_565
    ASSERT_FALSE(AndroidBitmap_lockPixels(env, obj_bitmap, &bitmapPixels) >= 0);  // 获取图片像素（锁定内存块）
    ASSERT_FALSE(bitmapPixels);

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

bool MatToBitmap(JNIEnv *env, cv::Mat &matrix, jobject obj_bitmap) {
    void *bitmapPixels;                                            // 保存图片像素数据
    AndroidBitmapInfo bitmapInfo;                                   // 保存图片参数

    ASSERT_FALSE(AndroidBitmap_getInfo(env, obj_bitmap, &bitmapInfo) >= 0);        // 获取图片参数
    ASSERT_FALSE(bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGBA_8888
                 || bitmapInfo.format ==
                    ANDROID_BITMAP_FORMAT_RGB_565);          // 只支持 ARGB_8888 和 RGB_565
    ASSERT_FALSE(matrix.dims == 2
                 && bitmapInfo.height == (uint32_t) matrix.rows
                 && bitmapInfo.width == (uint32_t) matrix.cols);                   // 必须是 2 维矩阵，长宽一致
    ASSERT_FALSE(matrix.type() == CV_8UC1 || matrix.type() == CV_8UC3 || matrix.type() == CV_8UC4);
    ASSERT_FALSE(AndroidBitmap_lockPixels(env, obj_bitmap, &bitmapPixels) >= 0);  // 获取图片像素（锁定内存块）
    ASSERT_FALSE(bitmapPixels);

    if (bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
        cv::Mat tmp(bitmapInfo.height, bitmapInfo.width, CV_8UC4, bitmapPixels);
        switch (matrix.type()) {
            case CV_8UC1:
                cv::cvtColor(matrix, tmp, cv::COLOR_GRAY2RGBA);
                break;
            case CV_8UC3:
                cv::cvtColor(matrix, tmp, cv::COLOR_RGB2RGBA);
                break;
            case CV_8UC4:
                matrix.copyTo(tmp);
                break;
            default:
                AndroidBitmap_unlockPixels(env, obj_bitmap);
                return false;
        }
    } else {
        cv::Mat tmp(bitmapInfo.height, bitmapInfo.width, CV_8UC2, bitmapPixels);
        switch (matrix.type()) {
            case CV_8UC1:
                cv::cvtColor(matrix, tmp, cv::COLOR_GRAY2BGR565);
                break;
            case CV_8UC3:
                cv::cvtColor(matrix, tmp, cv::COLOR_RGB2BGR565);
                break;
            case CV_8UC4:
                cv::cvtColor(matrix, tmp, cv::COLOR_RGBA2BGR565);
                break;
            default:
                AndroidBitmap_unlockPixels(env, obj_bitmap);
                return false;
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
    cv::cvtColor(imgData, imgData, COLOR_BGRA2BGR);

    //这里进行图像相关操作
    blur(imgData, imgData, Size(20, 20));

    //对图像相关操作完毕
    cv::cvtColor(imgData, imgData, COLOR_BGR2BGRA);
    //这里传回int数组。
    uchar *ptr = imgData.data;
    int size = w * h;
    jintArray result = env->NewIntArray(size);
    env->SetIntArrayRegion(result, 0, size, (const jint *) ptr);
    env->ReleaseIntArrayElements(buf, cbuf, 0);
    return result;
}

//马赛克
extern "C"
JNIEXPORT jobject JNICALL
Java_com_yq_opencv_1image_OpencvImageUtil_mosaic(JNIEnv *env, jclass type, jobject bitmap) {
    Mat src;
    BitmapToMat(env, bitmap, src);
    LOGD("nBitmapToMat: BitmapToMat finish");
    int size = 10; //分成8 * 8 的小块 填充相同的颜色
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
    Mat src;
    BitmapToMat(env, bitmap, src);
    //这里进行图像相关操作
    blur(src, src, Size(51, 51));
    MatToBitmap(env, src, bitmap);
    return bitmap;
}

//灰度化
extern "C"
JNIEXPORT jobject JNICALL
Java_com_yq_opencv_1image_OpencvImageUtil_gray(JNIEnv *env, jclass type, jobject bitmap) {

    Mat src;
    BitmapToMat(env, bitmap, src);
    //这里进行图像相关操作
    cv::cvtColor(src, src, COLOR_BGRA2GRAY);
    MatToBitmap(env, src, bitmap);
    return bitmap;

}

//浮雕
extern "C"
JNIEXPORT jobject JNICALL
Java_com_yq_opencv_1image_OpencvImageUtil_relief(JNIEnv *env, jclass type, jobject bitmap) {

    Mat src;
    BitmapToMat(env, bitmap, src);
    //这里进行图像相关操作
    /**
    * [1,0]
    * [0,-1]
    */
    Mat relief(src.size(), src.type());
    for (int row = 1; row < src.rows; ++row) {
        for (int col = 1; col < src.cols; ++col) {
            Vec4b pix_p = src.at<Vec4b>(row - 1, col - 1);
            Vec4b pix_n = src.at<Vec4b>(row, col);
            //b g r a
            relief.at<Vec4b>(row, col)[0] = static_cast<uchar>(pix_p[0] - pix_n[0] + 128);
            relief.at<Vec4b>(row, col)[1] = static_cast<uchar>(pix_p[1] - pix_n[1] + 128);
            relief.at<Vec4b>(row, col)[2] = static_cast<uchar>(pix_p[2] - pix_n[2] + 128);
        }
    }
    MatToBitmap(env, relief, bitmap);
    return bitmap;

}

//油画
extern "C"
JNIEXPORT jobject JNICALL
Java_com_yq_opencv_1image_OpencvImageUtil_oilPaiting(JNIEnv *env, jclass type, jobject bitmap) {

    Mat src;
    BitmapToMat(env, bitmap, src);
    //这里进行图像相关操作
    Mat gray;
    cvtColor(src, gray, COLOR_BGRA2GRAY);

    Mat res = src.clone();

    const int g_size = 5;
    const int t_size = 8;

    for (int row = 0; row < src.rows - t_size; ++row) {
        for (int col = 0; col < src.cols - t_size; ++col) {
            //统计灰度等级
            int grade[g_size + 1] = {0};
            int b[g_size + 1] = {0};
            int g[g_size + 1] = {0};
            int r[g_size + 1] = {0};
            for (int t_row = 0; t_row < t_size; ++t_row) {
                for (int t_col = 0; t_col < t_size; ++t_col) {

                    uchar gray_value = gray.at<uchar>(row + t_row, col + t_col);
                    int grade_index = gray_value / (255 / g_size);
                    grade[grade_index] += 1;

                    b[grade_index] += src.at<Vec4b>(row + t_row, col + t_col)[0];
                    g[grade_index] += src.at<Vec4b>(row + t_row, col + t_col)[1];
                    r[grade_index] += src.at<Vec4b>(row + t_row, col + t_col)[2];

                }

            }

            //找出最多落入像素最多的一个等级
            int max_index = 0;
            int max = grade[0];
            for (int index = 1; index <= g_size; ++index) {
                if (grade[index] > max) {
                    max_index = index;
                    max = grade[index];
                }
            }

            //求取这个等级的平均值
            res.at<Vec4b>(row, col)[0] = b[max_index] / max;
            res.at<Vec4b>(row, col)[1] = g[max_index] / max;
            res.at<Vec4b>(row, col)[2] = r[max_index] / max;
        }
    }
    MatToBitmap(env, res, bitmap);
    return bitmap;

}

//轮廓图
extern "C"
JNIEXPORT jobject JNICALL
Java_com_yq_opencv_1image_OpencvImageUtil_canary(JNIEnv *env, jclass type, jobject bitmap) {

    Mat src;
    BitmapToMat(env, bitmap, src);
    //这里进行图像相关操作
    cv::cvtColor(src, src, COLOR_BGRA2GRAY);
    cv::GaussianBlur(src, src, Size(3, 3), 0, 0, BORDER_DEFAULT);
    //调用Canny 方法实现边缘检测
    cv::Canny(src, src, 50, 150, 3, false);
    MatToBitmap(env, src, bitmap);
    return bitmap;

}

//毛玻璃
extern "C"
JNIEXPORT jobject JNICALL
Java_com_yq_opencv_1image_OpencvImageUtil_frostedGlass(JNIEnv *env, jclass type, jobject bitmap) {

    Mat src;
    BitmapToMat(env, bitmap, src);

    int size = 5; //分成5 * 5的小块 在小块中随机取值填充

    Mat frostedGlass = src.clone();

    RNG rng((unsigned) time(NULL));

    for (int row = 0; row < src.rows - size; ++row) {
        for (int col = 0; col < src.cols - size; ++col) {
            int roandnumber = rng.uniform(0, size);
            frostedGlass.at<Vec4b>(row, col)[0] = src.at<Vec4b>(row + roandnumber,
                                                                col + roandnumber)[0];
            frostedGlass.at<Vec4b>(row, col)[1] = src.at<Vec4b>(row + roandnumber,
                                                                col + roandnumber)[1];
            frostedGlass.at<Vec4b>(row, col)[2] = src.at<Vec4b>(row + roandnumber,
                                                                col + roandnumber)[2];
        }
    }

    MatToBitmap(env, frostedGlass, bitmap);
    return bitmap;

}

//直方图均衡化
extern "C"
JNIEXPORT jobject JNICALL
Java_com_yq_opencv_1image_OpencvImageUtil_equalizeHist(JNIEnv *env, jclass type, jobject bitmap) {

    Mat src;
    BitmapToMat(env, bitmap, src);
    //这里进行图片的处理
    Mat imgRGB[3];
    split(src, imgRGB);
    //针对每个通道进行直方图均衡化
    for (int i = 0; i < 3; i++) {
        equalizeHist(imgRGB[i], imgRGB[i]);
    }
    //合并图像
    merge(imgRGB, 3, src);

    MatToBitmap(env, src, bitmap);
    return bitmap;
}

//图像增强，拉普拉斯算子
extern "C"
JNIEXPORT jobject JNICALL
Java_com_yq_opencv_1image_OpencvImageUtil_lapulasi(JNIEnv *env, jclass type, jobject bitmap) {

    Mat src;
    BitmapToMat(env, bitmap, src);
    //这里进行图片的处理
    cv::Laplacian(src,src,CV_8UC3);
    MatToBitmap(env, src, bitmap);
    return bitmap;
}

//图像翻转
extern "C"
JNIEXPORT jobject JNICALL
Java_com_yq_opencv_1image_OpencvImageUtil_flip(JNIEnv *env, jclass type, jobject bitmap) {

    Mat src;
    BitmapToMat(env, bitmap, src);
    //这里进行图片的处理
    //0倒过来，1翻转，-1倒过来加翻转
    cv::flip(src,src,-1);
    MatToBitmap(env, src, bitmap);
    return bitmap;

}

//图像叠加
extern "C"
JNIEXPORT jobject JNICALL
Java_com_yq_opencv_1image_OpencvImageUtil_add(JNIEnv *env, jclass type, jobject bitmap,
                                              jobject bitmap2) {

    Mat src;
    BitmapToMat(env, bitmap, src);
    Mat src2;
    BitmapToMat(env, bitmap2, src2);
    //这里进行图片的处理
    addWeighted(src,0.7,src2,0.3,0.0,src);
    MatToBitmap(env, src, bitmap);
    return bitmap;

}

//图像膨胀
extern "C"
JNIEXPORT jobject JNICALL
Java_com_yq_opencv_1image_OpencvImageUtil_dilate(JNIEnv *env, jclass type, jobject bitmap) {

    Mat src;
    BitmapToMat(env, bitmap, src);
    //这里进行图片的处理
    Mat *kernel = new Mat();
    dilate(src,src,*kernel);
    MatToBitmap(env, src, bitmap);
    return bitmap;

}

//图像侵蚀
extern "C"
JNIEXPORT jobject JNICALL
Java_com_yq_opencv_1image_OpencvImageUtil_erode(JNIEnv *env, jclass type, jobject bitmap) {

    Mat src;
    BitmapToMat(env, bitmap, src);
    //这里进行图片的处理
    Mat *kernel = new Mat();
    erode(src,src,*kernel);
    MatToBitmap(env, src, bitmap);
    return bitmap;

}

//图像扭曲
extern "C"
JNIEXPORT jobject JNICALL
Java_com_yq_opencv_1image_OpencvImageUtil_warping(JNIEnv *env, jclass type, jobject bitmap) {

    Mat src;
    BitmapToMat(env, bitmap, src);
    //这里进行图片的处理
    Point2f srcTri[3];
    Point2f dstTri[3];

    Mat warp_mat(2, 3, CV_32FC1);
    //设置三个点来计算仿射变换
    srcTri[0] = Point2f(0, 0);
    srcTri[1] = Point2f(src.cols - 1, 0);
    srcTri[2] = Point2f(0, src.rows - 1);

    dstTri[0] = Point2f(src.cols*0.0, src.rows*0.33);
    dstTri[1] = Point2f(src.cols*0.85, src.rows*0.25);
    dstTri[2] = Point2f(src.cols*0.15, src.rows*0.7);

    //计算仿射变换矩阵
    warp_mat = getAffineTransform(srcTri, dstTri);
   //创建仿射变换目标图像与原图像尺寸类型相同
    Mat warp_dstImage = Mat::zeros(src.rows, src.cols, src.type());
    cv::warpAffine(src,src,warp_mat,warp_dstImage.size());

    MatToBitmap(env, src, bitmap);
    return bitmap;

}