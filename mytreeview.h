#ifndef MYTREEVIEW_H
#define MYTREEVIEW_H

#include <QTreeView>
#include <QMouseEvent>
#include <QApplication>
#include <QModelIndex>
#include <QDebug>
#include <QAbstractItemModel>
#include <QStandardItem>
#include <QDir>
#include <QDirModel>
#include <QList>
#include <QVariant>
//#include <QTreeWidgetItem>

class MyTreeView : public QTreeView
{
    Q_OBJECT
public:
    MyTreeView(QWidget *parent = nullptr) : QTreeView(parent){
        dirModel = new QDirModel;
        this->dirModel->sort(3);
        this->setModel(dirModel);
        this->setRootIndex(dirModel->index(QDir::currentPath()));
    }
    void setPath(QString dir);
    QDirModel *dirModel;
    QString path = QDir::currentPath();
    int num = 0;
protected:
    void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
private:

signals:
    void videoDoubleClick(QString);
};

#endif // MYTREEVIEW_H
