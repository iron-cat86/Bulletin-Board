#include "bull_thread.h"
#include <QCoreApplication>
#include <QPainter>

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
        _board->initNewSplash();

        for (int i = 0; i < _board->_jsonObjectArray.size(); ++i) {
            int random_number = QRandomGenerator::global()->bounded(20);
            QString text = randomBulletins[random_number];
            QJsonObject newobj = _board->_jsonObjectArray[i].toObject();
            newobj["text"]=text;
            _board->_jsonObjectArray.replace(i, newobj);
            BulletinPaintData newdata = _board->createNewPaintData(newobj);

            bool found = _board->findAndUpdatePaintData(newobj["author"].toString(), newdata);
            QPainter painter(&_board->_cachePixmap);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setClipRect(_board->rect());
            painter.setClipping(true);
            _board->drawOneBulletin(newdata, painter);
        }
        //_board->updateCache();
        _board->update();
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
