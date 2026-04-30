//
// Created by User on 23.04.2026.
//

#include "Tracker.h"

#include <QAction>
#include <QApplication>
#include <QIcon>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QSettings>
#include <QStyle>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QWidget>

namespace {
constexpr int DefaultReminderIntervalMinutes = 60;
constexpr int MinReminderIntervalMinutes = 1;
constexpr int MaxReminderIntervalMinutes = 1440;
constexpr int MillisecondsInMinute = 60 * 1000;

QString settingsKey() {
    return QStringLiteral("reminders/intervalMinutes");
}
}

Tracker::Tracker(QWidget *parent)
    : QObject(parent),
      reminderTimer(new QTimer(this)),
      reminderTexts({
          QString::fromUtf8("Вода - наша жизнь. Будь крутым - пей воду!"),
          QString::fromUtf8("Не забудь о норме воды!"),
          QString::fromUtf8("Пара глотков сейчас - энергия на потом."),
          QString::fromUtf8("Твой организм просит воды. Дай ему шанс сиять."),
          QString::fromUtf8("Стакан воды - маленькая победа дня."),
          QString::fromUtf8("Будь в ресурсе: сделай водную паузу."),
          QString::fromUtf8("Гидратация - это стиль. Пей воду!"),
          QString::fromUtf8("Вода рядом? Самое время сделать глоток.")
      }) {
    QSettings settings(QStringLiteral("WaterTracker"), QStringLiteral("WaterTracker"));
    reminderInterval = settings.value(settingsKey(), DefaultReminderIntervalMinutes).toInt();
    if (reminderInterval < MinReminderIntervalMinutes || reminderInterval > MaxReminderIntervalMinutes) {
        reminderInterval = DefaultReminderIntervalMinutes;
    }

    createTrayIcon();

    connect(reminderTimer, &QTimer::timeout, this, &Tracker::showWaterReminder);
    restartReminderTimer();
}

Tracker::~Tracker() {
    if (trayIcon != nullptr) {
        trayIcon->hide();
    }
    delete trayMenu;
}

int Tracker::reminderIntervalMinutes() const {
    return reminderInterval;
}

void Tracker::openReminderSettings(QWidget *parent) {
    bool ok = false;
    const int minutes = QInputDialog::getInt(
        parent,
        QString::fromUtf8("Настройки напоминаний"),
        QString::fromUtf8("Через сколько минут напоминать пить воду?"),
        reminderInterval,
        MinReminderIntervalMinutes,
        MaxReminderIntervalMinutes,
        1,
        &ok
    );

    if (!ok) {
        return;
    }

    setReminderIntervalMinutes(minutes);

    QMessageBox::information(
        parent,
        QString::fromUtf8("Готово"),
        QString::fromUtf8("Напоминания будут приходить каждые %1 мин.").arg(reminderInterval)
    );
}

void Tracker::setReminderIntervalMinutes(int minutes) {
    if (minutes < MinReminderIntervalMinutes) {
        minutes = MinReminderIntervalMinutes;
    } else if (minutes > MaxReminderIntervalMinutes) {
        minutes = MaxReminderIntervalMinutes;
    }

    reminderInterval = minutes;

    QSettings settings(QStringLiteral("WaterTracker"), QStringLiteral("WaterTracker"));
    settings.setValue(settingsKey(), reminderInterval);

    restartReminderTimer();
}

void Tracker::showWaterReminder() {
    if (trayIcon == nullptr) {
        return;
    }

    trayIcon->showMessage(
        QString::fromUtf8("Water Tracker"),
        nextReminderText(),
        QSystemTrayIcon::Information,
        8000
    );
}

void Tracker::createTrayIcon() {
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        return;
    }

    trayIcon = new QSystemTrayIcon(this);
    QIcon icon(QStringLiteral(":/icons/Iconka.ico"));
    if (icon.isNull()) {
        icon = QIcon::fromTheme(
            QStringLiteral("preferences-system-notifications"),
            QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation)
        );
    }
    trayIcon->setIcon(icon);
    trayIcon->setToolTip(QString::fromUtf8("Water Tracker"));

    trayMenu = new QMenu();
    auto *settingsAction = trayMenu->addAction(QString::fromUtf8("Настройки напоминаний"));
    auto *remindNowAction = trayMenu->addAction(QString::fromUtf8("Напомнить сейчас"));
    trayMenu->addSeparator();
    auto *quitAction = trayMenu->addAction(QString::fromUtf8("Выход"));

    connect(settingsAction, &QAction::triggered, this, [this]() {
        openReminderSettings(qobject_cast<QWidget *>(parent()));
    });
    connect(remindNowAction, &QAction::triggered, this, &Tracker::showWaterReminder);
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();
}

void Tracker::restartReminderTimer() {
    reminderTimer->start(reminderInterval * MillisecondsInMinute);
}

QString Tracker::nextReminderText() {
    if (reminderTexts.isEmpty()) {
        return QString::fromUtf8("Не забудь выпить воды!");
    }

    const QString text = reminderTexts.at(currentReminderIndex);
    currentReminderIndex = (currentReminderIndex + 1) % reminderTexts.size();
    return text;
}

