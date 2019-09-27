#-------------------------------------------------
#
# Project created by QtCreator 2019-09-24T16:21:43
#
#-------------------------------------------------

QT       += core gui
QT       += widgets
QT       += multimedia
QT       += multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = videoplayer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    customerslider.cpp \
    videowidget.cpp

HEADERS += \
        mainwindow.h \
    customerslider.h \
    videowidget.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win64:CONFIG(release, debug|release): LIBS += -L$$PWD/../../ffmpeg-20190722-3883c9d-win64-dev/ffmpeg-20190722-3883c9d-win64-dev/lib/ -lavcodec
else:win64:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../ffmpeg-20190722-3883c9d-win64-dev/ffmpeg-20190722-3883c9d-win64-dev/lib/ -lavcodec

INCLUDEPATH += $$PWD/../../ffmpeg-20190722-3883c9d-win64-dev/ffmpeg-20190722-3883c9d-win64-dev/include
DEPENDPATH += $$PWD/../../ffmpeg-20190722-3883c9d-win64-dev/ffmpeg-20190722-3883c9d-win64-dev/include

INCLUDEPATH += $$PWD/../../opencv4.1/opencv/build/include
DEPENDPATH += $$PWD/../../opencv4.1/opencv/build/include

win64:CONFIG(release, debug|release): LIBS += -L$$PWD/../../ffmpeg-20190722-3883c9d-win64-dev/ffmpeg-20190722-3883c9d-win64-dev/lib/ -lavdevice
else:win64:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../ffmpeg-20190722-3883c9d-win64-dev/ffmpeg-20190722-3883c9d-win64-dev/lib/ -lavdevice

win64:CONFIG(release, debug|release): LIBS += -L$$PWD/../../ffmpeg-20190722-3883c9d-win64-dev/ffmpeg-20190722-3883c9d-win64-dev/lib/ -lavfilter
else:win64:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../ffmpeg-20190722-3883c9d-win64-dev/ffmpeg-20190722-3883c9d-win64-dev/lib/ -lavfilter

win64:CONFIG(release, debug|release): LIBS += -L$$PWD/../../ffmpeg-20190722-3883c9d-win64-dev/ffmpeg-20190722-3883c9d-win64-dev/lib/ -lavformat
else:win64:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../ffmpeg-20190722-3883c9d-win64-dev/ffmpeg-20190722-3883c9d-win64-dev/lib/ -lavformat

win64:CONFIG(release, debug|release): LIBS += -L$$PWD/../../ffmpeg-20190722-3883c9d-win64-dev/ffmpeg-20190722-3883c9d-win64-dev/lib/ -lavutil
else:win64:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../ffmpeg-20190722-3883c9d-win64-dev/ffmpeg-20190722-3883c9d-win64-dev/lib/ -lavutil

win64:CONFIG(release, debug|release): LIBS += -L$$PWD/../../ffmpeg-20190722-3883c9d-win64-dev/ffmpeg-20190722-3883c9d-win64-dev/lib/ -lpostproc
else:win64:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../ffmpeg-20190722-3883c9d-win64-dev/ffmpeg-20190722-3883c9d-win64-dev/lib/ -lpostproc

win64:CONFIG(release, debug|release): LIBS += -L$$PWD/../../ffmpeg-20190722-3883c9d-win64-dev/ffmpeg-20190722-3883c9d-win64-dev/lib/ -lswresample
else:win64:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../ffmpeg-20190722-3883c9d-win64-dev/ffmpeg-20190722-3883c9d-win64-dev/lib/ -lswresample

win64:CONFIG(release, debug|release): LIBS += -L$$PWD/../../ffmpeg-20190722-3883c9d-win64-dev/ffmpeg-20190722-3883c9d-win64-dev/lib/ -lswscale
else:win64:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../ffmpeg-20190722-3883c9d-win64-dev/ffmpeg-20190722-3883c9d-win64-dev/lib/ -lswscale
