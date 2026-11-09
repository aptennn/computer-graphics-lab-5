#ifndef LSYSTEMWIDGET_H
#define LSYSTEMWIDGET_H

#include <QHash>
#include <QPointF>
#include <QRandomGenerator>
#include <QRectF>
#include <QVector>
#include <QWidget>

// Часть 1а и 1б — Михаил. Модель и визуализация L-системы.
class LSystemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LSystemWidget(QWidget *parent = nullptr);

    // Читает аксиому, угол, направление и правила из текстового файла.
    bool loadFile(const QString &path, QString *error);
    // Загружает один из примеров из лекции или пример фрактального дерева.
    void loadPreset(int index);
    // Перестраивает строку и рисунок при изменении числа итераций.
    void setIterations(int iterations);
    // Устанавливает разброс угла поворота в градусах.
    void setAngleVariation(double degrees);
    // Устанавливает начальное состояние генератора случайных чисел.
    void setSeed(quint32 seed);
    // Включает оформление дерева с изменением толщины и цвета.
    void setTreeStyle(bool enabled);

    QString errorText() const { return m_error; }
    int segmentCount() const { return m_segments.size(); }

protected:
    // Рисует подготовленные отрезки с сохранением пропорций внутри окна.
    void paintEvent(QPaintEvent *event) override;

private:
    struct Segment {
        QPointF from;
        QPointF to;
        int branchDepth = 0;
        int pathLength = 0;
    };

    // Разбирает текстовый формат L-системы и проверяет правила.
    bool parse(const QString &text, QString *error);
    // Параллельно применяет правила ко всем символам очередного поколения.
    bool expand(QString *result, QRandomGenerator &random);
    // Выполняет команды черепашки; скобки сохраняют и восстанавливают её состояние.
    bool trace(const QString &commands, QRandomGenerator &random);
    // Пересчитывает рисунок после изменения описания или параметров.
    void rebuild();

    QString m_axiom;
    QHash<QChar, QVector<QString>> m_rules;
    double m_angle = 0.0;
    double m_initialDirection = 0.0;
    double m_angleVariation = 0.0;
    int m_iterations = 4;
    quint32 m_seed = 12345;
    bool m_treeStyle = false;
    QString m_error;
    QVector<Segment> m_segments;
    QRectF m_bounds;
};

#endif // LSYSTEMWIDGET_H
