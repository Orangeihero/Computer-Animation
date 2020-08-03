#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    n = 0; //初始控制点为0
    np = 2;
    n0 = -1;
    flag = 0; //初始绘制模式
    repaint = false;
    pix = QPixmap(780, 330);
    pix.fill(Qt::white); //将画布填充为白色

    //信号和槽通过connect建立连接
    timer = new QTimer;
    connect(timer, SIGNAL(timeout()), this, SLOT(rotate())); //当达到超过时间，则发射信号，执行指定的槽函数
    connect(ui->draw_pushbutton,&QPushButton::clicked,this,&Widget::on_draw_pushbutton_clicked);
    connect(ui->clear_pushbutton,&QPushButton::clicked,this,&Widget::on_play_pushbutton_clicked);
    connect(ui->point_pushbutton,&QPushButton::clicked,this,&Widget::on_point_pushbutton_clicked);
    connect(ui->play_pushbutton,&QPushButton::clicked,this,&Widget::on_play_pushbutton_clicked);

    //设置按钮状态
    ui->draw_pushbutton->setEnabled(false);
    ui->clear_pushbutton->setEnabled(true);
    ui->play_pushbutton->setEnabled(false);
    ui->point_pushbutton->setEnabled(false);
    ui->play_pushbutton->setText("Play");

    //设置obj控件来承载图片
    obj = new QLabel(this);
    obj->setFixedSize(100,100);
    obj->hide(); //隐藏小车的label

    //图片
    QImage * ori_img= new QImage(":/image/car.png");
    img = new QImage();
    if(img == nullptr) printf("Image is null! \n");
    *img= ori_img->scaled(75,60,Qt::IgnoreAspectRatio);

    //gif
    mov= new QMovie();
    mov->setFileName(":/image/car.gif");
    if(mov == NULL) printf("Gif is null! \n");
    mov->setScaledSize((QSize(75,50)));
    mov->start();

    this->setWindowTitle("Cardinal Spline Demo");
}

void Widget::mousePressEvent(QMouseEvent *ev)
{
    if((ev->pos().x() <size().width() )&& (ev->pos().x() >0 )
            && (ev->pos().y() < (size().height()-190)) &&  (ev->pos().y() >0))
    {
        px[n] = ev->pos().x();
        py[n] = ev->pos().y();
        n++;
        np++;
        n0++;
    }
    if(n >= 2) ui->draw_pushbutton->setEnabled(true); //激活绘制曲线按钮
    update();
}


void Widget::paintEvent(QPaintEvent *ev)
{
    QPainter paint(&pix);
    QPainter painter(this);

    paint.setRenderHint(QPainter::Antialiasing); //反走样，抗锯齿

    //如果需要重新绘制控制点及其连线
    if(repaint == true){
        if(n ==1){ //如果只有一个点，则不需要连线
            paint.setPen(Qt::black); //画点的颜色为黑色
            paint.drawEllipse(px[0], py[0], 2, 2);
        }
        else{
            for(int i = 0; i < n-1; i++){
                paint.setPen(QPen(QColor(30,200,250), 1, Qt::SolidLine, Qt::RoundCap)); //画直线的颜色为蓝色
                paint.drawLine(px[i],py[i],px[i+1],py[i+1]);
                paint.setPen(Qt::black); //画点的颜色为黑色
                paint.drawEllipse(px[i], py[i], 2, 2);
            }
            paint.drawEllipse(px[n-1], py[n-1], 2, 2);
        }
    }

    //设置控制点及其连线
    if(flag == 0 && n != 0){
        if(n == 1){
            paint.setPen(QPen(QColor(30,200,250), 1, Qt::SolidLine, Qt::RoundCap)); //画直线的颜色为蓝色
            paint.drawLine(px[0], py[0], px[0], py[0]);
            paint.setPen(Qt::black); //画点的颜色为黑色
            paint.drawEllipse(px[0], py[0], 2, 2);
        }
        else{
            paint.setPen(QPen(QColor(30,200,250), 1, Qt::SolidLine, Qt::RoundCap)); //画直线的颜色为蓝色
            paint.drawLine(px[n-2], py[n-2], px[n-1], py[n-1]);
            paint.setPen(Qt::black); //画点的颜色为黑色
            paint.drawEllipse(px[n-1], py[n-1], 2, 2);
        }
    }
    else if(flag == 1){ //绘制曲线
        for(int j = 0; j < n0 * grain; j++){
            paint.setPen(QPen(QColor(200, 50, 200), 1, Qt::SolidLine, Qt::RoundCap)); //画Cardinal曲线的颜色为紫色
            paint.drawLine(mCSpline->spline[j].x, mCSpline->spline[j].y, mCSpline->spline[j+1].x, mCSpline->spline[j+1].y);
        }
    }
    else if(flag == 2){ //显示插值点
        for(int j = 0; j < n0 * grain; j++){
            paint.setPen(QPen(QColor(30, 200, 30), 1, Qt::SolidLine, Qt::RoundCap)); //画插值点的颜色为绿色
            paint.drawEllipse(mCSpline->spline[j].x, mCSpline->spline[j].y, 2, 2);
        }
    }

    painter.drawPixmap(0, 0, pix);
}

