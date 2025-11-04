#include "board.h"
#include <QPainter>
#include <QPaintEvent>
#include <QtMath>
#include <QDebug>
#include <QFontMetrics>
#include <QFile>

Board::Board(const QString &fileName, QWidget *parent)
    : QWidget(parent), _x(10), _y(20), _angle(0.0), _fileName(fileName)
{
    // Задаем минимальный размер и белый фон
    setMinimumSize(400, 300);
    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, Qt::white);
    this->setPalette(palette);
    //чтобы палитра применялась автоматически
    this->setAutoFillBackground(true);
    _font.setPointSize(12);
    _font.setFamily("Arial");

    readDataFromFile();

    for (int i = 0; i < _jsonObjectArray.size(); ++i) {
        QJsonObject currentObj = _jsonObjectArray[i].toObject();
        cacheBulletinPaintData(currentObj);
    }
    update();
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

        // Если файл валиден, извлекаем массив сообщений
        if (_doc.isObject() && _doc.object().contains("messages")) {
            _rootJsonObject = _doc.object();
            _jsonObjectArray = _rootJsonObject["messages"].toArray();
        }
   }
}

QJsonObject Board::findByUser(const QString &user)
{
    for (int i = 0; i < _jsonObjectArray.size(); ++i) {
        QJsonObject currentObj = _jsonObjectArray[i].toObject();
        // Проверяем наличие ключа "author" и совпадение значения с _userName
        if (currentObj.contains("author") && currentObj["author"].toString() == user) {
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
    cacheBulletinPaintData(newDataObject);

    bool foundAndUpdated = false;

    //Парсинг существующего файла
    //Ищем запись по ключу "author" и обновляем ее
    for (int i = 0; i < _jsonObjectArray.size(); ++i) {
        QJsonObject currentObj = _jsonObjectArray[i].toObject();
        // Проверяем наличие ключа "author" и совпадение значения с _userName
        if (currentObj.contains("author") && currentObj["author"].toString() == _userName) {
            // Нашли совпадение! Заменяем весь объект новым (сохраняем user неизменным)
            QJsonObject updatedObject = newDataObject;
            updatedObject["author"] = _userName; // Гарантируем, что user не изменится

            _jsonObjectArray.replace(i, updatedObject);
            foundAndUpdated = true;
            qDebug() << "Найдены и обновлены данные для пользователя: " << _userName;
            break; // Выходим из цикла после обновления
        }
    }
    //Если не нашли, добавляем новый объект
    if (!foundAndUpdated) {
        _jsonObjectArray.append(newDataObject);
        qDebug() << "Добавлен новый пользователь: " << _userName;
    }
    update();
}

void Board::writeDataToFile()
{
    //Запись ОБНОВЛЕННОГО массива обратно в файл ---
    _rootJsonObject["messages"] = _jsonObjectArray;
    _doc.setObject(_rootJsonObject);

    QFile file(_fileName);
    // Открываем файл в режиме перезаписи (WriteOnly | Truncate)
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        file.write(_doc.toJson(QJsonDocument::Indented)); // Записываем ВЕСЬ документ целиком
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
    if(color == "Белый") {
        pen = QPen(Qt::white);
    }
    else if(color == "Черный") {
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
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(event->rect(), Qt::white); // Заливаем фон белым

    //Цикл рисует кэшированные данные
    for (const auto& data : _bulletinPaintDataList) {
        // Устанавливаем готовые параметры
        painter.setFont(data.font);
        setFontColor(painter, data.color);
        // Переносим систему координат
        painter.translate(data.position);
        painter.rotate(-data.angle);

        // Отрисовываем рамку и текст, используя кэшированный boundRect
        painter.drawRect(data.boundRect);
        painter.drawText(data.boundRect, Qt::AlignLeft | Qt::AlignTop | Qt::TextExpandTabs, data.fullText);

        painter.resetTransform(); // Восстанавливаем состояние
    }
}

void Board::cacheBulletinPaintData(QJsonObject& obj)
{
    setBulletinFromJson(obj); // Обновляет _userName, _message, _x, _y, _angle, _font

    if(_userName != "" && _message != "") {
        QString fullText = QString("%1: %2").arg(_userName).arg(_message);

        // Расчеты (flags, maxWidth, padding)
        int flags = Qt::AlignLeft | Qt::AlignTop | Qt::TextExpandTabs;
        int maxWidth = 200;
        int padding = 5;

        QFontMetrics fm(_font);
        QRect boundRect = fm.boundingRect(0, 0, maxWidth, 10000, flags, fullText);
        boundRect.adjust(-padding, -padding, padding, padding);

        // Сохраняем все в структуру
        BulletinPaintData data;
        data.user = _userName;
        data.fullText = fullText;
        data.font = _font;
        data.color = _textColor;
        data.boundRect = boundRect;
        data.position = QPointF(_x, _y);
        data.angle = _angle;

        bool found = false;
        // Ищем элемент в списке по user
        for (int i = 0; i < _bulletinPaintDataList.size(); ++i) {
            if (_bulletinPaintDataList[i].user == _userName) {
                    // Нашли! Заменяем существующий элемент новым
               _bulletinPaintDataList[i] = data;
               found = true;
               break; // Выходим из цикла после обновления
           }
        }
        // Если элемент не найден, добавляем его как новый
        if (!found) {
            _bulletinPaintDataList.append(data);
        }
    }
}
