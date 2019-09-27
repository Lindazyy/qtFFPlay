#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <QPushButton>
#include <QImage>
#include <QBoxLayout>
#include <QLabel>
#include <QTime>
#include <QTimer>
#include <QDir>
#include <QFileDialog>
#include "customerslider.h"
#include "videowidget.h"
#include "omp.h"


#include <QMainWindow>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private:
    QPushButton *openFileBtn;
    QPushButton *playBtn;
    QPushButton *replayBtn;
    QPushButton *fullScrBtn;

    QImage *image;
    videoWidget *video;
    QLabel *beginTimeLabel;
    QLabel *endTimeLabel;
    QTime  *totelTime;
    QTime  *currentTime;
    CustomSlider *progressBar;
    QTimer * sliderTimer;
    QTimer * frameUpdateTimer;
    QTimer * renderTimer;
    int maxValue = 1000;
    bool play_state = false;
private slots:
    void openFile();
    void replayVideo();
    void fullScr();
    void on_pushButton_play_and_pause_clicked();

    void slider_progress_clicked();
    void slider_progress_moved();
    void slider_progress_released();
    void sliderRenew();
    void videoStop();
};

#endif // MAINWINDOW_H
