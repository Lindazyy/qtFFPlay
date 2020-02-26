#ifndef CACHEDFRAMESEQUENCE_H
#define CACHEDFRAMESEQUENCE_H
#include <QImage>


class cachedFrameSequence
{
public:
    QImage      *front;
    QImage      *back;
    cachedFrameSequence();
    ~cachedFrameSequence();
    void pop();
    void push(QImage*);
    int length();
};

#endif // CACHEDFRAMESEQUENCE_H
