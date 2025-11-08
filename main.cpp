#include "mainwindow.h"
#include <QApplication>
//thread pool
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}

