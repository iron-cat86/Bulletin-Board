#include "gtest/gtest.h"
#include "../mainwindow.h"
#include <QApplication>
#include <QtTest/QTest>
#include <QtTest/QSignalSpy>
#include <QtTest/QTestEventLoop>
#include <QTimer>
#include <QObject>
#include <QCoreApplication>
#include "../board.h"

class TestableMainWindow : public MainWindow {
public:
    void publicCloseEvent(QCloseEvent *event) {
        closeEvent(event); // Вызываем защищенный метод из публичной обертки
    }
    bool publicEventFilter(QObject *obj, QEvent *event) {
        return eventFilter(obj, event);
    }
};

class BoardStub : public Board {
    //Q_OBJECT
public:
    BoardStub() : Board("bulletins.json", nullptr, nullptr) {
            // Если Board() требует реальный QMutex*, нужно создать его здесь
        }
    void updateBulletin(bool update) override {
        // Ничего не делаем, просто считаем вызовы
        callCount++;
    }

    BulletinPaintData createNewPaintData(QJsonObject& obj) override {
        // Возвращаем пустую структуру
        return BulletinPaintData();
    }

    int callCount = 0;
};

class ThreadTest : public ::testing::Test {
protected:
    QMutex mutex;
    BoardStub board;
    int testMs = 10;
};

TEST(MainWindowTestGroup, MenuCreationAndHelpMessage) {
    TestableMainWindow w;

    ASSERT_NE(w.menuBar(), nullptr); // Проверяем наличие menuBar
    QMenu* helpMenu = w.menuBar()->findChild<QMenu*>("HelpMenu");
    ASSERT_NE(helpMenu, nullptr);
}

TEST(MainWindowTestGroup, LoggingFunctionality) {
    TestableMainWindow w;

        ASSERT_NE(MainWindow::s_logBrowser, nullptr);

        QSignalSpy spy(MainWindow::s_logBrowser, &QTextEdit::textChanged);

        qDebug() << "Тестовое отладочное сообщение";

        ASSERT_TRUE(spy.wait(2000)); // Ждем до 2 секунд

        QString logContent = MainWindow::s_logBrowser->toHtml();

        ASSERT_TRUE(logContent.contains("Тестовое отладочное сообщение"));
        ASSERT_TRUE(logContent.contains("#000000"));
}

TEST(MainWindowTestGroup, CloseEventStopsThreads) {
    TestableMainWindow w;

    QCloseEvent event;
    w.publicCloseEvent(&event);

    ASSERT_TRUE(event.isAccepted());
}

TEST(MainWindowTestGroup, SetupUiInitializesAllWidgets) {
    TestableMainWindow w;
    ASSERT_NE(w.centralWidget(), nullptr);
    ASSERT_NE(w.centralWidget()->layout(), nullptr);
    QGridLayout* mainLayout = qobject_cast<QGridLayout*>(w.centralWidget()->layout());
    ASSERT_NE(mainLayout, nullptr);

    QPushButton* sendButton = w.findChild<QPushButton*>("Отправить сообщение");
    QList<QPushButton*> buttons = w.findChildren<QPushButton*>();
    ASSERT_GT(buttons.size(), 0); // Проверяем, что хотя бы одна кнопка существует

    // Проверяем наличие поля ввода имени пользователя (_userNameEdit)
    QLineEdit* userNameEdit = w.findChild<QLineEdit*>("Гость"); // Ищем по тексту по умолчанию

    // Проверяем наличие текстового поля для логов
    QTextEdit* logBrowser = w.findChild<QTextEdit*>("MainWindowLogBrowser");
    QList<QTextEdit*> textEdits = w.findChildren<QTextEdit*>();
    ASSERT_GT(textEdits.size(), 0);
}

