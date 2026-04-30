#include <QApplication>
#include <QAbstractAnimation>
#include <QColor>
#include <QEasingCurve>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QInputDialog>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QPoint>
#include <QProgressBar>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QQuickWidget>
#include <QRect>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QToolButton>
#include <QtMath>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

#include "utility/Models.h"
#include "utility/Tracker.h"
#include "utility/stats/checkinfo.h"

namespace {
QIcon createStatisticsIcon() {
    QPixmap pixmap(64, 64);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QRect chartRect(9, 9, 46, 46);
    painter.setPen(QPen(QColor(QStringLiteral("#0077B6")), 4));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(chartRect);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(QStringLiteral("#00AEEF")));
    painter.drawPie(chartRect.adjusted(2, 2, -2, -2), 90 * 16, -120 * 16);

    painter.setPen(QPen(QColor(QStringLiteral("#06445C")), 3));
    painter.drawLine(chartRect.center(), QPoint(chartRect.center().x(), chartRect.top() + 3));
    painter.drawLine(chartRect.center(), QPoint(chartRect.right() - 5, chartRect.center().y() + 8));

    return QIcon(pixmap);
}

QIcon createWaterIcon() {
    QPixmap pixmap(64, 64);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(QStringLiteral("#00AEEF")));
    painter.drawRoundedRect(QRect(18, 18, 28, 34), 12, 12);
    painter.setBrush(QColor(255, 255, 255, 80));
    painter.drawEllipse(QRect(25, 22, 8, 14));

    return QIcon(pixmap);
}

QIcon createSettingsIcon() {
    QPixmap pixmap(64, 64);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(QColor(QStringLiteral("#06445C")), 5, Qt::SolidLine, Qt::RoundCap));
    painter.drawEllipse(QRect(18, 18, 28, 28));
    painter.drawLine(QPoint(32, 7), QPoint(32, 16));
    painter.drawLine(QPoint(32, 48), QPoint(32, 57));
    painter.drawLine(QPoint(7, 32), QPoint(16, 32));
    painter.drawLine(QPoint(48, 32), QPoint(57, 32));
    painter.drawLine(QPoint(14, 14), QPoint(20, 20));
    painter.drawLine(QPoint(44, 44), QPoint(50, 50));
    painter.drawLine(QPoint(50, 14), QPoint(44, 20));
    painter.drawLine(QPoint(20, 44), QPoint(14, 50));

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(QStringLiteral("#00AEEF")));
    painter.drawEllipse(QRect(27, 27, 10, 10));

    return QIcon(pixmap);
}

