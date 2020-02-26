#include "mytreeview.h"


void MyTreeView::mouseDoubleClickEvent(QMouseEvent *event){
    if(event->button() == Qt::LeftButton){
        //获取当前被点击的项的地址，并传到主窗口控件
        QModelIndex selected = this->currentIndex();
        //qDebug() << selected;
        if(!selected.isValid()){

            return;
        }
        num = selected.row();
        selected = selected.sibling(num,0);
        QString strColumn0(this->model()->itemData(selected).values()[0].toString());
        dirModel->setItemData(selected,this->model()->itemData(selected));
        QString openFile = path + "/" + strColumn0;
        emit videoDoubleClick(openFile);
    }
}

void MyTreeView::setPath(QString dir){
    path = dir;
}

