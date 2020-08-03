#include "cspline.h"

CSpline::CSpline(float x[100], float y[100], int n, int grain, float tension)
{
    int i, np, n0;
    CPT jd[100];

    for(i = 1; i <= n; i++){
        jd[i].x = x[i-1];
        jd[i].y = y[i-1];
    }

    //复制首尾端点
    jd[0].x = x[0];
    jd[0].y = y[0];
    jd[n+1].x = x[n-1];
    jd[n+1].y = y[n-1];

    knots = jd;
    np = n + 2;
    n0 = n - 1;
    CubicSpline(np, knots, grain, tension);
}

//给Cardinal矩阵赋值
void CSpline::getCardinalMatrix(float a1)
{
    m[0]=-a1;   m[1]=2.-a1;  m[2]=a1-2.;   m[3]=a1;
    m[4]=2.*a1; m[5]=a1-3.;  m[6]=3.-2*a1; m[7]=-a1;
    m[8]=-a1;   m[9]=0.;     m[10]=a1;     m[11]=0.;
    m[12]=0.;   m[13]=1.;    m[14]=0.;     m[15]=0.;
}

//进行矩阵运算
float CSpline::Matrix(float a, float b, float c, float d, float u)
{
    float p0, p1, p2, p3;
    p0 = m[0]*a + m[1]*b + m[2]*c + m[3]*d;
    p1 = m[4]*a + m[5]*b + m[6]*c + m[7]*d;
    p2 = m[8]*a + m[9]*b + m[10]*c + m[11]*d;
    p3 = m[12]*a + m[13]*b + m[14]*c + m[15]*d;
    return(u*u*u*p0 + u*u*p1 + u*p2 + p3);
}

//计算插值点的坐标
void CSpline::CubicSpline(int np, CPT* knots, int grain, float tension)
{
    CPT *s, *k0, *kml, *k1, *k2;
    int i, j;
    float u[50]; //u
    getCardinalMatrix(tension); //根据tension计算出矩阵M

    //根据分段值确定u[]，绘制曲线(u[]∈[0,1])
    for(i = 0; i < grain; i++){
        u[i] = ((float) i)/grain;
    }

    s =spline;
    kml = knots;
    k0 = kml + 1;
    k1 = k0 + 1;
    k2 = k1 + 1;

    //求插值点的坐标，一共np-3段（除去开始和最后的重合段），每段grain个插值点
    for(i = 0; i < np-3; i++){
        for(j = 0; j < grain; j++){
            s->x = Matrix(kml->x, k0->x, k1->x, k2->x, u[j]);
            s->y = Matrix(kml->y, k0->y, k1->y, k2->y, u[j]);
            s++;
        }
        k0++; kml++; k1++; k2++;
    }

    //末尾的端点
    s->x = k0->x;
    s->y = k0->y;
}

CSpline::~CSpline() {
}









