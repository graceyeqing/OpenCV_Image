package com.yq.opencv_image.widget;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.res.TypedArray;
import android.graphics.Bitmap;
import android.graphics.BitmapShader;
import android.graphics.Canvas;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.RectF;
import android.graphics.Shader.TileMode;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.os.Parcelable;
import android.util.AttributeSet;
import android.util.TypedValue;
import android.view.View;
import android.widget.ImageView;

import com.bumptech.glide.Glide;
import com.yq.opencv_image.R;
import com.yq.opencv_image.utils.GlideOption;


@SuppressLint("AppCompatCustomView")
public class XImageView extends ImageView {

    /**
     * 图片的类型，圆形or圆角
     */
    private int type;
    public static final int TYPE_NORMAL = 1000;
    public static final int TYPE_CIRCLE = 0;
    public static final int TYPE_ROUND = 1;
    /**
     * 圆角大小的默认值
     */
    private static final int BODER_RADIUS_DEFAULT = 10;
    /**
     * 圆角的大小
     */
    private int mBorderRadius;
    /**
     * 绘图的Paint
     */
    private Paint mBitmapPaint;
    /**
     * 圆角的半径
     */
    private int mRadius;
    /**
     * 3x3 矩阵，主要用于缩小放大
     */
    private Matrix mMatrix;
    /**
     * 渲染图像，使用图像为绘制图形着色
     */
    private BitmapShader mBitmapShader;
    /**
     * view的宽度
     */
    private int mWidth;
    private RectF mRoundRect;
    /**
     *
     */
    private float mZoomSize = 0;

    private Context mContext;
    private Paint mStrokePaint;
    private boolean isNeedStroke;

    public XImageView(Context context, AttributeSet attrs) {
        super(context, attrs);
        mMatrix = new Matrix();
        mBitmapPaint = new Paint();
        mBitmapPaint.setAntiAlias(true);
        mContext = context;
        TypedArray a = context.obtainStyledAttributes(attrs, R.styleable.XImageView);

        mBorderRadius = a.getDimensionPixelSize(
                R.styleable.XImageView_borderRadius, (int) TypedValue
                        .applyDimension(TypedValue.COMPLEX_UNIT_DIP,
                                BODER_RADIUS_DEFAULT, getResources()
                                        .getDisplayMetrics()));// 默认为10dp
        type = a.getInt(R.styleable.XImageView_type, TYPE_NORMAL);// 默认为Circle
        mZoomSize = a.getFloat(R.styleable.XImageView_zoomSize, 0f);
        isNeedStroke = a.getBoolean(R.styleable.XImageView_isNeedStroke, false);
        int strokeColor = a.getColor(R.styleable.XImageView_strokeColor, 0XFFEEEEEE);

        mStrokePaint = new Paint();
        mStrokePaint.setAntiAlias(true);
        mStrokePaint.setColor(strokeColor);
        mStrokePaint.setStyle(Paint.Style.STROKE);
        mStrokePaint.setStrokeWidth(3);
        a.recycle();
    }

    public void setShape(int type) {
        this.type = type;
    }

    public void setZoomSize(float zoom) {
        this.mZoomSize = zoom;
    }

    public XImageView(Context context) {
        this(context, null);
    }

    @SuppressLint("WrongConstant")
    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        if (mZoomSize != 0 && getVisibility() != GONE) {
            int w = View.MeasureSpec.getSize(widthMeasureSpec);
            int h = (int) (w * mZoomSize);
            setMeasuredDimension(w, h);
        } else {
            super.onMeasure(widthMeasureSpec, heightMeasureSpec);
        }

