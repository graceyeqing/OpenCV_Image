#include <jni.h>
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <vector>
#include <android/bitmap.h>
#include <android/log.h>
#include <opencv2/opencv.hpp>
#include <random>
#include <math.h>

using namespace cv;
using namespace std;

#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "debug", __VA_ARGS__))

#define ASSERT(status, ret)     if (!(status)) { return ret; }
#define ASSERT_FALSE(status)    ASSERT(status, false)

CascadeClassifier cascadeClassifier;

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
    cv::Laplacian(src, src, CV_8UC3);
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
    cv::flip(src, src, -1);
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
    addWeighted(src, 0.7, src2, 0.3, 0.0, src);
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
    dilate(src, src, *kernel);
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
    erode(src, src, *kernel);
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

    dstTri[0] = Point2f(src.cols * 0.0, src.rows * 0.33);
    dstTri[1] = Point2f(src.cols * 0.85, src.rows * 0.25);
    dstTri[2] = Point2f(src.cols * 0.15, src.rows * 0.7);

    //计算仿射变换矩阵
    warp_mat = getAffineTransform(srcTri, dstTri);
    //创建仿射变换目标图像与原图像尺寸类型相同
    Mat warp_dstImage = Mat::zeros(src.rows, src.cols, src.type());
    cv::warpAffine(src, src, warp_mat, warp_dstImage.size());

    MatToBitmap(env, src, bitmap);
    return bitmap;

}
//人脸检测
extern "C"
JNIEXPORT jobject JNICALL
Java_com_yq_opencv_1image_OpencvImageUtil2_detectFace(JNIEnv *env, jclass type, jobject bitmap) {

    Mat src;
    BitmapToMat(env, bitmap, src);
    //这里进行图片的处理
    // 处理灰度 opencv 处理灰度图, 提高效率，一般所有的操作都会对其进行灰度处理
    Mat gray_mat;
    cvtColor(src, gray_mat, COLOR_BGRA2GRAY);
    // 再次处理 直方均衡补偿
    Mat equalize_mat;
    equalizeHist(gray_mat, equalize_mat);

    // 识别人脸，也可以直接用 彩色图去做,识别人脸要加载人脸分类器文件
    std::vector<Rect> faces;
    cascadeClassifier.detectMultiScale(equalize_mat, faces, 1.1, 5, 0 ,Size(160, 160));
    LOGD("人脸个数：%d", faces.size());
    if (faces.size() != 0) {
        for (Rect faceRect : faces) {
            // 在人脸部分画个图
            rectangle(src, faceRect, Scalar(255, 155, 155), 8);
            // 把 mat 我们又放到 bitmap 里面
            MatToBitmap(env, src, bitmap);
            // 保存人脸信息
            // 保存人脸信息 Mat , 图片 jpg
            Mat face_info_mat(equalize_mat, faceRect);
            // 保存 face_info_mat
        }
    }

    return bitmap;

}

//加载分类器文件
extern "C"
JNIEXPORT void JNICALL
Java_com_yq_opencv_1image_OpencvImageUtil2_loadCascade(JNIEnv *env, jclass type,
                                                       jstring filePath_) {
    const char *filePath = env->GetStringUTFChars(filePath_, 0);
    cascadeClassifier.load(filePath);
    LOGD("加载分类器文件成功");
    env->ReleaseStringUTFChars(filePath_, filePath);
}

//人眼检测
extern "C"
JNIEXPORT jobject JNICALL
Java_com_yq_opencv_1image_OpencvImageUtil2_detectEyes(JNIEnv *env, jclass type, jobject bitmap) {

    Mat src;
    BitmapToMat(env, bitmap, src);
    //这里进行图片的处理
    // 处理灰度 opencv 处理灰度图, 提高效率，一般所有的操作都会对其进行灰度处理
    Mat gray_mat;
    cvtColor(src, gray_mat, COLOR_BGRA2GRAY);
    // 再次处理 直方均衡补偿
    Mat equalize_mat;
    equalizeHist(gray_mat, equalize_mat);

    std::vector<Rect> eyes;
    cascadeClassifier.detectMultiScale(equalize_mat, eyes, 1.1, 5);
    LOGD("人眼个数：%d", eyes.size());
    if (eyes.size() != 0) {
        for (Rect faceRect : eyes) {
            // 在眼睛部分画个图
            rectangle(src, faceRect, Scalar(255, 155, 155), 2);
            // 把 mat 我们又放到 bitmap 里面
            MatToBitmap(env, src, bitmap);
        }
    }

    return bitmap;

}

