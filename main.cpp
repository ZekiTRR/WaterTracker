#include <QApplication>
#include <QPushButton>
#include <QInputDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QString>
#include "utility/Models.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    UserProfile profile;

    // Логика первого запуска или загрузки существующего профиля
    if (profile.isFirstStart()) {
        // Запрашиваем данные у пользователя через диалоговые окна (или можно сделать кастомное окно)
        bool ok;

        int age = QInputDialog::getInt(nullptr, "Первый запуск", "Введите ваш возраст:", 25, 1, 120, 1, &ok);
        if (!ok) return 0; // Пользователь отменил ввод

        int weight = QInputDialog::getInt(nullptr, "Первый запуск", "Введите ваш вес (кг):", 70, 20, 300, 1, &ok);
        if (!ok) return 0;

        QStringList genders;
        genders << "Мужской" << "Женский";
        QString genderSelection = QInputDialog::getItem(nullptr, "Первый запуск", "Выберите ваш пол:", genders, 0, false, &ok);
        if (!ok) return 0;

        // Заполняем профиль
        profile.age = age;
        profile.weightKg = weight;
        profile.gender = (genderSelection == "Мужской") ? "male" : "female";

        // Сохраняем в JSON
        if (!profile.createProfile()) {
            QMessageBox::critical(nullptr, "Ошибка", "Не удалось сохранить профиль пользователя.");
            return -1;
        }

        QMessageBox::information(nullptr, "Успех", "Профиль успешно создан!\nВаша дневная норма воды: " + QString::number(profile.dailyGoalMl()) + " мл.");
    } else {
        // Профиль уже существует, просто загружаем его
        if (!profile.loadProfile()) {
            QMessageBox::warning(nullptr, "Ошибка", "Профиль существует, но не удалось его загрузить.");
        }
    }




    // Дизайн основного окна
    QWidget mainWindow;
    mainWindow.resize(300, 200);
    mainWindow.setWindowTitle("Water Tracker");

    QVBoxLayout *layout = new QVBoxLayout(&mainWindow);

    QString infoText = QString("Возраст: %1\nВес: %2 кг\nПол: %3\nНорма: %4 мл")
            .arg(profile.age)
            .arg(profile.weightKg)
            .arg(profile.gender == "male" ? "Мужской" : "Женский")
            .arg(profile.dailyGoalMl());

    QLabel *infoLabel = new QLabel(infoText);
    layout->addWidget(infoLabel);

    QPushButton *button = new QPushButton("Выпить стакан воды (250 мл)");
    layout->addWidget(button);

    mainWindow.show();

    return QApplication::exec();
}