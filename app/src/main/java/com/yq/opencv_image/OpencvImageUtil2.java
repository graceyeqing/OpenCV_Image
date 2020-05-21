package com.yq.opencv_image;

import android.graphics.Bitmap;
import android.util.Log;

import org.opencv.android.Utils;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.core.MatOfPoint;
import org.opencv.core.Point;
import org.opencv.core.Rect;
import org.opencv.core.Scalar;
import org.opencv.imgproc.Imgproc;

import java.util.LinkedList;
import java.util.List;

/**
 * @author yeqing
 * @des
 * @date 2020/1/8 15:58
 */
public class OpencvImageUtil2 {

    //加载分类器文件
    public static native void loadCascade(String filePath);

    //图像人脸检测
    public static native Bitmap detectFace(Bitmap bitmap);

    //图像人脸检测
    public static native int[] dlibDetectFace(int[] pixels,int height,int width);

    //图像人眼检测
    public static native Bitmap detectEyes(Bitmap bitmap);

    //笑脸检测
    public static native Bitmap detectSmile(String filePath,Bitmap bitmap);

    //唇部检测
    public static native Bitmap detectLips(Bitmap bitmap);

    //文字提取
    public static native Bitmap detectWords(Bitmap bitmap);

    //美颜
    public static native Bitmap beautiful(Bitmap bitmap);



    public static int[] face_detection(Bitmap origin_image){
        float scale = 240.f/ Math.max(origin_image.getHeight(), origin_image.getWidth());
        int width = (int)(origin_image.getWidth()*scale);
        int height = (int)(origin_image.getHeight()*scale);
        Bitmap resize_image=Bitmap.createScaledBitmap(origin_image,width,height , false);

        // 保存所有的像素的数组，图片宽×高
        int[] pixels = new int[width * height];
        resize_image.getPixels(pixels, 0, width, 0, 0, width, height);
        int[] rect=dlibDetectFace(pixels,height,width);
        int[] result_rect=new int[4];
        result_rect[0]=(int)(rect[0]/scale);
        result_rect[1]=(int)(rect[1]/scale);
        result_rect[2]=(int)(rect[2]/scale);
        result_rect[3]=(int)(rect[3]/scale);
        result_rect[2]=result_rect[2]+result_rect[0];
        result_rect[3]=result_rect[3]+result_rect[1];

        return  result_rect;
    }

    /**
     * 种子填充法进行联通区域检测
     */
    public static Bitmap seedFill(Bitmap bitbinImg) {
        Mat binImg = new Mat();
        Utils.bitmapToMat(bitbinImg,binImg);
        Mat lableImg = new Mat();
        Mat showMat = new Mat(binImg.size(), CvType.CV_8UC3);
        binImg.convertTo(lableImg, CvType.CV_32SC1);

        int rows = lableImg.rows();
        int cols = lableImg.cols();

        double lable = 0;
        List<List<Point>> texts = new LinkedList<>();

        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                // 获取种子
                double[] data = lableImg.get(r, c);
                if (data == null || data.length < 1) {
                    continue;
                }
                if (data[0] == 255) {
                    // 背景
                    showMat.put(r, c, 255, 255, 255);
                    continue;
                }
                if (data[0] != 0) {
                    // 已经标记过了
                    continue;
                }

                // 新的填充开始
                lable++;
                double[] color = {Math.random() * 255, Math.random() * 255, Math.random() * 255};

                // 开始种子填充
                LinkedList<Point> neighborPixels = new LinkedList<>();
                neighborPixels.push(new Point(r, c));

                List<Point> textPoint = new LinkedList<>();

                while (!neighborPixels.isEmpty()) {
                    Point curPx = neighborPixels.pop();
                    int row = (int) curPx.x;
                    int col = (int) curPx.y;
                    textPoint.add(new Point(col, row));
                    lableImg.put(row, col, lable);
                    showMat.put(row, col, color);

                    // 左边
                    double[] left = lableImg.get(row, col - 1);
                    if (left != null && left.length > 0 && left[0] == 0) {
                        neighborPixels.push(new Point(row, col - 1));
                    }

                    // 右边
                    double[] right = lableImg.get(row, col + 1);
                    if (right != null && right.length > 0 && right[0] == 0) {
                        neighborPixels.push(new Point(row, col + 1));
                    }

                    // 上边
                    double[] top = lableImg.get(row - 1, col);
                    if (top != null && top.length > 0 && top[0] == 0) {
                        neighborPixels.push(new Point(row - 1, col));
                    }

                    // 下边
                    double[] bottom = lableImg.get(row + 1, col);
                    if (bottom != null && bottom.length > 0 && bottom[0] == 0) {
                        neighborPixels.push(new Point(row + 1, col));
                    }
                }
                texts.add(textPoint);
            }
        }

//        List<Mat> textMats = new LinkedList<>();
//        for (List<Point> data : texts) {
//            MatOfPoint mat = new MatOfPoint();
//            mat.fromList(data);
//            Rect rect = Imgproc.boundingRect(mat);
//            Imgproc.rectangle(showMat, rect.tl(), rect.br(), new Scalar(255, 0, 0, 255));
//            textMats.add(src.submat(rect));
//        }
//        Log.d("yeqing","textMats==="+textMats);

        // 返回展示图
//        return showMat;
        Bitmap bmp = Bitmap.createBitmap(showMat.cols(), showMat.rows(), Bitmap.Config.ARGB_8888);
        Utils.matToBitmap(showMat,bmp);
        return bmp;

    }
}
