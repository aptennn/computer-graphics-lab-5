#include "mountainwidget.h"

#include <QApplication>
#include <QRandomGenerator>
#include <cstdlib>
#include <cmath>
#include <iostream>

// Проверяет формулу и последовательные шаги горного массива.
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MountainWidget mountain;
    mountain.setSeed(123);
    mountain.setRoughness(0.7);
    mountain.setDepth(1);
    QRandomGenerator random(123);
    const double expectedMidpoint = 0.5 + (2.0 * random.generateDouble() - 1.0) * 0.7;
    if (std::abs(mountain.pointAtStep(1, 1).y() - expectedMidpoint) > 1e-12)
        return 14; // Формула h=(hL+hR)/2+random(-R*L,R*L), L=1.
    mountain.setDepth(8);
    for (int step = 0; step <= 8; ++step) {
        if (mountain.pointCountAtStep(step) != (1 << step) + 1) return 8;
        mountain.setDisplayStep(step);
        if (mountain.displayStep() != step) return 9;
    }
    std::cout << "Midpoint displacement: OK\n";
    return EXIT_SUCCESS;
}
