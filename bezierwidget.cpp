#include "bezierwidget.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

namespace {
// Возвращает координаты мыши для поддерживаемых CMake версий Qt 5 и Qt 6.
QPointF mousePosition(const QMouseEvent *event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return event->position();
#else
    return event->localPos();
#endif
}
}

// Создаёт поле редактирования и начальный пример.
BezierWidget::BezierWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(650, 420);
    setMouseTracking(true);
    resetExample();
}

// Очищает точки и отменяет перетаскивание.
void BezierWidget::clearPoints()
{
    m_points.clear();
    m_draggedPoint = -1;
    update();
}

// Возвращает шесть опорных точек демонстрационной кривой.
void BezierWidget::resetExample()
{
    m_points = {
        QPointF(90, 300), QPointF(180, 100), QPointF(310, 330),
        QPointF(440, 150), QPointF(570, 270), QPointF(720, 120)
    };
    update();
}

// Ищет ближайшую опорную точку в пределах её видимого радиуса.
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

// Для соседних опорных точек вычисляет управляющие точки Безье.
// На стыке соседних кубических сегментов касательные коллинеарны, как в лекции 7.
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

// Показывает опорную ломаную, кубическую кривую и доступные для правки точки.
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

// Левый клик добавляет или захватывает точку, правый удаляет её.
void BezierWidget::mousePressEvent(QMouseEvent *event)
{
    const int index = pointAt(mousePosition(event));
    if (event->button() == Qt::RightButton) {
        if (index >= 0) {
            m_points.removeAt(index);
            m_draggedPoint = -1;
        }
        update();
        return;
    }
    if (event->button() != Qt::LeftButton) return;

    if (index >= 0) {
        m_draggedPoint = index;
    } else {
        m_points.append(mousePosition(event));
        m_draggedPoint = m_points.size() - 1;
    }
    update();
}

// Перемещает захваченную опорную точку и перестраивает кривую.
void BezierWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_draggedPoint < 0) return;
    m_points[m_draggedPoint] = mousePosition(event);
    update();
}

// Завершает перетаскивание опорной точки.
void BezierWidget::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    m_draggedPoint = -1;
    update();
}
