package com.yq.opencv_image;

import android.graphics.Bitmap;

import org.opencv.core.Mat;

/**
 * @author yeqing
 * @des
 * @date 2020/1/3 15:43
 */
public class OpencvImageUtil {
    public static native int[] ImageBlur(int[] pixels,int w,int h);

    public static native Bitmap blur(Bitmap bitmap);

    //马赛克效果
    public static native Bitmap mosaic(Bitmap bitmap);

    //灰度化
    public static native Bitmap gray(Bitmap bitmap);

    //浮雕
    public static native Bitmap relief(Bitmap bitmap);

    //油画
    public static native Bitmap oilPaiting(Bitmap bitmap);

    //轮廓图
    public static native Bitmap canary(Bitmap bitmap);

    //毛玻璃
    public static native Bitmap frostedGlass(Bitmap bitmap);

    //图像增强，直方图均衡化
    public static native Bitmap equalizeHist(Bitmap bitmap);

    //图像增强，拉普拉斯算子
    public static native Bitmap lapulasi(Bitmap bitmap);

    //图像翻转
    public static native Bitmap flip(Bitmap bitmap);

    //图像叠加
    public static native Bitmap add(Bitmap bitmap,Bitmap bitmap2);

    //图像膨胀
    public static native Bitmap dilate(Bitmap bitmap);

    //图像侵蚀
    public static native Bitmap erode(Bitmap bitmap);

    //图像扭曲
    public static native Bitmap warping(Bitmap bitmap);


}
