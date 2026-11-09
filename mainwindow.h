#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class MountainWidget;
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
    // Часть 2 — Лилия: параметры и просмотр этапов горного массива.
    QWidget *createMountainTab();
    // Часть 3 — Егор: редактирование только опорных точек кривой Безье.
    QWidget *createBezierTab();

    MountainWidget *m_mountainView = nullptr;
    QSlider *m_stepSlider = nullptr;
    QLabel *m_stepLabel = nullptr;
};

#endif // MAINWINDOW_H
