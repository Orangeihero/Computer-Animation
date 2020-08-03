#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include <QTimer>
#include <QtMath>
#include <QPixmap>
#include <qmath.h>
#include <QVector2D>
#include "shape.h"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();
    void mousePressEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent *);

private slots:

    //开始绘制
    void on_begin_pushbutton_clicked();
    //结束绘制第一个/第二个图形
    void on_end_pushbutton_clicked();
    //清空画布，重新绘制
    void on_clear_pushbutton_clicked();
    //撤回，返回上一步
    void on_undo_pushbutton_clicked();
    //开始演示
    void on_start_pushbutton_clicked();
    //停止演示
    void on_stop_pushbutton_clicked();
    //动画效果
    void animation();

private:
    Ui::Widget *ui;
    QPainter *paint;
    QPoint point;
    Shape begin_shape; //开始的图形
    Shape end_shape; //结束时的图形
    Shape process_shape; //从开始到结束过程中变化的图形
    bool isClicked; //Begin按钮是否被点击
    bool isBegin; //绘制的图形是否为开始的图形
    bool isLinear; //变化模式为线性插值或线性矢量插值
    QTimer *timer;
    int speed; //速度
    int time;
    int total_time;
    int number;
    bool isStart; //是否开始演示变化过程
    void editPoint(); //对新加入的点进行编辑
};
#endif // WIDGET_H
