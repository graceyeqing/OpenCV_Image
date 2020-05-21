package com.yq.opencv_image.bean;

/**
 * @author yeqing
 * @des
 * @date 2020/1/14 11:44
 */
public final class VisionDetRet {

    private int mLeft;
    private int mTop;
    private int mRight;
    private int mBottom;

    public VisionDetRet() {}

    public VisionDetRet(int l, int t, int r, int b) {
        mLeft = l;
        mTop = t;
        mRight = r;
        mBottom = b;
    }

    public int getLeft() {
        return mLeft;
    }

    public int getTop() {
        return mTop;
    }

    public int getRight() {
        return mRight;
    }

    public int getBottom() {
        return mBottom;
    }
}
