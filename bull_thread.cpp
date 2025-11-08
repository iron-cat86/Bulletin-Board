#include "bull_thread.h"
#include <QCoreApplication>
#include <QPainter>
#include <QTimer>

BullThread::BullThread(Board *board, QMutex* mutex, int ms, QObject *parent)
    : QThread(parent), _board(board), _mutex(mutex), _ms(ms)
{
    _quitFlag.storeRelaxed(0);
}

UpdateThread::UpdateThread(Board *board, QMutex* mutex, int ms, QObject *parent)
    : BullThread(board, mutex, ms, parent)
{}

// Реализация конструктора TaskThread
TaskThread::TaskThread(Board *board, QMutex* mutex, int ms, QObject *parent)
    : BullThread(board, mutex, ms, parent)
{}

BullThread::~BullThread()
{
    this->stopThread();
}

void BullThread::run()
{
    // Базовый класс ничего не делает, логика в наследниках
}

void UpdateThread::run()
{
    _quitFlag.storeRelaxed(0);

    while (_quitFlag.loadRelaxed() == 0) {
        _mutex->lock();
        QElapsedTimer timer;
        timer.start();
        int bulAmount = _board->_jsonObjectArray.size();

        for (int i = 0; i < bulAmount; ++i) {
            int random_number = QRandomGenerator::global()->bounded(20);
            QString text = randomBulletins[random_number];
            QJsonObject newobj = _board->_jsonObjectArray[i].toObject();
            newobj["text"]=text;
            _board->_jsonObjectArray.replace(i, newobj);
            BulletinPaintData newdata = _board->createNewPaintData(newobj);
            _board->_bulletinPaintDataList.replace(i, newdata);
        }
        _board->updateBulletin(false);
        qint64 elapsedMs = timer.elapsed();
        qDebug() << "Обновление "<<_board->_jsonObjectArray.size()<<" объявлений составляет "
                 << elapsedMs << " милисекунд, что на одно объявление составляет "
                 << (((double)elapsedMs)/((double)_board->_jsonObjectArray.size()))<<" миллисекунд, задержка между обновлениями: " << _ms << " миллисекунд.";
        _mutex->unlock();
        QThread::msleep(_ms);
    }
}

void TaskThread::run()
{
    qDebug() << "Авто добавление новых объявлений запускается...";
    _quitFlag.storeRelaxed(0);

    while (_quitFlag.loadRelaxed() == 0) {
        _mutex->lock();
        QElapsedTimer timer;
        timer.start();
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
        _board->updateBulletin(true);
        qint64 elapsedMs = timer.elapsed();
        qDebug() << "Добавление одного объявления составляет "<< elapsedMs << " милисекунд,  задержка между добавлениями: " << _ms << " миллисекунд.";
        _mutex->unlock();
        QThread::msleep(_ms);
    }
    qDebug() << "Авто добавление новых объявлений запущено.";
}

void BullThread::stopThread()
{
    _quitFlag.storeRelaxed(1);

    while(this->isRunning()) {
        QCoreApplication::processEvents();
    }
}

void UpdateThread::stopThread()
{
    qDebug() << "Останавливаем авто обновление...";
    BullThread::stopThread();
    qDebug() << "Авто обновление остановлено.";
}

void TaskThread::stopThread()
{
    qDebug() << "Останавливаем поток, добавляющий объявления...";
    BullThread::stopThread();
    qDebug() << "Добавление объявлений остановлено.";
}
