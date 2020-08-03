#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QLabel>
#include<QMovie>
#include <QPropertyAnimation>
#include<QTime>
#include <QTimer>
#include <QPoint>
#include <QPixmap>
#include <QPixmapCache>
#include <iostream>
#include <QImage>
#include<QDebug>
#include <qmath.h>
#include <QMatrix>
#include <cspline.h>
using namespace std;

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();
    CSpline *mCSpline;

    void mousePressEvent(QMouseEvent *ev);
    void paintEvent(QPaintEvent *ev);

private slots:
    //显示样条曲线
    void on_draw_pushbutton_clicked();
    //显示插值点
    void on_point_pushbutton_clicked();
    //小车运动
    void on_play_pushbutton_clicked();
    //清除画布
    void on_clear_pushbutton_clicked();
    //控制点数量变化
    void on_grain_spinbox_valueChanged();
    //tension变化
    void on_tension_doublespinbox_valueChanged();
    //控制小车角度
    void rotate();

private:
    Ui::Widget *ui;

    //画布上的点
    float px[100];
    float py[100];

    //绘制模式
    int flag;

    bool repaint; //重画基础线条
    QPixmap pix; //图像
    int n; //控制点
    int np; //总的点
    int n0; //段数
    int grain; //控制点之间的插值平滑点（即控制点之间有多少个插值点）
    int count; //插值点的总个数
    float tension; //通过控制点位置的曲线平滑程度

    //有关小车
    QLabel *obj;
    QPropertyAnimation *ani;
    QTimer *timer;
    QImage *img; //图片
    QMovie *mov; //gif
    bool is_image = true;

    float speed;
    float angle;


};

#endif // WIDGET_H
