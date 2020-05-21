#include <jni.h>
#include <iostream>
#include <ctime>
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <vector>
#include <android/bitmap.h>
#include <android/log.h>
#include <opencv2/opencv.hpp>
#include "dlib/dlib/geometry/rectangle.h"
#include "dlib/dlib/opencv/cv_image.h"
#include "dlib/dlib/image_processing/frontal_face_detector.h"

using namespace cv;

#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "debug", __VA_ARGS__))

#define ASSERT(status, ret)     if (!(status)) { return ret; }
#define ASSERT_FALSE(status)    ASSERT(status, false)

//定义 VisionDetRet 类对应的 C++ 类
#define CLASSNAME_VISION_DET_RET "com/yq/opencv_image/bean/VisionDetRet"
#define CONSTSIG_VISION_DET_RET "()V"

#define CLASSNAME_FACE_DET "com/yq/opencv_image/FaceDet"

//class JNI_VisionDetRet {
//public:
//    JNI_VisionDetRet(JNIEnv *env) {
//        // 查找VisionDetRet类信息
//        jclass detRetClass = env->FindClass(CLASSNAME_VISION_DET_RET);
//        // 获取VisionDetRet类成员变量
//        jID_left = env->GetFieldID(detRetClass, "mLeft", "I");
//        jID_top = env->GetFieldID(detRetClass, "mTop", "I");
//        jID_right = env->GetFieldID(detRetClass, "mRight", "I");
//        jID_bottom = env->GetFieldID(detRetClass, "mBottom", "I");
//    }
//
//    void setRect(JNIEnv *env, jobject &jDetRet, const int &left, const int &top,
//                 const int &right, const int &bottom) {
//        // 设置VisionDetRet类对象jDetRet的成员变量值
//        env->SetIntField(jDetRet, jID_left, left);
//        env->SetIntField(jDetRet, jID_top, top);
//        env->SetIntField(jDetRet, jID_right, right);
//        env->SetIntField(jDetRet, jID_bottom, bottom);
//    }
//
//    // 创建VisionDetRet类实例
//    static jobject createJObject(JNIEnv *env) {
//        //定义java类
//        jclass detRetClass = env->FindClass(CLASSNAME_VISION_DET_RET);
//        //获取java jmethodID
//        jmethodID constructor_mid = env->GetMethodID( detRetClass, "VisionDetRet", "()");
//        return env->NewObject(detRetClass, constructor_mid);
//    }
//
//    // 创建VisionDetRet类对象数组
//    static jobjectArray createJObjectArray(JNIEnv *env, const int &size) {
//        jclass detRetClass = env->FindClass(CLASSNAME_VISION_DET_RET);
//        return (jobjectArray) env->NewObjectArray(size, detRetClass, NULL);
//    }
//
//private:
//    jfieldID jID_left;
//    jfieldID jID_top;
//    jfieldID jID_right;
//    jfieldID jID_bottom;
//};
//
////定义人脸检测类
//class FaceDetector {
//private:
//
//    dlib::frontal_face_detector face_detector;
//    std::vector<dlib::rectangle> det_rects;
//
//public:
//
//    FaceDetector();
//
//    // 实现人脸检测算法
//    int Detect(const cv::Mat &image);
//
//    // 返回检测结果
//    std::vector<dlib::rectangle> getDetResultRects();
//};
//
//FaceDetector::FaceDetector() {
//    // 定义人脸检测器
//    face_detector = dlib::get_frontal_face_detector();
//}
//
//int FaceDetector::Detect(const cv::Mat &image) {
//
//    if (image.empty())
//        return 0;
//
//    if (image.channels() == 1) {
//        cv::cvtColor(image, image, COLOR_GRAY2BGR);
//    }
//
//    dlib::cv_image<dlib::bgr_pixel> dlib_image(image);
//
//    det_rects.clear();
//
//    // 返回检测到的人脸矩形特征框
//    det_rects = face_detector(dlib_image);
//
//    return det_rects.size();
//}
//
//std::vector<dlib::rectangle> FaceDetector::getDetResultRects() {
//    return det_rects;
//}
//
//JNI_VisionDetRet *g_pJNI_VisionDetRet;
//
//JavaVM *g_javaVM = NULL;
//
//// 该函数在加载本地库时被调用
//JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
//    g_javaVM = vm;
//    JNIEnv *env;
//    vm->GetEnv((void **) &env, JNI_VERSION_1_6);
//    // 初始化 g_pJNI_VisionDetRet
//    g_pJNI_VisionDetRet = new JNI_VisionDetRet(env);
//    return JNI_VERSION_1_6;
//}
//
//// 该函数用于执行清理操作
//void JNI_OnUnload(JavaVM *vm, void *reserved) {
//    g_javaVM = NULL;
//    delete g_pJNI_VisionDetRet;
//}
//
//namespace {
//#define JAVA_NULL 0
//    using DetPtr = FaceDetector *;
//
//    // 用于存放人脸检测类对象的指针，关联Jave层对象与C++底层对象（相互对应）
//    class JNI_FaceDet {
//    public:
//        JNI_FaceDet(JNIEnv *env) {
//            jclass clazz = env->FindClass(CLASSNAME_FACE_DET);
//            mNativeContext = env->GetFieldID(clazz, "mNativeFaceDetContext", "J");
//            env->DeleteLocalRef(clazz);
//        }
//
//        DetPtr getDetectorPtrFromJava(JNIEnv *env, jobject thiz) {
//            DetPtr const p = (DetPtr) env->GetLongField(thiz, mNativeContext);
//            return p;
//        }
//
//        void setDetectorPtrToJava(JNIEnv *env, jobject thiz, jlong ptr) {
//            env->SetLongField(thiz, mNativeContext, ptr);
//        }
//
//        jfieldID mNativeContext;
//    };
//
//    // Protect getting/setting and creating/deleting pointer between java/native
//    std::mutex gLock;
//
//    std::shared_ptr<JNI_FaceDet> getJNI_FaceDet(JNIEnv *env) {
//        static std::once_flag sOnceInitflag;
//        static std::shared_ptr<JNI_FaceDet> sJNI_FaceDet;
//        std::call_once(sOnceInitflag, [env]() {
//            sJNI_FaceDet = std::make_shared<JNI_FaceDet>(env);
//        });
//        return sJNI_FaceDet;
//    }
//
//    // 从java对象获取它持有的c++对象指针
//    DetPtr const getDetPtr(JNIEnv *env, jobject thiz) {
//        std::lock_guard<std::mutex> lock(gLock);
//        return getJNI_FaceDet(env)->getDetectorPtrFromJava(env, thiz);
//    }
//
//    // The function to set a pointer to java and delete it if newPtr is empty
//    // C++对象new以后，将指针转成long型返回给java对象持有
//    void setDetPtr(JNIEnv *env, jobject thiz, DetPtr newPtr) {
//        std::lock_guard<std::mutex> lock(gLock);
//        DetPtr oldPtr = getJNI_FaceDet(env)->getDetectorPtrFromJava(env, thiz);
//        if (oldPtr != JAVA_NULL) {
//            delete oldPtr;
//        }
//        getJNI_FaceDet(env)->setDetectorPtrToJava(env, thiz, (jlong) newPtr);
//    }
//
//}
//
//
//// 生成需要返回的结果数组
//jobjectArray getRecResult(JNIEnv *env, DetPtr faceDetector, const int &size) {
//    // 根据检测到的人脸数创建相应大小的jobjectArray
//    jobjectArray jDetRetArray = JNI_VisionDetRet::createJObjectArray(env, size);
//    for (int i = 0; i < size; i++) {
//        // 对检测到的每一个人脸创建对应的实例对象，然后插入数组
//        jobject jDetRet = JNI_VisionDetRet::createJObject(env);
//        env->SetObjectArrayElement(jDetRetArray, i, jDetRet);
//        dlib::rectangle rect = faceDetector->getDetResultRects()[i];
//        // 将人脸矩形框的值赋给对应的jobject实例对象
//        g_pJNI_VisionDetRet->setRect(env, jDetRet, rect.left(), rect.top(),
//                                     rect.right(), rect.bottom());
//    }
//    return jDetRetArray;
//}
//
//extern "C"
//JNIEXPORT void JNICALL
//Java_com_yq_opencv_1image_FaceDet_jniNativeClassInit(JNIEnv *env, jclass type) {
//
//
//}
//
//extern "C"
//JNIEXPORT jint JNICALL
//Java_com_yq_opencv_1image_FaceDet_jniInit(JNIEnv *env, jobject instance) {
//
//    DetPtr mDetPtr = new FaceDetector();
//    // 设置人脸检测类指针
//    setDetPtr(env, instance, mDetPtr);
//    return JNI_OK;
//
//}
//
//extern "C"
//JNIEXPORT jint JNICALL
//Java_com_yq_opencv_1image_FaceDet_jniDeInit(JNIEnv *env, jobject instance) {
//
//    // 指针置0
//    setDetPtr(env, instance, JAVA_NULL);
//    return JNI_OK;
//
//}
//
//bool BitmapToMat(JNIEnv *env, jobject obj_bitmap, cv::Mat &matrix) {
//    void *bitmapPixels;                                            // 保存图片像素数据
//    AndroidBitmapInfo bitmapInfo;                                   // 保存图片参数
//
//    ASSERT_FALSE(AndroidBitmap_getInfo(env, obj_bitmap, &bitmapInfo) >= 0);        // 获取图片参数
//    ASSERT_FALSE(bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGBA_8888
//                 || bitmapInfo.format ==
//                    ANDROID_BITMAP_FORMAT_RGB_565);          // 只支持 ARGB_8888 和 RGB_565
//    ASSERT_FALSE(AndroidBitmap_lockPixels(env, obj_bitmap, &bitmapPixels) >= 0);  // 获取图片像素（锁定内存块）
//    ASSERT_FALSE(bitmapPixels);
//
//    if (bitmapInfo.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
//        cv::Mat tmp(bitmapInfo.height, bitmapInfo.width, CV_8UC4, bitmapPixels);    // 建立临时 mat
//        tmp.copyTo(matrix);                                                         // 拷贝到目标 matrix
//    } else {
//        cv::Mat tmp(bitmapInfo.height, bitmapInfo.width, CV_8UC2, bitmapPixels);
//        cv::cvtColor(tmp, matrix, cv::COLOR_BGR5652RGB);
//    }
//
//    AndroidBitmap_unlockPixels(env, obj_bitmap);            // 解锁
//    return true;
//}
//
//extern "C"
//JNIEXPORT jobjectArray JNICALL
//Java_com_yq_opencv_1image_FaceDet_jniBitmapDet(JNIEnv *env, jobject instance, jobject bitmap) {
//
//    cv::Mat rgbaMat;
//    cv::Mat bgrMat;
//    BitmapToMat(env, bitmap, rgbaMat);
//    cv::cvtColor(rgbaMat, bgrMat, cv::COLOR_RGBA2BGR);
//    // 获取人脸检测类指针
//    DetPtr mDetPtr = getDetPtr(env, instance);
//    // 调用人脸检测算法，返回检测到的人脸数
//    jint size = mDetPtr->Detect(bgrMat);
//    // 返回检测结果
//    return getRecResult(env, mDetPtr, size);
//
//}


