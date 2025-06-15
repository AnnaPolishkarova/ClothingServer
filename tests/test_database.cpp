#include <gtest/gtest.h>
#include "../include/Database.h"

class DatabaseTest : public ::testing::Test {
protected:
    Database db;

    void SetUp() override {
        db.add_item(ClothingItem(70.0, 95.0, 60.0, "Skirt A"));
        db.add_item(ClothingItem(75.0, 100.0, 65.0, "Skirt B"));
    }
};

// Проверка инициализации и базового поиска
TEST_F(DatabaseTest, FindsMatches) {
    auto results = db.find_matches(70, 95, 60);
    for (const auto& r : results) {
        std::cout << "Found: " << r.first << " (prob: " << r.second << ")\n";
    }
    ASSERT_EQ(results.size(), 2);
    EXPECT_EQ(results[0].first, "Skirt A");
    EXPECT_GT(results[0].second, 0.5);
    // EXPECT_GT(results[0].second, 1.0);
}

// Проверка обработки невалидных параметров
TEST_F(DatabaseTest, HandlesExtremeValues) {
    EXPECT_TRUE(db.find_matches(700000, 68, 90).empty());
    EXPECT_TRUE(db.find_matches(-10, -10, -10).empty());
}

// Проверка точности расчета вероятностей
TEST_F(DatabaseTest, ProbabilityCalculation) {
    EXPECT_DOUBLE_EQ(db.calculate_probability(70, 70, 5), 1.0);    // Точное совпадение
    EXPECT_DOUBLE_EQ(db.calculate_probability(70, 75, 5), 0.0);     // За пределами tolerance
    EXPECT_DOUBLE_EQ(db.calculate_probability(70, 72.5, 5), 0.5);   // Граничное значение
}

// Тесты для операций добавления/удаления
TEST_F(DatabaseTest, ItemManagement) {
    // Добавление
    db.add_item(ClothingItem(72.0, 97.0, 62.0, "Skirt C"));
    EXPECT_FALSE(db.find_matches(72, 97, 62).empty());

    // Удаление
    db.delete_item("Skirt C");
    EXPECT_TRUE(db.find_matches(72, 97, 62).empty());
}

// Проверка потокобезопасности
TEST_F(DatabaseTest, ThreadSafety) {
    auto task = [this]() {
        for (int i = 0; i < 100; ++i) {
            db.find_matches(70, 95, 60);
        }
    };

    std::thread t1(task), t2(task);
    t1.join(); t2.join();
    SUCCEED(); // Если не упало - мьютекс работает
}
