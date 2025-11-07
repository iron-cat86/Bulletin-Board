#include "bull_thread.h"
#include <QCoreApplication>
#include <QPainter>
#include <QTimer>

BullThread::BullThread(Board *board, QMutex* mutex, int ms, QObject *parent)
    : QThread(parent), _board(board), _mutex(mutex), _ms(ms)
{
    _quitFlag.storeRelaxed(0);
}

BullThread::~BullThread()
{
    stopThread();
}

void BullThread::run()
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
            bool found = _board->findAndUpdatePaintData(newobj["author"].toString(), newdata);
        }
        _board->allBulletinsUpdated();
        qint64 elapsedMs = timer.elapsed();
        qDebug() << "Обновление "<<_board->_jsonObjectArray.size()<<" объявлений составляет "
                 << elapsedMs << " милисекунд, что на одно объявление составляет "
                 << (((double)elapsedMs)/((double)_board->_jsonObjectArray.size()))<<" миллисекунд, задержка между обновлениями: " << _ms << " миллисекунд.";
        _mutex->unlock();
        QThread::msleep(_ms);
    }
}

void BullThread::stopThread()
{
    qDebug() << "Останавливаем авто обновление...";

    _quitFlag.storeRelaxed(1);

    while(this->isRunning()) {
        QCoreApplication::processEvents();
    }
    qDebug() << "Авто обновление остановлено.";
}
