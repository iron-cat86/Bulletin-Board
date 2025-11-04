#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include "board.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QCloseEvent>
#include <QTextEdit>
#include <QDebug>

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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    // Статическая функция для перехвата сообщений Qt
    // Статический указатель на виджет логов
    static QTextEdit *s_logBrowser;
    static void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
signals:
    //пользовательский сигнал
    void cursorFocused();
private slots:
    void updateBulletin();
    void getMyData();
    void blockSendButton();
    void onStartOrStopButton();

protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private://functionn
    void setupUi();
    // Метод для безопасного добавления текста в QTextEdit из обработчика
    void appendLogMessage(QtMsgType type, const QString &msg);
private://members
    QTextEdit   *_logBrowser; //виджет для отображения логов
    // UI elements
    Board       *_board;
    QLineEdit   *_userNameEdit;
    QComboBox   *_fontComboBox;
    QComboBox   *_colorComboBox;
    QLineEdit   *_fontSizeEdit;
    QLineEdit   *_coordXEdit;
    QLineEdit   *_coordYEdit;
    QLineEdit   *_angleEdit;
    QLineEdit   *_newMsgRateEdit;
    QLineEdit   *_editMsgRateEdit;
    QTextEdit   *_bulletinEdit;
    QPushButton *_sendButton;
    QPushButton *_autoUpdateButton;
    QPushButton *_getMyDataButton;
    QWidget     *_centralWidget;
    QGridLayout *_mainLayout;
};

#endif // MAINWINDOW_H
