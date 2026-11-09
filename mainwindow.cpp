#include "mainwindow.h"

#include "bezierwidget.h"
#include "lsystemwidget.h"
#include "mountainwidget.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QRandomGenerator>
#include <QSlider>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>

// Создаёт вкладки всех частей лабораторной работы.
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("Компьютерная графика — лабораторная работа 5");
    auto *tabs = new QTabWidget(this);
    tabs->addTab(createLSystemTab(false), "1а. L-системы — Михаил");
    tabs->addTab(createLSystemTab(true), "1б. Дерево — Михаил");
    tabs->addTab(createMountainTab(), "2. Горный массив — Лилия");
    tabs->addTab(createBezierTab(), "3. Кривая Безье — Егор");
    setCentralWidget(tabs);
    resize(1100, 700);
}

// Предоставляет примеры из лекции, чтение файла и параметры случайного поворота.
QWidget *MainWindow::createLSystemTab(bool tree)
{
    auto *tab = new QWidget(this);
    auto *layout = new QHBoxLayout(tab);
    auto *view = new LSystemWidget(tab);
    layout->addWidget(view, 1);
    if (tree) {
        m_treeView = view;
        view->loadPreset(6);
        view->setTreeStyle(true);
        view->setIterations(5);
        view->setAngleVariation(8.0);
    } else {
        m_patternView = view;
    }

    auto *panel = new QGroupBox("Параметры L-системы", tab);
    auto *form = new QFormLayout(panel);
    form->setRowWrapPolicy(QFormLayout::WrapLongRows);
    if (!tree) {
        auto *presets = new QComboBox(panel);
        presets->addItems({"Кривая Коха", "Квадратный остров Коха", "Кривая Гильберта",
                           "Кривая дракона", "Куст 1", "Куст 2", "Куст 3"});
        form->addRow("Пример из лекции:", presets);
        connect(presets, QOverload<int>::of(&QComboBox::currentIndexChanged),
                view, &LSystemWidget::loadPreset);

        auto *openButton = new QPushButton("Открыть файл с правилами…", panel);
        form->addRow(openButton);
        connect(openButton, &QPushButton::clicked, this, [this, view, presets] {
            const QString path = QFileDialog::getOpenFileName(this, "Открыть L-систему", QString(),
                                      "Текстовые файлы (*.txt *.lsys);;Все файлы (*)");
            if (path.isEmpty()) return;
            QString error;
            if (!view->loadFile(path, &error))
                QMessageBox::warning(this, "Ошибка L-системы", error);
            else
                presets->setCurrentIndex(-1);
        });
    } else {
        auto *description = new QLabel("Куст 3 из лекции: F→FF, X→F−[[X]+X]+F[+FX]−X", panel);
        description->setWordWrap(true);
        form->addRow(description);
    }

    auto *iterations = new QSpinBox(panel);
    iterations->setRange(0, 8);
    iterations->setValue(tree ? 5 : 4);
    form->addRow("Итерации:", iterations);
    connect(iterations, QOverload<int>::of(&QSpinBox::valueChanged), view,
            &LSystemWidget::setIterations);

    auto *variation = new QDoubleSpinBox(panel);
    variation->setRange(0.0, 45.0);
    variation->setSuffix("°");
    variation->setValue(tree ? 8.0 : 0.0);
    form->addRow("Разброс угла:", variation);
    connect(variation, QOverload<double>::of(&QDoubleSpinBox::valueChanged), view,
            &LSystemWidget::setAngleVariation);

    auto *seed = new QSpinBox(panel);
    seed->setRange(0, 999999999);
    seed->setValue(12345);
    form->addRow("Seed:", seed);
    connect(seed, QOverload<int>::of(&QSpinBox::valueChanged), view,
            [view](int value) { view->setSeed(quint32(value)); });
    auto *newSeed = new QPushButton("Новый случайный рисунок", panel);
    form->addRow(newSeed);
    connect(newSeed, &QPushButton::clicked, seed, [seed] {
        seed->setValue(int(QRandomGenerator::global()->bounded(1000000000u)));
    });

    if (!tree) {
        auto *treeStyle = new QCheckBox("Цвет и толщина дерева", panel);
        form->addRow(treeStyle);
        connect(treeStyle, &QCheckBox::toggled, view, &LSystemWidget::setTreeStyle);
    }
    auto *help = new QLabel("Файл: первая строка — аксиома угол направление; далее строки F -> ... . "
                            "Повторные правила дают случайный выбор. F рисует, +/− поворачивают, "
                            "[ ] создают ветви.", panel);
    help->setWordWrap(true);
    form->addRow(help);
    auto *panelColumn = new QVBoxLayout;
    panelColumn->addWidget(panel);
    panelColumn->addStretch();
    layout->addLayout(panelColumn);
    panel->setFixedWidth(280);
    return tab;
}

