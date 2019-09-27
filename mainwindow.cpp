#include "mainwindow.h"
extern omp_lock_t lock;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    //MainWindow自带layout，所以需要自定义layout并把它设置成中心layout
    //否则将会出现不能设置layout错误
    QWidget *widget = new QWidget;
    this->setCentralWidget(widget);

    //设置按钮样式和文字
    openFileBtn = new QPushButton(widget);
    openFileBtn->setText("open");
    playBtn = new QPushButton(widget);
    playBtn->setText("play");
    replayBtn = new QPushButton(widget);
    replayBtn->setText("replay");
    fullScrBtn = new QPushButton(widget);
    fullScrBtn->setText("fullscr");

    //image = new QImage();
    video = new videoWidget(widget);
    video->setGeometry(0,0,720,640);
    video->setMinimumSize(720,640);
    video->playVideo();

    totelTime = new QTime(0,0,0);
    currentTime = new QTime();
    beginTimeLabel = new QLabel(widget);
    endTimeLabel = new QLabel(widget);
    beginTimeLabel->setText("00:00:00");
    *totelTime = totelTime->addSecs((int)video->duration/1000000);
    endTimeLabel->setText(totelTime->toString("hh:mm:ss"));

    this->setGeometry(0,0,video->width,video->height);

    progressBar = new CustomSlider;
    progressBar->setOrientation(Qt::Horizontal);
    progressBar->setRange(0, maxValue);

    //设置计时器
    sliderTimer = new QTimer(this);
    sliderTimer->setInterval(100);//如果想看起来流畅些，可以把时间间隔调小，如100ms
    frameUpdateTimer = new QTimer(this);
    //frameUpdateTimer->setInterval(20);
    renderTimer = new QTimer(this);
    //renderTimer->setInterval(20);

    //水平布局，控制按钮
    QBoxLayout *ctlLayout = new QHBoxLayout();
    ctlLayout->addWidget(openFileBtn);
    ctlLayout->addWidget(playBtn);
    ctlLayout->addWidget(fullScrBtn);
    ctlLayout->addWidget(replayBtn);

    //滑条的水平布局
    QBoxLayout *sliderLayout = new QHBoxLayout;
    sliderLayout->addWidget(beginTimeLabel);
    sliderLayout->addWidget(progressBar);
    sliderLayout->addWidget(endTimeLabel);

    //垂直布局：视频播放器、进度条、控制按钮布局
    QBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(video);
    mainLayout->addLayout(sliderLayout);
    mainLayout->addLayout(ctlLayout);

    //设置布局
    widget->setLayout(mainLayout);

    //信号槽
    QObject::connect(openFileBtn,SIGNAL(clicked()),this,SLOT(openFile()));
    QObject::connect(playBtn,SIGNAL(clicked()),this,SLOT(on_pushButton_play_and_pause_clicked()));
    QObject::connect(replayBtn,SIGNAL(clicked()),this,SLOT(replayVideo()));
    QObject::connect(fullScrBtn,SIGNAL(clicked()),this,SLOT(fullScr()));

    connect(progressBar, &CustomSlider::costomSliderClicked, this, &MainWindow::slider_progress_clicked);
    connect(progressBar, &CustomSlider::sliderMoved, this, &MainWindow::slider_progress_moved);
    connect(progressBar, &CustomSlider::sliderReleased, this, &MainWindow::slider_progress_released);
    connect(sliderTimer, SIGNAL(timeout()), this, SLOT(sliderRenew()));
    connect(renderTimer, SIGNAL(timeout()), video, SLOT(update()));
    connect(video, SIGNAL(videoEnd()), this, SLOT(videoStop()));
    connect(frameUpdateTimer, SIGNAL(timeout()), video, SLOT(updateFrame()));
    sliderTimer->start();
    frameUpdateTimer->start(30);
    renderTimer->start(30);


}

MainWindow::~MainWindow()
{

}

void MainWindow::openFile()
{
    //可以多选，返回值为多选的文件List
    QString dir;
    QString srcDirPath = QFileDialog::getOpenFileName(this, "Open Document", QDir::currentPath(), "Media files (*.mp4 *.avi *.ts *.flv *.mpg *.wmv *.mkv *.mov *.vob);;All files(*.*)");
    if (!srcDirPath.isNull()) {//用户选择取消的情况
        QFileInfo fileInfo(srcDirPath);
        dir = fileInfo.path();
    }else{
        dir = QDir::currentPath();
    }

}

void MainWindow::on_pushButton_play_and_pause_clicked()
{

    //反转播放状态
    if(play_state)
    {
        frameUpdateTimer->stop();
        //renderTimer->stop();
        playBtn->setText("play");
    }
    else
    {
        //renderTimer->start();
        frameUpdateTimer->start();
        playBtn->setText("stop");
    }

    play_state = !play_state;
}

void MainWindow::replayVideo()
{
    video->stopVideo();
}

void MainWindow::fullScr()
{
    if(play_state)
    {
        //videoWidget->setFullScreen(true);
    }
    else
    {
        //videoWidget->setFullScreen(false);
    }

}

void MainWindow::slider_progress_clicked()
{
    //player->setPosition(progressBar->value()*player->duration()/maxValue);
    video->setPosition(progressBar->value()*video->duration/maxValue);

}

void MainWindow::slider_progress_moved()
{
    //暂时停止计时器，在用户拖动过程中不修改slider的值
    sliderTimer->stop();
    //player->setPosition(progressBar->value()*player->duration()/maxValue);
    video->quickFlash(progressBar->value()*video->duration/maxValue);
}

void MainWindow::slider_progress_released()
{
    //用户释放滑块后，重启定时器
    sliderTimer->start();

}

void MainWindow::sliderRenew()
{
    omp_set_lock(&lock);
    progressBar->setValue((int) (float)video->timestamp/(float)video->duration*(float)maxValue);
    omp_unset_lock(&lock);
    QTime n(0, 0, 0);                // n = 00:00:00
    *currentTime = n.addSecs((int)video->timestamp/1000000);
    beginTimeLabel->setText(currentTime->toString("hh:mm:ss"));
}

void MainWindow::videoStop(){
    frameUpdateTimer->stop();
    //video->stopVideo();
}
