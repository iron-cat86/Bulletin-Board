#include "task_thread.h"
#include <QCoreApplication>
#include <QPainter>
#include <QTimer>

TaskThread::TaskThread(QMutex* mutex, Board *board, int ms, QObject *parent)
    : QThread(parent), _mutex(mutex), _board(board), _ms(ms)
{
    _quitFlag.storeRelaxed(0);
}

TaskThread::~TaskThread()
{
    stopThread();
}

void TaskThread::stopThread()
{
    qDebug() << "Останавливаем поток, добавляющий объявления...";
    _quitFlag.storeRelaxed(1); // Устанавливаем флаг завершения

    while(this->isRunning()) {
        QCoreApplication::processEvents();
    }
    qDebug() << "Добавление объявлений остановлено.";
}

void TaskThread::run()
{
    qDebug() << "Авто добавление новых объявлений запускается...";
    _quitFlag.storeRelaxed(0);

    while (_quitFlag.loadRelaxed() == 0) {
        _mutex->lock();
        QElapsedTimer timer;
        timer.start();
        /*QPainter painter(&_board->_cachePixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setClipRect(_board->rect());
        painter.setClipping(true);*/
        int id = _board->_bulletinPaintDataList.size();

        QString user = QString::number(id);
        QString text = randomBulletins[QRandomGenerator::global()->bounded(20)];
        QString t_font = fam_fonts[QRandomGenerator::global()->bounded(5)];
        QString color = t_colors[QRandomGenerator::global()->bounded(8)];
        int x = QRandomGenerator::global()->bounded(800);
        int y = QRandomGenerator::global()->bounded(400);
        int size = 12 +  QRandomGenerator::global()->bounded(29);
        double angle = (double)QRandomGenerator::global()->bounded(360);

        QJsonObject newDataObject;
        newDataObject["author"] = user;
        newDataObject["text"] = text;
        newDataObject["color"] = color;
        newDataObject["left"] = x;
        newDataObject["top"] = y;
        newDataObject["angle"] = angle;
        newDataObject["font"] = t_font;
        newDataObject["size"] = size;

        BulletinPaintData data = _board->createNewPaintData(newDataObject);

        _board->_jsonObjectArray.append(newDataObject);
        _board->_bulletinPaintDataList.append(data);
        _board->oneBulletinAdded();
        qint64 elapsedMs = timer.elapsed();
        qDebug() << "Добавление одного объявления составляет "<< elapsedMs << " милисекунд,  задержка между добавлениями: " << _ms << " миллисекунд.";
        _mutex->unlock();
        QThread::msleep(_ms);
    }
    qDebug() << "Авто добавление новых объявлений запущено.";
}
