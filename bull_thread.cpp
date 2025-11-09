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
{}

void UpdateThread::run()
{
    qDebug() << "Авто обновление новых объявлений запускается...";
    _averageIterationTime = 0.;
    _averageOneupdateTime = 0.;
    _amountIteration = 0;
    _minIterationTime = 100000.;
    _maxIterationTime = 0.;
    _minOneupdateTime = 100000.;
    _maxOneupdateTime = 0.;

    _mutex->lock();
    _startAmountBulletins = _board->_bulletinPaintDataList.size();
    _mutex->unlock();

    _quitFlag.storeRelaxed(0);

    while (_quitFlag.loadRelaxed() == 0) {
        _mutex->lock();
        QElapsedTimer timer;
        timer.start();
        _board->updateBulletin(false);
        int elapsedMs = timer.elapsed();
        double elapsedOne = ((double)elapsedMs)/((double)_board->_jsonObjectArray.size());

        _mutex->unlock();
        ++_amountIteration;
        _averageIterationTime += (double)elapsedMs;
        _averageOneupdateTime += elapsedOne;

        if((double)elapsedMs < _minIterationTime) {
            _minIterationTime = (double)elapsedMs;
        }

        if((double)elapsedMs > _maxIterationTime) {
            _maxIterationTime = (double)elapsedMs;
        }

        if(elapsedOne < _minOneupdateTime) {
            _minOneupdateTime = elapsedOne;
        }

        if(elapsedOne > _maxOneupdateTime) {
            _maxOneupdateTime = elapsedOne;
        }
        QThread::msleep(_ms);
    }
}

void TaskThread::run()
{
    qDebug() << "Авто добавление новых объявлений запускается...";
    _quitFlag.storeRelaxed(0);
    _averageIterationTime = 0.;
    _amountIteration = 0;
    _minIterationTime = 100000.;
    _maxIterationTime = 0.;

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
        //qDebug() << "Добавление одного объявления составляет "<< elapsedMs << " милисекунд,  задержка между добавлениями: " << _ms << " миллисекунд.";
        _mutex->unlock();
        ++_amountIteration;
        _averageIterationTime += (double)elapsedMs;

        if((double)elapsedMs < _minIterationTime) {
            _minIterationTime = (double)elapsedMs;
        }

        if((double)elapsedMs > _maxIterationTime) {
            _maxIterationTime = (double)elapsedMs;
        }

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
    _averageIterationTime/=(double)_amountIteration;
}

void UpdateThread::stopThread()
{
    qDebug() << "Останавливаем авто обновление...";
    BullThread::stopThread();
    _averageOneupdateTime/=(double)_amountIteration;
    qDebug() << "Авто обновление остановлено.\n Обявления обновлялись "<<_amountIteration<<" раз. На данный момент накоплено "<<_board->_bulletinPaintDataList.size()
             <<" объявлений. (Было "<<_startAmountBulletins<<" объявлений к моменту старта). \nМинимальное время одного обновления составляет "<<_minIterationTime
             <<" мс, максимальное - "<<_maxIterationTime<<" мс, в среднем одна итерация обновления занимает "<<_averageIterationTime<<" мс. \nНа обновление одного объявления приходится: минимум "
             <<_minOneupdateTime<<" мс, максимум "<<_maxOneupdateTime<<" мс, и в среднем "<<_averageOneupdateTime<<" мс.\nВремя задержки между итерациями было установлено "<<_ms<<" мс.";
}

void TaskThread::stopThread()
{
    qDebug() << "Останавливаем поток, добавляющий объявления...";
    BullThread::stopThread();
    qDebug() << "Авто добавление остановлено.\n Добавлено "<<_amountIteration<<" объявлений.\nМинимальное время добавления обновления составляет "<<_minIterationTime
             <<" мс, максимальное - "<<_maxIterationTime<<" мс, в среднем одна итерация добавления занимает "<<_averageIterationTime<<" мс.\nВремя задержки между итерациями было установлено "
             <<_ms<<" мс.";
}
