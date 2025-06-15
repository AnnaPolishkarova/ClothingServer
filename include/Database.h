#ifndef DATABASE_H
#define DATABASE_H

#include <vector>
#include <string>
#include <boost/thread/shared_mutex.hpp>
#include "ClothingItem.h"

// Класс для управления базой данных
class Database {
    std::vector<ClothingItem> items;
    mutable boost::shared_mutex mutex;

public:
    Database();
    std::vector<std::pair<std::string, double>> find_matches(double user_waist, double user_hip, double user_length);

    //Методы для администрирования базы данных
    void add_item(const ClothingItem& item);
    void edit_item(const std::string& name, const ClothingItem& new_item);
    void delete_item(const std::string& name);
    std::vector<ClothingItem> get_all_items() const;  // новый метод

private:
    double calculate_probability(double user_value, double item_value, double tolerance);
    friend class DatabaseTest_ProbabilityCalculation_Test;
    friend class DatabaseTest_ProbabilityCalculationAccuracy_Test;
};

#endif //DATABASE_H
