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
        background: qlineargradient(
            x1:0, y1:0, x2:1, y2:1,
            stop:0 #EAFBFF,
            stop:0.45 #D7F4FF,
            stop:1 #BDEBFF
        );
        color: #063B4F;
        font-family: "Segoe UI";
        font-size: 11pt;
    }

    QLabel {
        background: transparent;
        color: #06445C;
        font-size: 13pt;
        font-weight: 600;
        padding: 6px;
        qproperty-alignment: AlignCenter;
    }

    QLineEdit {
        background-color: rgba(255, 255, 255, 220);
        color: #063B4F;
        border: 2px solid #7ED6F6;
        border-radius: 12px;
        padding: 9px 12px;
        selection-background-color: #4FC3F7;
        font-size: 11pt;
    }

    QLineEdit:focus {
        border: 2px solid #00AEEF;
        background-color: white;
    }

    QLineEdit::placeholder {
        color: #6FAFC2;
    }

    QPushButton {
        background: qlineargradient(
            x1:0, y1:0, x2:1, y2:1,
            stop:0 #00B4D8,
            stop:1 #0077B6
        );
        color: white;
        border: none;
        border-radius: 14px;
        padding: 11px 18px;
        font-size: 11pt;
        font-weight: 700;
    }

    QPushButton:hover {
        background: qlineargradient(
            x1:0, y1:0, x2:1, y2:1,
            stop:0 #20CFFF,
            stop:1 #0096C7
        );
    }

    QPushButton:pressed {
        background-color: #005F8F;
        padding-top: 12px;
        padding-bottom: 10px;
    }

    QProgressBar {
        background-color: rgba(255, 255, 255, 180);
        border: 2px solid #7ED6F6;
        border-radius: 10px;
        text-align: center;
        color: #06445C;
        font-weight: bold;
        height: 18px;
    }

    QProgressBar::chunk {
        background: qlineargradient(
            x1:0, y1:0, x2:1, y2:0,
            stop:0 #48CAE4,
            stop:0.5 #00B4D8,
            stop:1 #0077B6
        );
        border-radius: 8px;
    }

    QMessageBox, QInputDialog {
        background-color: #EAFBFF;
        color: #063B4F;
    }

    QMessageBox QLabel, QInputDialog QLabel {
        color: #063B4F;
        font-size: 10.5pt;
        font-weight: 500;
        qproperty-alignment: AlignLeft;
    }

    QSpinBox, QComboBox {
        background-color: white;
        color: #063B4F;
        border: 2px solid #7ED6F6;
        border-radius: 10px;
        padding: 6px;
    }

    QComboBox::drop-down {
        border: none;
        width: 24px;
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