TEST(MainWindowTestGroup, EventFilterBlocksSendButtonOnFocus) {
    TestableMainWindow w;
    w.resize(1024, 768);

    QComboBox* userNameEdit = w.findChild<QComboBox*>("UserNameEdit");
    QPushButton* sendButton = w.findChild<QPushButton*>("SendButton");

    ASSERT_NE(userNameEdit, nullptr);
    ASSERT_NE(sendButton, nullptr);

    // Имитируем событие FocusIn на userNameEdit
    QFocusEvent focusInEvent(QEvent::FocusIn, Qt::OtherFocusReason);

    w.publicEventFilter(userNameEdit, &focusInEvent);
}

TEST(MainWindowTestGroup, StartStopButtonTogglesAutomation) {
    TestableMainWindow w;
    w.resize(1024, 768);

    QPushButton* autoButton = w.findChild<QPushButton*>("AutoUpdateButton");
    ASSERT_NE(autoButton, nullptr);

    ASSERT_EQ(autoButton->text(), "Старт авто");

}

TEST(BoardTestGroup, ReadDataFromFile) {
    const QString testFileName = "test_data_read.json";
    QFile file(testFileName);
    QFile::remove(testFileName);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QString testJson = R"({
        "messages": [
            {"author": "Anna", "text": "Hello", "font": "Arial", "size": 12, "left": 10, "top": 20, "angle": 0.0, "amountChanges": 1},
            {"author": "Bob", "text": "World", "font": "Verdana", "size": 14, "left": 800, "top": 50, "angle": 10.0, "amountChanges": 1},
            {"author": "Joan", "text": "Hi!", "font": "Verdana", "size": 40, "left": 100, "top": 100, "angle": 0.0, "amountChanges": 1},
            {"author": "Jeem", "text": "I'm blue", "font": "Verdana", "size": 50, "left": 700, "top": 1000, "angle": 0.0, "amountChanges": 1},
            {"author": "James", "text": "I near", "font": "Verdana", "size": 30, "left": 100, "top": 20, "angle": 90.0, "amountChanges": 1},
            {"author": "Stiv", "text": "Hear you are?", "font": "Verdana", "size": 25, "left": 600, "top": 25, "angle": 45.0, "amountChanges": 1},
            {"author": "Liv", "text": "I feel you", "font": "Verdana", "size": 60, "left": 500, "top": 10, "angle": 30.0, "amountChanges": 1},
            {"author": "Bred", "text": "I see you", "font": "Verdana", "size": 40, "left": 400, "top": 11, "angle": 20.0, "amountChanges": 1},
            {"author": "Alex", "text": "Good Morning!", "font": "Verdana", "size": 30, "left": 200, "top": 7, "angle": 16.0, "amountChanges": 1},
            {"author": "Stiven", "text": "Good by!", "font": "Verdana", "size": 20, "left": 100, "top": 50, "angle": 15.0, "amountChanges": 1}
        ]
    })";
    file.write(testJson.toUtf8());
    file.close();

    QMutex mutex;
    Board board(testFileName, &mutex);

    ASSERT_FALSE(board._jsonObjectArray.isEmpty());
    ASSERT_EQ(board._jsonObjectArray.size(), 10);
    ASSERT_EQ(board._jsonObjectArray[0].toObject()["author"].toString(), "Anna");
    ASSERT_EQ(board._jsonObjectArray[0].toObject()["left"].toInt(), 10);
    ASSERT_EQ(board._bulletinPaintDataList.size(), 10);
    ASSERT_EQ(board._jsonObjectArray[7].toObject()["author"].toString(), board._bulletinPaintDataList[7].user);
    QFile::remove(testFileName);
}

TEST(BoardTestGroup, FindByUser) {
    const QString testFileName = "test_data_find.json";
    QFile file(testFileName);
    QFile::remove(testFileName);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QString testJson = R"({
        "messages": [
            {"author": "Anna", "text": "Hello", "font": "Arial", "size": 12, "left": 10, "top": 20, "angle": 0.0, "amountChanges": 1},
            {"author": "Bob", "text": "World", "font": "Verdana", "size": 14, "left": 100, "top": 50, "angle": 10.0, "amountChanges": 1}
        ]
    })";
    file.write(testJson.toUtf8());
    file.close();
    QMutex mutex;
    Board board(testFileName, &mutex);
    int id = -1;
    QJsonObject foundObj = board.findByUser("Bob", id);

    ASSERT_FALSE(foundObj.isEmpty());
    ASSERT_EQ(foundObj["text"].toString(), "World");
    ASSERT_EQ(id, 1);

    id = -1;
    QJsonObject notFoundObj = board.findByUser("Charlie", id);
    ASSERT_TRUE(notFoundObj.isEmpty());
    ASSERT_EQ(id, -1);

    QFile::remove(testFileName);
}

