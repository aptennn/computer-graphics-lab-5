#include "mountainwidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <QtMath>
#include <algorithm>

MountainWidget::MountainWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(600, 400);
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(20, 25, 40));
    setPalette(pal);
    regenerate();
}

void MountainWidget::setDepth(int depth)
{
    depth = std::clamp(depth, 0, 16);
    if (m_depth == depth) return;
    m_depth = depth;
    regenerate();
}

void MountainWidget::setInitialDisplacement(double d)
{
    d = std::clamp(d, 0.0, 1.0);
    if (qFuzzyCompare(m_initialDisplacement, d)) return;
    m_initialDisplacement = d;
    regenerate();
}

void MountainWidget::setRoughness(double r)
{
    r = std::clamp(r, 0.0, 1.0);
    if (qFuzzyCompare(m_roughness, r)) return;
    m_roughness = r;
    regenerate();
}

void MountainWidget::setSeed(quint32 seed)
{
    if (m_seed == seed) return;
    m_seed = seed;
    regenerate();
}

void MountainWidget::regenerate()
{
    buildMountain();
    update();
}

void MountainWidget::buildMountain()
{
    m_steps.clear();
    m_final.clear();

    QVector<QPointF> pts;
    pts << QPointF(0.0, 0.5) << QPointF(1.0, 0.5);

    QRandomGenerator rng(m_seed);

    m_steps.append({pts, QColor(80, 120, 200)});

    double displacement = m_initialDisplacement;

    for (int level = 0; level < m_depth; ++level) {
        QVector<QPointF> newPts;
        newPts.reserve(pts.size() * 2 - 1);

        for (int i = 0; i < pts.size() - 1; ++i) {
            const QPointF &a = pts[i];
            const QPointF &b = pts[i + 1];

            QPointF mid((a.x() + b.x()) * 0.5, (a.y() + b.y()) * 0.5);

            double offset = (rng.generateDouble() * 2.0 - 1.0) * displacement;
            mid.setY(mid.y() + offset);

            newPts.append(a);
            newPts.append(mid);
        }
        newPts.append(pts.last());

        pts = newPts;

        double t = (m_depth > 1) ? double(level) / (m_depth - 1) : 0.0;
        QColor c(
            int(80  + t * (255 - 80)),
            int(120 + t * (140 - 120)),
            int(200 + t * (0   - 200))
            );

        m_steps.append({pts, c});

        displacement *= m_roughness;
    }

    m_final = pts;
}

void MountainWidget::fitToWindow(QVector<QPointF> &pts) const
{
    if (pts.isEmpty()) return;

    const double margin = 20.0;
    double w = width()  - 2 * margin;
    double h = height() - 2 * margin;

    for (QPointF &p : pts) {
        p.setX(margin + p.x() * w);
        p.setY(margin + (1.0 - p.y()) * h);
    }
}

QRectF MountainWidget::boundingBox(const QVector<QPointF> &pts) const
{
    if (pts.isEmpty()) return QRectF();
    double minX = pts[0].x(), maxX = pts[0].x();
    double minY = pts[0].y(), maxY = pts[0].y();
    for (const QPointF &p : pts) {
        minX = std::min(minX, p.x());
        maxX = std::max(maxX, p.x());
        minY = std::min(minY, p.y());
        maxY = std::max(maxY, p.y());
    }
    return QRectF(QPointF(minX, minY), QPointF(maxX, maxY));
}

void MountainWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.fillRect(rect(), QColor(20, 25, 40));

    if (m_steps.isEmpty()) return;

    QVector<QPointF> finalPts = m_final;
    fitToWindow(finalPts);

    QPen finalPen(m_steps.last().color);
    finalPen.setWidthF(2.5);
    painter.setPen(finalPen);
    painter.drawPolyline(finalPts.data(), finalPts.size());

    if (finalPts.size() >= 2) {
        QPolygonF poly;
        poly << QPointF(finalPts.first().x(), height())
             << finalPts
             << QPointF(finalPts.last().x(), height());

        QLinearGradient grad(0, 0, 0, height());
        grad.setColorAt(0.0, QColor(255, 140, 0, 120));
        grad.setColorAt(1.0, QColor(40, 20, 10, 40));
        painter.setBrush(grad);
        painter.setPen(Qt::NoPen);
        painter.drawPolygon(poly);
    }

    painter.setPen(QColor(220, 220, 220));
    painter.setOpacity(0.8);
    QFont f = painter.font();
    f.setPointSize(10);
    painter.setFont(f);
    painter.drawText(10, 20,
                     QString("Глубина: %1   Шагов: %2   Seed: %3")
                         .arg(m_depth)
                         .arg(m_steps.size())
                         .arg(m_seed));
    painter.setOpacity(1.0);
}