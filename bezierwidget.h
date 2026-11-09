#ifndef BEZIERWIDGET_H
#define BEZIERWIDGET_H

#include <QWidget>
#include <QVector>
#include <QPointF>

// Часть 3 — Егор. Редактор опорных точек составной кубической кривой Безье.
class BezierWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BezierWidget(QWidget *parent = nullptr);

    void clearPoints();   // Удаляет все опорные точки.
    void resetExample();  // Восстанавливает пример кривой.

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    int pointAt(const QPointF &position) const; // Находит точку под курсором.
    QPainterPath makeCurve() const;             // Строит составной кубический путь.

    QVector<QPointF> m_points;
    int m_draggedPoint = -1;
    static constexpr qreal PointRadius = 7.0;
};

#endif // BEZIERWIDGET_H
