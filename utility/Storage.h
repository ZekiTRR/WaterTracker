#ifndef APPTRACKER_STORAGE_H
#define APPTRACKER_STORAGE_H

#include <string>


class Storage {
public:
    static std::string return_current_time_and_date(); // Получить текущую дату и время
    static std::string getDataDirectory(); // Получить или создать путь к папке data

    // Пример функции для сохранения любой строки в json
    static bool save_string_to_json(const std::string& text, const std::string& filename = "data.json");

    // Работа с профилем пользователя


    static bool isFirstStart(const std::string& filename = "UserProfile.json"); // Проверка первого запуска
    static bool CreateUserProfileSettings(int age, int weightKg, int dailyGoal, const std::string& gender, const std::string& filename = "UserProfile.json"); // Создать профиль
    static bool loadUserProfileSettings(int& age, int& dailygoal, int& weightKg, std::string& gender, std::string& firststart, const std::string& filename = "UserProfile.json"); // Загрузить профиль
    bool CreateDailyProfile(std::string &filename); // Создать профиль для ежедневных записей (можно расширить для сохранения consumedMl)
};

#endif //APPTRACKER_STORAGE_H