#include <jni.h>
#include <string>
#include <malloc.h>
#include <cstring>
#include <android/log.h>
#include <android/bitmap.h>
#include "gif_lib.h"

#define  LOG_TAG    "TMING"
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define  argb(a, r, g, b) ( ((a) & 0xff) << 24 ) | ( ((b) & 0xff) << 16 ) | ( ((g) & 0xff) << 8 ) | ((r) & 0xff)

typedef struct GifBean {
    int current_frame;//播放帧数
    int total_frame;//总帧数
    int *dealys;//延迟时间数组,长度不确定

} GifBean;

//绘制一张图片
void drawFrame(GifFileType *gifFileType, GifBean *gifBean, AndroidBitmapInfo info, void *pixels) {
    //当前帧
    SavedImage savedImage = gifFileType->SavedImages[gifBean->current_frame];
    //整幅图片的首地址
    int *px = (int *) pixels;
    int pointPixel;
    GifImageDesc frameInfo = savedImage.ImageDesc;
    GifByteType gifByteType;
    ColorMapObject *colorMapObject = frameInfo.ColorMap;
    px = (int *) ((char *) px + info.stride * frameInfo.Top);
    //每一行的首地址
    int *line;
    for (int y = frameInfo.Top; y < frameInfo.Top + frameInfo.Height; ++y) {
        line = px;
        for (int x = frameInfo.Left; x < frameInfo.Left + frameInfo.Width; ++x) {
            //拿到每一个坐标的位置  索引    --->  数据
            pointPixel = (y - frameInfo.Top) * frameInfo.Width + (x - frameInfo.Left);
            //解压
            gifByteType = savedImage.RasterBits[pointPixel];
            GifColorType gifColorType = colorMapObject->Colors[gifByteType];
            line[x] = argb(255, gifColorType.Red, gifColorType.Green, gifColorType.Blue);
        }
        px = (int *) ((char *) px + info.stride);
    }


}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_tming_wy_gifplayer_GifHandler_loadPath(JNIEnv *env, jobject instance, jstring path_) {
    const char *path = env->GetStringUTFChars(path_, 0);
    int err;
    GifFileType *gifFileType = DGifOpenFileName(path, &err);
    DGifSlurp(gifFileType);
    //new GifBean
    GifBean *gifBean = (GifBean *) malloc(sizeof(GifBean));
    //清空内存地址  32 000000000
    memset(gifBean, 0, sizeof(GifBean));
    gifFileType->UserData = gifBean;
    //初始化数组
    gifBean->dealys = (int *) malloc(sizeof(int) * gifFileType->ImageCount);
    memset(gifBean->dealys, 0, sizeof(int) * gifFileType->ImageCount);
    //延迟事件  读取
    //Delay Time - 单位1/100秒，如果值不为1，表示暂停规定的时间后再继续往下处理数据流
    //获取时间
    gifFileType->UserData = gifBean;
    gifBean->current_frame = 0;
    gifBean->total_frame = gifFileType->ImageCount;
    ExtensionBlock *ext;
    for (int i = 0; i < gifFileType->ImageCount; ++i) {
        SavedImage frame = gifFileType->SavedImages[i];
        for (int j = 0; j < frame.ExtensionBlockCount; ++j) {
            if (frame.ExtensionBlocks[j].Function == GRAPHICS_EXT_FUNC_CODE) {
                ext = &frame.ExtensionBlocks[j];
                break;
            }
        }
        if (ext) {
            //Delay Time - 单位1/100秒   1s/100
            int frame_delay = 10 * (ext->Bytes[1] | (ext->Bytes[2] << 8));//ms
            LOGE("时间  %d   ", frame_delay);
            gifBean->dealys[i] = frame_delay;
        }
    }
    LOGE("gif  长度大小    %d  ", gifFileType->ImageCount);

    env->ReleaseStringUTFChars(path_, path);
    return (jlong) gifFileType;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_tming_wy_gifplayer_GifHandler_getWidth__J(JNIEnv *env, jobject instance, jlong ndkGif) {

    GifFileType *gifFileType = (GifFileType *) ndkGif;
    return gifFileType->SWidth;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_tming_wy_gifplayer_GifHandler_getHeight__J(JNIEnv *env, jobject instance, jlong ndkGif) {
    GifFileType *gifFileType = (GifFileType *) ndkGif;
    return gifFileType->SHeight;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_tming_wy_gifplayer_GifHandler_updateFrame__JLandroid_graphics_Bitmap_2(JNIEnv *env,
                                                                                jobject instance,
                                                                                jlong ndkGif,
                                                                                jobject bitmap) {
    GifFileType *gifFileType = (GifFileType *) ndkGif;
    GifBean *gifBean = (GifBean *) gifFileType->UserData;
    AndroidBitmapInfo info;
    AndroidBitmap_getInfo(env, bitmap, &info);
    void *pixels;
    AndroidBitmap_lockPixels(env, bitmap, &pixels);
    drawFrame(gifFileType, gifBean, info, pixels);
    gifBean->current_frame += 1;
    if (gifBean->current_frame >= gifBean->total_frame - 1) {
        gifBean->current_frame = 0;
        LOGE("重新过来  %d  ", gifBean->current_frame);
    }
    AndroidBitmap_unlockPixels(env, bitmap);
    return gifBean->dealys[gifBean->current_frame];
}

