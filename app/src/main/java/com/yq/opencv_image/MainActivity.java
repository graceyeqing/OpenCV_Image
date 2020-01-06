package com.yq.opencv_image;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.Log;
import android.util.TypedValue;
import android.view.View;
import android.widget.ImageView;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.Toast;

import com.yanzhenjie.album.Action;
import com.yanzhenjie.album.Album;
import com.yanzhenjie.album.AlbumFile;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.android.Utils;

import java.util.ArrayList;

public class MainActivity extends AppCompatActivity {
    private String TAG = "yeqing";
    private String mPicFilePath;
    private ImageView imageView;
    private ImageView imageView2;
    private BaseLoaderCallback mLoaderCallback = new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            switch (status) {
                case LoaderCallbackInterface.SUCCESS:
                {
                    Log.i(TAG, "OpenCV loaded successfully");

                    // Load native library after(!) OpenCV initialization
                    System.loadLibrary("native-lib");

                } break;
                default:
                {
                    super.onManagerConnected(status);
                } break;
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        imageView = findViewById(R.id.image);
        imageView2  = findViewById(R.id.image2);

        imageView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                selectImage();
            }
        });

        RadioGroup radioGroup = findViewById(R.id.radiogroup);
        //图像处理
        radioGroup.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                Log.d("yeqing", "checkedId===" + checkedId);
                Bitmap src = null;
                Bitmap resultBit = null;
                if (!TextUtils.isEmpty(mPicFilePath)) {
                    src = FileUtil.picFileToBitmap(mPicFilePath);
                } else {
                    src = FileUtil.resourceToBitmap(MainActivity.this, R.mipmap.ic_launcher);
                }
                switch (checkedId) {
                    case 1://灰度化
                        resultBit = OpencvImageUtil.gray(src);
                        break;
                    case 2://浮雕
//                        resultBit = OpencvUtil.relief(src);
                        break;
                    case 3://油画
                        break;
                    case 4://轮廓图
//                        resultBit = OpencvUtil.canary(src);
                        break;
                    case 5://模糊
                        resultBit = OpencvImageUtil.blur(src);
                        break;
                    case 6://不规则模糊
//                        resultBit = OpencvUtil.IrregularBlur(src);
                        break;
                    case 7://马赛克
                        resultBit = OpencvImageUtil.mosaic(src);
                        break;
                }
                if (resultBit != null) {
                    imageView2.setImageBitmap(resultBit);
                } else {
                    Toast.makeText(MainActivity.this, "处理出错", Toast.LENGTH_SHORT).show();
                }


            }
        });
    }

    @Override
    public void onResume()
    {
        super.onResume();
        if (!OpenCVLoader.initDebug()) {
            Log.d(TAG, "Internal OpenCV library not found. Using OpenCV Manager for initialization");
            OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_3_0_0, this, mLoaderCallback);
        } else {
            Log.d(TAG, "OpenCV library found inside package. Using it!");
            mLoaderCallback.onManagerConnected(LoaderCallbackInterface.SUCCESS);
        }
    }

    private void selectImage() {
        Album.image(this)
                .singleChoice() // Multi-Mode, Single-Mode: singleChoice().
                .columnCount(4) // The number of columns in the page list.
                .camera(true) // Whether the camera appears in the Item.
                .onResult(new Action<ArrayList<AlbumFile>>() {
                    @Override
                    public void onAction(@NonNull ArrayList<AlbumFile> result) {

                        if (result != null && result.size() > 0) {
                            mPicFilePath = result.get(0).getPath();
                            imageView.setImageBitmap(BitmapFactory.decodeFile(mPicFilePath));
                            imageView2 .setImageBitmap(doImageBlur(BitmapFactory.decodeFile(mPicFilePath)));

                        }
                    }
                })
                .onCancel(new Action<String>() {
                    @Override
                    public void onAction(@NonNull String result) {
                        // The user canceled the operation.
                    }
                })
                .start();
    }

    /**

     * 调用JNI的ImageBlur(int[] pixels,int w,int h)接口实现图像模糊

     */

    public Bitmap doImageBlur(Bitmap origImage) {
        int w = origImage.getWidth();
        int h = origImage.getHeight();
        int[] pixels = new int[w * h];
        origImage.getPixels(pixels, 0, w, 0, 0, w, h);
        int[] image=OpencvImageUtil.ImageBlur(pixels,w,h);//JNI
        //最后将返回的int数组转为bitmap类型。
        Bitmap desImage=Bitmap.createBitmap(w,h,Bitmap.Config.ARGB_8888);
        //faceall为返回的int数组
        desImage.setPixels(image,0,w,0,0,w,h);
        return desImage;
    }
}
