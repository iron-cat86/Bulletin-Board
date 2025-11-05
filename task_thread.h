#ifndef TASK_THREAD_H
#define TASK_THREAD_H

#include <QThread>
#include <QAtomicInt>
#include <QDebug>
#include "bull_thread.h"
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

class TaskThread : public QThread
{
    Q_OBJECT

public:
    // Конструктор принимает указатель на наш пул потоков BullThread и доску объявлений Board
    explicit TaskThread(QMutex* mutex, Board *board, int ms = 1000, QObject *parent = nullptr);
    ~TaskThread();

    void stopThread(); // Метод для безопасной остановки потока
    void setMs(int ms) {_ms = ms;}

protected:
    void run() override; // Переопределяем метод QThread::run()

private:
    int         _ms;
    Board      *_board;
    //BullThread *_threadPool; // Указатель на пул потоков
    QAtomicInt  _quitFlag;    // Флаг для управления циклом потока
    QMutex     *_mutex;
};

#endif // TASK_THREAD_H
