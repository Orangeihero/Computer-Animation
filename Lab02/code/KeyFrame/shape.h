#ifndef SHAPE_H
#define SHAPE_H

#include<QVector>
#include<QPoint>

class Shape
{
public:
    QVector<QPoint> point;
    Shape();
    void addPoint(QPoint point); //添加点
    void undo(); //撤回点的绘制
    void clear(); //清空
};

#endif // SHAPE_H