void animateProgress(QProgressBar *progressBar, int targetValue) {
    auto *animation = new QPropertyAnimation(progressBar, "value", progressBar);
    animation->setDuration(520);
    animation->setStartValue(progressBar->value());
    animation->setEndValue(qBound(0, targetValue, 100));
    animation->setEasingCurve(QEasingCurve::OutCubic);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void fadeInWindow(QWidget *window) {
    window->setWindowOpacity(0.0);
    auto *animation = new QPropertyAnimation(window, "windowOpacity", window);
    animation->setDuration(240);
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);
    animation->setEasingCurve(QEasingCurve::OutCubic);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

int progressPercent(int consumed, int goal) {
    if (goal <= 0) {
        return 0;
    }
    return qRound((static_cast<double>(consumed) / goal) * 100.0);
}

QQuickWidget *createAmbientView(QWidget *parent, int height) {
    auto *view = new QQuickWidget(parent);
    view->setResizeMode(QQuickWidget::SizeRootObjectToView);
    view->setClearColor(QColor(QStringLiteral("#EAFBFF")));
    view->setSource(QUrl(QStringLiteral("qrc:/qml/WaterAmbient.qml")));
    view->setMinimumHeight(height);
    view->setMaximumHeight(height);
    return view;
}
}

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    const QIcon appIcon(QStringLiteral(":/icons/Iconka.ico"));
    a.setWindowIcon(appIcon);

    a.setStyleSheet(R"(
    QWidget {
        background: qlineargradient(
            x1:0, y1:0, x2:1, y2:1,
            stop:0 #EAFBFF,
            stop:0.48 #D7F4FF,
            stop:1 #BDEBFF
        );
        color: #063B4F;
        font-family: "Segoe UI";
        font-size: 11pt;
    }

    QFrame#surfaceCard {
        background-color: rgba(255, 255, 255, 232);
        border: 1px solid rgba(126, 214, 246, 150);
        border-radius: 18px;
    }

    QFrame#inputCard {
        background-color: rgba(255, 255, 255, 210);
        border: 1px solid rgba(126, 214, 246, 130);
        border-radius: 16px;
    }

    QLabel {
        background: transparent;
        color: #06445C;
        padding: 0;
    }

    QLabel#appTitle {
        color: #063B4F;
        font-size: 20pt;
        font-weight: 800;
    }

    QLabel#appSubtitle,
    QLabel#captionLabel,
    QLabel#remainingLabel {
        color: #5C7A89;
        font-size: 9.5pt;
        font-weight: 600;
    }

    QLabel#progressLabel {
        color: #0077B6;
        font-size: 25pt;
        font-weight: 850;
    }

    QLabel#percentLabel {
        color: #06445C;
        font-size: 12pt;
        font-weight: 800;
    }

    QLineEdit {
        background-color: rgba(255, 255, 255, 245);
        color: #063B4F;
        border: 1px solid #9ADFF6;
        border-radius: 12px;
        padding: 10px 12px;
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
        font-weight: 800;
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

    QPushButton#secondaryButton {
        background: rgba(255, 255, 255, 205);
        color: #0077B6;
        border: 1px solid #8ADCF7;
    }

    QPushButton#secondaryButton:hover {
        background: white;
        color: #005F8F;
        border: 1px solid #00AEEF;
    }

    QToolButton {
        background-color: rgba(255, 255, 255, 225);
        border: 1px solid rgba(126, 214, 246, 180);
        border-radius: 12px;
        padding: 7px;
    }

    QToolButton:hover {
        background-color: white;
        border: 1px solid #00AEEF;
    }

    QToolButton:pressed {
        background-color: #D7F4FF;
    }

    QProgressBar {
        background-color: rgba(226, 244, 250, 210);
        border: none;
        border-radius: 10px;
        text-align: center;
        color: transparent;
        height: 20px;
    }

    QProgressBar::chunk {
        background: qlineargradient(
            x1:0, y1:0, x2:1, y2:0,
            stop:0 #48CAE4,
            stop:0.55 #00B4D8,
            stop:1 #0077B6
        );
        border-radius: 10px;
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

    QQuickWidget {
        border: none;
        background: transparent;
    }
)");

    UserProfile profile;
    DailyEntry dailyEntry;

    if (profile.isFirstStart()) {
        bool ok;
        int age = QInputDialog::getInt(nullptr, QString::fromUtf8("Первый запуск"), QString::fromUtf8("Введите ваш возраст:"), 25, 1, 120, 1, &ok);
        if (!ok) return 0;

        int weight = QInputDialog::getInt(nullptr, QString::fromUtf8("Первый запуск"), QString::fromUtf8("Введите ваш вес (кг):"), 70, 20, 300, 1, &ok);
        if (!ok) return 0;

        QStringList genders;
        genders << QString::fromUtf8("Мужской") << QString::fromUtf8("Женский");
        QString genderSelection = QInputDialog::getItem(nullptr, QString::fromUtf8("Первый запуск"), QString::fromUtf8("Выберите ваш пол:"), genders, 0, false, &ok);
        if (!ok) return 0;

        profile.age = age;
        profile.weightKg = weight;
        profile.gender = (genderSelection == QString::fromUtf8("Мужской")) ? "male" : "female";
        profile.dailyGoal = profile.dailyGoalMl();

        if (!profile.createProfile()) {
            QMessageBox::critical(nullptr, QString::fromUtf8("Ошибка"), QString::fromUtf8("Не удалось сохранить профиль пользователя."));
            return -1;
        }
        QMessageBox::information(nullptr, QString::fromUtf8("Успех"), QString::fromUtf8("Профиль успешно создан!\nВаша дневная норма воды: %1 мл.").arg(profile.dailyGoal));
    } else {
        profile.loadProfile();
    }

    dailyEntry.create_daily_profile();

    QWidget mainWindow;
    mainWindow.resize(430, 520);
    mainWindow.setMinimumSize(360, 480);
    mainWindow.setWindowTitle(QStringLiteral("Water Tracker"));
    mainWindow.setWindowIcon(appIcon);
    Tracker tracker(&mainWindow);

    auto *layout = new QVBoxLayout(&mainWindow);
    layout->setContentsMargins(20, 18, 20, 20);
    layout->setSpacing(14);

    auto *topBarLayout = new QHBoxLayout();
    topBarLayout->setSpacing(10);

    auto *statisticsButton = new QToolButton(&mainWindow);
    statisticsButton->setIcon(createStatisticsIcon());
    statisticsButton->setIconSize(QSize(24, 24));
    statisticsButton->setFixedSize(42, 42);
    statisticsButton->setToolTip(QString::fromUtf8("Статистика"));
    topBarLayout->addWidget(statisticsButton, 0, Qt::AlignLeft);

    auto *titleLayout = new QVBoxLayout();
    titleLayout->setSpacing(0);
    auto *titleLabel = new QLabel(QStringLiteral("Water Tracker"), &mainWindow);
    titleLabel->setObjectName(QStringLiteral("appTitle"));
    auto *subtitleLabel = new QLabel(QString::fromUtf8("Твой дневной ритм воды"), &mainWindow);
    subtitleLabel->setObjectName(QStringLiteral("appSubtitle"));
    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(subtitleLabel);
    topBarLayout->addLayout(titleLayout, 1);

    auto *settingsToolButton = new QToolButton(&mainWindow);
    settingsToolButton->setIcon(createSettingsIcon());
    settingsToolButton->setIconSize(QSize(23, 23));
    settingsToolButton->setFixedSize(42, 42);
    settingsToolButton->setToolTip(QString::fromUtf8("Настройки напоминаний"));
    topBarLayout->addWidget(settingsToolButton, 0, Qt::AlignRight);
    layout->addLayout(topBarLayout);

    auto *progressCard = new QFrame(&mainWindow);
    progressCard->setObjectName(QStringLiteral("surfaceCard"));
    auto *progressLayout = new QVBoxLayout(progressCard);
    progressLayout->setContentsMargins(20, 18, 20, 18);
    progressLayout->setSpacing(10);

    auto *ambientView = createAmbientView(progressCard, 108);
    auto *captionLabel = new QLabel(QString::fromUtf8("Сегодня выпито"), progressCard);
    captionLabel->setObjectName(QStringLiteral("captionLabel"));
    auto *progressLabel = new QLabel(progressCard);
    progressLabel->setObjectName(QStringLiteral("progressLabel"));
    auto *waterProgressBar = new QProgressBar(progressCard);
    waterProgressBar->setRange(0, 100);
    waterProgressBar->setTextVisible(false);
    auto *percentLabel = new QLabel(progressCard);
    percentLabel->setObjectName(QStringLiteral("percentLabel"));
    auto *remainingLabel = new QLabel(progressCard);
    remainingLabel->setObjectName(QStringLiteral("remainingLabel"));

    auto *progressMetaLayout = new QHBoxLayout();
    progressMetaLayout->addWidget(percentLabel, 0, Qt::AlignLeft);
    progressMetaLayout->addStretch();
    progressMetaLayout->addWidget(remainingLabel, 0, Qt::AlignRight);

    progressLayout->addWidget(ambientView);
    progressLayout->addWidget(captionLabel);
    progressLayout->addWidget(progressLabel);
    progressLayout->addWidget(waterProgressBar);
    progressLayout->addLayout(progressMetaLayout);
    layout->addWidget(progressCard);

    auto *inputCard = new QFrame(&mainWindow);
    inputCard->setObjectName(QStringLiteral("inputCard"));
    auto *inputLayout = new QVBoxLayout(inputCard);
    inputLayout->setContentsMargins(16, 16, 16, 16);
    inputLayout->setSpacing(10);

    QLineEdit *inputField = new QLineEdit(inputCard);
    inputField->setPlaceholderText(QString::fromUtf8("Количество воды, мл"));
    inputField->setValidator(new QIntValidator(1, 10000, inputField));

    QPushButton *addButton = new QPushButton(QString::fromUtf8("Выпить воду"), inputCard);
    addButton->setIcon(createWaterIcon());
    addButton->setIconSize(QSize(22, 22));

    QPushButton *settingsButton = new QPushButton(QString::fromUtf8("Настройки напоминаний"), inputCard);
    settingsButton->setObjectName(QStringLiteral("secondaryButton"));
    settingsButton->setIcon(createSettingsIcon());
    settingsButton->setIconSize(QSize(18, 18));

    inputLayout->addWidget(inputField);
    inputLayout->addWidget(addButton);
    inputLayout->addWidget(settingsButton);
    layout->addWidget(inputCard);
    layout->addStretch();

    auto updateLabels = [&](bool animated = true) {
        const int consumed = dailyEntry.getConsumed();
        const int goal = profile.dailyGoal;
        const int remaining = qMax(0, goal - consumed);
        const int percent = progressPercent(consumed, goal);

        progressLabel->setText(QString::fromUtf8("%1 / %2 мл").arg(consumed).arg(goal));
        percentLabel->setText(QString::fromUtf8("%1% цели").arg(percent));
        remainingLabel->setText(QString::fromUtf8("Осталось %1 мл").arg(remaining));

        if (animated) {
            animateProgress(waterProgressBar, percent);
        } else {
            waterProgressBar->setValue(qBound(0, percent, 100));
        }
    };

    updateLabels(false);

    QObject::connect(addButton, &QPushButton::clicked, [&]() {
        int amount = inputField->text().toInt();
        if (amount <= 0) {
            QMessageBox::warning(&mainWindow, QString::fromUtf8("Ошибка"), QString::fromUtf8("Введите количество воды больше нуля."));
            return;
        }

        if (dailyEntry.add_consumed(amount)) {
            inputField->clear();
            updateLabels(true);
        } else {
            QMessageBox::warning(&mainWindow, QString::fromUtf8("Ошибка"), QString::fromUtf8("Не удалось добавить выпитую воду."));
        }
    });

    QObject::connect(inputField, &QLineEdit::returnPressed, addButton, &QPushButton::click);

    QObject::connect(settingsButton, &QPushButton::clicked, [&]() {
        tracker.openReminderSettings(&mainWindow);
    });

    QObject::connect(settingsToolButton, &QToolButton::clicked, [&]() {
        tracker.openReminderSettings(&mainWindow);
    });

    QObject::connect(statisticsButton, &QToolButton::clicked, [&]() {
        CheckInfoDialog statisticsDialog(profile.dailyGoal, &mainWindow);
        statisticsDialog.exec();
        updateLabels(false);
    });

    mainWindow.setWindowOpacity(0.0);
    mainWindow.show();
    fadeInWindow(&mainWindow);

    return QApplication::exec();
}
