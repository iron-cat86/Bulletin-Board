#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include "board.h"
#include "bull_thread.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QCloseEvent>
#include <QTextEdit>
#include <QDebug>
#include <QAction>
#include <QMenu>
#include <QMenuBar>

const int MAX_CHAR_LIMIT = 300;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    static QTextEdit *s_logBrowser;
    static void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
signals:
    void shown();
private slots:
    void updateBulletin();
    void getMyData();
    void onStartOrStopButton();
    void showHelpMessage();
    void onUserNameSelected(const QString &userName);
    void userEditUpdate();
    void updateCharCount();

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;

private://functions
    void setupUi();
    void appendLogMessage(QtMsgType type, const QString &msg);
    void createMenus();
private://members
    QTextEdit   *_logBrowser;
    // UI elements
    QMenu       *_helpMenu;
    QAction     *_aboutAction;
    Board       *_board;
    QComboBox   *_userNameEdit;
    QComboBox   *_fontComboBox;
    QComboBox   *_colorComboBox;
    QLineEdit   *_fontSizeEdit;
    QLineEdit   *_coordXEdit;
    QLineEdit   *_coordYEdit;
    QLineEdit   *_angleEdit;
    QLineEdit   *_newMsgRateEdit;
    QLineEdit   *_editMsgRateEdit;
    QTextEdit   *_bulletinEdit;
    QLabel      *_charCountLabel;       // Label для отображения количества символов
    QLabel      *_limitWarningLabel;
    QPushButton *_sendButton;
    QPushButton *_autoUpdateButton;
    QPushButton *_clearButton;
    QWidget     *_centralWidget;
    QGridLayout *_mainLayout;
    // Threads
    BullThread  *_updateThread;
    TaskThread  *_tasks;
    QMutex       _mutex; // Общий мьютекс для всех потоков
};

#endif // MAINWINDOW_H
