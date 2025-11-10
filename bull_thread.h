#ifndef BULL_THREAD_H
#define BULL_THREAD_H

#include <QThread>
#include <QMutex>
#include <QQueue>
#include <QDebug>
#include <QWaitCondition>
#include <QList>
#include <QRunnable>
#include <QThreadPool>
#include "board.h"

const QList<QString> fam_fonts = {
    "Arial",
    "Times New Roman",
    "Courier New",
    "Verdana",
    "Tahoma"
};

const QList<QString> t_colors = {
    "Черный",
    "Красный",
    "Оранжевый",
    "Желтый",
    "Зеленый",
    "Голубой",
    "Синий",
    "Фиолетовый"
};

class BullThread : public QThread
{
    Q_OBJECT

public:
    explicit BullThread(Board *board, QMutex* mutex, int ms=1000, QObject *parent = nullptr);
    ~BullThread();
    void setMs(int ms) {_ms = ms;}
    virtual void stopThread();
    virtual void giveStatistics();
    double     _averageIterationTime = 0.;
    double     _minIterationTime = 100000.;
    double     _maxIterationTime = 0.;
    int        _amountIteration=0;
signals:
    void iamstop(const QString &str);
protected:
    virtual void run();
    int        _ms;
    Board     *_board;
    QAtomicInt _quitFlag;
    QMutex    *_mutex;

};

class UpdateThread : public BullThread
{
    Q_OBJECT
public:
    UpdateThread(Board *board, QMutex* mutex, int ms=1000, QObject *parent = nullptr);
    void giveStatistics() override;
protected:
    void run() override;
private:
    double      _averageOneupdateTime = 0.;
    double      _minOneupdateTime = 100000.;
    double      _maxOneupdateTime = 0.;
    int         _startAmountBulletins;
};

class TaskThread : public BullThread
{
    Q_OBJECT
public:
    TaskThread(Board *board, QMutex* mutex, int ms=1000, QObject *parent = nullptr);
    void giveStatistics() override;

protected:
    void run() override;
};

#endif
