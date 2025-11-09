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
}

void BullThread::stopThread()
{
    _quitFlag.storeRelaxed(1);
}

void BullThread::giveStatistics()
{
    _averageIterationTime/=(double)_amountIteration;
}

void UpdateThread::giveStatistics()
{
    qDebug() << "Останавливаем авто обновление...";

    BullThread::giveStatistics();
    _averageOneupdateTime/=(double)_amountIteration;
    QString answ;
    QTextStream qtext(&answ);
    qtext<<QString::fromUtf8("Обявления обновлялись ")<<_amountIteration
         <<QString::fromUtf8(" раз. На данный момент накоплено ")<<_board->_bulletinPaintDataList.size()
         <<QString::fromUtf8(" объявлений. (Было ")<<_startAmountBulletins
         <<QString::fromUtf8(" объявлений к моменту старта). \nМинимальное время одного обновления составляет ")<<_minIterationTime
         <<QString::fromUtf8(" мс, максимальное - ")<<_maxIterationTime
         <<QString::fromUtf8(" мс, в среднем одна итерация обновления занимает ")<<_averageIterationTime
         <<QString::fromUtf8(" мс. \nНа обновление одного объявления приходится: минимум ")<<_minOneupdateTime
         <<QString::fromUtf8(" мс, максимум ")<<_maxOneupdateTime
         <<QString::fromUtf8(" мс, и в среднем ")<<_averageOneupdateTime
         <<QString::fromUtf8(" мс.\nВремя задержки между итерациями было установлено ")<<_ms
         <<QString::fromUtf8(" мс.");
    emit iamstop(answ);
    qDebug() << "Авто обновление остановлено.";
}

void TaskThread::giveStatistics()
{
    qDebug() << "Останавливаем поток, добавляющий объявления...";
    BullThread::giveStatistics();
    QString answ;
    QTextStream qtext(&answ);
    qtext<<QString::fromUtf8("Авто добавление остановлено.\n Добавлено ")<<_amountIteration
         <<QString::fromUtf8(" объявлений.\nМинимальное время добавления обновления составляет ")<<_minIterationTime
         <<QString::fromUtf8(" мс, максимальное - ")<<_maxIterationTime
         <<QString::fromUtf8(" мс, в среднем одна итерация добавления занимает ")<<_averageIterationTime
         <<QString::fromUtf8(" мс.\nВремя задержки между итерациями было установлено ")<<_ms
         <<QString::fromUtf8(" мс.");
    emit iamstop(answ);
    qDebug()<<"Авто добавление остановлено.";
}