//笑脸检测
extern "C"
JNIEXPORT jobject JNICALL
Java_com_yq_opencv_1image_OpencvImageUtil2_detectSmile(JNIEnv *env, jclass type, jstring filePath_,
                                                       jobject bitmap) {

    Mat src;
    BitmapToMat(env, bitmap, src);
    //这里进行图片的处理
    // 处理灰度 opencv 处理灰度图, 提高效率，一般所有的操作都会对其进行灰度处理
    Mat gray_mat;
    cvtColor(src, gray_mat, COLOR_BGRA2GRAY);
    // 再次处理 直方均衡补偿
    Mat equalize_mat;
    equalizeHist(gray_mat, equalize_mat);

    //加载笑脸分类器
    CascadeClassifier smilcascade;
    const char *filePath = env->GetStringUTFChars(filePath_, 0);
    smilcascade.load(filePath);
    LOGD("加载分类器文件成功");
    env->ReleaseStringUTFChars(filePath_, filePath);

    // 识别人脸，也可以直接用 彩色图去做,识别人脸要加载人脸分类器文件
    std::vector<Rect> faces;
    cascadeClassifier.detectMultiScale(equalize_mat, faces, 1.1, 5);
    LOGD("人脸个数：%d", faces.size());
    if (faces.size() != 0) {
        for (size_t i = 0; i < faces.size(); i++) {
            // 在人脸部分画个图
            rectangle(src, faces[i], Scalar(255, 155, 155), 8);

            Mat faceROI = gray_mat(faces[i]);
            std::vector<Rect> smiles;
            //-- In each face, detect smile
            smilcascade.detectMultiScale(faceROI, smiles, 1.1, 55, CASCADE_SCALE_IMAGE);

            for (size_t j = 0; j < smiles.size(); j++) {
                Rect rect(faces[i].x + smiles[j].x, faces[i].y + smiles[j].y, smiles[j].width,
                          smiles[j].height);
                rectangle(src, rect, Scalar(0, 255, 255), 2, 8, 0);
            }
            // 把 mat 我们又放到 bitmap 里面
            MatToBitmap(env, src, bitmap);
        }
    }

    return bitmap;

}

//唇部检测
extern "C"
JNIEXPORT jobject JNICALL
Java_com_yq_opencv_1image_OpencvImageUtil2_detectLips(JNIEnv *env, jclass type, jobject bitmap) {

    Mat src;
    BitmapToMat(env, bitmap, src);
    //这里进行图片的处理
    // 处理灰度 opencv 处理灰度图, 提高效率，一般所有的操作都会对其进行灰度处理
    Mat gray_mat;
    cvtColor(src, gray_mat, COLOR_BGRA2GRAY);
    // 再次处理 直方均衡补偿
    Mat equalize_mat;
    equalizeHist(gray_mat, equalize_mat);

    // 识别人脸，也可以直接用 彩色图去做,识别人脸要加载人脸分类器文件
    std::vector<Rect> faces;
    cascadeClassifier.detectMultiScale(equalize_mat, faces, 1.1, 5);
    LOGD("人脸个数：%d", faces.size());
    if (faces.size() != 0) {
        for (size_t i = 0; i < faces.size(); i++) {
            // 在人脸部分画个图
            rectangle(src, faces[i], Scalar(255, 155, 155), 8);
            // 把 mat 我们又放到 bitmap 里面
            double k = 0;
            int b=0,g=0,r=0;
            Mat faceROI = gray_mat(faces[i]);
            for (int row = 0; row < src.rows; ++row) {
                for (int col = 0; col < src.cols; ++col) {
                    Vec4b pix_n = src.at<Vec4b>(row, col);
                    b = pix_n[0];
                    g = pix_n[1];
                    r = pix_n[2];
                    //算法
                    k = log((double)g / (pow((double)b, 0.391) * pow((double)r, 0.609)));
                    if (k < -0.15)
                    {
                        //b g r a
                        src.at<Vec4b>(row, col)[0] = static_cast<uchar>(255);
                        src.at<Vec4b>(row, col)[1] = static_cast<uchar>(0);
                        src.at<Vec4b>(row, col)[2] = static_cast<uchar>(255);
                    }

                }
            }
            MatToBitmap(env, src, bitmap);
        }
    }

    return bitmap;

}

