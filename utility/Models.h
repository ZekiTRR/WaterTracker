#ifndef APPTRACKER_MODELS_H
#define APPTRACKER_MODELS_H

#include <string>
#include "Storage.h"

class UserProfile {
public:
    std::string firststart; // Первый запуск приложения (дата и время)
    int age = 0;            // Возраст пользователя
    int weightKg = 0;       // Вес пользователя в килограммах
    int dailyGoal = 0;
    std::string gender;     // Пол пользователя ("male" / "female")

    // Проверка, является ли этот запуск первым
    bool isFirstStart() const {
        return Storage::isFirstStart();
    }

    // Сохранить текущие данные профиля в JSON (вызывается при первом запуске)
    bool createProfile() {
        return Storage::CreateUserProfileSettings(age, weightKg, dailyGoal, gender);
    }

    // Загрузить данные из JSON (вызывается при последующих запусках)
    bool loadProfile() {
        return Storage::loadUserProfileSettings(age, dailyGoal, weightKg, gender, firststart);
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
    Storage storage;
    std::string directory = storage.return_current_time_and_date() + ".json"; // Имя файла для ежедневного профиля

    bool create_daily_profile() {
        return Storage::CreateDailyProfile(directory);
    }

    bool isFirstStart() const {
        return Storage::isFirstStart();
    }

    // Получить количество выпитой воды
    int getConsumed();

    // Добавить выпитую воду
    bool add_consumed(int amount);
};

#endif //APPTRACKER_MODELS_H