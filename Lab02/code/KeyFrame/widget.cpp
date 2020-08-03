#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    //设置画布
    paint = new QPainter;
    paint->begin(this);
    paint->drawRect(0, 0, 650, 540); //画布大小

    //设置按钮状态
    this->ui->end_pushbutton->setText("End1");
    this->ui->undo_pushbutton->setDisabled("true");
    this->ui->end_pushbutton->setDisabled("true");
    this->ui->stop_pushbutton->setDisabled("true");
    this->ui->start_pushbutton->setDisabled("true");
    isClicked = false; //Begin按钮是否被点击，初始设置为否，当被点击时为是
    this->isBegin = true; //设置初始绘制为开始的图形

    this->time = 0;
    timer = new QTimer(this);
    //信号和槽通过connect建立连接
    connect(timer,SIGNAL(timeout()),this,SLOT(animation())); //当达到超过时间，则发射信号，执行指定的槽函数
    this->isLinear = true; //设置初始变化模式为线性插值
    isStart = false; //设置初始播放状态为不播放
}

Widget::~Widget()
{
    delete ui;
}


void Widget::mousePressEvent(QMouseEvent *e)
{
    if(isClicked == true){
        this->point = e->pos();
        this->editPoint();
        update();
    }
}

void Widget::paintEvent(QPaintEvent *)
{

    paint = new QPainter;
    paint->begin(this);

    //绘制开始图形
    for(int i = 0; i < this->begin_shape.point.size(); i++){
        paint->setPen(QPen(QColor(240,100,100),2));
        paint->drawEllipse(this->begin_shape.point[i],2,2);
        if(i>=1){
            paint->setPen(QPen(QColor(50,50,180),2));
            paint->drawLine(this->begin_shape.point[i-1],this->begin_shape.point[i]);
        }
    }

    //绘制结束图形
    for(int i = 0; i < this->end_shape.point.size(); i++){
        paint->setPen(QPen(QColor(240,100,100),2));
        paint->drawEllipse(this->end_shape.point[i],2,2);
        if(i>=1){
            paint->setPen(QPen(QColor(50,50,180),2));
            paint->drawLine(this->end_shape.point[i-1],this->end_shape.point[i]);
        }
    }

    //绘制过程中的图形
    for(int i = 0; i < this->process_shape.point.size(); i++){
        paint->setPen(QPen(QColor(0,255,255),2));
        paint->drawEllipse(this->process_shape.point[i],2,2);
        if(i>=1){
            paint->setPen(QPen(QColor(100,200,20),2));
            paint->drawLine(this->process_shape.point[i-1],this->process_shape.point[i]);
        }
    }

    paint->~QPainter();

}

void Widget::editPoint()
{
    if(this->point.x() > 0 && this->point.x() < 650 && this->point.y() > 0 && this->point.y() < 540){
        if(this->isBegin == true) //绘制的是开始的图形
            this->begin_shape.addPoint(this->point);
        if(this->isBegin == false) //绘制的是结束的图形
            this->end_shape.addPoint(this->point);
    }
}

void Widget::on_begin_pushbutton_clicked()
{
    isClicked = true;
    //设置按钮状态
    this->ui->end_pushbutton->setEnabled("true");
    this->ui->undo_pushbutton->setEnabled("true");
}

void Widget::on_undo_pushbutton_clicked()
{
    //判断此步撤销的是开始图形还是结束图形
    if(this->isBegin == true) this->begin_shape.undo();
    else this->end_shape.undo();
    update();
}

void Widget::on_end_pushbutton_clicked()
{
    if(this->begin_shape.point.isEmpty()){ //开始的图形尚未绘制
        this->isBegin = true;
        this->ui->undo_pushbutton->setEnabled("true");
        this->ui->begin_pushbutton->setDisabled("true");
        this->ui->end_pushbutton->setText("End1");
    }
    else if(!this->begin_shape.point.isEmpty()){ //开始的图形绘制完毕
        this->isBegin = false;
        if(this->end_shape.point.isEmpty()){ //结束的图形尚未绘制
            //设置按钮状态
            this->ui->undo_pushbutton->setEnabled("true");
            this->ui->begin_pushbutton->setDisabled("true");
            this->ui->end_pushbutton->setText("End2");
        }
        if(!this->end_shape.point.isEmpty()){ //结束的图形绘制完毕
            //设置按钮状态
            this->ui->undo_pushbutton->setDisabled("true");
            this->ui->begin_pushbutton->setDisabled("true");
            this->ui->stop_pushbutton->setEnabled("true");
            this->ui->start_pushbutton->setEnabled("true");
            this->ui->end_pushbutton->setText("Done");
            isClicked = false;
        }
    }
}


