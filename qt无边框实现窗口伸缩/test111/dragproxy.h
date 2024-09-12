#ifndef DRAGPROXY_H
#define DRAGPROXY_H
/******************************************************************************
**FileName: 无边框可拉伸窗口代理器
**Function:
**          1:无边框可拉伸窗口代理器
*******************************************************************************/
#include <QObject>
#include <QWidget>
#include <QEvent>
#include <QRect>
#include <QPoint>
#include <QDesktopWidget>
#include <QApplication>

enum WidgetRegion
{
    Top = 0,
    TopRight,
    Right,
    RightBottom,
    Bottom,
    LeftBottom,
    Left,
    LeftTop,
    Inner,
    Unknown
};

class DragProxy : public QObject
{
    Q_OBJECT

public:
    DragProxy(QWidget *parent);
    ~DragProxy();


public:
    /******************************************************************************
    * Function: 设置可拉伸窗口四边可探测边界距离
    * InPut   : 边缘大小
    * OutPut  : 无
    * Return  : 无

    *******************************************************************************/
    void SetBorderWidth(int top, int right, int bottom, int left);

protected:
    /******************************************************************************
    * Function: 事件过滤器
    * InPut   : 无
    * OutPut  : 无
    * Return  : 无
    *******************************************************************************/
    virtual bool eventFilter(QObject* obj, QEvent* event);

    /******************************************************************************
    * Function: 设置可拉伸窗口边缘范围
    * InPut   : 无
    * OutPut  : 无
    * Return  : 无
    *******************************************************************************/
    void MakeRegions();
    WidgetRegion HitTest(const QPoint& pos);
    void UpdateGeometry(int x, int y, int w, int h);

    /******************************************************************************
    * Function: 开始/结束计时器
    * InPut   : 无
    * OutPut  : 无
    * Return  : 无
    * Other   : 鼠标从边框快速移到窗体内子控件上，可能会造成鼠标样式未改变，这里使用计时器监控
    *******************************************************************************/
    void StartCursorTimer();
    void StopCursorTimer();

private:
    QWidget*            m_proxyWidget;// 代理的窗体
    int                 m_top, m_right, m_bottom, m_left;// 四周宽度
    QRect               m_regions[9];// 九宫格，对应9个区域

    QPoint              m_originPosGlobal;// 拖拽前鼠标位置
    QRect               m_originGeo;// 拖拽前窗体位置和大小
    QRect               m_availableWindRect;//可视化桌面

    bool                m_mousePressed;	// 鼠标是否按下
    WidgetRegion        m_regionPressed;// 记录鼠标按下时所点击的区域

    int m_cursorTimerId;

};



#endif // DRAGPROXY_H
