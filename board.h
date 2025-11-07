#ifndef BOARD_H
#define BOARD_H

#include <QWidget>
#include <QFont>
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMutex>

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
    QJsonObject findByUser(const QString &user);
    void updateCache();
    void drawOneBulletin(BulletinPaintData data, QPainter &painter);
    void setNewBulletin(bool newBulletin) {_newBulletin = newBulletin;}
public://member
    QList<BulletinPaintData> _bulletinPaintDataList;
    QJsonArray _jsonObjectArray;
protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private://functions
    void writeDataToFile();
    void readDataFromFile();
    void setFontColor(QPainter &painter, QString color);
    void setBulletinFromJson(QJsonObject &obj);
    void cacheBulletinPaintData(QJsonObject& obj);

private://members
    QPixmap _cachePixmap;
    QString _message;
    QString _userName;
    QString _textColor;
    QFont _font;
    int _x; 
    int _y;
    double _angle;
    bool _newBulletin = false;

    //JSON-объекты:
    QJsonDocument _doc;
    QJsonObject _rootJsonObject;
    QString _fileName;
    QMutex  *_mutex;
};

#endif // BOARD_H
