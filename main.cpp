#include "mainwindow.h"
#include <QApplication>

omp_lock_t lock;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    omp_init_lock(&lock);
    MainWindow w;
    w.show();
    a.exec();
    omp_destroy_lock(&lock);
    return 0;
}
