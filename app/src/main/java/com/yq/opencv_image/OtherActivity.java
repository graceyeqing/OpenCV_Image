package com.yq.opencv_image;

import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.net.Uri;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.widget.RadioGroup;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import com.yanzhenjie.album.Action;
import com.yanzhenjie.album.Album;
import com.yanzhenjie.album.AlbumFile;
import com.yq.opencv_image.utils.FileUtil;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.core.Point;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.LinkedList;

public class OtherActivity extends AppCompatActivity {
    private String TAG = "yeqing";
    private String mPicFilePath;
    private ImageView imageView;
    private ImageView imageView2;
    private final int PHOTO_REQUEST = 0;
    private RadioGroup radioGroup, radioGroup2;
    private boolean isGroup2;
    private File mCascadeFile;
    private File mEyeCascadeFile;
    private File smileCascadeFile;

    private BaseLoaderCallback mLoaderCallback = new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            switch (status) {
                case LoaderCallbackInterface.SUCCESS: {
                    Log.i(TAG, "OpenCV loaded successfully");

                    // Load native library after(!) OpenCV initialization
                    System.loadLibrary("native-lib");
                    System.loadLibrary("native-lib2");

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
        setContentView(R.layout.activity_other);
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

    public void onClickView(View view) {
        Bitmap src = null;
        Bitmap resultBit = null;
        if (!TextUtils.isEmpty(mPicFilePath)) {
            src = FileUtil.picFileToBitmap(mPicFilePath);
        } else {
            src = FileUtil.resourceToBitmap(OtherActivity.this, R.mipmap.test);
        }
        int id = view.getId();
        switch (id) {
            case R.id.radio1:
                //人脸检测
                copyCascadeFile("haarcascade_profileface.xml");
                if (mCascadeFile != null) {
                    OpencvImageUtil2.loadCascade(mCascadeFile.getAbsolutePath());
                }
                resultBit = OpencvImageUtil2.detectFace(src);
                isGroup2 = false;
                break;
            case R.id.radio2:
                //人眼检测
                copyCascadeFile("haarcascade_eye.xml");
                if (mEyeCascadeFile != null) {
                    OpencvImageUtil2.loadCascade(mEyeCascadeFile.getAbsolutePath());
                }
                resultBit = OpencvImageUtil2.detectEyes(src);
                isGroup2 = false;
                break;
            case R.id.radio3:
                //笑脸检测
                copyCascadeFile("haarcascade_frontalface_alt.xml");
                copyCascadeFile("haarcascade_smile.xml");
                if (mCascadeFile != null && smileCascadeFile != null) {
                    OpencvImageUtil2.loadCascade(mCascadeFile.getAbsolutePath());
                }
                resultBit = OpencvImageUtil2.detectSmile(smileCascadeFile.getAbsolutePath(),src);
                isGroup2 = false;
                break;
            case R.id.radio4:
                //唇部检测
                copyCascadeFile("haarcascade_profileface.xml");
                if (mCascadeFile != null) {
                    OpencvImageUtil2.loadCascade(mCascadeFile.getAbsolutePath());
                }
                resultBit = OpencvImageUtil2.detectLips(src);
                isGroup2 = false;
                break;
            case R.id.radio5:
                //文字提取
                resultBit = OpencvImageUtil2.detectWords(src);
//                resultBit = OpencvImageUtil2.seedFill(temp);
                isGroup2 = false;
                break;
            case R.id.radio6:
                //毛玻璃
                int[] rect=OpencvImageUtil2.face_detection(src);
                Canvas canvas=new Canvas(resultBit);
                Paint p=new Paint();
                p.setColor(Color.RED);
                p.setStrokeWidth(3.0f);
                canvas.drawLine(rect[0], rect[1], rect[2], rect[1], p);//up
                canvas.drawLine(rect[0], rect[1], rect[0], rect[3], p);//left
                canvas.drawLine(rect[0], rect[3], rect[2], rect[3], p);//down
                canvas.drawLine(rect[2], rect[1], rect[2], rect[3], p);
                isGroup2 = false;
                break;
            case R.id.radio7:
                //美颜
                resultBit = OpencvImageUtil2.beautiful(src);
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
                src = FileUtil.resourceToBitmap(OtherActivity.this, R.mipmap.test);
                Bitmap src2 = FileUtil.resourceToBitmap(OtherActivity.this, R.mipmap.test);
                resultBit = OpencvImageUtil.add(src, src);
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
        if (isGroup2) {
            radioGroup.clearCheck();
        } else {
            radioGroup2.clearCheck();
        }
        if (resultBit != null) {
            imageView2.setImageBitmap(resultBit);
        } else {
            Toast.makeText(OtherActivity.this, "处理出错", Toast.LENGTH_SHORT).show();
        }
    }

    private void copyCascadeFile( String cascadeFile) {
        try {
            // load cascade file from application resources
            InputStream inputStream;
            FileOutputStream os;
            File cascadeDir = getDir("cascade", Context.MODE_PRIVATE);
            if (cascadeFile.contains("face")) {
                inputStream = getResources().openRawResource(R.raw.lbpcascade_frontalface);
                mCascadeFile = new File(cascadeDir, cascadeFile);
                if (mCascadeFile.exists()) return;
                os = new FileOutputStream(mCascadeFile);
            } else if(cascadeFile.contains("eye")){
                inputStream = getResources().openRawResource(R.raw.haarcascade_eye);
                mEyeCascadeFile = new File(cascadeDir, cascadeFile);
                if (mEyeCascadeFile.exists()) return;
                os = new FileOutputStream(mEyeCascadeFile);
            }else {
                inputStream = getResources().openRawResource(R.raw.haarcascade_smile);
                smileCascadeFile = new File(cascadeDir, cascadeFile);
                if (smileCascadeFile.exists()) return;
                os = new FileOutputStream(smileCascadeFile);
            }

            byte[] buffer = new byte[4096];
            int bytesRead;
            while ((bytesRead = inputStream.read(buffer)) != -1) {
                os.write(buffer, 0, bytesRead);
            }
            inputStream.close();
            os.close();
        } catch (Exception e) {
            e.printStackTrace();
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
//        File file = new File(mPicFilePath);
//        if (!file.exists()) {
//            file.getParentFile().mkdirs();
//        } else {
//            file.delete();
//        }
//        Intent intent = new Intent(Intent.ACTION_PICK, MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
//        intent.setDataAndType(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, "image/*");
//        startActivityForResult(intent, PHOTO_REQUEST);

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
                            radioGroup.clearCheck();
                            radioGroup2.clearCheck();
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
                            Log.d("yeqing", "filePath==" + filePath);
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
