#include "bull_thread.h"
#include <QCoreApplication>

BullThread::BullThread(Board *board, QMutex* mutex, int ms, QObject *parent)
    : QObject(parent), _board(board), _mutex(mutex), _ms(ms)
{
    _numThreads = _board->_bulletinPaintDataList.size();
}

BullThread::~BullThread()
{
    stop();
}

void BullThread::start()
{
    qDebug() << "Запускаем авто обновление " << _numThreads << " объявлений.";
    _quitFlag.storeRelaxed(0);

    for (int i = 0; i < _numThreads; ++i) {
       addTask(i); // Используем addTask для создания начальных потоков
    }
}

void BullThread::stop()
{
    qDebug() << "Останавливаем авто обновление...";

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
    qDebug() << "Авто обновление остановлено.";
}

// все потоки запускают
// однотипную бесконечную задачу (Worker::process), метод addTask()
// просто запускает еще одну такую же задачу в уже работающем пуле.
void BullThread::addTask(int id)
{
    if(id >= _numThreads) {
        ++_numThreads;
    }
    qDebug() << "Добавлено еще одно объявление для обновления. Всего:" << _threads.count() + 1;

    QThread *thread = new QThread();
    Worker *worker = new Worker(_mutex, &_quitFlag, id, _board, _ms);

    worker->moveToThread(thread);

    connect(thread, &QThread::started, worker, &Worker::process);

    _threads.append(thread);
    _workers.append(worker);
    thread->start();
}
