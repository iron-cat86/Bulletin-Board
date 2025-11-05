#include "bull_thread.h"
#include <QCoreApplication>

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
        for (int i = 0; i < _board->_jsonObjectArray.size(); ++i) {
            int random_number = QRandomGenerator::global()->bounded(20);
            QString text = randomBulletins[random_number];
            QJsonObject newobj = _board->_jsonObjectArray[i].toObject();
            newobj["text"]=text;
            _board->_jsonObjectArray.replace(i, newobj);
            QString fullText = QString("%1: %2").arg(newobj["author"].toString()).arg(text);

            int flags = Qt::AlignLeft | Qt::AlignTop | Qt::TextExpandTabs;
            int maxWidth = 200;
            int padding = 5;

            QFont font;
            font.setFamily(newobj["font"].toString());
            font.setPointSize(newobj["size"].toInt());
            QFontMetrics fm(font);
            QRect boundRect = fm.boundingRect(0, 0, maxWidth, 10000, flags, fullText);
            boundRect.adjust(-padding, -padding, padding, padding);

            bool found = false;
            // Ищем элемент в списке по совпадению author == user
            for (int j = 0; j < _board->_bulletinPaintDataList.size(); ++j) {
                if (_board->_bulletinPaintDataList[j].user == newobj["author"].toString()) {
                    _board->_bulletinPaintDataList[j].boundRect = boundRect;
                    _board->_bulletinPaintDataList[j].fullText = fullText;
                    break;
                }
            }
        }
        _board->updateCache();
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
