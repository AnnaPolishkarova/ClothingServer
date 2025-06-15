
#ifndef CLOTHINGITEM_H
#define CLOTHINGITEM_H

#include <string>
#include <utility>

struct ClothingItem {
    double waist_size;
    double hip_size;
    double length;
    std::string name;

    // Конструктор с перемещением для строки
    ClothingItem(double waist, double hip, double len, std::string n);

    // Конструктор по умолчанию
    ClothingItem() = default;
};

#endif // CLOTHINGITEM_H