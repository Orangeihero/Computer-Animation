#include "shape.h"

Shape::Shape()
{

}

//添加点
void Shape::addPoint(QPoint point)
{
    this->point.append(point);
}

//撤回点的绘制
void Shape::undo()
{
    this->point.pop_back();
}

//清空
void Shape::clear()
{
    this->point.clear();
}
