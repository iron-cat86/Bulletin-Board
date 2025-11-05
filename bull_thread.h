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

class BullThread : public QThread
{
    Q_OBJECT

public:
    explicit BullThread(Board *board, QMutex* mutex, int ms=1000, QObject *parent = nullptr);
    ~BullThread();
    void stopThread();
    void setMs(int ms) {_ms = ms;}

protected:
    void run() override;

private:
    int        _ms;
    Board     *_board;
    QAtomicInt _quitFlag;
    QMutex    *_mutex;
};

#endif
