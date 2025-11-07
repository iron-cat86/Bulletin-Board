#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPainter>
#include <QDateTime>
#include <QMutex>
#include <QMutexLocker>

const QString SETTINGS_FILE = "bulletins.json";

QTextEdit* MainWindow::s_logBrowser = nullptr;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
    createMenus();
    s_logBrowser = _logBrowser;
    qInstallMessageHandler(customMessageHandler);
}

MainWindow::~MainWindow()
{
    _tasks->stopThread();
    _updateThread->stopThread();
    qInstallMessageHandler(nullptr);
}

void MainWindow::showHelpMessage()
{
    QMessageBox::information(
        this,
        "Приложение \"Доска объявлений\"",
        "Приложение имитирует браузер добавления ваших объявлений\n\n1. Чтобы создать новое объявление, задайте свое уникальное имя пользователя, и все остальные настройки.\nВведите текст сообщения, нажмите кнопку \"Получить данные\" (это необходимо для безопасности, чтобы случайно\nне перезаписать существующее объявление), затем \"Отправить сообщение\"\n2. Чтобы редактировать существующее объявление, повторите действия п. 1, и если объявление существует, кнопка \n\"Отправить сообщение\" станет кнопкой \"Редактировать\", и в полях ввода будут ваши заданные настройки, \nкоторые вы можете изменить.\n3. Чтобы потестировать наше приложение в экстремальном режиме, нажмите кнопку \"Старт авто\": случайным образом будут \nобновляться существующие объявления и добавляться новые. \nВНИМАНИЕ: ваши объявления в этом режиме будут измененены!\nПриятного использования!"
    );
}

void MainWindow::createMenus()
{
    _aboutAction = new QAction(tr("&О приложении"), this);

    connect(_aboutAction, &QAction::triggered, this, &MainWindow::showHelpMessage);

    _helpMenu = menuBar()->addMenu(tr("&Помощь"));
    _helpMenu->addAction(_aboutAction);
}

void MainWindow::customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);

    if (s_logBrowser) {
       QString color = "black";
       switch (type) {
           case QtDebugMsg:
               color = "black";
               break;
           case QtWarningMsg:
               color = "orange";
               break;
           case QtCriticalMsg:
           case QtFatalMsg:
           case QtInfoMsg:
               color = "red";
               break;
       }

       QString formattedMsg = QString("<span style=\"color:%1;\">%2</span><br>").arg(color, msg);

       QMetaObject::invokeMethod(s_logBrowser, "insertHtml",
                                 Qt::QueuedConnection,
                                 Q_ARG(QString, formattedMsg));
    }
}

void MainWindow::appendLogMessage(QtMsgType type, const QString &msg)
{
    if (!_logBrowser) return;

    QString color = "black";
    switch (type) {
        case QtDebugMsg:
            color = "gray";
            break;
        case QtWarningMsg:
            color = "orange";
            break;
        case QtCriticalMsg:
        case QtFatalMsg:
            color = "red";
            break;
        default:
            break;
    }

    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss");
    QString logMessage = QString("<span style=\"color:%1;\">[%2] %3</span>").arg(color, timestamp, msg);

    _logBrowser->append(logMessage);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->accept();
}


