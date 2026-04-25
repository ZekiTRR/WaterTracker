#include "Storage.h"
#include "Models.h"
#include <windows.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// --- Функции класса DailyEntry ---

int DailyEntry::getConsumed() {
    // Просто вызываем статическую функцию из Storage, передавая имя файла
    return Storage::getConsumedMl(directory);
}

bool DailyEntry::add_consumed(int amount) {
    // Вызываем статическую функцию и проверяем результат
    int newAmount = Storage::addConsumedMl(directory, amount);
    return newAmount >= 0; // Возвращаем true, если не было ошибки (-1)
}


// --- Статические функции класса Storage ---

std::string Storage::return_current_time_and_date() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::tm local_tm{};
#ifdef _WIN32
    localtime_s(&local_tm, &in_time_t);
#else
    localtime_r(&in_time_t, &local_tm);
#endif

    std::stringstream ss;
    ss << std::put_time(&local_tm, "%Y-%m-%d");
    return ss.str();
}

std::string Storage::getDataDirectory() {
    char path[MAX_PATH];
    DWORD len = GetModuleFileNameA(nullptr, path, MAX_PATH);
    if (len == 0) {
        std::cerr << "Не удалось получить путь к исполняемому файлу." << std::endl;
        return "";
    }

    std::string exePath(path);
    std::string dirPath = exePath.substr(0, exePath.find_last_of('\\')) + "\\data";

    CreateDirectoryA(dirPath.c_str(), nullptr);
    return dirPath;
}

bool Storage::isFirstStart(const std::string& filename) {
    std::string dirPath = getDataDirectory();
    if (dirPath.empty()) return true;
    std::string fullPath = dirPath + "\\" + filename;
    DWORD attributes = GetFileAttributesA(fullPath.c_str());
    return (attributes == INVALID_FILE_ATTRIBUTES || (attributes & FILE_ATTRIBUTE_DIRECTORY));
}

bool Storage::CreateUserProfileSettings(int age, int weightKg, int dailyGoal, const std::string& gender, const std::string& filename) {
    std::string dirPath = getDataDirectory();
    if (dirPath.empty()) return false;

    json j;
    j["firststart"] = return_current_time_and_date();
    j["age_onstart"] = age;
    j["weightKg_onstart"] = weightKg;
    j["gender"] = gender;
    j["dailyGoal"] = dailyGoal;

    std::ofstream file(dirPath + "\\" + filename);
    if (!file.is_open()) {
        std::cerr << "Не удалось создать профиль пользователя: " << filename << '\n';
        return false;
    }

    file << j.dump(4);
    return true;
}

bool Storage::loadUserProfileSettings(int& age, int& dailygoal, int& weightKg, std::string& gender, std::string& firststart, const std::string& filename) {
    std::string dirPath = getDataDirectory();
    if (dirPath.empty()) return false;

    std::ifstream file(dirPath + "\\" + filename);
    if (!file.is_open()) {
        // Это не ошибка, если файл просто еще не создан
        return false;
    }

    try {
        json j;
        file >> j;
        firststart = j.value("firststart", "");
        age = j.value("age_onstart", 0);
        weightKg = j.value("weightKg_onstart", 0);
        gender = j.value("gender", "unknown");
        dailygoal = j.value("dailyGoal", 0);
        return true;
    } catch (const json::exception& e) {
        std::cerr << "Ошибка парсинга профиля пользователя: " << e.what() << '\n';
        return false;
    }
}

bool Storage::CreateDailyProfile(const std::string& filename) {
    std::string dirPath = getDataDirectory();
    if (dirPath.empty()) return false;

    std::string path = dirPath + "\\" + filename;

    // Проверяем, существует ли файл, чтобы не перезаписать его
    std::ifstream checkFile(path);
    if (checkFile.good()) {
        return true; // Файл уже существует, ничего не делаем
    }

    json j;
    j["date"] = return_current_time_and_date();
    j["consumedMl"] = 0;

    std::ofstream file(path);
    if (!file.is_open()) {
        std::cerr << "Не удалось создать ежедневный профиль: " << filename << '\n';
        return false;
    }
    file << j.dump(4);
    return true;
}

int Storage::addConsumedMl(const std::string& filename, int amount) {
    std::string dirPath = getDataDirectory();
    if (dirPath.empty()) return -1;

    std::string path = dirPath + "\\" + filename;
    json j;

    // Сначала читаем текущее значение
    std::ifstream inputFile(path);
    if (inputFile.is_open()) {
        try {
            inputFile >> j;
        } catch (const json::exception& e) {
            std::cerr << "Ошибка чтения JSON для добавления воды: " << e.what() << '\n';
            // Можно создать новый файл, если старый поврежден
            j["consumedMl"] = 0;
        }
        inputFile.close();
    } else {
        // Если файл не открылся, возможно, его еще нет. Создадим его.
        CreateDailyProfile(filename);
        j["consumedMl"] = 0;
    }


    int currentAmount = j.value("consumedMl", 0);
    currentAmount += amount;
    j["consumedMl"] = currentAmount;

    // Теперь записываем обновленное значение
    std::ofstream outputFile(path);
    if (!outputFile.is_open()) {
        std::cerr << "Не удалось открыть файл для записи: " << filename << '\n';
        return -1;
    }

    outputFile << j.dump(4);
    return currentAmount;
}

int Storage::getConsumedMl(const std::string& filename) {
    std::string dirPath = getDataDirectory();
    if (dirPath.empty()) return 0;

    std::string path = dirPath + "\\" + filename;
    std::ifstream file(path);

    if (!file.is_open()) {
        return 0; // Если файла нет, значит, еще ничего не выпито
    }

    try {
        json j;
        file >> j;
        return j.value("consumedMl", 0);
    } catch (const json::exception& e) {
        std::cerr << "Ошибка чтения JSON для получения выпитого: " << e.what() << '\n';
        return 0;
    }
}