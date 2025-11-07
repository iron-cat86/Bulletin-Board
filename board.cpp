#include "board.h"
#include <QPainter>
#include <QPaintEvent>
#include <QtMath>
#include <QDebug>
#include <QFontMetrics>
#include <QFile>

Board::Board(const QString &fileName, QMutex* mutex, QWidget *parent)
    : QWidget(parent), _x(10), _y(20), _angle(0.0), _fileName(fileName), _mutex(mutex)
{
    // Задаем минимальный размер и белый фон
    setMinimumSize(400, 300);
    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, Qt::white);
    this->setPalette(palette);
    this->setAutoFillBackground(true);
    _font.setPointSize(12);
    _font.setFamily("Arial");

    readDataFromFile();

    for (int i = 0; i < _jsonObjectArray.size(); ++i) {
        QJsonObject currentObj = _jsonObjectArray[i].toObject();
        cacheBulletinPaintData(currentObj);
    }
   // update();
}

Board::~Board()
{
    writeDataToFile();
}

void Board::setBulletinFromJson(QJsonObject &obj)
{
    _userName = obj["author"].toString();
    _message = obj["text"].toString();
    _textColor = obj["color"].toString();
    _x = obj["left"].toInt();
    _y = obj["top"].toInt();
    _angle = obj["angle"].toDouble();
    QString fullFont=obj["font"].toString();
    QString the_font=QString(fullFont[0]);
    int i=1;

    while(i<fullFont.length() && fullFont[i]!=',') {
        the_font += fullFont[i];
        ++i;
    }
    _font.setFamily(the_font);
    _font.setPointSize(obj["size"].toInt());
}

void Board::readDataFromFile()
{
    QFile file(_fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QByteArray rawData = file.readAll();
        file.close();
        _doc = QJsonDocument::fromJson(rawData);

        if (_doc.isObject() && _doc.object().contains("messages")) {
            _rootJsonObject = _doc.object();
            _jsonObjectArray = _rootJsonObject["messages"].toArray();
        }
   }
}

QJsonObject Board::findByUser(const QString &user, QJsonObject updatedObj, bool replace)
{
    for (int i = 0; i < _jsonObjectArray.size(); ++i) {
        QJsonObject currentObj = _jsonObjectArray[i].toObject();

        if (currentObj.contains("author") && currentObj["author"].toString() == user) {
            if(replace) {
                _jsonObjectArray.replace(i, updatedObj);
                qDebug() << "Найдены и обновлены данные для пользователя: " << user;
            }
            else {
                qDebug() << "Найдены данные для пользователя: " << user;
            }
            return currentObj;
        }
    }
    return QJsonObject();
}
void Board::writeData()
{
    //Подготовка нового объекта данных
    QJsonObject newDataObject;
    newDataObject["author"] = _userName;
    newDataObject["text"] = _message;
    newDataObject["color"] = _textColor;
    newDataObject["left"] = _x;
    newDataObject["top"] = _y;
    newDataObject["angle"] = _angle;
    newDataObject["font"] = _font.family();
    newDataObject["size"] = _font.pointSize();

    //Ищем запись по ключу "author" и обновляем ее
    QJsonObject updatedObject = findByUser(_userName, newDataObject, true);

    if(updatedObject["author"].toString() != _userName) {
        _jsonObjectArray.append(newDataObject);
        qDebug() << "Добавлен новый пользователь: " << _userName;
    }
    cacheBulletinPaintData(newDataObject);
}

void Board::writeDataToFile()
{
    _rootJsonObject["messages"] = _jsonObjectArray;
    _doc.setObject(_rootJsonObject);

    QFile file(_fileName);

    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        file.write(_doc.toJson(QJsonDocument::Indented));
        file.close();
        qDebug() << "Данные успешно обновлены";
    } else {
        qWarning() << "Невозможно перезаписать  данные!" << file.errorString();
    }
}

void Board::setMessage(const QString &text)
{
    _message = text;
}

void Board::setUserName(const QString &name)
{
    _userName = name;
}

void Board::setFontName(const QString &fontName)
{
    _font.setFamily(fontName);
}

