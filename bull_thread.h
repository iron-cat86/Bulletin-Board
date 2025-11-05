#ifndef BULL_THREAD_H
#define BULL_THREAD_H

#include <QThread>
#include <QMutex>
#include <QQueue>
#include <QDebug>
#include <QWaitCondition>
#include <QList>
#include <QRandomGenerator>
#include <QRunnable>
#include <QThreadPool>
#include "board.h"

const QList<QString> randomBulletins = {
    "Ищу собутыльника на пятницу.\nВася.\nКвартира 3, тел 6547",
    "Я закодировался!!!\nВася",
    "Продам нервную систему.\nСрочно!\nНедорого.",
    "Рыба моей мечты,\nприди и пожарься!\nТел. 555-01",
    "Меняю жену-программиста на две жены-сисадмина.\nОлег",
    "Научу кота молчать.\nДорого.\nС гарантией.",
    "Объявление нечитабельно.\nПозвоните по номеру 123.",
    "Не хочу ничего решать.\nХочу на ручки и печеньку.",
    "Учу английский.\nУже могу сказать:\nHelp me, I am in danger.",
    "Пропала совесть.\nНашедшему просьба не возвращать.\nМне и без нее неплохо.",
    "Ремонтирую память.\nДорого.\nВспоминаем вместе.",
    "Собираю сплетни.\nДешево. Конфиденциально.",
    "Ищу смысл жизни.\nЖелательно в твердой валюте.",
    "Продам будильник.\nБудит мертвых.\nПроверено.",
    "Куплю справку о том, что я здоров.\nСрочно.",
    "Осторожно!\nЗлая собака.\nДобрая только по пятницам.",
    "Не читайте это объявление.\nЭто секрет.",
    "Хочу быть деревом.\nВесной распускаться,\nосенью желтеть.",
    "Улыбнитесь!\nЭто приказ!",
    "Мыло ручной работы.\nНатуральное.\nПахнет кактус."
};
// Worker класс, выполняющий задачу в отдельном потоке
class Worker : public QObject
{
    Q_OBJECT
public:
    Worker(QMutex* mutex, QAtomicInt* quitFlag, int id, Board *board, int ms=1000) : _mutex(mutex), _quitFlag(quitFlag), _id(id), _board(board), _ms(ms) {}
public slots:
    void process() {
        while (_quitFlag->loadRelaxed() == 0) {
                int random_number = QRandomGenerator::global()->bounded(20);
                QString text = randomBulletins[random_number];
                _mutex->lock();
                QJsonObject newobj = _board->_jsonObjectArray[_id].toObject();
                newobj["text"]=text;
                _board->_jsonObjectArray.replace(_id, newobj);
                QString fullText = QString("%1: %2").arg(newobj["author"].toString()).arg(text);

                // Расчеты (flags, maxWidth, padding)
                int flags = Qt::AlignLeft | Qt::AlignTop | Qt::TextExpandTabs;
                int maxWidth = 200;
                int padding = 5;

                QFont font;
                font.setFamily(newobj["font"].toString());
                font.setPointSize(newobj["size"].toInt());
                QFontMetrics fm(font);
                QRect boundRect = fm.boundingRect(0, 0, maxWidth, 10000, flags, fullText);
                boundRect.adjust(-padding, -padding, padding, padding);

                // Сохраняем все в структуру
                bool found = false;
                // Ищем элемент в списке по user
                for (int i = 0; i < _board->_bulletinPaintDataList.size(); ++i) {
                    if (_board->_bulletinPaintDataList[i].user == newobj["author"].toString()) {
                            // Нашли! Заменяем существующий элемент новым
                       _board->_bulletinPaintDataList[i].boundRect = boundRect;
                       _board->_bulletinPaintDataList[i].fullText = fullText;
                       break; // Выходим из цикла после обновления
                   }
                }
                _board->updateCache();
                _board->update();
                _mutex->unlock();

                // Небольшая задержка
                QThread::msleep(_ms);
            }
       }

private:
    int         _id;
    int         _ms;
    QMutex*     _mutex;
    QAtomicInt *_quitFlag;
    Board      *_board;
};

// Основной класс пула потоков
class BullThread : public QObject
{
    Q_OBJECT

public:
    explicit BullThread(Board *board, QMutex* mutex, int ms=1000, QObject *parent = nullptr);
    ~BullThread();

    void start();
    void stop();
    void addTask(int id); // Добавляет задачу в пул
    void setMs(int ms) {_ms = ms;}
    int getAmountThreads() {return _numThreads;}

private:
    QList<QThread*> _threads;
    QList<Worker*>  _workers;
    int             _numThreads;
    int             _ms;
    Board          *_board;
    QAtomicInt      _quitFlag;
    QMutex         *_mutex; // Общий мьютекс для всех потоков
};

#endif
