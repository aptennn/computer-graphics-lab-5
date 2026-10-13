#include "mainwindow.h"
#include "mountainwidget.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QRandomGenerator>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Midpoint Displacement — горный массив");

    QWidget *central = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(central);

    m_view = new MountainWidget(this);
    mainLayout->addWidget(m_view, 1);

    QGroupBox *panel = new QGroupBox("Параметры", this);
    QFormLayout *form = new QFormLayout(panel);

    m_depthBox = new QSpinBox(panel);
    m_depthBox->setRange(0, 16);
    m_depthBox->setValue(m_view->depth());
    form->addRow("Глубина (шаги):", m_depthBox);

    m_dispBox = new QDoubleSpinBox(panel);
    m_dispBox->setRange(0.0, 1.0);
    m_dispBox->setSingleStep(0.05);
    m_dispBox->setDecimals(2);
    m_dispBox->setValue(m_view->initialDisplacement());
    form->addRow("Нач. смещение:", m_dispBox);

    m_roughBox = new QDoubleSpinBox(panel);
    m_roughBox->setRange(0.0, 1.0);
    m_roughBox->setSingleStep(0.05);
    m_roughBox->setDecimals(2);
    m_roughBox->setValue(m_view->roughness());
    form->addRow("Затухание:", m_roughBox);

    m_seedBox = new QSpinBox(panel);
    m_seedBox->setRange(0, 999999);
    m_seedBox->setValue(int(m_view->seed()));
    form->addRow("Seed:", m_seedBox);

    m_randSeedBtn = new QPushButton("Случайный seed", panel);
    form->addRow("", m_randSeedBtn);

    m_stepsCheck = new QCheckBox("Шаги", panel);
    m_stepsCheck->setChecked(m_view->showSteps());
    form->addRow("", m_stepsCheck);

    QVBoxLayout *panelLayout = new QVBoxLayout;
    panelLayout->addWidget(panel);
    panelLayout->addStretch();

    QWidget *panelWidget = new QWidget(this);
    panelWidget->setLayout(panelLayout);
    panelWidget->setFixedWidth(240);

    mainLayout->addWidget(panelWidget, 0);

    setCentralWidget(central);
    resize(1000, 600);

    connect(m_depthBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int v){ m_view->setDepth(v); });

    connect(m_dispBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this](double v){ m_view->setInitialDisplacement(v); });

    connect(m_roughBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this](double v){ m_view->setRoughness(v); });

    connect(m_seedBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onSeedChanged);

    connect(m_randSeedBtn, &QPushButton::clicked,
            this, &MainWindow::onRandomSeed);

    connect(m_stepsCheck, &QCheckBox::toggled,
            this, [this](bool v){ m_view->setShowSteps(v); });
}

void MainWindow::onSeedChanged()
{
    m_view->setSeed(quint32(m_seedBox->value()));
}

void MainWindow::onRandomSeed()
{
    quint32 s = QRandomGenerator::global()->generate() % 1000000u;
    m_seedBox->setValue(int(s));
}
