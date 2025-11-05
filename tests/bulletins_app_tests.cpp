#include "gtest/gtest.h"
#include "../board.h" // Подключаем ваши классы для тестирования

// Пример простого теста для класса Board
TEST(BoardTestGroup, InitialState) {
    QMutex mutex;
    const QString set_file = "settings.json";
    Board *b = new Board(set_file, &mutex); // Предположим, у вас есть класс Board
    ASSERT_TRUE(!b->_bulletinPaintDataList.isEmpty()); // Проверяем его начальное состояние
    delete b;
}

// GTest предоставляет свою функцию main, если вы линкуетесь с gtest_main,
// но при использовании find_package(GTest REQUIRED) в CMake лучше написать свою:
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