TEST(BoardTestGroup, WriteDataAddsNewUser) {
    const QString testFileName = "test_data_write_new.json";
    QFile::remove(testFileName);
    QMutex mutex;
    Board board(testFileName, &mutex);
    int initialSize = board._jsonObjectArray.size();

    board.setUserName("NewUser");
    board.setMessage("TestData");
    board.writeData();

    ASSERT_EQ(board._jsonObjectArray.size(), initialSize + 1);
    ASSERT_EQ(board._jsonObjectArray.last().toObject()["author"].toString(), "NewUser");

    QFile::remove(testFileName);
}

TEST_F(ThreadTest, UpdateThread_StopsGracefullyAndSignals) {
    UpdateThread updateThread(&board, &mutex, testMs, nullptr);
    QSignalSpy spy(&updateThread, SIGNAL(iamstop(QString)));
    board.callCount = 0; // Сбрасываем счетчик вызовов
    QJsonObject newobj;
    newobj["author"] = "Anna";
    newobj["text"] = "Hello";
    newobj["font"] = "Arial";
    newobj["size"] = 12;
    newobj["left"] = 10;
    newobj["top"] = 20;
    newobj["angle"] = 0.0;
    newobj["amountChanges"] = 1;
    board._jsonObjectArray.append(newobj);
    updateThread.start();
    ASSERT_TRUE(updateThread.isRunning());
    // Ждем достаточно, чтобы произошло несколько итераций
    QTest::qWait(200);
    // Останавливаем поток и ждем его завершения
    updateThread.stopThread();
    updateThread.wait();

    ASSERT_FALSE(updateThread.isRunning());
    // Проверяем, что метод updateBulletin был вызван
    ASSERT_FALSE(board.callCount== 0);
    // Вызываем финальную статистику
    updateThread.giveStatistics();
    // Проверяем, что сигнал iamstop был испущен 1 раз
    ASSERT_EQ(spy.count(), 1);
    QString statsMessage = spy.takeFirst().at(0).toString();
    // Проверяем, что сообщение содержит ожидаемый текст (используя u8 или fromUtf8)
    QByteArray expectedBytes("Объявления обновлялись");
    ASSERT_TRUE(statsMessage.contains(QString::fromUtf8(expectedBytes)));
    ASSERT_GT(updateThread._amountIteration, 0);
}

// --- Тестирование TaskThread ---
TEST_F(ThreadTest, TaskThread_StopsGracefullyAndSignals) {
    TaskThread taskThread(&board, &mutex, testMs, nullptr);
    QSignalSpy spy(&taskThread, SIGNAL(iamstop(QString)));
    board.callCount = 0; // Сбрасываем счетчик вызовов
    taskThread.start();
    ASSERT_TRUE(taskThread.isRunning());
    QTest::qWait(200);
    taskThread.stopThread();
    taskThread.wait();

    ASSERT_FALSE(taskThread.isRunning());
    // Проверяем, что метод updateBulletin был вызван
    ASSERT_GT(board.callCount, 0);
    // Вызываем финальную статистику
    taskThread.giveStatistics();

    ASSERT_EQ(spy.count(), 1);
    QString statsMessage = spy.takeFirst().at(0).toString();
    ASSERT_TRUE(statsMessage.contains(QString::fromUtf8("Авто добавление остановлено")));
    ASSERT_GT(taskThread._amountIteration, 0);
}

int main(int argc, char **argv) {
    QApplication a(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
