#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <QImage>
#include <QPainter>
#include <QDebug>
#include <QVideoWidget>
#include <QTime>
#pragma comment(lib, "F:\\Lindazyy\\ffmpeg-20190722-3883c9d-win64-dev\\ffmpeg-20190722-3883c9d-win64-dev\\lib\\avcodec.lib")
#pragma comment(lib, "F:\\Lindazyy\\ffmpeg-20190722-3883c9d-win64-dev\\ffmpeg-20190722-3883c9d-win64-dev\\lib\\avdevice.lib")
#pragma comment(lib, "F:\\Lindazyy\\ffmpeg-20190722-3883c9d-win64-dev\\ffmpeg-20190722-3883c9d-win64-dev\\lib\\avfilter.lib")
#pragma comment(lib, "F:\\Lindazyy\\ffmpeg-20190722-3883c9d-win64-dev\\ffmpeg-20190722-3883c9d-win64-dev\\lib\\avformat.lib")
#pragma comment(lib, "F:\\Lindazyy\\ffmpeg-20190722-3883c9d-win64-dev\\ffmpeg-20190722-3883c9d-win64-dev\\lib\\avutil.lib")
#pragma comment(lib, "F:\\Lindazyy\\ffmpeg-20190722-3883c9d-win64-dev\\ffmpeg-20190722-3883c9d-win64-dev\\lib\\postproc.lib")
#pragma comment(lib, "F:\\Lindazyy\\ffmpeg-20190722-3883c9d-win64-dev\\ffmpeg-20190722-3883c9d-win64-dev\\lib\\swresample.lib")
#pragma comment(lib, "F:\\Lindazyy\\ffmpeg-20190722-3883c9d-win64-dev\\ffmpeg-20190722-3883c9d-win64-dev\\lib\\swscale.lib")
#pragma comment(lib, "F:\\Lindazyy\\opencv4.1\\opencv\\build\\x64\\vc15\\lib\\opencv_world411d.lib")
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include <libswscale/swscale.h>
#include <libavutil/mathematics.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/timestamp.h>
#include <libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
}
#include "opencv2/opencv.hpp"
#include "omp.h"

class videoWidget : public QVideoWidget
{
    Q_OBJECT
public:
    explicit videoWidget(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *e);
    void playVideo();
    void stopVideo();
    void setPosition(int64_t);
    void quickFlash(int64_t);
    void frameFlash(int64_t, int64_t);
    int width; int height;
    int64_t timestamp = 0;
    int64_t duration;
    QString path;
    int framecnt = 0;
private:
    QImage image;
    AVFormatContext *pFormatCtx;
    AVCodec *pCodec;
    AVCodecContext*		pCodecCtx;
    AVFrame *frame;
    struct SwsContext* sws_ctx = NULL;
    AVFrame* pFrameRGB;		//yuv frame, transmit source stream data
    uint8_t* bufferRGB;
    AVStream *stream;
    int videoStream;
    //QPainter *pp;
signals:
    void videoEnd();
public slots:
    void updateFrame();
};

#endif // VIDEOWIDGET_H
