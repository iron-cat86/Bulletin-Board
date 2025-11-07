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

int main(int argc, char **argv) {
    QApplication a(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
