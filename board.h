#ifndef BOARD_H
#define BOARD_H

#include <QWidget>
#include <QFont>
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QList>
#include <QMutex>
#include <QRandomGenerator>


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


struct BulletinPaintData {
    QString user;
    QString fullText;
    QFont font;
    QString color;
    QRect boundRect;
    QPointF position; // x, y
    int size;
    double angle;
};

class Board : public QWidget
{
    Q_OBJECT

public:
    explicit Board(const QString &fileName, QMutex* mutex, QWidget *parent = nullptr);
    ~Board();
    void setMessage(const QString &text);
    void setUserName(const QString &name);
    void setFontName(const QString &fontName);
    void setTextColor(const QString &textColor);
    void setFontSize(int size);
    void setTextCoords(int x, int y);
    void setTextAngle(double angle);
    void writeData();
    QJsonObject findByUser(const QString &user, int &id, QJsonObject updatedObj = QJsonObject(), bool replace=false);
    void updateCache(bool random_mode=false);
    void drawOneBulletin(BulletinPaintData data, QPainter &painter);
    void setNewBulletin(bool newBulletin) {_newBulletin = newBulletin;}
    BulletinPaintData createNewPaintData(QJsonObject &obj);
    bool findAndUpdatePaintData(QString user, BulletinPaintData &data);
    void initNewSplash();
    void updateBulletin(bool newBul);
public://member
    QList<BulletinPaintData> _bulletinPaintDataList;
    QJsonArray _jsonObjectArray;
    QPixmap _cachePixmap;
public slots:
    void onClear();
    void onStopThread(const QString &str);
signals:
    void userDataGetted();
protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private://functions
    void writeDataToFile();
    void readDataFromFile();
    void setFontColor(QPainter &painter, QString color);
    void setBulletinFromJson(QJsonObject &obj);
    void cacheBulletinPaintData(QJsonObject& obj, int i);

private://members
    QString _message;
    QString _userName;
    QString _textColor;
    QFont _font;
    int _x; 
    int _y;
    double _angle;
    bool _newBulletin = false;

    int _flags = Qt::AlignLeft | Qt::AlignTop | Qt::TextExpandTabs;
    int _maxWidth = 200;
    int _padding = 5;

    //JSON-объекты:
    QJsonDocument _doc;
    QJsonObject _rootJsonObject;
    QString _fileName;
    QMutex  *_mutex;
};

#endif // BOARD_H