void Board::setFontSize(int size)
{
    if (size > 0) {
        _font.setPointSize(size);
    }
}

void Board::setTextColor(const QString &textColor)
{
    _textColor = textColor;
}

void Board::setFontColor(QPainter &painter, QString color)
{
    QPen pen;
    if(color == "Черный") {
        pen = QPen(Qt::black);
    }
    else if(color == "Красный") {
        pen = QPen(Qt::red);
    }
    else if(color == "Оранжевый") {
        pen = QPen(QColor(255, 165, 0));
    }
    else if(color == "Желтый") {
        pen = QPen(Qt::yellow);
    }
    else if(color == "Зеленый") {
        pen = QPen(Qt::green);
    }
    else if(color == "Голубой") {
        pen = QPen(Qt::cyan);
    }
    else if(color == "Синий") {
        pen = QPen(Qt::blue);
    }
    else if(color == "Фиолетовый") {
        pen = QPen(Qt::magenta);
    }
    else {
        pen = QPen(Qt::black);
    }
    painter.setPen(pen);
}

void Board::setTextCoords(int x, int y)
{
    _x = x;
    _y = y;
}

void Board::setTextAngle(double angle)
{
    _angle = angle;
}

void Board::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, _cachePixmap);
}

void Board::resizeEvent(QResizeEvent *event)
{
    _mutex->lock();
    setNewBulletin(false);
    QWidget::resizeEvent(event);
    updateCache();
    update();
    _mutex->unlock();
}
void Board::initNewSplash()
{
    if (_cachePixmap.size() != this->size()) {
        _cachePixmap = QPixmap(this->size());
    }
    _cachePixmap.fill(Qt::white);
}

void Board::updateCache()
{
    if(!_newBulletin) {
        initNewSplash();
    }
    QPainter painter(&_cachePixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setClipRect(this->rect());
    painter.setClipping(true);

    if(!_newBulletin) {
        for (const auto& data : _bulletinPaintDataList) {
           drawOneBulletin(data, painter);
        }
    }
    else {
        drawOneBulletin(_bulletinPaintDataList.last(), painter);
    }
}

void Board::drawOneBulletin(BulletinPaintData data, QPainter &painter)
{
    painter.setFont(data.font);
    setFontColor(painter, data.color);

    painter.translate(data.position);
    painter.rotate(-data.angle);
    painter.drawRect(data.boundRect);
    painter.drawText(data.boundRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextExpandTabs, data.fullText);

    painter.resetTransform();
}

BulletinPaintData Board::createNewPaintData(QJsonObject &obj)
{
    QString fullText = QString("%1: %2").arg(obj["author"].toString()).arg(obj["text"].toString());
    QFont font;
    font.setFamily(obj["font"].toString());
    font.setPointSize(obj["size"].toInt());
    QFontMetrics fm(font);
    QRect boundRect = fm.boundingRect(0, 0, _maxWidth, 10000, _flags, fullText);
    boundRect.adjust(-_padding, -_padding, _padding, _padding);

    BulletinPaintData data;
    data.user = obj["author"].toString();
    data.fullText = fullText;
    data.font = font;
    data.color = obj["color"].toString();
    data.boundRect = boundRect;
    data.position = QPointF(obj["left"].toInt(), obj["top"].toInt());
    data.angle = obj["angle"].toDouble();
    return data;
}

bool Board::findAndUpdatePaintData(QString user, BulletinPaintData &data)
{
    for (int i = 0; i < _bulletinPaintDataList.size(); ++i) {
        if (_bulletinPaintDataList[i].user == user) {
           _bulletinPaintDataList[i] = data;
           return true;
       }
    }
    return false;
}

void Board::cacheBulletinPaintData(QJsonObject& obj)
{
    _mutex->lock();

    if(obj["author"].toString() != "" && obj["text"].toString() != "") {
        BulletinPaintData data = createNewPaintData(obj);

        bool found = findAndUpdatePaintData(obj["author"].toString(), data);

        if (!found) {
            setNewBulletin(true);
            _bulletinPaintDataList.append(data);
        }
        else {
            setNewBulletin(false);
        }
        updateCache();
    }
    update();
    _mutex->unlock();
}
