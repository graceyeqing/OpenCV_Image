package com.yq.opencv_image.utils;

import android.graphics.Bitmap;

import com.bumptech.glide.request.RequestOptions;
import com.yq.opencv_image.R;


/**
 * Glide框架 options
 *
 * ***/

public class GlideOption {
    public static RequestOptions newInstance(){
        RequestOptions options = new RequestOptions()
                .placeholder(R.mipmap.ic_launcher)    //加载成功之前占位图
                .error(R.mipmap.ic_launcher);   //加载错误之后的错误图
        return options;
    }

    public static RequestOptions override(int width, int height){
        RequestOptions options = new RequestOptions()
                .placeholder(R.mipmap.ic_launcher)    //加载成功之前占位图
                .error(R.mipmap.ic_launcher)
                .override(width, height);   //加载错误之后的错误图
        return options;

    }

    public static RequestOptions override(int width, int height, int encodeQuality){
        RequestOptions options = new RequestOptions()
                .placeholder(R.mipmap.ic_launcher)    //加载成功之前占位图
                .error(R.mipmap.ic_launcher)
                .encodeFormat(Bitmap.CompressFormat.WEBP)
                .encodeQuality(encodeQuality)//(0-100的int类型)
                .override(width, height);   //加载错误之后的错误图
        return options;

    }
}
