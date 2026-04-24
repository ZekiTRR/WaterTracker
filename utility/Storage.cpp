#include "Storage.h"
#include <windows.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

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
    ss << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S"); // Добавил время для точности
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

    // Создаем папку, если она не существует
    CreateDirectoryA(dirPath.c_str(), nullptr);

    return dirPath;
}

bool Storage::save_string_to_json(const std::string& text, const std::string& filename) {
    std::string dirPath = getDataDirectory();
    if (dirPath.empty()) return false;

    json j;
    j["date"] = return_current_time_and_date();
    j["text"] = text;

    std::ofstream file(dirPath + "\\" + filename);
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл: " << filename << '\n';
        return false;
    }

    file << j.dump(4);
    return true;
}

bool Storage::isFirstStart(const std::string& filename) {
    std::string dirPath = getDataDirectory();
    if (dirPath.empty()) return true; // Если не можем получить путь, считаем первым запуском (или можно кинуть исключение)

    std::string fullPath = dirPath + "\\" + filename;
    return FindFirstFileA(fullPath.c_str(), nullptr) == INVALID_HANDLE_VALUE;
}

bool Storage::CreateUserProfileSettings(int age, int weightKg, const std::string& gender, const std::string& filename) {
    std::string dirPath = getDataDirectory();
    if (dirPath.empty()) return false;

    json j;
    j["firststart"] = return_current_time_and_date();
    j["age_onstart"] = age;
    j["weightKg_onstart"] = weightKg;
    j["gender"] = gender;

    std::ofstream file(dirPath + "\\" + filename);
    if (!file.is_open()) {
        std::cerr << "Не удалось создать профиль пользователя: " << filename << '\n';
        return false;
    }

    file << j.dump(4);
    return true;
}

bool Storage::loadUserProfileSettings(int& age, int& weightKg, std::string& gender, std::string& firststart, const std::string& filename) {
    std::string dirPath = getDataDirectory();
    if (dirPath.empty()) return false;

    std::ifstream file(dirPath + "\\" + filename);
    if (!file.is_open()) {
        std::cerr << "Не удалось загрузить профиль пользователя: " << filename << '\n';
        return false;
    }

    try {
        json j;
        file >> j;

        firststart = j.value("firststart", "");
        age = j.value("age_onstart", 0);
        weightKg = j.value("weightKg_onstart", 0);
        gender = j.value("gender", "unknown");

        return true;
    } catch (const json::exception& e) {
        std::cerr << "Ошибка парсинга профиля пользователя: " << e.what() << '\n';
        return false;
    }
}