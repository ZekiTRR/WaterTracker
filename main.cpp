#include <QApplication>
#include <QPushButton>
#include <QInputDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QString>
#include <QProgressBar>
#include "utility/Models.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    a.setStyleSheet(R"(
         QWidget {
             background-color: #2E3440; /* Темно-серый фон */
             color: #ECEFF4;             /* Светлый текст */
             font-family: "Segoe UI";
             font-size: 11pt;
         }
         QPushButton {
             background-color: #5E81AC; /* Синий цвет для кнопки */
             border: none;
             padding: 8px 16px;
             border-radius: 4px;
             font-weight: bold;
         }
         QPushButton:hover {
             background-color: #81A1C1; /* Более светлый синий при наведении */
         }
         QPushButton:pressed {
             background-color: #4C566A; /* Темно-серый при нажатии */
         }
         QProgressBar {
             border: 1px solid #4C566A;
             border-radius: 4px;
             text-align: center;
             color: #2E3440;
         }
         QProgressBar::chunk {
             background-color: #A3BE8C; /* Зеленый для заполнения */
             border-radius: 3px;
         }
         QLineEdit {
             background-color: #3B4252;
             border: 1px solid #4C566A;
             padding: 6px;
             border-radius: 4px;
         }
         QLabel#progressLabel { /* Стиль только для метки с прогрессом */
             font-size: 12pt;
             font-weight: bold;
             qproperty-alignment: 'AlignCenter';
         }
     )");


    UserProfile profile;
    DailyEntry dailyEntry;

    // Логика первого запуска или загрузки существующего профиля
    if (profile.isFirstStart()) {
        // Запрашиваем данные у пользователя через диалоговые окна
        bool ok;
        int age = QInputDialog::getInt(nullptr, "Первый запуск", "Введите ваш возраст:", 25, 1, 120, 1, &ok);
        if (!ok) return 0;

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
        profile.dailyGoal = profile.dailyGoalMl();

        // Сохраняем профиль
        if (!profile.createProfile()) {
            QMessageBox::critical(nullptr, "Ошибка", "Не удалось сохранить профиль пользователя.");
            return -1;
        }
        QMessageBox::information(nullptr, "Успех", "Профиль успешно создан!\nВаша дневная норма воды: " + QString::number(profile.dailyGoal) + " мл.");
    } else {
        // Если не первый запуск, просто загружаем профиль, чтобы получить dailyGoal
        profile.loadProfile();
    }

    // Убедимся, что ежедневный файл существует
    dailyEntry.create_daily_profile();

    // --- Дизайн основного окна ---
    QWidget mainWindow;
    mainWindow.resize(300, 200);
    mainWindow.setWindowTitle("Water Tracker");

    QVBoxLayout *layout = new QVBoxLayout(&mainWindow);

    // Создаем текстовые метки для прогресса
    QLabel *progressLabel = new QLabel();
    QLabel *remainingLabel = new QLabel();

    // Лямбда-функция для обновления текста
    auto updateLabels = [&]() {
        int consumed = dailyEntry.getConsumed();
        int goal = profile.dailyGoal;
        int remaining = (goal - consumed > 0) ? (goal - consumed) : 0;

        progressLabel->setText(QString("Выпито: %1 / %2 мл").arg(consumed).arg(goal));
        remainingLabel->setText(QString("Осталось выпить: %1 мл").arg(remaining));
    };

    updateLabels(); // Вызываем один раз для установки начальных значений

    layout->addWidget(progressLabel);
    layout->addWidget(remainingLabel);

    QLineEdit *inputField = new QLineEdit();
    inputField->setPlaceholderText("Введите количество воды (мл)");
    layout->addWidget(inputField);

    // Кнопка для добавления воды
    QPushButton *addButton = new QPushButton("Выпить стакан воды");
    layout->addWidget(addButton);

    // Соединяем нажатие кнопки с действием
    QObject::connect(addButton, &QPushButton::clicked, [&]() {
        int amount = inputField->text().toInt();
        if (dailyEntry.add_consumed(amount)) {
            updateLabels(); // Обновляем текст после добавления
        } else {
            QMessageBox::warning(&mainWindow, "Ошибка", "Не удалось добавить выпитую воду.");
        }
    });

    mainWindow.show();

    return QApplication::exec();
}