Mat result;
//文字提取
extern "C"
JNIEXPORT jobject JNICALL
Java_com_yq_opencv_1image_OpencvImageUtil2_detectWords(JNIEnv *env, jclass type, jobject bitmap) {

    Mat src;
    BitmapToMat(env, bitmap, src);
    //这里进行图片的处理
    //1.阈值化,腐蚀
    threshold(src, src, 100, 255, THRESH_BINARY);
    Mat erodeKernel = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
    erode(src, src, erodeKernel);
    //2.滤波，降噪  中值滤波
    medianBlur(src,src,7);
    //3.种子联通法进行连通区域检测 调用java 静态方法方法

//     //加载类 得到class 对象
//    jclass jc = env->FindClass( "com/yq/opencv_image/OpencvImageUtil2");
//    //得到对应方法的 Method 对象
//    jmethodID jmethodID1 = env->GetMethodID(jc, "seedFill","(Lorg/opencv/core/Mat;)Lorg/opencv/core/Mat;");
////    //3.创建对象
////    jobject jobject1 = env->AllocObject(jc);
////    //4.调用方法
////    jint result = env->CallIntMethod(jobject1, jmethodID1, 99, 1);
//    //3.调用静态方法
//    jobject result = env->CallObjectMethod(jc,jmethodID1, src);

    MatToBitmap(env, src, bitmap);
    return bitmap;

}

int getBlockSum(Mat &sum_mat, int x0, int y0, int x1, int y1, int ch){
    // 获取四个点的值
    int lt = sum_mat.at<Vec3i>(y0, x0)[ch];
    int lb = sum_mat.at<Vec3i>(y1, x0)[ch];
    int rt = sum_mat.at<Vec3i>(y0, x1)[ch];
    int rb = sum_mat.at<Vec3i>(y1, x1)[ch];

    // 区块的合
    int sum = rb - rt - lb + lt;
    return sum;
}

float getBlockSqSum(Mat &sqsum_mat, int x0, int y0, int x1, int y1, int ch){
    // 获取四个点的值
    float lt = sqsum_mat.at<Vec3f>(y0, x0)[ch];
    float lb = sqsum_mat.at<Vec3f>(y1, x0)[ch];
    float rt = sqsum_mat.at<Vec3f>(y0, x1)[ch];
    float rb = sqsum_mat.at<Vec3f>(y1, x1)[ch];

    // 区块的合
    float sqsum = rb - rt - lb + lt;
    return sqsum;
}


// 积分图的模糊算法 size 模糊的直径
void fatsBilateralBlur(Mat & src, Mat &dst, int size, int sigma){
    // size % 2 == 1
    // 把原来进行填充，方便运算
    Mat mat;
    int radius = size / 2;
    copyMakeBorder(src, mat, radius, radius, radius, radius, BORDER_DEFAULT);
    // 求积分图 (作业去手写积分图的源码)
    Mat sum_mat, sqsum_mat;
    integral(mat, sum_mat, sqsum_mat, CV_32S, CV_32F);

    dst.create(src.size(), src.type());
    int imageH = src.rows;
    int imageW = src.cols;
    int area = size*size;
    // 求四个点，左上，左下，右上，右下
    int x0 = 0, y0 = 0, x1 = 0, y1 = 0;
    int lt = 0, lb = 0, rt = 0, rb = 0;
    int channels = src.channels();
    for (int row = 0; row < imageH; row++)
    {
        // 思考，x0，y0 , x1 , y1  sum_mat
        // 思考，row, col, dst
        y0 = row;
        y1 = y0 + size;
        for (int col = 0; col < imageW; col++)
        {
            x0 = col;
            x1 = x0 + size;
            for (int i = 0; i < channels; i++)
            {
                int sum = getBlockSum(sum_mat, x0, y0, x1, y1, i);
                float sqsum = getBlockSqSum(sqsum_mat, x0, y0, x1, y1, i);

                float diff_sq = (sqsum - (sum * sum) / area) / area;
                float k = diff_sq / (diff_sq + sigma);

                int pixels = src.at<Vec3b>(row, col)[i];
                pixels = static_cast<int>((1 - k) * (sum / area) + k * pixels);

                dst.at<Vec3b>(row, col)[i] = static_cast<uchar>(pixels);
            }
        }
    }
}

