#ifndef SMOOTHSCROLLBAR_H
#define SMOOTHSCROLLBAR_H

#include <QScrollBar>
#include <QPropertyAnimation>
class SmoothScrollBar : public QScrollBar
{
    Q_OBJECT
public:
    SmoothScrollBar(QWidget *parent=nullptr);
private:
    //这里重写鼠标事件的目的是在手动点击或拖动滚动条时更新m_targetValue_v变量，并且在拖动时立即结束滚动的动画。
    //这里如果不明白作用，可以先注释掉看看手动拖动滚动条时对动画有什么影响。
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;

    QPropertyAnimation *m_scrollAni; //用来实现动画
    int m_targetValue_v; //用来记录目标位置的变量
public slots:
    void scrollTop();
    void setValue(int value); //重写的setValue槽函数，实现动画效果
    void scroll(int value); //新增相对滚动的槽函数，value为滚动距离的矢量表示
signals:
};

#endif // SMOOTHSCROLLBAR_H