void Widget::on_grain_spinbox_valueChanged()
{
    pix.fill(Qt::white);
    repaint = true;
    ui->draw_pushbutton->clicked(true);
}

void Widget::on_tension_doublespinbox_valueChanged()
{
    pix.fill(Qt::white);
    repaint = true;
    ui->draw_pushbutton->clicked(true);
}

void Widget::on_draw_pushbutton_clicked()
{
    flag = 1; //绘制曲线模式
    grain = ui->grain_spinbox->value();
    tension = ui->tension_doublespinbox->value();
    if(n > 1){

        //生成Cardinal曲线
        mCSpline = new CSpline(px, py, n, grain, tension);

        //激活显示插值点按钮和小车运动按钮
        ui->point_pushbutton->setEnabled(true);
        ui->play_pushbutton->setEnabled(true);

        update();
    }
}

void Widget::on_clear_pushbutton_clicked()
{
    flag = 0;
    n = 0;
    np = 2;
    n0 = -1;
    grain = 0;
    tension = 0;
    repaint = false;

    pix.fill(Qt::white);
    ui->point_pushbutton->setEnabled(false);
    ui->play_pushbutton->setEnabled(false);

    ui->speed_slider->setValue(7500);
    ui->grain_spinbox->setValue(5);
    ui->tension_doublespinbox->setValue(0);
    obj->hide(); //隐藏小车的label

    update();
}

void Widget::on_point_pushbutton_clicked()
{
    flag = 2; //显示插值点模式
    update();
}

void Widget::on_play_pushbutton_clicked()
{
    flag = 3; // 小车运动模式
    grain = ui->grain_spinbox->value();
    tension = ui->tension_doublespinbox->value();
    speed = ui->speed_slider->value();

    if(ui->image_radiobutton->isChecked() == true){ //选择image运动
        QPixmapCache::clear();
        obj->setPixmap(QPixmap::fromImage(*img));
        is_image = true;
    }
    else if(ui->gif_radiobutton->isChecked() == true){ //选择gif运动
        QPixmapCache::clear();
        obj->setMovie(mov);
        is_image = false;
    }

    //每过一定的时长调整小车的角度
    count = n0 * grain;
    timer->setInterval((15000 - speed)/ count);
    double u = 1.0f / count;

    //设置动画时间，初始帧，关键帧，结束帧
    ani = new QPropertyAnimation(obj , "pos");
    ani->setDuration(15000 - speed); //动画时间
    ani->setStartValue(QPoint(mCSpline->spline[0].x-40, mCSpline->spline[0].y - 75)); //初始帧
    for(int i = 1; i < count; i++){ //关键帧
        ani->setKeyValueAt(u*i, QPoint(mCSpline->spline[i].x-40, mCSpline->spline[i].y - 75));
    }
    ani->setEndValue(QPoint(mCSpline->spline[count].x-40, mCSpline->spline[count].y - 75)); //结束帧

    timer->start();
    update();
}

void Widget::rotate()
{
    int t = ani->currentTime();

    if(t == 0) ani->start();
    if(t > 0){
        if(is_image == true){//image，可以改变角度
            int i = round(t/(15000-speed) * count);

            // 计算斜率和角度
            if(i == 0){
                angle = atan2(tension*(2*mCSpline->spline[1].y-mCSpline->spline[0].y-mCSpline->spline[2].y),(2*mCSpline->spline[1].x-mCSpline->spline[0].x-mCSpline->spline[2].x))/ M_PI * 180;
            }
            else if(i == count){
                angle = atan2(tension*(2*mCSpline->spline[n-1].y-mCSpline->spline[n].y-mCSpline->spline[n-2].y),(2*mCSpline->spline[n-1].x-mCSpline->spline[n].x-mCSpline->spline[n-2].x))/ M_PI * 180;
            }
            else angle = atan2(tension*(mCSpline->spline[i+1].y-mCSpline->spline[i-1].y),(mCSpline->spline[i+1].x-mCSpline->spline[i-1].x))/ M_PI * 180;

            QMatrix matrix;
            matrix.rotate(angle);
            QPixmapCache::clear();
            obj->setPixmap(QPixmap::fromImage(*img).transformed(matrix, Qt::SmoothTransformation));

        }
        else if(is_image == false) obj->setMovie(mov); //gif，无法改变角度
        obj->show();
    }
    if(t >= 15000 - speed - 200){
        if(is_image == true)
            obj->setPixmap(QPixmap::fromImage(*img));
        else if(is_image == false) obj->setMovie(mov);
        timer->stop();
        ani->stop();
    }
}

Widget::~Widget()
{
    delete ui;
}
