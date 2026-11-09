#include "lsystemwidget.h"
#include "mainwindow.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFile>
#include <QImage>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QTemporaryDir>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace {
void require(bool condition, const char *message)
{
    if (!condition) throw std::runtime_error(message);
}

QImage render(QWidget &widget)
{
    QImage image(widget.size(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    widget.render(&image);
    return image;
}

bool hasInk(const QImage &image, int x, int y)
{
    return image.pixelColor(x, y) != QColor(22, 28, 38);
}

QRect inkBounds(const QImage &image)
{
    QRect bounds;
    for (int y = 0; y < image.height(); ++y)
        for (int x = 0; x < image.width(); ++x)
            if (hasInk(image, x, y)) bounds |= QRect(x, y, 1, 1);
    return bounds;
}
}

// Проверяет результат подстановок, черепашку, загрузку и управление вкладками.
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    try {
        require(argc >= 2, "Examples directory is required");
        const QDir examples(QString::fromLocal8Bit(argv[1]));
        QTemporaryDir temporary;
        require(temporary.isValid(), "Cannot create temporary directory");
        LSystemWidget view;
        view.resize(600, 500);
        QString error;
        const auto loadText = [&](const QByteArray &source) {
            const QString path = temporary.filePath("test.lsys");
            QFile file(path);
            require(file.open(QIODevice::WriteOnly), "Cannot write test rules");
            require(file.write(source) == source.size(), "Incomplete write");
            file.close();
            return view.loadFile(path, &error);
        };

        const int firstGeneration[] = {4, 32, 3, 1, 8, 2, 3};
        const int secondGeneration[] = {16, 256, 15, 3, 64, 10, 18};
        for (int preset = 0; preset < 7; ++preset) {
            view.loadPreset(preset);
            view.setIterations(1);
            require(view.errorText().isEmpty(), "Invalid preset");
            require(view.segmentCount() == firstGeneration[preset], "Wrong first generation");
            view.setIterations(2);
            require(view.segmentCount() == secondGeneration[preset], "Wrong second generation");
        }

        view.setIterations(1);
        require(loadText("F 90 0\nF -> X\nX -> FF\n"), "Cannot load parallel rules");
        require(view.segmentCount() == 0, "Rules must be applied in parallel");
        view.setIterations(2);
        require(view.segmentCount() == 2, "Symbols without drawing must still expand");

        // Нулевая итерация рисует саму аксиому. Ветвь возвращает и точку, и угол.
        view.setIterations(0);
        require(loadText("F[+F]F 90 0\n"), "Cannot load branching axiom");
        require(view.segmentCount() == 3, "Wrong branch segment count");
        QImage image = render(view);
        require(hasInk(image, 163, 387), "Missing first trunk segment");
        require(hasInk(image, 300, 250), "Missing branch");
        require(hasInk(image, 437, 387), "Turtle state was not restored");
        require(!hasInk(image, 437, 113), "Branch must not move the trunk");

        // Осевые отрезки и вся кривая должны оставаться внутри поля с отступами.
        require(loadText("FFFF 90 0\n"), "Cannot load straight line");
        QRect ink = inkBounds(render(view));
        require(ink.left() >= 24 && ink.right() <= 575 && ink.width() >= 546,
                "Horizontal line is not fitted to the viewport");
        view.loadPreset(2);
        view.setIterations(4);
        ink = inkBounds(render(view));
        require(ink.top() >= 24 && ink.bottom() <= 475 && ink.height() >= 446,
                "Hilbert curve bounds are incorrect");
        require(std::abs(ink.width() - ink.height()) <= 2, "Drawing aspect ratio changed");

        // Ошибки правил и скобок не должны затирать предыдущий рисунок.
        const QImage previous = render(view);
        const int previousCount = view.segmentCount();
        const QByteArray invalid[] = {
            "", "F nan 90\n", "F 90\n", "F 90 0\nFF -> F\n",
            "F 90 0\n+ -> F\n", "F 90 0\nF = FF\n", "F 90 0\nF ->\n",
            "F] 90 0\n", "[F 90 0\n"
        };
        for (const auto &source : invalid) {
            require(!loadText(source), "Malformed file was accepted");
            require(!error.isEmpty(), "Missing load error");
            require(view.segmentCount() == previousCount && render(view) == previous,
                    "Failed load changed the current drawing");
            require(view.errorText().isEmpty(), "Failed load left a persistent error");
        }
        require(!view.loadFile(temporary.filePath("missing.lsys"), &error),
                "Missing file was accepted");
        require(render(view) == previous, "Missing file changed the drawing");

        require(loadText(QString::fromUtf8("# Комментарий\r\n\r\nF 60 0\r\nF → F-F++F-F\r\n")
                             .toUtf8()), "Unicode arrow, comments or CRLF are not supported");
        require(error.isEmpty() && view.segmentCount() == 256, "Wrong Koch file generation");

        const char *files[] = {"koch.lsys", "hilbert.lsys", "bush3.lsys", "stochastic-tree.lsys"};
        for (const char *file : files) {
            require(view.loadFile(examples.filePath(file), &error), "Cannot load supplied example");
            require(view.segmentCount() > 0, "Example did not draw anything");
        }

        // Проверяем отдельно случайные правила и случайные углы с тем же seed.
        view.setIterations(5);
        view.setSeed(123);
        const QImage stochastic = render(view);
        view.setSeed(123);
        require(render(view) == stochastic, "Stochastic rules are not reproducible");
        view.setSeed(456);
        require(render(view) != stochastic, "Seed did not change stochastic rules");
        view.loadPreset(6);
        view.setAngleVariation(8.0);
        view.setSeed(123);
        const QImage tree = render(view);
        view.setSeed(123);
        require(render(view) == tree, "Angle variation is not reproducible");
        view.setSeed(456);
        require(render(view) != tree, "Seed did not change random angles");
        view.setSeed(123);
        view.setTreeStyle(true);
        require(render(view) != tree, "Tree styling did not change rendering");

        // Рост ограничен; после снижения итераций виджет восстанавливается.
        view.loadPreset(1);
        view.setIterations(8);
        require(!view.errorText().isEmpty() && view.segmentCount() == 0,
                "Expansion size limit was not enforced");
        require(!loadText("F 90 0\nF -> FFFFFFFFFF\n"), "Oversized generation was accepted");
        require(!view.errorText().isEmpty(), "Failed load lost the previous generation error");
        view.setIterations(2);
        require(view.errorText().isEmpty() && view.segmentCount() == 256,
                "Cannot recover from expansion limit");

        MainWindow window;
        window.show();
        app.processEvents();
        auto *tabs = window.findChild<QTabWidget *>();
        require(tabs && tabs->count() == 4, "Expected four application tabs");
        auto *patterns = tabs->widget(0)->findChild<LSystemWidget *>();
        auto *treeView = tabs->widget(1)->findChild<LSystemWidget *>();
        auto *presets = tabs->widget(0)->findChild<QComboBox *>();
        require(patterns && treeView && presets && presets->count() == 7,
                "L-system tabs are incomplete");
        presets->setCurrentIndex(2);
        require(patterns->segmentCount() == 255, "Preset selector is not connected");
        const auto patternSpins = tabs->widget(0)->findChildren<QSpinBox *>();
        require(patternSpins.size() == 2, "Missing iterations or seed control");
        patternSpins[0]->setValue(2);
        require(patterns->segmentCount() == 15, "Iterations control is not connected");
        auto *style = tabs->widget(0)->findChild<QCheckBox *>();
        require(style, "Missing tree style control");
        image = render(*patterns);
        style->setChecked(true);
        require(render(*patterns) != image, "Style control is not connected");
        tabs->setCurrentIndex(1);
        app.processEvents();
        require(treeView->segmentCount() > 0 && treeView->errorText().isEmpty(),
                "Tree tab is empty");
        auto *variation = tabs->widget(1)->findChild<QDoubleSpinBox *>();
        require(variation, "Missing angle variation control");
        image = render(*treeView);
        variation->setValue(0.0);
        require(render(*treeView) != image, "Variation control is not connected");
        const auto treeSpins = tabs->widget(1)->findChildren<QSpinBox *>();
        variation->setValue(8.0);
        image = render(*treeView);
        treeSpins[1]->setValue(321);
        require(render(*treeView) != image, "Seed control is not connected");
        const auto buttons = tabs->widget(1)->findChildren<QPushButton *>();
        require(buttons.size() == 1, "Missing random tree button");
        buttons.first()->click();
        require(treeSpins[1]->value() != 321, "Random tree button did not change seed");

        // Необязательные изображения для ручной проверки интерфейса.
        if (argc >= 3) {
            const QDir output(QString::fromLocal8Bit(argv[2]));
            tabs->setCurrentIndex(0);
            presets->setCurrentIndex(0);
            patternSpins[0]->setValue(4);
            style->setChecked(false);
            app.processEvents();
            require(render(window).save(output.filePath("lab5-lsystems.png")), "Cannot save L-system preview");
            tabs->setCurrentIndex(1);
            treeSpins[1]->setValue(12345);
            app.processEvents();
            require(render(window).save(output.filePath("lab5-tree.png")), "Cannot save tree preview");
        }
        std::cout << "L-systems, turtle, files, randomness and tabs: OK\n";
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