        /**
         * 如果类型是圆形，则强制改变view的宽高一致，以小值为准
         */
        if (type == TYPE_CIRCLE) {
            mWidth = Math.min(getMeasuredWidth(), getMeasuredHeight());
            mRadius = mWidth / 2;
            setMeasuredDimension(mWidth, mWidth);
        }

    }

    /**
     * 初始化BitmapShader
     */
    private void setUpShader() {
        Drawable drawable = getDrawable();
        if (drawable == null) {
            return;
        }

        try {
            Bitmap bmp = drawableToBitamp(drawable);
            // 将bmp作为着色器，就是在指定区域内绘制bmp
            mBitmapShader = new BitmapShader(bmp, TileMode.CLAMP, TileMode.CLAMP);
            float scale = 1.0f;
            if (type == TYPE_CIRCLE) {
                // 拿到bitmap宽或高的小值
                int bSize = Math.min(bmp.getWidth(), bmp.getHeight());
                scale = mWidth * 1.0f / bSize;

            } else if (type == TYPE_ROUND) {
                if (!(bmp.getWidth() == getWidth() && bmp.getHeight() == getHeight())) {
                    // 如果图片的宽或者高与view的宽高不匹配，计算出需要缩放的比例；缩放后的图片的宽高，一定要大于我们view的宽高；所以我们这里取大值；
                    scale = Math.max(getWidth() * 1.0f / bmp.getWidth(), getHeight() * 1.0f / bmp.getHeight());
                }

            }
            // shader的变换矩阵，我们这里主要用于放大或者缩小
            mMatrix.setScale(scale, scale);
            // 设置变换矩阵
            mBitmapShader.setLocalMatrix(mMatrix);
            // 设置shader
            mBitmapPaint.setShader(mBitmapShader);
        } catch (Exception e) {

        }
    }

    @Override
    protected void onDraw(Canvas canvas) {
        if (getDrawable() == null) {
            return;
        }

        if (type == TYPE_ROUND) {
            setUpShader();
            canvas.drawRoundRect(mRoundRect, mBorderRadius, mBorderRadius, mBitmapPaint);
        } else if (type == TYPE_CIRCLE) {
            setUpShader();
            canvas.drawCircle(mRadius, mRadius, mRadius, mBitmapPaint);
            if (isNeedStroke) {
                canvas.drawCircle(mRadius, mRadius, mRadius - 1, mStrokePaint);
            }
        } else {
            super.onDraw(canvas);
        }
    }

    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);

        // 圆角图片的范围
        if (type == TYPE_ROUND)
            mRoundRect = new RectF(0, 0, w, h);
    }

    /**
     * drawable转bitmap
     *
     * @param drawable
     * @return
     */
    private Bitmap drawableToBitamp(Drawable drawable) {
        if (drawable instanceof BitmapDrawable) {
            BitmapDrawable bd = (BitmapDrawable) drawable;
            return bd.getBitmap();
        }
        int w = drawable.getIntrinsicWidth();
        int h = drawable.getIntrinsicHeight();
        Bitmap bitmap = Bitmap.createBitmap(w, h, Bitmap.Config.RGB_565);
        Canvas canvas = new Canvas(bitmap);
        drawable.setBounds(0, 0, w, h);
        drawable.draw(canvas);
        return bitmap;
    }

    private static final String STATE_INSTANCE = "state_instance";
    private static final String STATE_TYPE = "state_type";
    private static final String STATE_BORDER_RADIUS = "state_border_radius";

    @Override
    protected Parcelable onSaveInstanceState() {
        Bundle bundle = new Bundle();
        bundle.putParcelable(STATE_INSTANCE, super.onSaveInstanceState());
        bundle.putInt(STATE_TYPE, type);
        bundle.putInt(STATE_BORDER_RADIUS, mBorderRadius);
        return bundle;
    }

    @Override
    protected void onRestoreInstanceState(Parcelable state) {
        if (state instanceof Bundle) {
            Bundle bundle = (Bundle) state;
            super.onRestoreInstanceState(((Bundle) state)
                    .getParcelable(STATE_INSTANCE));
            this.type = bundle.getInt(STATE_TYPE);
            this.mBorderRadius = bundle.getInt(STATE_BORDER_RADIUS);
        } else {
            super.onRestoreInstanceState(state);
        }

    }

    public void setBorderRadius(int borderRadius) {
        int pxVal = dp2px(borderRadius);
        if (this.mBorderRadius != pxVal) {
            this.mBorderRadius = pxVal;
            invalidate();
        }
    }

    public void setType(int type) {
        if (this.type != type) {
            this.type = type;
            requestLayout();
        }

    }

    public int dp2px(int dpVal) {
        return (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP,
                dpVal, getResources().getDisplayMetrics());
    }

    public void setGifURL(String url) {
        loadImage(url, 0, 0, 0, 0);
    }

    public void setImageURL(String url) {
        loadImage(url, 0, 0, 0, 0);
    }

    public void setImageURL(String url, int defImgRes) {
        loadImage(url, 0, 0, 0, defImgRes);
    }

    public void setImageURL(String url, int with, int height, int defImgRes) {
        loadImage(url, with, height, 0, defImgRes);
    }

    public void setImageURL(String url, int with, int height, int encodeQuality, int defImgRes) {
        loadImage(url, with, height, encodeQuality, defImgRes);
    }

    private void loadImage(String url, int with, int height, int encodeQuality, int defImgRes) {
        if (encodeQuality > 0 && with > 0 && height > 0) {
            Glide.with(mContext)
                    .load(url)
                    .centerCrop()
                    .dontAnimate()//优化图片加载慢
                    .apply(GlideOption.override(with, height, encodeQuality))
                    .into(this);
        }else if(with > 0 && height > 0){
            Glide.with(mContext)
                    .load(url)
                    .centerCrop()
                    .dontAnimate()//优化图片加载慢
                    .apply(GlideOption.override(with, height))
                    .into(this);
        }else if (defImgRes != 0) {
            Glide.with(mContext)
                    .load(url).centerCrop()
                    .error(defImgRes)
                    .placeholder(defImgRes)
                    .dontAnimate()//优化图片加载慢
                    .into(this);
        } else {
            Glide.with(mContext)
                    .load(url)
                    .error(R.mipmap.ic_launcher)
                    .placeholder(R.mipmap.ic_launcher)
                    .centerCrop()
                    .dontAnimate()//优化图片加载慢
                    .into(this);
        }
    }

}