//dlib::frontal_face_detector m_facedetector=dlib::get_frontal_face_detector();
//
////获取人脸框
//std::vector<int > getfacerect(const std::vector<int>img,int height,int width)
//{
//    dlib::array2d<unsigned char>image;
//    image.set_size(height,width);
//
//    for (int i = 0; i < height; i++)
//    {
//        for(int j=0;j<width;j++)
//        {
//            int clr = img[i*width+j];
//            int red = (clr & 0x00ff0000) >> 16; // 取高两位
//            int green = (clr & 0x0000ff00) >> 8; // 取中两位
//            int blue = clr & 0x000000ff; // 取低两位
//            unsigned char gray=red*0.299+green*0.587+blue*0.114;
//            //dlib::rgb_pixel pt(red,green,blue);
//            image[i][j]=gray;
//        }
//    }
//
//    clock_t begin = clock();
//    std::vector<dlib::rectangle> dets= m_facedetector(image);
//    std::vector<int >rect;
//    if (!dets.empty())
//    {
//        rect.push_back(dets[0].left());
//        rect.push_back(dets[0].top());
//        rect.push_back(dets[0].width());
//        rect.push_back(dets[0].height());
//    }
//    return rect;
//
//}

extern "C"
JNIEXPORT jintArray JNICALL
Java_com_yq_opencv_1image_OpencvImageUtil2_dlibDetectFace(JNIEnv *env, jclass type,
                                                          jintArray pixels_, jint height,
                                                          jint width) {
    jint *pixels = env->GetIntArrayElements(pixels_, NULL);
    std::vector<int>image_datacpp(height*width);
    jsize len = env->GetArrayLength(pixels_);
    jint *body = env->GetIntArrayElements(pixels_, 0);
    for (jsize i=0;i<len;i++){
        image_datacpp[i]=(int)body[i];
    }
//    std::vector<int>rect=getfacerect(image_datacpp,height,width);
    jintArray result =env->NewIntArray(4);
//    env->SetIntArrayRegion(result, 0, 4, &rect[0]);

    return result;
}






