package com.yq.opencv_image;

import android.graphics.Bitmap;
import android.util.Log;

import androidx.annotation.Keep;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.WorkerThread;

import com.yq.opencv_image.bean.VisionDetRet;

import java.util.Arrays;
import java.util.List;

/**
 * @author yeqing
 * @des
 * @date 2020/1/14 11:47
 */
public class FaceDet {
    private static final String TAG = "FaceDet";

    // accessed by native methods
    @SuppressWarnings("unused")
    private long mNativeFaceDetContext;

    static {
        try {
            // 预加载native方法库
            System.loadLibrary("native-lib2");
            jniNativeClassInit();
            Log.d(TAG, "jniNativeClassInit success");
        } catch (UnsatisfiedLinkError e) {
            Log.e(TAG, "library not found");
        }
    }

    public FaceDet() {
        jniInit();
    }

    @Nullable
    @WorkerThread
    public List<VisionDetRet> detect(@NonNull Bitmap bitmap) {
        VisionDetRet[] detRets = jniBitmapDet(bitmap);
        return Arrays.asList(detRets);
    }

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        release();
    }

    public void release() {
        jniDeInit();
    }

    @Keep
    private native static void jniNativeClassInit();

    @Keep
    private synchronized native int jniInit();

    @Keep
    private synchronized native int jniDeInit();

    @Keep
    private synchronized native VisionDetRet[] jniBitmapDet(Bitmap bitmap);
}
