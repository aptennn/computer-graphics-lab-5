#ifndef MOUNTAINWIDGET_H
#define MOUNTAINWIDGET_H

#include <QWidget>
#include <QVector>
#include <QPointF>
#include <QRandomGenerator>

class MountainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MountainWidget(QWidget *parent = nullptr);

    // Параметры построения
    void setDepth(int depth);
    void setInitialDisplacement(double d);
    void setRoughness(double r);
    void setSeed(quint32 seed);
    void setShowSteps(bool show);

    int    depth() const              { return m_depth; }
    double initialDisplacement() const{ return m_initialDisplacement; }
    double roughness() const          { return m_roughness; }
    quint32 seed() const              { return m_seed; }

public slots:
    void regenerate();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    struct Step {
        QVector<QPointF> points;
        QColor color;
    };

    void buildMountain();
    void midpointDisplace(QVector<QPointF> &pts, double displacement, int level);
    void fitToWindow(QVector<QPointF> &pts) const;
    QRectF boundingBox(const QVector<QPointF> &pts) const;

    int     m_depth = 6;
    double  m_initialDisplacement = 0.5;
    double  m_roughness = 0.5;
    quint32 m_seed = 12345;

    QVector<Step> m_steps;
    QVector<QPointF> m_final;
};

#endif // MOUNTAINWIDGET_H
