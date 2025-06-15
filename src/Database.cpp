#include "../include/Database.h"
#include "../include/Global.h"
#include <iostream>
#include <algorithm>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/mutex.hpp>

boost::mutex db_cout_mutex;

// Конструктор
Database::Database() {
    // Инициализация базы данных
    items.emplace_back(70.0, 95.0, 60.0, "Skirt A");
    items.emplace_back(75.0, 100.0, 65.0, "Skirt B");
    items.emplace_back(65.0, 90.0, 55.0, "Skirt C");

    // // отладочный вывод
    // boost::lock_guard<boost::mutex> lock(db_cout_mutex);
    // std::cout << "Database initialised with " << items.size() << " items.\n";
    // for (const auto& item : items) {
    //     std::cout << "Item: " << item.name
    //               << ", Waist: " << item.waist_size
    //               << ", Hips: " << item.hip_size
    //               << ", Length: " << item.length << "\n";
    // }
}

// Поиск подходящих предметов одежды
std::vector<std::pair<std::string, double>> Database::find_matches(double user_waist, double user_hip, double user_length) {
    boost::shared_lock<boost::shared_mutex> lock(mutex); // shared lock для чтения
    std::vector<std::pair<std::string, double>> results;
    const double min_individual_prob = 0.5;

    for (const auto& item : items) {
        double waist_prob = calculate_probability(user_waist, item.waist_size, 5.0);
        double hip_prob = calculate_probability(user_hip, item.hip_size, 5.0);
        double length_prob = calculate_probability(user_length, item.length, 3.0);
        // double final_probability = (waist_prob + hip_prob + length_prob) / 3.0;

        if (waist_prob >= min_individual_prob && hip_prob >= min_individual_prob && length_prob >= min_individual_prob)
        {
            double final_probability = (waist_prob + hip_prob + length_prob) / 3.0;
            results.emplace_back(item.name, final_probability);
        }
        // // для отладки
        // boost::lock_guard<boost::mutex> lock(db_cout_mutex);
        // std::cout << "Item: " << item.name << ", Waist Prob: " << waist_prob << ", Hip Prob: " << hip_prob
        // << ", Length Prob: " << length_prob << ", Final Prob: " << final_probability << "\n";

        // if (final_probability > 0.5) { // Фильтр по вероятности
        //     results.emplace_back(item.name, final_probability);
        // }
    }

    return results;
}

std::vector<ClothingItem> Database::get_all_items() const {
    boost::shared_lock<boost::shared_mutex> lock(mutex);
    return items;
}

// Добавление предмета одежды в базу данных
void Database::add_item(const ClothingItem& item) {
    boost::unique_lock<boost::shared_mutex> lock(mutex); // exclusive lock для записи
    items.push_back(item);
    std::cout << "Item " << item.name << " added to the database.\n";
}

// Редактирование предмета одежды в базе данных
void Database::edit_item(const std::string& name, const ClothingItem& new_item) {
    boost::unique_lock<boost::shared_mutex> lock(mutex); // exclusive lock для записи
    auto it = std::find_if(items.begin(), items.end(), [&name](const ClothingItem& item) {
        return item.name == name;
    });
    if (it != items.end()) {
        *it = new_item;
        std::cout << "Item " << name << " edited in the database.\n";
    } else {
        std::cout << "Item " << name << " not found in the database.\n";
    }
}

// Удаление предмета одежды из базы данных
void Database::delete_item(const std::string& name) {
    boost::unique_lock<boost::shared_mutex> lock(mutex); // exclusive lock для записи
    auto it = std::remove_if(items.begin(), items.end(), [&name](const ClothingItem& item) {
        return item.name == name;
    });
    if (it != items.end()) {
        items.erase(it, items.end());
        std::cout << "Item " << name << " deleted from the database.\n";
    } else {
        std::cout << "Item " << name << " not found in the database.\n";
    }
}

// Расчет вероятности совпадения
double Database::calculate_probability(const double user_value, const double item_value, const double tolerance) {
    if (const double difference = std::abs(user_value - item_value); difference <= tolerance) {
        return 1.0 - (difference / tolerance);
    }
    return 0.0;
}
