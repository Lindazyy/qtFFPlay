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
    video->setGeometry(0,0,720,480);
    video->setMinimumSize(720,480);
    video->installEventFilter(this);
    QPalette palette;
    palette.setBrush(QPalette::Active, QPalette::Window, QBrush(Qt::black));
    video->setPalette(palette);

    playerTree = new MyTreeView();
    playerTree->setMinimumWidth(200);
    playerTree->setMaximumWidth(400);

    totelTime = new QTime(0,0,0);
    currentTime = new QTime(0,0,0);
    beginTimeLabel = new QLabel(widget);
    endTimeLabel = new QLabel(widget);
    beginTimeLabel->setText("00:00:00");
    endTimeLabel->setText("00:00:00");
    beginTimeLabel->setFixedHeight(15);
    endTimeLabel->setFixedHeight(15);
    //this->setGeometry(0,0,video->width,video->height);

    progressBar = new CustomSlider(widget);
    progressBar->setOrientation(Qt::Horizontal);
    progressBar->setRange(0, maxValue);

    //设置计时器
    sliderTimer = new QTimer(this);
    sliderTimer->setInterval(100);//如果想看起来流畅些，可以把时间间隔调小，如100ms
    frameUpdateTimer = new QTimer(this);
    renderTimer = new QTimer(this);
    listTimer = new QTimer(this);

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

    //水平布局：播放列表和视频播放器
    QBoxLayout *Layout = new QHBoxLayout();
    Layout->addWidget(playerTree);
    Layout->addLayout(mainLayout);

    //设置布局
    widget->setLayout(Layout);

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
    connect(playerTree, &MyTreeView::videoDoubleClick, this, &MainWindow::playClickedVideo);
    connect(listTimer, SIGNAL(timeout()), this, SLOT(listRenew()));
    connect(mPlayBack, &QAction::triggered, [=](){video->mode = 2;});
    connect(mBlackSCN, &QAction::triggered, [=](){video->mode = 1;});
    connect(mLastFrame, &QAction::triggered, [=](){video->mode = 0;});
    connect(video, &videoWidget::blackEnd, this, &MainWindow::changeFPS);
    sliderTimer->start();
    renderTimer->start(30);


}

MainWindow::~MainWindow()
{

}

void MainWindow::openFile()
{
    //可以多选，返回值为多选的文件List
    QString srcDirPath, dir;
    frameUpdateTimer->stop();
    if(path.isEmpty())
        srcDirPath = QFileDialog::getOpenFileName(this, "Open Document", QDir::currentPath(), "Media files (*.mp4 *.avi *.ts *.flv *.mpg *.wmv *.mkv *.mov *.vob);;All files(*.*)");
    else srcDirPath = QFileDialog::getOpenFileName(this, "Open Document", path, "Media files (*.mp4 *.avi *.ts *.flv *.mpg *.wmv *.mkv *.mov *.vob);;All files(*.*)");
    playClickedVideo(srcDirPath);
    omp_set_lock(&lock);
    playerTree->setRootIndex(playerTree->dirModel->index(path));
    playerTree->path = path;
    omp_unset_lock(&lock);
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
    frameUpdateTimer->start();
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
                video->setCursor(Qt::ArrowCursor);
            }
        }
    }
    return QWidget::eventFilter(obj, ev);
}

void MainWindow::playClickedVideo(QString dir){
    video->setCursor(Qt::BlankCursor);
    video->blackcnt = 0;
    if (!dir.isNull()) {
        if(!video->path.isEmpty()){
            video->stopVideo();
        }
        video->path = dir;
        video->playVideo();
        play_state = true;
        playBtn->setText("stop");
        QTime time(0,0,0);
        *totelTime = time.addSecs((int)video->duration/1000000);
        endTimeLabel->setText(totelTime->toString("hh:mm:ss"));
        frameUpdateTimer->start(30);
        QFileInfo fileInfo(dir);
        path = fileInfo.path();
    }else{//用户选择取消的情况
        if(!video->path.isEmpty()){
            playBtn->setText("stop");
            play_state = true;
            frameUpdateTimer->start(30);
        }
    }
    listTimer->start(2000);
    video->setFullScreen(true);
}


void MainWindow::listRenew()
{
    QString dir = playerTree->path;
    omp_set_lock(&lock);
    delete playerTree->dirModel;
    playerTree->dirModel = new QDirModel;
    //按时间排序
    playerTree->dirModel->sort(3);
    playerTree->setModel(playerTree->dirModel);
    playerTree->setRootIndex(playerTree->dirModel->index(dir));
    playerTree->setCurrentIndex(playerTree->dirModel->index(playerTree->num,0,playerTree->rootIndex()));
    omp_unset_lock(&lock);
}

void MainWindow::changeFPS(int fps){
    frameUpdateTimer->stop();
    frameUpdateTimer->setInterval((int)(1000.0/(float)fps));
    frameUpdateTimer->start();
}