// Показывает алгоритм из лекции 6 по одному поколению; R задаёт шероховатость.
QWidget *MainWindow::createMountainTab()
{
    auto *tab = new QWidget(this);
    auto *layout = new QHBoxLayout(tab);
    m_mountainView = new MountainWidget(tab);
    layout->addWidget(m_mountainView, 1);
    auto *panel = new QGroupBox("Параметры", tab);
    auto *form = new QFormLayout(panel);

    auto *depth = new QSpinBox(panel);
    depth->setRange(0, 16);
    depth->setValue(m_mountainView->depth());
    form->addRow("Глубина:", depth);
    connect(depth, QOverload<int>::of(&QSpinBox::valueChanged), m_mountainView,
            &MountainWidget::setDepth);

    auto *roughness = new QDoubleSpinBox(panel);
    roughness->setRange(0.0, 1.0);
    roughness->setSingleStep(0.05);
    roughness->setValue(m_mountainView->roughness());
    form->addRow("Шероховатость R:", roughness);
    connect(roughness, QOverload<double>::of(&QDoubleSpinBox::valueChanged), m_mountainView,
            &MountainWidget::setRoughness);

    auto *seed = new QSpinBox(panel);
    seed->setRange(0, 999999999);
    seed->setValue(int(m_mountainView->seed()));
    form->addRow("Seed:", seed);
    connect(seed, QOverload<int>::of(&QSpinBox::valueChanged), m_mountainView,
            [this](int value) { m_mountainView->setSeed(quint32(value)); });
    auto *newSeed = new QPushButton("Новый случайный профиль", panel);
    form->addRow(newSeed);
    connect(newSeed, &QPushButton::clicked, seed, [seed] {
        seed->setValue(int(QRandomGenerator::global()->bounded(1000000000u)));
    });

    m_stepSlider = new QSlider(Qt::Horizontal, panel);
    m_stepSlider->setRange(0, m_mountainView->depth());
    m_stepSlider->setValue(m_mountainView->depth());
    m_stepLabel = new QLabel(QString("Шаг %1 из %1").arg(m_mountainView->depth()), panel);
    form->addRow("Поколение:", m_stepSlider);
    form->addRow(m_stepLabel);
    connect(m_stepSlider, &QSlider::valueChanged, m_mountainView, &MountainWidget::setDisplayStep);
    connect(m_stepSlider, &QSlider::valueChanged, this, [this](int step) {
        m_stepLabel->setText(QString("Шаг %1 из %2").arg(step).arg(m_mountainView->depth()));
    });
    connect(depth, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
        m_stepSlider->setMaximum(value);
        m_stepSlider->setValue(value);
        m_stepLabel->setText(QString("Шаг %1 из %1").arg(value));
    });

    auto *formula = new QLabel("h = (hL + hR) / 2 + random(−R·L, R·L)", panel);
    formula->setWordWrap(true);
    form->addRow(formula);
    auto *column = new QVBoxLayout;
    column->addWidget(panel);
    column->addStretch();
    layout->addLayout(column);
    panel->setFixedWidth(260);
    return tab;
}

// Добавляет управление только опорными точками; касательные вычисляет виджет.
QWidget *MainWindow::createBezierTab()
{
    auto *tab = new QWidget(this);
    auto *layout = new QVBoxLayout(tab);
    auto *view = new BezierWidget(tab);
    layout->addWidget(view, 1);
    auto *buttons = new QHBoxLayout;
    auto *reset = new QPushButton("Восстановить пример", tab);
    auto *clear = new QPushButton("Очистить точки", tab);
    buttons->addWidget(reset);
    buttons->addWidget(clear);
    buttons->addStretch();
    layout->addLayout(buttons);
    connect(reset, &QPushButton::clicked, view, &BezierWidget::resetExample);
    connect(clear, &QPushButton::clicked, view, &BezierWidget::clearPoints);
    return tab;
}