// 皮肤区域检测
void skinDetect(const Mat &src, Mat &skinMask){
    skinMask.create(src.size(), CV_8UC1);
    int rows = src.rows;
    int cols = src.cols;

    Mat ycrcb;
    cvtColor(src, ycrcb, COLOR_BGR2YCrCb);

    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < cols; col++)
        {
            Vec3b pixels = ycrcb.at<Vec3b>(row, col);
            uchar y = pixels[0];
            uchar cr = pixels[1];
            uchar cb = pixels[2];

            if (y>80 && 85<cb<135 && 135<cr<180){
                skinMask.at<uchar>(row, col) = 255;
            }
            else{
                skinMask.at<uchar>(row, col) = 0;
            }
        }
    }
}

// 皮肤区域融合
void fuseSkin(const Mat &src, const  Mat &blur_mat, Mat &dst, const Mat &mask){
    // 融合？
    dst.create(src.size(),src.type());
    GaussianBlur(mask, mask, Size(3, 3), 0.0);
    Mat mask_f;
    mask.convertTo(mask_f, CV_32F);
    normalize(mask_f, mask_f, 1.0, 0.0, NORM_MINMAX);

    int rows = src.rows;
    int cols = src.cols;
    int ch = src.channels();

    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < cols; col++)
        {
            // mask_f (1-k)
            /*
            uchar mask_pixels = mask.at<uchar>(row,col);
            // 人脸位置
            if (mask_pixels == 255){
                dst.at<Vec3b>(row, col) = blur_mat.at<Vec3b>(row, col);
            }
            else{
                dst.at<Vec3b>(row, col) = src.at<Vec3b>(row, col);
            }
            */

            // src ，通过指针去获取， 指针 -> Vec3b -> 获取
            uchar b1 = src.at<Vec3b>(row, col)[0];
            uchar g1 = src.at<Vec3b>(row, col)[1];
            uchar r1 = src.at<Vec3b>(row, col)[2];

            // blur_mat
            uchar b2 = blur_mat.at<Vec3b>(row, col)[0];
            uchar g2 = blur_mat.at<Vec3b>(row, col)[1];
            uchar r2 = blur_mat.at<Vec3b>(row, col)[2];

            // dst 254  1
            float k = mask_f.at<float>(row,col);

            dst.at<Vec3b>(row, col)[0] = static_cast<uchar>(b2 * k + (1 - k) * b1);
            dst.at<Vec3b>(row, col)[1] = static_cast<uchar>(g2 * k + (1 - k) * g1);
            dst.at<Vec3b>(row, col)[2] = static_cast<uchar>(r2 * k + (1 - k) * r1);
        }
    }
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_yq_opencv_1image_OpencvImageUtil2_beautiful(JNIEnv *env, jclass type, jobject bitmap) {

    Mat src;
    BitmapToMat(env, bitmap, src);
    //这里进行图片的处理
    Mat blurmat ;
    //快速模糊
    int size = 13;
    Mat kernel = Mat::ones(Size(size,size),CV_32FC1)/(size*size);
    filter2D(src,blurmat,src.depth(),kernel);
//    GaussianBlur(src,blurmat,Size(size,size),10);

    //1.积分图模糊
//    fatsBilateralBlur(src,blurmat,16,0);
    //2.检测与模糊皮肤区域
    Mat skinMask ;
    skinDetect(src,skinMask);
    Mat dst;
    fuseSkin(src,blurmat,dst,skinMask);
    // 边缘的提升 (可有可无)
//    Mat cannyMask;
//    Canny(src, cannyMask, 150, 300, 3, false);
//    // & 运算  0 ，255
//    bitwise_and(src, src, dst, cannyMask);
    // 稍微提升一下对比度(亮度)
    add(dst, Scalar(10, 10, 10), dst);

    MatToBitmap(env, dst, bitmap);
    return bitmap;

}