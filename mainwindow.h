#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class MountainWidget;
class LSystemWidget;
class QSlider;
class QLabel;
class QWidget;

// Собирает интерфейс всех трёх частей лабораторной работы.
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    // Части 1а и 1б — Михаил: L-системы и фрактальное дерево.
    QWidget *createLSystemTab(bool tree);
    // Часть 2 — Лилия: параметры и просмотр этапов горного массива.
    QWidget *createMountainTab();
    // Часть 3 — Егор: редактирование только опорных точек кривой Безье.
    QWidget *createBezierTab();

    LSystemWidget *m_patternView = nullptr;
    LSystemWidget *m_treeView = nullptr;
    MountainWidget *m_mountainView = nullptr;
    QSlider *m_stepSlider = nullptr;
    QLabel *m_stepLabel = nullptr;
};

#endif // MAINWINDOW_H
