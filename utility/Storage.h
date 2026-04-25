#ifndef APPTRACKER_STORAGE_H
#define APPTRACKER_STORAGE_H

#include <string>

class Storage {
public:
    static std::string return_current_time_and_date();
    static std::string getDataDirectory();

    // Проверка, первый ли это запуск приложения в целом
    static bool isFirstStart(const std::string& filename = "UserProfile.json");

    // --- Функции для работы с профилем пользователя ---
    static bool CreateUserProfileSettings(int age, int weightKg, int dailyGoal, const std::string& gender, const std::string& filename = "UserProfile.json");
    static bool loadUserProfileSettings(int& age, int& dailygoal, int& weightKg, std::string& gender, std::string& firststart, const std::string& filename = "UserProfile.json");

    // --- Функции для работы с ежедневными записями ---
    static bool CreateDailyProfile(const std::string& filename);
    static int addConsumedMl(const std::string& filename, int amount);
    static int getConsumedMl(const std::string& filename);
};

#endif //APPTRACKER_STORAGE_H