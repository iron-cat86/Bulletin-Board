#include "mainwindow.h"
#include <QApplication>
#include <QIcon>
#include <QSplashScreen>
#include <QTimer>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString imagePath = ":/splash.png";
    QString iconPath = ":/icon.png";
    QPixmap pixmap(imagePath);

    if (pixmap.isNull()) {
        qDebug() << "ERROR: Failed to load splash image!";
    }
    QIcon icon(iconPath);
    if(icon.availableSizes().isEmpty()) {
        qDebug()<<"ERROR: Fauld to load icon!";
    }
    a.setWindowIcon(icon);
    int desiredWidth = 800;
    int desiredHeight = 800;

    QPixmap scaledPixmap = pixmap.scaled(
            desiredWidth,
            desiredHeight,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        );
    QSplashScreen splash(scaledPixmap);
    splash.show();
    a.processEvents();
    MainWindow w;

    QTimer::singleShot(3000, &w, &MainWindow::show);
    QObject::connect(&w, &MainWindow::shown, &splash, &QSplashScreen::close);
    w.setWindowIcon(icon);
    return a.exec();
}

