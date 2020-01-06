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


}
