#ifndef BEZIERWIDGET_H
#define BEZIERWIDGET_H

#include <QWidget>
#include <QVector>
#include <QPointF>

class BezierWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BezierWidget(QWidget *parent = nullptr);

    void clearPoints();
    void resetExample();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    int pointAt(const QPointF &position) const;
    QPainterPath makeCurve() const;

    QVector<QPointF> m_points;
    int m_draggedPoint = -1;
    static constexpr qreal PointRadius = 7.0;
};

#endif // BEZIERWIDGET_H
