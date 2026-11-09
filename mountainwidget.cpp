#include "mountainwidget.h"

#include <QPainter>
#include <QRandomGenerator>
#include <algorithm>

// Часть 2 — Лилия. Начальный профиль состоит из горизонтального отрезка.
MountainWidget::MountainWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(600, 400);
    buildMountain();
}

// Меняет число разбиений; при смене глубины показывает новое последнее поколение.
void MountainWidget::setDepth(int depth)
{
    depth = std::clamp(depth, 0, 16);
    if (depth == m_depth) return;
    m_depth = depth;
    m_displayStep = depth;
    buildMountain();
    update();
}

// R — коэффициент шероховатости в формуле из лекции 6, слайд 59.
void MountainWidget::setRoughness(double roughness)
{
    roughness = std::clamp(roughness, 0.0, 1.0);
    if (roughness == m_roughness) return;
    m_roughness = roughness;
    buildMountain();
    update();
}

// Seed делает результат повторяемым и позволяет сгенерировать новый вариант.
void MountainWidget::setSeed(quint32 seed)
{
    if (seed == m_seed) return;
    m_seed = seed;
    buildMountain();
    update();
}

// Переключает отображение между сохранёнными последовательными шагами.
void MountainWidget::setDisplayStep(int step)
{
    m_displayStep = std::clamp(step, 0, m_depth);
    update();
}

// На каждом шаге вставляет случайно смещённую середину каждого отрезка.
void MountainWidget::buildMountain()
{
    m_steps.clear();
    QVector<QPointF> points{QPointF(0.0, 0.5), QPointF(1.0, 0.5)};
    m_steps.append(points);
    QRandomGenerator random(m_seed);
    for (int level = 0; level < m_depth; ++level) {
        QVector<QPointF> next;
        next.reserve(points.size() * 2 - 1);
        for (int i = 0; i < points.size() - 1; ++i) {
            const QPointF &left = points[i];
            const QPointF &right = points[i + 1];
            const double length = right.x() - left.x();
            const double offset = (2.0 * random.generateDouble() - 1.0) * m_roughness * length;
            next.append(left);
            next.append(QPointF((left.x() + right.x()) / 2.0,
                                (left.y() + right.y()) / 2.0 + offset));
        }
        next.append(points.last());
        points = std::move(next);
        m_steps.append(points);
    }
}

// Масштабирует по общему диапазону всех поколений, чтобы шаги не прыгали при просмотре.
void MountainWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), QColor(20, 25, 40));
    if (m_steps.isEmpty()) return;

    double minY = 0.5, maxY = 0.5;
    for (const QPointF &point : m_steps.last()) {
        minY = std::min(minY, point.y());
        maxY = std::max(maxY, point.y());
    }
    const double range = std::max(maxY - minY, 0.5);
    const double center = (minY + maxY) / 2.0;
    const double margin = 25.0;
    const double spanY = height() - 2.0 * margin;
    QVector<QPointF> display;
    const QVector<QPointF> &step = m_steps[m_displayStep];
    display.reserve(step.size());
    for (const QPointF &point : step) {
        display.append(QPointF(margin + point.x() * (width() - 2.0 * margin),
                               height() / 2.0 - (point.y() - center) * spanY / range));
    }
    QPolygonF polygon;
    polygon << QPointF(display.first().x(), height()) << display
            << QPointF(display.last().x(), height());
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(48, 78, 102));
    painter.drawPolygon(polygon);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(QPen(QColor(247, 172, 73), 2.5));
    painter.drawPolyline(display.constData(), display.size());

    painter.setPen(QColor(235, 235, 240));
    painter.drawText(12, 24, QString("Шаг %1 из %2 · точек: %3 · seed: %4")
                                 .arg(m_displayStep).arg(m_depth).arg(step.size()).arg(m_seed));
}
