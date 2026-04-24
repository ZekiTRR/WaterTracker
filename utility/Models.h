#ifndef APPTRACKER_MODELS_H
#define APPTRACKER_MODELS_H

#include <string>
#include "Storage.h"

class UserProfile {
public:
    std::string firststart; // Первый запуск приложения (дата и время)
    int age = 0;            // Возраст пользователя
    int weightKg = 0;       // Вес пользователя в килограммах
    std::string gender;     // Пол пользователя ("male" / "female")

    // Проверка, является ли этот запуск первым
    bool isFirstStart() const {
        return Storage::isFirstStart();
    }

    // Сохранить текущие данные профиля в JSON (вызывается при первом запуске)
    bool createProfile() {
        return Storage::CreateUserProfileSettings(age, weightKg, gender);
    }

    // Загрузить данные из JSON (вызывается при последующих запусках)
    bool loadProfile() {
        return Storage::loadUserProfileSettings(age, weightKg, gender, firststart);
    }

    // Расчет дневной нормы воды (не static, так как зависит от конкретного юзера)
    int dailyGoalMl() const {
        if (weightKg <= 0) return 2000; // Базовое значение, если вес не задан

        // Пример формулы: 35 мл на кг веса для мужчин, 31 мл для женщин (упрощенно)
        if (gender == "female") {
            return weightKg * 31;
        }
        return weightKg * 35;
    }
};

class DailyEntry {
public:
    std::string date; // Дата и время записи
    int consumedMl = 0; // Количество выпитой воды в миллилитрах сегодня
    UserProfile* user;  // Указатель на профиль пользователя

    DailyEntry(UserProfile* u) : user(u) {}

    int goalMl_of_today() const {
        if (!user) return 0;
        int remaining = user->dailyGoalMl() - consumedMl;
        return (remaining > 0) ? remaining : 0; // Не возвращаем отрицательные значения
    }

    void addWater(int ml) {
        consumedMl += ml;
        // Здесь в будущем можно добавить сохранение consumedMl в базу или JSON
    }
};

#endif //APPTRACKER_MODELS_H