#include "videowidget.h"
extern omp_lock_t lock;



videoWidget::videoWidget(QWidget *parent) : QVideoWidget(parent)
{
    avformat_network_init();
}

void videoWidget::paintEvent(QPaintEvent *e)
{
    QRect rec = this->geometry();
    width = rec.width(); height = rec.height();
    QPainter pp(this);
    QRect Temp(0,0, width, height);
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

    v_w = pCodecCtx->width; v_h = pCodecCtx->height;
    this->setGeometry(0, 0, v_w, v_h);

    sws_ctx = sws_getContext(v_w, v_h, pCodecCtx->pix_fmt, v_w, v_h, AV_PIX_FMT_RGB24, SWS_BICUBIC, nullptr, nullptr, nullptr);

    pFrameRGB = av_frame_alloc();
    frame = av_frame_alloc();
    int rgbBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, v_w, v_h, 1);
    bufferRGB = (uint8_t*)av_malloc(rgbBytes * sizeof(uint8_t));
    av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, bufferRGB, AV_PIX_FMT_RGB24, v_w, v_h, 1);
    stream=pFormatCtx->streams[videoStream];
    fps = stream->avg_frame_rate.num/stream->avg_frame_rate.den;
    std::cout << fps << std::endl;
}

cv::Mat videoWidget::avframe_to_cvmat(AVFrame *frame){
    AVFrame dst;
    cv::Mat m = cv::Mat(v_h, v_w, CV_8UC3);
    memset(&dst, 0, sizeof(dst));
    dst.data[0] = (uint8_t *)m.data;
    avpicture_fill((AVPicture *)&dst, dst.data[0], AV_PIX_FMT_BGR24, v_w, v_h);
    sws_scale(sws_ctx, frame->data, frame->linesize, 0, v_h, dst.data, dst.linesize);
    return m;
}

cv::Mat videoWidget::findBestSize(cv::Mat mat, int w, int h){
    cv::Mat fin;
    float tmp_w = (float)height / (float)mat.rows * (float)mat.cols;
    float tmp_h = (float)width / (float)mat.cols * (float)mat.rows;

    if (tmp_w <= width) cv::resize(mat, fin, cv::Size((int)tmp_w, height), 0, 0, cv::INTER_NEAREST);
    else cv::resize(mat, fin, cv::Size(width, (int)tmp_h), 0, 0, cv::INTER_NEAREST);
    cv::Mat tmppic = cv::Mat::zeros(h,w,CV_8UC3);
    cv::Mat imageROI = tmppic(cv::Rect((w - fin.cols) / 2, (height - fin.rows) / 2, fin.cols, fin.rows));				//top
    fin.copyTo(imageROI);

    return tmppic;
}

void videoWidget::setBlack(){
    cv::Mat mat = cv::Mat(cv::Size(width, height), CV_8UC3, cv::Scalar(0,0,0));
    const uchar *pSrc = (const uchar*)mat.data;
    QImage img(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
    omp_set_lock(&lock);
    image = img.copy();
    omp_unset_lock(&lock);
}

void videoWidget::updateFrame(){
    while(blackcnt!=30){
        setBlack(); blackcnt++;
        return;
    }
    emit blackEnd(fps);
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
        //sws_scale(sws_ctx, (uint8_t const* const*)frame->data, frame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
        //QImage img((uchar *)bufferRGB,width,height,QImage::Format_RGB888);

        cv::Mat m = avframe_to_cvmat(frame);
        cv::Mat last = findBestSize(m, width, height);
        const uchar *pSrc = (const uchar*)last.data;
        QImage img(pSrc, last.cols, last.rows, last.step, QImage::Format_RGB888);

        omp_set_lock(&lock);
        image = img.copy();
        omp_unset_lock(&lock);
    }else{
        if(mode==2){
            setPosition(0);
            return;
        }
        else if(mode==1){
            blackcnt = 0;
            while(blackcnt!=30){
                setBlack(); blackcnt++;
                return;
            }
            blackcnt = 0;
            emit videoEnd();
            return;
        }
        emit videoEnd();
        blackcnt = 0;
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

    //sws_scale(sws_ctx, (uint8_t const* const*)frame->data, frame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
    //QImage img((uchar *)bufferRGB,width,height,QImage::Format_RGB888);
    cv::Mat m = avframe_to_cvmat(frame);
    cv::Mat last = findBestSize(m, width, height);
    const uchar *pSrc = (const uchar*)last.data;
    QImage img(pSrc, last.cols, last.rows, last.step, QImage::Format_RGB888);
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
        //sws_scale(sws_ctx, (uint8_t const* const*)frame->data, frame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
        //QImage img((uchar *)bufferRGB,width,height,QImage::Format_RGB888);
        cv::Mat m = avframe_to_cvmat(frame);
        cv::Mat last = findBestSize(m, width, height);
        const uchar *pSrc = (const uchar*)last.data;
        QImage img(pSrc, last.cols, last.rows, last.step, QImage::Format_RGB888);
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
        //sws_scale(sws_ctx, (uint8_t const* const*)frame->data, frame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
        //QImage img((uchar *)bufferRGB,width,height,QImage::Format_RGB888);
        cv::Mat m = avframe_to_cvmat(frame);
        cv::Mat last = findBestSize(m, width, height);
        const uchar *pSrc = (const uchar*)last.data;
        QImage img(pSrc, last.cols, last.rows, last.step, QImage::Format_RGB888);
        omp_set_lock(&lock);
        image = img.copy();
        omp_unset_lock(&lock);
        av_packet_unref(&pkt);
    }
    else{

    }
}
