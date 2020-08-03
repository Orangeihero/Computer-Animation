#ifndef CSPLINE_H
#define CSPLINE_H

struct CPT
{
    float x;
    float y;
};

class CSpline
{
public:
    // x[],y[]为控制点的坐标，n为控制点的数量
    // grain为控制点之间的插值平滑点（即控制点之间有多少个插值点），tension为通过控制点位置的曲线平滑程度
    CSpline(float x[100], float y[100], int n, int grain, float tension); //生成插值点
    ~CSpline();
    CPT spline[2056]; //插值点数组

private:
    void CubicSpline(int np, CPT* konts, int grain, float tension); //计算插值点的坐标
    void getCardinalMatrix(float a1); //给Cardinal矩阵赋值
    float Matrix(float a, float b, float c, float d, float u); //进行矩阵运算
    CPT* knots;  // 控制点数组
    float m[16]; //Cardinal矩阵
};

#endif // CSPLINE_H