void Widget::on_clear_pushbutton_clicked()
{
    //设置画布
    paint = new QPainter;
    paint->begin(this);
    paint->drawRect(0, 0, 650, 540); //画布大小

    this->time = 0;
    this->total_time = 0;
    this->number = 0;
    this->speed = 0;

    //清空已有的图形
    this->begin_shape.clear();
    this->process_shape.clear();
    this->end_shape.clear();
    this->isBegin = true;


    //设置按钮状态
    isLinear = true;
    this->ui->end_pushbutton->setText("End1");
    this->ui->begin_pushbutton->setEnabled("true");
    this->ui->undo_pushbutton->setDisabled("true");
    this->ui->end_pushbutton->setDisabled("true");
    this->ui->stop_pushbutton->setDisabled("true");
    this->ui->start_pushbutton->setDisabled("true");
    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(animation())); //当达到超过时间，则发射信号，执行指定的槽函数
    isStart = false;
    isClicked = false;

    update();
}

void Widget::on_start_pushbutton_clicked()
{
    //判断动画是否已经开始：未开始，绘制动画
    if(isStart == false){
        if(this->ui->linear_radiobutton->isChecked()){
            isLinear = true;
        }
        if(this->ui->vector_radiobutton->isChecked()){
            isLinear = false;
        }

        this->speed = this->ui->speed_box->currentIndex(); //速度模式
        this->total_time = this->ui->time_box->value();
        this->number = this->total_time * 50;
        timer->start(5);
        isStart = true;
    }

    //判断动画是否已经开始：已经开始，则重新绘制动画
    else{
        isStart = false;
        this->time = 0;
        if(timer->isActive()) timer->stop();
        this->process_shape.clear();
        update();
    }
}

void Widget::on_stop_pushbutton_clicked()
{
    isStart = false;
    if(timer->isActive()) timer->stop();
}


void Widget::animation()
{
    float pi = 3.14; //Π
    if(isLinear == true){ //线性插值绘制模式
        this->process_shape.clear();
        float v = (float)this->time / this->number;
        if(this->speed == 1) v = 1 - qCos(pi/2*v); //加速
        if(this->speed == 2) v = qSin(pi/2*v); //减速
        for(int i=0; i < this->begin_shape.point.size(); i++){
            this->process_shape.addPoint((1-v)*this->begin_shape.point[i] + v*this->end_shape.point[i]);
        }
    }
    else{ //矢量线性插值绘制模式
        this->process_shape.clear();
        float v = (float)this->time / this->number;
        if(this->speed == 1) v = 1 - qCos(pi/2*v); //加速
        if(this->speed == 2) v = qSin(pi/2*v); //减速
        this->process_shape.addPoint((1-v)*this->begin_shape.point[0] + v*this->end_shape.point[0]); //第一个点用线性插值算法
        for(int i=1; i<this->begin_shape.point.size(); i++){

            //开始图形的向量
            QVector2D d0 = QVector2D(this->begin_shape.point[i]-this->begin_shape.point[i-1]);
            float r0 = d0.length();
            float a0 = qAtan(d0.y()/d0.x());
            if(d0.x()<0) a0 += pi;

            //结束图形的向量
            QVector2D d1 = QVector2D(this->end_shape.point[i]-this->end_shape.point[i-1]);
            float r1 = d1.length();
            float a1 = qAtan(d1.y()/d1.x());
            if(d1.x()<0) a1 += pi;

            //过程向量
            float ri = (1-v)*r0 + v*r1;
            float ai = (1-v)*a0 + v*a1;

            //当两个向量的夹角中有一个大于180°时，选择小于180°的方向进行旋转
            if((a0 <= 0) && (a1 >= a0 + pi)) ai = (1-v)*a0 + v*a1 - 2*pi*v;
            if((0 <= a0) && (a0 <= ai/2)  && (a0 + pi <= a1 )) ai = (1-v)*a0 + v*a1 - 2*pi*v;
            if((pi/2 <= a0) && (a0 <= pi) && (a1 <= a0 - pi)) ai = (1-v)*a0 + v*a1 + 2*pi*v;
            if((pi <= a0) && (a0 <= 3*pi/2) && (a1 <= a0 - pi)) ai = (1-v)*a0 + v*a1 + 2*pi*v;

            QVector2D di = QVector2D(ri*qCos(ai),ri*qSin(ai));
            this->process_shape.addPoint(this->process_shape.point[i-1]+di.toPoint());
        }
    }

    this->time++;
    if(this->time > this->number){ //时间结束时停止播放变化过程
        this->time = 0;
        timer->stop();
    }
    update();
}




























