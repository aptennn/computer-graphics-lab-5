#ifndef MOUNTAINWIDGET_H
#define MOUNTAINWIDGET_H

#include <QPointF>
#include <QVector>
#include <QWidget>

// Часть 2 — Лилия. Одномерный midpoint displacement для силуэта гор.
class MountainWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MountainWidget(QWidget *parent = nullptr);

    // Устанавливают параметры генерации ломаной и создают её заново.
    void setDepth(int depth);
    void setRoughness(double roughness);
    void setSeed(quint32 seed);
    // Выбирает показанное поколение: 0 — прямая, depth — конечная ломаная.
    void setDisplayStep(int step);

    int depth() const { return m_depth; }
    double roughness() const { return m_roughness; }
    quint32 seed() const { return m_seed; }
    int displayStep() const { return m_displayStep; }
    int pointCountAtStep(int step) const { return m_steps.at(step).size(); }
    QPointF pointAtStep(int step, int index) const { return m_steps.at(step).at(index); }

protected:
    // Рисует выбранное поколение, подгоняя весь профиль по высоте окна.
    void paintEvent(QPaintEvent *event) override;

private:
    // Строит поколения: h=(hL+hR)/2+random(-R*L, R*L).
    void buildMountain();

    int m_depth = 6;
    double m_roughness = 0.5;
    quint32 m_seed = 12345;
    int m_displayStep = 6;
    QVector<QVector<QPointF>> m_steps;
};

#endif // MOUNTAINWIDGET_H