void MainWindow::setupUi()
{
    _centralWidget = new QWidget(this);
    _mainLayout = new QGridLayout(_centralWidget);

    // 0. Область отображения
    _board = new Board(SETTINGS_FILE, &_mutex, this);
    // Занимает 3 строки, 2 столбца в макете
    _mainLayout->addWidget(_board, 0, 0, 3, 2); 

    // Левый контейнер
    QWidget *leftContainer = new QWidget(this);
    // Устанавливаем серый, непрозрачный фон
    leftContainer->setStyleSheet("background-color: lightgray;");
    QVBoxLayout *leftLayout = new QVBoxLayout(leftContainer);
    leftLayout->setContentsMargins(0, 0, 0, 0); // Убираем лишние отступы

    // Правый контейнер
    QWidget *rightContainer = new QWidget(this);
    // Устанавливаем серый, непрозрачный фон
    rightContainer->setStyleSheet("background-color: lightgray;");
    QVBoxLayout *rightLayout = new QVBoxLayout(rightContainer);
    rightLayout->setContentsMargins(0, 0, 0, 0); // Убираем лишние отступы

    // Добавляем контейнеры в основную сетку (во вторую строку)
    _mainLayout->addWidget(leftContainer, 4, 0, 1, 1);
    _mainLayout->addWidget(rightContainer, 4, 1, 1, 1);
    // left-1. Поле ввода имени пользователя
    QHBoxLayout *userLayout = new QHBoxLayout();
    userLayout->addWidget(new QLabel("Имя пользователя:", this));
    _userNameEdit = new QLineEdit("Гость", this);
    _userNameEdit->setStyleSheet("background-color: white;");
    _userNameEdit->installEventFilter(this);
    userLayout->addWidget(_userNameEdit);
    leftLayout->addLayout(userLayout);

    // left-2. Вариант шрифта
    QHBoxLayout *fontLayout = new QHBoxLayout();
    fontLayout->addWidget(new QLabel("Шрифт:", this));
    _fontComboBox = new QComboBox(this);
    _fontComboBox->addItem("Arial");
    _fontComboBox->addItem("Times New Roman");
    _fontComboBox->addItem("Courier New");
    _fontComboBox->addItem("Verdana");
    _fontComboBox->addItem("Tahoma");
    _fontComboBox->setStyleSheet("background-color: white;");
    fontLayout->addWidget(_fontComboBox);
    leftLayout->addLayout(fontLayout);

    // left-3. Поле ввода размера шрифта
    QHBoxLayout *sizeLayout = new QHBoxLayout();
    sizeLayout->addWidget(new QLabel("Размер шрифта:", this));
    _fontSizeEdit = new QLineEdit("12", this);
    _fontSizeEdit->setStyleSheet("background-color: white;");
    sizeLayout->addWidget(_fontSizeEdit);
    leftLayout->addLayout(sizeLayout);

    // left-4. Поле ввода цвета шрифта
    QHBoxLayout *colorLayout = new QHBoxLayout();
    colorLayout->addWidget(new QLabel("Цвет шрифта:", this));
    _colorComboBox = new QComboBox(this);
    _colorComboBox->addItem("Черный");
    _colorComboBox->addItem("Красный");
    _colorComboBox->addItem("Оранжевый");
    _colorComboBox->addItem("Желтый");
    _colorComboBox->addItem("Зеленый");
    _colorComboBox->addItem("Голубой");
    _colorComboBox->addItem("Синий");
    _colorComboBox->addItem("Фиолетовый");
    _colorComboBox->setStyleSheet("background-color: white;");
    colorLayout->addWidget(_colorComboBox);
    leftLayout->addLayout(colorLayout);

    // left-5. Поля ввода координаты x
    QHBoxLayout *xLayout = new QHBoxLayout();
    xLayout->addWidget(new QLabel("Координата X:", this));
    _coordXEdit = new QLineEdit("10", this);
    _coordXEdit->setStyleSheet("background-color: white;");
    xLayout->addWidget(_coordXEdit);
    leftLayout->addLayout(xLayout);
    
    //left-6. Поле ввода координаты y
    QHBoxLayout *yLayout = new QHBoxLayout();
    yLayout->addWidget(new QLabel("Координата Y:", this));
    _coordYEdit = new QLineEdit("20", this);
    _coordYEdit->setStyleSheet("background-color: white;");
    yLayout->addWidget(_coordYEdit);
    leftLayout->addLayout(yLayout);

    // right-1. Поле ввода угла наклона
    QHBoxLayout *angleLayout = new QHBoxLayout();
    angleLayout->addWidget(new QLabel("Угол наклона (град):", this));
    _angleEdit = new QLineEdit("0", this);
    _angleEdit->setStyleSheet("background-color: white;");
    angleLayout->addWidget(_angleEdit);
    rightLayout->addLayout(angleLayout);

    // right-2. Поле ввода количества новых сообщений в секунду.
    QHBoxLayout *newMsgRateLayout = new QHBoxLayout();
    newMsgRateLayout->addWidget(new QLabel("Скорость ввода новых сообщений (1/сек.):", this));
    _newMsgRateEdit = new QLineEdit("1", this);
    _newMsgRateEdit->setStyleSheet("background-color: white;");
    newMsgRateLayout->addWidget(_newMsgRateEdit);
    rightLayout->addLayout(newMsgRateLayout);

    // right-3. Поле ввода количества изменений существующих сообщений в секунду.
    QHBoxLayout *editMsgRateLayout = new QHBoxLayout();
    editMsgRateLayout->addWidget(new QLabel("Скорость изменений (1/сек.):", this));
    _editMsgRateEdit = new QLineEdit("1", this);
    _editMsgRateEdit->setStyleSheet("background-color: white;");
    editMsgRateLayout->addWidget(_editMsgRateEdit);
    rightLayout->addLayout(editMsgRateLayout);

    // right-4. Кнопка запуска/остановки автоматического обновления.
    _autoUpdateButton = new QPushButton("Старт авто", this);
    // Кнопка займет всю ширину
    rightLayout->addWidget(_autoUpdateButton);

    // right-5. Кнопка "Получить мои данные".
    _getMyDataButton = new QPushButton("Получить мои данные", this);
    // Кнопка займет всю ширину
    rightLayout->addWidget(_getMyDataButton);

    QWidget *bottomContainer = new QWidget(this);
    bottomContainer->setStyleSheet("background-color: lightgray;");
    QVBoxLayout *bottomLayout = new QVBoxLayout(bottomContainer);
    //bottom-1. Поле ввода текста сообщения
    bottomLayout->addWidget(new QLabel("Сообщение:", this));
    _bulletinEdit = new QTextEdit(this);
    _bulletinEdit->setStyleSheet("background-color: white;");
    bottomLayout->addWidget(_bulletinEdit);

    // bottom-2. Кнопка «Отправить сообщение»
    _sendButton = new QPushButton("Отправить сообщение", this);
    _sendButton->setEnabled(false);
    bottomLayout->addWidget(_sendButton);

    //bottom-3. Браузер для логов.
    _logBrowser = new QTextEdit(this);
    _logBrowser->setReadOnly(true); // Только для чтения
    bottomLayout->addWidget(_logBrowser);

    _mainLayout->addWidget(bottomContainer, 5, 0, 1, 2);
    bottomLayout->setStretch(bottomLayout->count() - 1, 1);

    _mainLayout->addWidget(bottomContainer, 5, 0, 1, 2);

    //Устанавливаем растяжение
    _mainLayout->setRowStretch(0, 3);
    _mainLayout->setRowStretch(5, 1);

    setCentralWidget(_centralWidget);
    setWindowTitle("Доска объявлений");

    connect(_sendButton, &QPushButton::clicked, this, &MainWindow::updateBulletin);
    connect(_getMyDataButton, &QPushButton::clicked, this, &MainWindow::getMyData);
    connect(_userNameEdit, &QLineEdit::selectionChanged, this, &MainWindow::blockSendButton);
    connect(this, &MainWindow::cursorFocused, this, &MainWindow::blockSendButton);
    connect(_autoUpdateButton, &QPushButton::clicked, this, &MainWindow::onStartOrStopButton);

    _updateThread = new BullThread(_board, &_mutex, 1000, this);
    _tasks = new TaskThread(&_mutex, _board, 1000, this);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == _userNameEdit && event->type() == QEvent::FocusIn)
    {
        emit cursorFocused();
        qDebug() << "Курсор установлен в поле редактирования имени пользователя!\n"
                  <<"Для безопасности сохранения данных возможность изменить их отключена.\n"
                  <<"Чтобы вернуться к редактированию, нажмите кнопку \"Получить мои данные\"";
        return false;
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::onStartOrStopButton()
{
    if(_autoUpdateButton->text() == "Старт авто")
    {
        _autoUpdateButton->setText("Стоп авто");
        _userNameEdit->setEnabled(false);
        _fontComboBox->setEnabled(false);
        _colorComboBox->setEnabled(false);
        _coordXEdit->setEnabled(false);
        _coordYEdit->setEnabled(false);
        _angleEdit->setEnabled(false);
        _fontSizeEdit->setEnabled(false);
        _newMsgRateEdit->setEnabled(false);
        _editMsgRateEdit->setEnabled(false);
        _bulletinEdit->setEnabled(false);
        _getMyDataButton->setEnabled(false);
        _sendButton->setEnabled(false);
        _board->setNewBulletin(false);

         //Обновление
        QString amountInSecStr = _editMsgRateEdit->text();
        int ms = 1000;

        if(amountInSecStr != "") {
            int amountInSec = amountInSecStr.toInt();
            ms = 1000/amountInSec;
        }
        _updateThread->setMs(ms);
        _updateThread->start();
        //Добавление
        QString addInSecStr = _newMsgRateEdit->text();
        int newms = 1000;

        if(addInSecStr != "") {
            int addInSec = addInSecStr.toInt();
            newms = 1000/addInSec;
        }
        _tasks->setMs(newms);
        _tasks->start();
    }
    else
    {
        _autoUpdateButton->setText("Старт авто");
        _userNameEdit->setEnabled(true);
        _fontComboBox->setEnabled(true);
        _colorComboBox->setEnabled(true);
        _coordXEdit->setEnabled(true);
        _coordYEdit->setEnabled(true);
        _angleEdit->setEnabled(true);
        _fontSizeEdit->setEnabled(true);
        _newMsgRateEdit->setEnabled(true);
        _editMsgRateEdit->setEnabled(true);
        _bulletinEdit->setEnabled(true);
        _getMyDataButton->setEnabled(true);
        _tasks->stopThread();
        _updateThread->stopThread();
    }
    _autoUpdateButton->update();
}

void MainWindow::blockSendButton()
{
    _sendButton->setText("Отправить сообщение");
    _sendButton->setEnabled(false);
    _sendButton->update();
    _board->setNewBulletin(true);
}

void MainWindow::updateBulletin()
{
    _board->setMessage(_bulletinEdit->toPlainText());
    _board->setUserName(_userNameEdit->text());
    _board->setFontName(_fontComboBox->currentText());
    
    bool ok;
    int size = _fontSizeEdit->text().toInt(&ok);
    
    if (ok) {
        _board->setFontSize(size);
    }
    else {
        qWarning()<<"Не удалось получить новый размер для шрифта!";
    }
    _board->setTextColor(_colorComboBox->currentText());
    int x = _coordXEdit->text().toInt(&ok);

    if(!ok) {
        qWarning()<<"Не удалось получить новую координату x! x=0";
        x=0;
    }
    int y = _coordYEdit->text().toInt(&ok);

    if(!ok) {
        qWarning()<<"Не удалось получить новую координату y! y=0";
        y=0;
    }
    _board->setTextCoords(x, y);

    double angle = _angleEdit->text().toDouble(&ok);

    if (ok) {
        _board->setTextAngle(angle);
    }
    else {
        qWarning()<<"Не удалось получить новый угол поворота!";
    }
    _board->writeData();
}

void MainWindow::getMyData()
{
    if(_userNameEdit->text() == "")
    {
        qErrnoWarning("Введите имя пользователя!");
        return;
    }

    QJsonObject findObj = _board->findByUser(_userNameEdit->text());

    if(findObj.contains("author") && findObj["author"].toString() != "")
    {
        _userNameEdit->setText(findObj["author"].toString());
        if(findObj.contains("font")) _fontComboBox->setCurrentText(findObj["font"].toString());
        else qWarning("Данные повреждены! Нет значения для шрифта");
        if(findObj.contains("color")) _colorComboBox->setCurrentText(findObj["color"].toString());
        else qWarning("Данные повреждены! Нет значения для цвета");
        if(findObj.contains("size")) _fontSizeEdit->setText(QString::number(findObj["size"].toInt()));
        else qWarning("Данные повреждены! Нет значения для размера");
        if(findObj.contains("left")) _coordXEdit->setText(QString::number(findObj["left"].toInt()));
        else qWarning("Данные повреждены! Нет значения для x-координаты");
        if(findObj.contains("top")) _coordYEdit->setText(QString::number(findObj["top"].toInt()));
        else qWarning("Данные повреждены! Нет значения для y-координаты");
        if(findObj.contains("angle")) _angleEdit->setText(QString::number(findObj["angle"].toDouble()));
        else qWarning("Данные повреждены! Нет значения для угла поворота");
        if(findObj.contains("text")) _bulletinEdit->setText(findObj["text"].toString());
        else qWarning("Данные повреждены! Нет вашего объявления");

        _sendButton->setText("Редактировать");
        _board->setNewBulletin(false);
    }
    else {
        qWarning()<<"Пользователь с именем "<<_userNameEdit->text()<<" не найден!";
        _sendButton->setText("Отправить сообщение");
        _board->setNewBulletin(true);
    }
    _sendButton->setEnabled(true);
    _sendButton->update();
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    emit shown();
}
