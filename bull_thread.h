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
    virtual void stopThread();
    void setMs(int ms) {_ms = ms;}
protected:
    virtual void run();// override;

//private:
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
    virtual void stopThread() override;
protected:
    void run() override;
};

class TaskThread : public BullThread
{
    Q_OBJECT
public:
    TaskThread(Board *board, QMutex* mutex, int ms=1000, QObject *parent = nullptr);

    virtual void stopThread() override;
protected:
    void run() override;
};

#endif
