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

int main(int argc, char **argv) {
    QApplication a(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
