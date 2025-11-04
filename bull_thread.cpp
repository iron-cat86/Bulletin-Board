#include "bull_thread.h"
#include <QCoreApplication>

BullThread::BullThread(Board *board, int ms, QObject *parent)
    : QObject(parent), _board(board), _ms(ms)
{
    _numThreads = _board->_bulletinPaintDataList.size();
}

BullThread::~BullThread()
{
    stop();
}

void BullThread::start()
{
    qDebug() << "Starting thread pool with " << _numThreads << " threads.";
    _quitFlag.storeRelaxed(0);

    for (int i = 0; i < _numThreads; ++i) {
       addTask(i); // Используем addTask для создания начальных потоков
    }
}

void BullThread::stop()
{
    qDebug() << "Stopping thread pool gracefully...";

    _quitFlag.storeRelaxed(1);

    for (QThread* thread : _threads) {
        while(thread->isRunning()) {
            // мы позволяем Qt обрабатывать события, пока ждем.
            QCoreApplication::processEvents();
        }
    }

    qDeleteAll(_workers);
    qDeleteAll(_threads);

    _threads.clear();
    _workers.clear();
    qDebug() << "Thread pool stopped.";
}

// все потоки запускают
// однотипную бесконечную задачу (Worker::process), метод addTask()
// просто запускает еще одну такую же задачу в уже работающем пуле.
void BullThread::addTask(int id)
{
    qDebug() << "Adding one more task (thread). Total:" << _threads.count() + 1;

    QThread *thread = new QThread();
    Worker *worker = new Worker(&_mutex, &_quitFlag, id, _board, _ms);

    worker->moveToThread(thread);

    connect(thread, &QThread::started, worker, &Worker::process);

    _threads.append(thread);
    _workers.append(worker);
    thread->start();
}
