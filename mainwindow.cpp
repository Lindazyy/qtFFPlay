#include "mainwindow.h"
extern omp_lock_t lock;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    //MainWindow自带layout，所以需要自定义layout并把它设置成中心layout
    //否则将会出现不能设置layout错误
    widget = new QWidget;
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
    video->setMinimumSize(240,180);

    totelTime = new QTime(0,0,0);
    currentTime = new QTime(0,0,0);
    beginTimeLabel = new QLabel(widget);
    endTimeLabel = new QLabel(widget);
    beginTimeLabel->setText("00:00:00");
    endTimeLabel->setText("00:00:00");

    //this->setGeometry(0,0,video->width,video->height);

    progressBar = new CustomSlider(widget);
    progressBar->setOrientation(Qt::Horizontal);
    progressBar->setRange(0, maxValue);

    //设置计时器
    sliderTimer = new QTimer(this);
    sliderTimer->setInterval(100);//如果想看起来流畅些，可以把时间间隔调小，如100ms
    frameUpdateTimer = new QTimer(this);
    renderTimer = new QTimer(this);

    //水平布局，控制按钮
    ctlLayout = new QHBoxLayout;
    ctlLayout->addWidget(openFileBtn);
    ctlLayout->addWidget(playBtn);
    ctlLayout->addWidget(fullScrBtn);
    ctlLayout->addWidget(replayBtn);

    //滑条的水平布局
    sliderLayout = new QHBoxLayout;
    sliderLayout->addWidget(beginTimeLabel);
    sliderLayout->addWidget(progressBar);
    sliderLayout->addWidget(endTimeLabel);

    //垂直布局：视频播放器、进度条、控制按钮布局
    mainLayout = new QVBoxLayout;
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
    renderTimer->start(30);


}

MainWindow::~MainWindow()
{

}

void MainWindow::openFile()
{
    //可以多选，返回值为多选的文件List
    QString srcDirPath;
    frameUpdateTimer->stop();
    if(path.isEmpty())
        srcDirPath = QFileDialog::getOpenFileName(this, "Open Document", QDir::currentPath(), "Media files (*.mp4 *.avi *.ts *.flv *.mpg *.wmv *.mkv *.mov *.vob);;All files(*.*)");
    else srcDirPath = QFileDialog::getOpenFileName(this, "Open Document", path, "Media files (*.mp4 *.avi *.ts *.flv *.mpg *.wmv *.mkv *.mov *.vob);;All files(*.*)");
    if (!srcDirPath.isNull()) {
        if(!video->path.isEmpty()){
            video->stopVideo();
        }
        video->path = srcDirPath;
        video->playVideo();
        video->setGeometry(0,0,video->width,video->height);
        //widget->setGeometry(0,0,video->width,video->height);
        play_state = true;
        playBtn->setText("stop");
        QTime time(0,0,0);
        *totelTime = time.addSecs((int)video->duration/1000000);
        endTimeLabel->setText(totelTime->toString("hh:mm:ss"));
        frameUpdateTimer->start(30);
        QFileInfo fileInfo(srcDirPath);
        path = fileInfo.path();
    }else{//用户选择取消的情况
        if(!video->path.isEmpty()){
            playBtn->setText("stop");
            play_state = true;
            frameUpdateTimer->start(30);
        }
    }
}

void MainWindow::on_pushButton_play_and_pause_clicked()
{

    //反转播放状态
    if(play_state)
    {
        frameUpdateTimer->stop();
        playBtn->setText("play");
    }
    else
    {
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
        video->setFullScreen(true);
    }
    else
    {
        video->setFullScreen(false);
    }

}

void MainWindow::slider_progress_clicked()
{
    video->setPosition(progressBar->value()*video->duration/maxValue);

}

void MainWindow::slider_progress_moved()
{
    //暂时停止计时器，在用户拖动过程中不修改slider的值
    sliderTimer->stop();
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
}

bool MainWindow::eventFilter(QObject *obj, QEvent *ev)
{
    if(ev->type() == QEvent::KeyPress){
        QKeyEvent *event = (QKeyEvent *)ev;
        if(obj == video){
            if(event->key() == Qt::Key_Escape){
                video->setFullScreen(false);
            }
        }
    }
    return QWidget::eventFilter(obj, ev);
}
