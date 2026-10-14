#include "bezierwidget.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

BezierWidget::BezierWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(650, 420);
    setMouseTracking(true);
    resetExample();
}

void BezierWidget::clearPoints()
{
    m_points.clear();
    m_draggedPoint = -1;
    update();
}

void BezierWidget::resetExample()
{
    m_points = {
        QPointF(90, 300), QPointF(180, 100), QPointF(310, 330),
        QPointF(440, 150), QPointF(570, 270), QPointF(720, 120)
    };
    update();
}

int BezierWidget::pointAt(const QPointF &position) const
{
    const qreal r2 = PointRadius * PointRadius;
    for (int i = 0; i < m_points.size(); ++i) {
        const QPointF delta = m_points[i] - position;
        if (delta.x() * delta.x() + delta.y() * delta.y() <= r2)
            return i;
    }
    return -1;
}

QPainterPath BezierWidget::makeCurve() const
{
    QPainterPath path;
    if (m_points.isEmpty())
        return path;

    path.moveTo(m_points.first());
    if (m_points.size() == 1)
        return path;

    // Tangents are calculated from neighbouring support points.  The user only
    // edits the support points; Bezier control points remain internal.
    for (int i = 0; i < m_points.size() - 1; ++i) {
        const QPointF &p0 = m_points[i == 0 ? i : i - 1];
        const QPointF &p1 = m_points[i];
        const QPointF &p2 = m_points[i + 1];
        const QPointF &p3 = m_points[i + 2 < m_points.size() ? i + 2 : i + 1];

        const QPointF c1 = p1 + (p2 - p0) / 6.0;
        const QPointF c2 = p2 - (p3 - p1) / 6.0;
        path.cubicTo(c1, c2, p2);
    }
    return path;
}

void BezierWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor(23, 28, 42));

    painter.setPen(QPen(QColor(64, 73, 94), 1, Qt::DashLine));
    for (int x = 0; x < width(); x += 50) painter.drawLine(x, 0, x, height());
    for (int y = 0; y < height(); y += 50) painter.drawLine(0, y, width(), y);

    if (m_points.size() > 1) {
        painter.setPen(QPen(QColor(150, 160, 180), 1.2, Qt::DashLine));
        painter.drawPolyline(m_points.constData(), m_points.size());
    }

    painter.setPen(QPen(QColor(58, 205, 255), 3));
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(makeCurve());

    for (int i = 0; i < m_points.size(); ++i) {
        painter.setPen(QPen(Qt::white, 1.5));
        painter.setBrush(i == m_draggedPoint ? QColor(255, 189, 72) : QColor(236, 103, 113));
        painter.drawEllipse(m_points[i], PointRadius, PointRadius);
        painter.setPen(QColor(230, 230, 230));
        painter.drawText(m_points[i] + QPointF(10, -10), QString::number(i + 1));
    }

    painter.setPen(QColor(220, 225, 235));
    painter.drawText(14, 24, "Левый клик: добавить или переместить точку   |   Правый клик: удалить точку");
    painter.setPen(QColor(160, 170, 190));
    painter.drawText(14, height() - 14, "Пунктир — ломаная опорных точек; голубая линия — составная кубическая кривая Безье.");
}

void BezierWidget::mousePressEvent(QMouseEvent *event)
{
    const int index = pointAt(event->position());
    if (event->button() == Qt::RightButton) {
        if (index >= 0) m_points.removeAt(index);
        update();
        return;
    }
    if (event->button() != Qt::LeftButton) return;

    if (index >= 0) {
        m_draggedPoint = index;
    } else {
        m_points.append(event->position());
        m_draggedPoint = m_points.size() - 1;
    }
    update();
}

void BezierWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_draggedPoint < 0) return;
    m_points[m_draggedPoint] = event->position();
    update();
}

void BezierWidget::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    m_draggedPoint = -1;
    update();
}
