#include "../include/ClothingItem.h"

#include <utility>

// Конструктор
ClothingItem::ClothingItem(const double waist, const double hip, const double len, std::string  n)
    : waist_size(waist), hip_size(hip), length(len), name(std::move(n)) {}