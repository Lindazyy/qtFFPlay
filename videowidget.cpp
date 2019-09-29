#include "videowidget.h"
extern omp_lock_t lock;



videoWidget::videoWidget(QWidget *parent) : QVideoWidget(parent)
{
    avformat_network_init();
}

void videoWidget::paintEvent(QPaintEvent *e)
{

    QPainter pp(this);
    QRect Temp(0,0,this->width,this->height);
    omp_set_lock(&lock);
    pp.drawImage(Temp, image);
    omp_unset_lock(&lock);
}

void videoWidget::playVideo(){
    std::string filename = path.toStdString();
    pFormatCtx = avformat_alloc_context();
    if (avformat_open_input(&pFormatCtx, filename.c_str(), nullptr, nullptr) != 0) {
        qDebug() << "cannot open input file\n";
        exit(1);
    }
    else qDebug() << "opened video\n";
    if (avformat_find_stream_info(pFormatCtx, nullptr) < 0) {
        qDebug() << "cannot initialize pformatctx\n";
        exit(1);
    }

    duration = pFormatCtx->duration;

    //找到视频对应的avstream id，并初始化编解码器
    videoStream = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &pCodec, 0);
    if (pCodec == nullptr) {
        qDebug() << "Unsupported Codec!\n";
        exit(1);
    }

    pCodecCtx = pFormatCtx->streams[videoStream]->codec;
    if (avcodec_open2(pCodecCtx, pCodec, nullptr) < 0) {
        qDebug() << "Could not open codec.";
        exit(1);
    }

    width = pCodecCtx->width; height = pCodecCtx->height;
    this->setGeometry(0, 0, width, height);

    sws_ctx = sws_getContext(width, height, pCodecCtx->pix_fmt, width, height, AV_PIX_FMT_RGB24, SWS_BICUBIC, nullptr, nullptr, nullptr);

    pFrameRGB = av_frame_alloc(); frame = av_frame_alloc();
    int rgbBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, width, height, 1);
    bufferRGB = (uint8_t*)av_malloc(rgbBytes * sizeof(uint8_t));
    av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, bufferRGB, AV_PIX_FMT_RGB24, width, height, 1);
}

void videoWidget::updateFrame(){

    AVPacket                packet;
    int ret;
    do{
        ret = av_read_frame(pFormatCtx, &packet);
    }while(packet.stream_index != videoStream);
    stream = pFormatCtx->streams[packet.stream_index];
    if (ret >= 0) {
        framecnt++;
        if(avcodec_send_packet(pCodecCtx, &packet)!=0){
            qDebug() << "send packet failure";
            return;
        }
        ret = avcodec_receive_frame(pCodecCtx, frame);
        if(ret!=0){
            qDebug() << "receive frame failure";
            return;
        }
        timestamp = packet.pts* av_q2d(stream->time_base)*AV_TIME_BASE;
        sws_scale(sws_ctx, (uint8_t const* const*)frame->data, frame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
        QImage img((uchar *)bufferRGB,width,height,QImage::Format_RGB888);
        omp_set_lock(&lock);
        image = img.copy();
        omp_unset_lock(&lock);
    }else{
        qDebug() << "bbbbbbbb";
        emit videoEnd();
    }
    av_packet_unref(&packet);

}

void videoWidget::stopVideo(){
    av_free(pFrameRGB);
    av_free(bufferRGB);
    av_free(frame);
    avio_close(pFormatCtx->pb);
    avformat_free_context(pFormatCtx);
    sws_freeContext(sws_ctx);
}

void videoWidget::setPosition(int64_t position){
    QTime time;
    time.start();
    AVPacket pkt;
    av_seek_frame(pFormatCtx, -1, position, AVSEEK_FLAG_BACKWARD);
    int ret = av_read_frame(pFormatCtx, &pkt);
    avcodec_send_packet(pCodecCtx, &pkt);
    avcodec_receive_frame(pCodecCtx, frame);
    int64_t temp = 0;
    while(temp <= position) {
        AVPacket avpkt;
        ret = av_read_frame(pFormatCtx, &avpkt);
        if (ret >= 0 && avpkt.stream_index==0) {

            avcodec_send_packet(pCodecCtx, &avpkt);
            avcodec_receive_frame(pCodecCtx, frame);
            temp = avpkt.pts* av_q2d(stream->time_base)*AV_TIME_BASE;
            timestamp = temp;
        }
        av_packet_unref(&avpkt);
    }
    av_packet_unref(&pkt);

    sws_scale(sws_ctx, (uint8_t const* const*)frame->data, frame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
    QImage img((uchar *)bufferRGB,width,height,QImage::Format_RGB888);
    omp_set_lock(&lock);
    image = img.copy();
    omp_unset_lock(&lock);
    qDebug()<<time.elapsed()/1000.0<<"s";
}

void videoWidget::quickFlash(int64_t position){

    AVPacket pkt;
    av_seek_frame(pFormatCtx, -1, position, AVSEEK_FLAG_BACKWARD);
    int ret = av_read_frame(pFormatCtx, &pkt);
    if(ret>=0){
        avcodec_send_packet(pCodecCtx, &pkt);
        avcodec_receive_frame(pCodecCtx, frame);
        sws_scale(sws_ctx, (uint8_t const* const*)frame->data, frame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
        QImage img((uchar *)bufferRGB,width,height,QImage::Format_RGB888);
        omp_set_lock(&lock);
        image = img.copy();
        omp_unset_lock(&lock);
    }
    else return;
    av_packet_unref(&pkt);

}

void videoWidget::frameFlash(int64_t position, int64_t status){
    AVPacket pkt;
    int64_t temp = 0;
    if(position>status){
        while(temp <= position) {
            int ret = av_read_frame(pFormatCtx, &pkt);
            if(ret>=0 && pkt.stream_index==0){
                avcodec_send_packet(pCodecCtx, &pkt);
                avcodec_receive_frame(pCodecCtx, frame);
                temp = pkt.pts* av_q2d(stream->time_base)*AV_TIME_BASE;
                timestamp = temp;
            }
        }
        sws_scale(sws_ctx, (uint8_t const* const*)frame->data, frame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
        QImage img((uchar *)bufferRGB,width,height,QImage::Format_RGB888);
        omp_set_lock(&lock);
        image = img.copy();
        omp_unset_lock(&lock);
        av_packet_unref(&pkt);
    }
    else{

    }
}
