package com.yq.opencv_image;

import android.graphics.Bitmap;

/**
 * @author yeqing
 * @des
 * @date 2020/1/8 15:58
 */
public class OpencvImageUtil2 {

    //图像人脸检测
    public static native void loadCascade(String filePath);

    //图像人脸检测
    public static native Bitmap detectFace(Bitmap bitmap);
}
