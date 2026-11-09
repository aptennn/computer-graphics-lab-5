#include <QApplication>
#include "mainwindow.h"

// Запускает приложение с вкладками лабораторной работы.
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}
