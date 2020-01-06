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

    //毛玻璃效果
    public static native Bitmap mosaic(Bitmap bitmap);

    //灰度化
    public static native Bitmap gray(Bitmap bitmap);


}
