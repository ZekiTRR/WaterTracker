#ifndef WATERTRACKER_CHECKINFO_H
#define WATERTRACKER_CHECKINFO_H

#include <QDate>
#include <QDialog>
#include <QString>
#include <QVector>

class ContributionGrid;
class QLabel;
class QLineEdit;
class QListWidget;
class QProgressBar;
class QShowEvent;
class QWidget;

struct DailyStatistic {
    QDate date;
    int consumedMl = 0;
    int dailyGoalMl = 0;

    int completionPercent() const;
    bool isCompleted() const;
};

class CheckInfoDialog final : public QDialog {
public:
    explicit CheckInfoDialog(int dailyGoalMl, QWidget *parent = nullptr);

private:
    void showEvent(QShowEvent *event) override;

    void buildUi();
    void loadStatistics();
    void applyDateFilter(const QString &filterText);
    void selectDate(const QDate &date);
    void updateSummary(const DailyStatistic &statistic);

    int dailyGoalMl = 0;
    QVector<DailyStatistic> statistics;
    QLineEdit *dateFilter = nullptr;
    QListWidget *dateList = nullptr;
    QLabel *consumedValue = nullptr;
    QLabel *percentValue = nullptr;
    QLabel *selectedDateLabel = nullptr;
    QProgressBar *completionProgress = nullptr;
    ContributionGrid *contributionGrid = nullptr;
    bool hasPlayedOpenAnimation = false;
};

#endif //WATERTRACKER_CHECKINFO_H
