#include "task_thread.h"
#include <QCoreApplication> // Нужен для обработки событий в stopThread()

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
        int id = _board->_bulletinPaintDataList.size();
        _mutex->unlock();
        // Вызываем метод добавления задачи у нашего пула
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

        BulletinPaintData data;
        data.user = user;
        data.fullText = QString("%1: %2").arg(user).arg(text);
        data.color = color;
        // Расчеты (flags, maxWidth, padding)
        int flags = Qt::AlignLeft | Qt::AlignTop | Qt::TextExpandTabs;
        int maxWidth = 200;
        int padding = 5;

        QFont font;
        font.setFamily(t_font);
        font.setPointSize(size);
        data.font = font;
        QFontMetrics fm(font);
        QRect boundRect = fm.boundingRect(0, 0, maxWidth, 10000, flags, data.fullText);
        boundRect.adjust(-padding, -padding, padding, padding);
        data.boundRect = boundRect;
        data.position = QPointF(x, y);
        data.angle = angle;

        _mutex->lock();
        _board->_jsonObjectArray.append(newDataObject);
        _board->_bulletinPaintDataList.append(data);
        _board->updateCache();
        _board->update();
        /*/ Эта операция потокобезопасна благодаря мьютексу внутри BullThread/Worker
        if (_threadPool) {
            _threadPool->addTask(id);
        }*/
        _mutex->unlock();

        QThread::msleep(_ms);
    }
    qDebug() << "Авто добавление новых объявлений запущено.";
}
