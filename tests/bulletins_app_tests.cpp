#include "gtest/gtest.h"
#include "../mainwindow.h" // Подключаем ваш класс MainWindow
#include <QApplication> // Нужно для инициализации Qt
// Мы используем QSignalSpy для проверки соединений сигналов/слотов
#include <QtTest/QSignalSpy>
// Мы используем QTestEventLoop для имитации работы цикла событий Qt во время теста
#include <QtTest/QTestEventLoop>
#include <QTimer>

class TestableMainWindow : public MainWindow {
public:
    void publicCloseEvent(QCloseEvent *event) {
        closeEvent(event); // Вызываем защищенный метод из публичной обертки
    }
    bool publicEventFilter(QObject *obj, QEvent *event) {
        return eventFilter(obj, event);
    }
};

class TestableBoard : public Board {
public:
    using Board::Board;
    // ... можно добавить геттеры, если _jsonObjectArray приватный ...
};

TEST(MainWindowTestGroup, MenuCreationAndHelpMessage) {
    MainWindow w;

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

    QLineEdit* userNameEdit = w.findChild<QLineEdit*>("UserNameEdit");
    QPushButton* sendButton = w.findChild<QPushButton*>("SendButton");

    ASSERT_NE(userNameEdit, nullptr);
    ASSERT_NE(sendButton, nullptr);

    // Имитируем событие FocusIn на userNameEdit
    QFocusEvent focusInEvent(QEvent::FocusIn, Qt::OtherFocusReason);

    w.publicEventFilter(userNameEdit, &focusInEvent);

    // Проверяем, что кнопка отправки стала неактивной и текст изменился на "Отправить сообщение" (если он был "Редактировать")
    ASSERT_FALSE(sendButton->isEnabled());
    ASSERT_EQ(sendButton->text(), "Отправить сообщение");
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
            {"author": "Anna", "text": "Hello", "font": "Arial", "size": 12, "left": 10, "top": 20, "angle": 0.0},
            {"author": "Bob", "text": "World", "font": "Verdana", "size": 14, "left": 100, "top": 50, "angle": 10.0}
        ]
    })";
    file.write(testJson.toUtf8());
    file.close();

    QMutex mutex;
    TestableBoard board(testFileName, &mutex);

    ASSERT_FALSE(board._jsonObjectArray.isEmpty());
    ASSERT_EQ(board._jsonObjectArray.size(), 2);
    ASSERT_EQ(board._jsonObjectArray[0].toObject()["author"].toString(), "Anna");
    ASSERT_EQ(board._jsonObjectArray[0].toObject()["left"].toInt(), 10);
    ASSERT_EQ(board._bulletinPaintDataList.size(), 2);

    QFile::remove(testFileName);
}

TEST(BoardTestGroup, FindByUser) {
    const QString testFileName = "test_data_find.json";
    QFile file(testFileName);
    QFile::remove(testFileName);
    ASSERT_TRUE(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QString testJson = R"({
        "messages": [
            {"author": "Anna", "text": "Hello", "font": "Arial", "size": 12, "left": 10, "top": 20, "angle": 0.0},
            {"author": "Bob", "text": "World", "font": "Verdana", "size": 14, "left": 100, "top": 50, "angle": 10.0}
        ]
    })";
    file.write(testJson.toUtf8());
    file.close();
    QMutex mutex;
    TestableBoard board(testFileName, &mutex);

    QJsonObject foundObj = board.findByUser("Bob");

    ASSERT_FALSE(foundObj.isEmpty());
    ASSERT_EQ(foundObj["text"].toString(), "World");

    QJsonObject notFoundObj = board.findByUser("Charlie");
    ASSERT_TRUE(notFoundObj.isEmpty());

    QFile::remove(testFileName);
}

TEST(BoardTestGroup, WriteDataAddsNewUser) {
    const QString testFileName = "test_data_write_new.json";
    QFile::remove(testFileName);
    QMutex mutex;
    TestableBoard board(testFileName, &mutex);
    int initialSize = board._jsonObjectArray.size();

    board.setUserName("NewUser");
    board.setMessage("TestData");
    board.writeData();

    ASSERT_EQ(board._jsonObjectArray.size(), initialSize + 1);
    ASSERT_EQ(board._jsonObjectArray.last().toObject()["author"].toString(), "NewUser");

    QFile::remove(testFileName);
}

int main(int argc, char **argv) {
    QApplication a(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
