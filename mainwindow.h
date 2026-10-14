#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class MountainWidget;
class QSlider;
class QSpinBox;
class QDoubleSpinBox;
class QCheckBox;
class QPushButton;
class QLabel;
class BezierWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onSeedChanged();
    void onRandomSeed();

private:
    MountainWidget *m_view = nullptr;
    BezierWidget *m_bezierView = nullptr;

    QSpinBox       *m_depthBox = nullptr;
    QDoubleSpinBox *m_dispBox = nullptr;
    QDoubleSpinBox *m_roughBox = nullptr;
    QSpinBox       *m_seedBox = nullptr;
    QCheckBox      *m_stepsCheck = nullptr;
    QPushButton    *m_randSeedBtn = nullptr;
};

#endif // MAINWINDOW_H
