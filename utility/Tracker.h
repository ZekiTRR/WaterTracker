//
// Created by User on 23.04.2026.
//

#ifndef APPTRACKER_TRACKER_H
#define APPTRACKER_TRACKER_H

#include <QObject>
#include <QStringList>

class QMenu;
class QSystemTrayIcon;
class QTimer;
class QWidget;

class Tracker : public QObject {
    Q_OBJECT

public:
    explicit Tracker(QWidget *parent = nullptr);
    ~Tracker() override;

    int reminderIntervalMinutes() const;

public slots:
    void openReminderSettings(QWidget *parent = nullptr);
    void setReminderIntervalMinutes(int minutes);

private slots:
    void showWaterReminder();

private:
    void createTrayIcon();
    void restartReminderTimer();
    QString nextReminderText();

    QSystemTrayIcon *trayIcon = nullptr;
    QMenu *trayMenu = nullptr;
    QTimer *reminderTimer = nullptr;
    QStringList reminderTexts;
    int currentReminderIndex = 0;
    int reminderInterval = 60;
};

#endif //APPTRACKER_TRACKER_H
