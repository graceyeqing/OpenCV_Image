package com.yq.opencv_image;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.provider.MediaStore;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.widget.RadioGroup;
import android.widget.Toast;

import com.bumptech.glide.Glide;
import com.yanzhenjie.album.Action;
import com.yanzhenjie.album.Album;
import com.yanzhenjie.album.AlbumFile;
import com.yq.opencv_image.utils.FileUtil;
import com.yq.opencv_image.widget.XImageView;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;

import java.io.File;
import java.util.ArrayList;

import static com.bumptech.glide.request.RequestOptions.centerCropTransform;

public class MainActivity extends AppCompatActivity {
    private String TAG = "yeqing";
    private String mPicFilePath;
    private ImageView imageView;
    private ImageView imageView2;
    private final int PHOTO_REQUEST = 20;
    private RadioGroup radioGroup,radioGroup2;
    private boolean isGroup2;

    private BaseLoaderCallback mLoaderCallback = new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            switch (status) {
                case LoaderCallbackInterface.SUCCESS: {
                    Log.i(TAG, "OpenCV loaded successfully");

                    // Load native library after(!) OpenCV initialization
                    System.loadLibrary("native-lib");

                }
                break;
                default: {
                    super.onManagerConnected(status);
                }
                break;
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        // Example of a call to a native method
        imageView = findViewById(R.id.image);
        imageView2 = findViewById(R.id.image2);

        imageView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                //去选择头像
                selectImage();
            }
        });

        radioGroup = findViewById(R.id.radiogroup);
        radioGroup2 = findViewById(R.id.radiogroup2);
    }

    public void onClickView(View view){
        Bitmap src = null;
        Bitmap resultBit = null;
        if (!TextUtils.isEmpty(mPicFilePath)) {
            src = FileUtil.picFileToBitmap(mPicFilePath);
        } else {
            src = FileUtil.resourceToBitmap(MainActivity.this, R.mipmap.test);
        }
        int id = view.getId();
        switch (id){
            case R.id.radio1:
                //灰度化
                resultBit = OpencvImageUtil.gray(src);
                isGroup2 = false;
                break;
            case R.id.radio2:
                //浮雕
                resultBit = OpencvImageUtil.relief(src);
                isGroup2 = false;
                break;
            case R.id.radio3:
                //油画
                resultBit = OpencvImageUtil.oilPaiting(src);
                isGroup2 = false;
                break;
            case R.id.radio4:
                //轮廓图
                resultBit = OpencvImageUtil.canary(src);
                isGroup2 = false;
                break;
            case R.id.radio5:
                //模糊
                resultBit = OpencvImageUtil.blur(src);
                isGroup2 = false;
                break;
            case R.id.radio6:
                //毛玻璃
                resultBit = OpencvImageUtil.frostedGlass(src);
                isGroup2 = false;
                break;
            case R.id.radio7:
                //马赛克
                resultBit = OpencvImageUtil.mosaic(src);
                isGroup2 = false;
                break;
            case R.id.radio8:
                //图像增强
                resultBit = OpencvImageUtil.equalizeHist(src);
                isGroup2 = true;
                break;
            case R.id.radio9:
                //图像增强
                resultBit = OpencvImageUtil.lapulasi(src);
                isGroup2 = true;
                break;
            case R.id.radio10:
                //图像翻转
                resultBit = OpencvImageUtil.flip(src);
                isGroup2 = true;
                break;
            case R.id.radio11:
                //图像叠加
                src = FileUtil.resourceToBitmap(MainActivity.this, R.mipmap.test);
                Bitmap src2 = FileUtil.resourceToBitmap(MainActivity.this, R.mipmap.test2);
                resultBit = OpencvImageUtil.add(src,src2);
                isGroup2 = true;
                break;
            case R.id.radio12:
                //图像膨胀
                resultBit = OpencvImageUtil.dilate(src);
                isGroup2 = true;
                break;
            case R.id.radio13:
                //图像侵蚀
                resultBit = OpencvImageUtil.erode(src);
                isGroup2 = true;
                break;
            case R.id.radio14:
                //图像扭曲
                resultBit = OpencvImageUtil.warping(src);
                isGroup2 = true;
                break;

        }
        if(isGroup2){
            radioGroup.clearCheck();
        }else {
            radioGroup2.clearCheck();
        }
        if (resultBit != null) {
            imageView2.setImageBitmap(resultBit);
        } else {
            Toast.makeText(MainActivity.this, "处理出错", Toast.LENGTH_SHORT).show();
        }
    }


    @Override
    public void onResume() {
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
//        String imageName = String.valueOf(System.currentTimeMillis());
//        mPicFilePath = Environment.getExternalStorageDirectory() + "/headImage" + imageName + ".jpg";
//        File file2 = new File(mPicFilePath);
//        if (!file2.exists()) {
//            file2.getParentFile().mkdirs();
//        } else {
//            file2.delete();
//        }
//        Intent intent2 = new Intent(Intent.ACTION_PICK, MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
//        intent2.setDataAndType(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, "image/*");
//        startActivityForResult(intent2, PHOTO_REQUEST);

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
        int[] image = OpencvImageUtil.ImageBlur(pixels, w, h);//JNI
        //最后将返回的int数组转为bitmap类型。
        Bitmap desImage = Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888);
        //faceall为返回的int数组
        desImage.setPixels(image, 0, w, 0, 0, w, h);
        return desImage;
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        switch (requestCode) {
            case PHOTO_REQUEST:// 从相册取
                if (data != null && data.getData() != null) {
                    try {
                        Uri uri = data.getData();
                        if (uri != null) {
                            String filePath = FileUtil.getFileAbsolutePath(this, uri);
                            Log.d("yeqing","filePath=="+filePath);
                            imageView.setImageBitmap(BitmapFactory.decodeFile(filePath));
                        }
                    } catch (Exception e) {
                    }
                }
                break;
            default:
                break;
        }
    }
}
