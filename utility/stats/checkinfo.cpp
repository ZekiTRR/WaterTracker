#include "checkinfo.h"

#include "../Storage.h"

#include <QAbstractAnimation>
#include <QEasingCurve>
#include <QColor>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>
#include <QFont>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLocale>
#include <QMap>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QProgressBar>
#include <QPropertyAnimation>
#include <QQuickWidget>
#include <QRectF>
#include <QResizeEvent>
#include <QShowEvent>
#include <QStringList>
#include <QtMath>
#include <QVariant>
#include <QVariantAnimation>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

#include <algorithm>
#include <cmath>

namespace {
constexpr int GridWeeks = 26;
constexpr int GridRows = 7;
constexpr int GridLeftPadding = 36;
constexpr int GridRightPadding = 12;
constexpr int GridTopPadding = 28;
constexpr int GridBottomPadding = 54;
constexpr int MinCellSize = 8;
constexpr int MaxCellSize = 24;

QString formatMl(int value) {
    return QStringLiteral("%1 мл").arg(value);
}

DailyStatistic statisticForFile(const QString &filePath, int dailyGoalMl) {
    DailyStatistic statistic;
    statistic.dailyGoalMl = dailyGoalMl;

    const QFileInfo fileInfo(filePath);
    statistic.date = QDate::fromString(fileInfo.completeBaseName(), Qt::ISODate);

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return statistic;
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject()) {
        return statistic;
    }

    const QJsonObject object = document.object();
    const QDate jsonDate = QDate::fromString(object.value(QStringLiteral("date")).toString(), Qt::ISODate);
    if (jsonDate.isValid()) {
        statistic.date = jsonDate;
    }
    statistic.consumedMl = object.value(QStringLiteral("consumedMl")).toInt(0);
    return statistic;
}

QString shortMonthName(const QDate &date) {
    static const QStringList names = {
        QString::fromUtf8("янв"), QString::fromUtf8("фев"), QString::fromUtf8("мар"),
        QString::fromUtf8("апр"), QString::fromUtf8("май"), QString::fromUtf8("июн"),
        QString::fromUtf8("июл"), QString::fromUtf8("авг"), QString::fromUtf8("сен"),
        QString::fromUtf8("окт"), QString::fromUtf8("ноя"), QString::fromUtf8("дек")
    };
    return names.value(date.month() - 1);
}

struct GridMetrics {
    int columnCount = 0;
    qreal cellSize = MinCellSize;
    qreal gap = 3.0;
    qreal left = GridLeftPadding;
    qreal top = GridTopPadding;
};

void addPanelShadow(QWidget *widget, int blur = 28, int yOffset = 8, int alpha = 30) {
    auto *shadow = new QGraphicsDropShadowEffect(widget);
    shadow->setBlurRadius(blur);
    shadow->setOffset(0, yOffset);
    shadow->setColor(QColor(6, 68, 92, alpha));
    widget->setGraphicsEffect(shadow);
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

class ContributionGrid final : public QWidget {
public:
    explicit ContributionGrid(QWidget *parent = nullptr) : QWidget(parent) {
        setMouseTracking(true);
        setMinimumHeight(210);
        setMinimumWidth(320);

        hoverAnimation.setDuration(170);
        hoverAnimation.setEasingCurve(QEasingCurve::OutCubic);
        connect(&hoverAnimation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
            hoverProgress = value.toReal();
            update();
        });
        connect(&hoverAnimation, &QVariantAnimation::finished, this, [this]() {
            if (clearHoverAfterAnimation) {
                hoveredDate = {};
                clearHoverAfterAnimation = false;
                update();
            }
        });
    }

    void setStatistics(const QVector<DailyStatistic> &newStatistics, int newDailyGoalMl) {
        dailyGoalMl = newDailyGoalMl;
        statisticsByDate.clear();
        for (const DailyStatistic &statistic : newStatistics) {
            if (statistic.date.isValid()) {
                statisticsByDate.insert(statistic.date, statistic);
            }
        }

        const QDate today = QDate::currentDate();
        startDate = today.addDays(-(GridWeeks * GridRows - 1));
        startDate = startDate.addDays(-(startDate.dayOfWeek() - 1));
        updateGeometry();
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        const GridMetrics metrics = currentMetrics();
        if (!startDate.isValid() || metrics.columnCount <= 0) {
            return;
        }

        painter.setFont(QFont(QStringLiteral("Segoe UI"), qMax(8, qRound(metrics.cellSize * 0.48))));
        drawWeekdayLabels(painter, metrics);

        const QDate today = QDate::currentDate();
        QDate lastMonthLabel;

        for (QDate date = startDate; date <= today; date = date.addDays(1)) {
            const int daysFromStart = startDate.daysTo(date);
            const int column = daysFromStart / GridRows;
            const int row = date.dayOfWeek() - 1;
            const QRectF cell = cellRect(metrics, column, row);

            if (date.day() <= 7 && (!lastMonthLabel.isValid() || lastMonthLabel.month() != date.month())) {
                painter.setPen(QColor(QStringLiteral("#5C7A89")));
                painter.drawText(QRectF(cell.left(), 4, metrics.cellSize * 2.8, 18), Qt::AlignLeft | Qt::AlignVCenter, shortMonthName(date));
                lastMonthLabel = date;
            }

            const DailyStatistic statistic = statisticsByDate.value(date, DailyStatistic{date, 0, dailyGoalMl});
            const QColor color = statistic.isCompleted()
                                     ? QColor(QStringLiteral("#00AEEF"))
                                     : QColor(QStringLiteral("#D5E0E7"));
            const bool isHovered = date == hoveredDate && hoverProgress > 0.0;
            const qreal grow = isHovered ? (2.6 * hoverProgress) : 0.0;
            const QRectF animatedCell = cell.adjusted(-grow, -grow, grow, grow);

            painter.setPen(Qt::NoPen);
            painter.setBrush(color);
            painter.drawRoundedRect(animatedCell, qMax<qreal>(3.0, metrics.cellSize * 0.22), qMax<qreal>(3.0, metrics.cellSize * 0.22));

            if (isHovered) {
                QColor border(QStringLiteral("#06445C"));
                border.setAlphaF(0.34 * hoverProgress);
                painter.setPen(QPen(border, 1.4));
                painter.setBrush(Qt::NoBrush);
                painter.drawRoundedRect(animatedCell.adjusted(-1, -1, 1, 1), qMax<qreal>(4.0, metrics.cellSize * 0.24), qMax<qreal>(4.0, metrics.cellSize * 0.24));
            }
        }

        drawHoverCard(painter);
    }

    void mouseMoveEvent(QMouseEvent *event) override {
        const QDate date = dateAt(event->pos());
        if (!date.isValid() || date > QDate::currentDate()) {
            clearHover();
            return;
        }

        if (date != hoveredDate) {
            hoveredDate = date;
            clearHoverAfterAnimation = false;
            startHoverAnimation(1.0);
        }
    }

    void leaveEvent(QEvent *) override {
        clearHover();
    }

    void resizeEvent(QResizeEvent *) override {
        update();
    }

private:
    GridMetrics currentMetrics() const {
        GridMetrics metrics;
        if (!startDate.isValid()) {
            return metrics;
        }

        metrics.columnCount = startDate.daysTo(QDate::currentDate()) / GridRows + 1;
        const qreal availableWidth = qMax<qreal>(120.0, width() - GridLeftPadding - GridRightPadding);
        const qreal availableHeight = qMax<qreal>(90.0, height() - GridTopPadding - GridBottomPadding);
        const qreal cellByWidth = availableWidth / (metrics.columnCount + (metrics.columnCount - 1) * 0.28);
        const qreal cellByHeight = availableHeight / (GridRows + (GridRows - 1) * 0.28);

        metrics.cellSize = qBound<qreal>(MinCellSize, qMin(cellByWidth, cellByHeight), MaxCellSize);
        metrics.gap = qMax<qreal>(2.0, metrics.cellSize * 0.28);
        metrics.left = GridLeftPadding;
        metrics.top = GridTopPadding;
        return metrics;
    }

    QRectF cellRect(const GridMetrics &metrics, int column, int row) const {
        const qreal step = metrics.cellSize + metrics.gap;
        return QRectF(
            metrics.left + column * step,
            metrics.top + row * step,
            metrics.cellSize,
            metrics.cellSize
        );
    }

    QDate dateAt(const QPoint &point) const {
        const GridMetrics metrics = currentMetrics();
        const qreal x = point.x() - metrics.left;
        const qreal y = point.y() - metrics.top;
        if (x < 0 || y < 0) {
            return {};
        }

        const qreal step = metrics.cellSize + metrics.gap;
        const int column = qFloor(x / step);
        const int row = qFloor(y / step);
        if (row < 0 || row >= GridRows || column < 0 || column >= metrics.columnCount) {
            return {};
        }
        if (std::fmod(x, step) > metrics.cellSize || std::fmod(y, step) > metrics.cellSize) {
            return {};
        }

        return startDate.addDays(column * GridRows + row);
    }

    void drawWeekdayLabels(QPainter &painter, const GridMetrics &metrics) const {
        const QStringList labels = {
            QString::fromUtf8("Пн"), QString(), QString::fromUtf8("Ср"), QString(),
            QString::fromUtf8("Пт"), QString(), QString::fromUtf8("Вс")
        };

        painter.setPen(QColor(QStringLiteral("#5C7A89")));
        for (int row = 0; row < labels.size(); ++row) {
            if (labels.at(row).isEmpty()) {
                continue;
            }

            const QRectF labelRect(
                0,
                metrics.top + row * (metrics.cellSize + metrics.gap) - 1,
                metrics.left - 8,
                metrics.cellSize + 2
            );
            painter.drawText(labelRect, Qt::AlignRight | Qt::AlignVCenter, labels.at(row));
        }
    }

    void drawHoverCard(QPainter &painter) const {
        if (!hoveredDate.isValid() || hoverProgress <= 0.01) {
            return;
        }

        const DailyStatistic statistic = statisticsByDate.value(hoveredDate, DailyStatistic{hoveredDate, 0, dailyGoalMl});
        const QLocale locale(QLocale::Russian, QLocale::Russia);
        const QString dateText = locale.toString(hoveredDate, QStringLiteral("d MMMM yyyy"));
        const QString detailText = QString::fromUtf8("%1 из %2  •  %3%")
                                       .arg(formatMl(statistic.consumedMl))
                                       .arg(formatMl(statistic.dailyGoalMl))
                                       .arg(statistic.completionPercent());

        const qreal cardWidth = qMin<qreal>(width() - 18, 330);
        const QRectF cardRect(
            qMax<qreal>(8.0, width() - cardWidth - 10),
            height() - 44 - 8 * hoverProgress,
            cardWidth,
            40
        );

        QColor shadow(6, 68, 92, qRound(32 * hoverProgress));
        painter.setPen(Qt::NoPen);
        painter.setBrush(shadow);
        painter.drawRoundedRect(cardRect.translated(0, 4), 8, 8);

        QColor background(255, 255, 255, qRound(238 * hoverProgress));
        painter.setBrush(background);
        painter.drawRoundedRect(cardRect, 8, 8);

        QColor accent = statistic.isCompleted() ? QColor(QStringLiteral("#00AEEF")) : QColor(QStringLiteral("#AEBCC6"));
        accent.setAlphaF(hoverProgress);
        painter.setBrush(accent);
        painter.drawRoundedRect(QRectF(cardRect.left() + 10, cardRect.top() + 11, 16, 16), 4, 4);

        QColor titleColor(QStringLiteral("#06445C"));
        titleColor.setAlphaF(hoverProgress);
        painter.setPen(titleColor);
        painter.setFont(QFont(QStringLiteral("Segoe UI"), 9, QFont::DemiBold));
        painter.drawText(QRectF(cardRect.left() + 34, cardRect.top() + 5, cardRect.width() - 44, 16), Qt::AlignLeft | Qt::AlignVCenter, dateText);

        QColor detailColor(QStringLiteral("#5C7A89"));
        detailColor.setAlphaF(hoverProgress);
        painter.setPen(detailColor);
        painter.setFont(QFont(QStringLiteral("Segoe UI"), 8));
        painter.drawText(QRectF(cardRect.left() + 34, cardRect.top() + 21, cardRect.width() - 44, 14), Qt::AlignLeft | Qt::AlignVCenter, detailText);
    }

    void startHoverAnimation(qreal target) {
        hoverAnimation.stop();
        hoverAnimation.setStartValue(hoverProgress);
        hoverAnimation.setEndValue(target);
        hoverAnimation.start();
    }

    void clearHover() {
        if (!hoveredDate.isValid() || clearHoverAfterAnimation) {
            return;
        }

        clearHoverAfterAnimation = true;
        startHoverAnimation(0.0);
    }

    QDate startDate;
    QDate hoveredDate;
    int dailyGoalMl = 0;
    qreal hoverProgress = 0.0;
    bool clearHoverAfterAnimation = false;
    QVariantAnimation hoverAnimation;
    QMap<QDate, DailyStatistic> statisticsByDate;
};

int DailyStatistic::completionPercent() const {
    if (dailyGoalMl <= 0) {
        return 0;
    }
    return qRound((static_cast<double>(consumedMl) / dailyGoalMl) * 100.0);
}

bool DailyStatistic::isCompleted() const {
    return dailyGoalMl > 0 && consumedMl >= dailyGoalMl;
}

CheckInfoDialog::CheckInfoDialog(int dailyGoalMl, QWidget *parent)
    : QDialog(parent), dailyGoalMl(dailyGoalMl) {
    setWindowOpacity(0.0);
    buildUi();
    loadStatistics();
    applyDateFilter(QString());

    if (!statistics.isEmpty()) {
        selectDate(statistics.first().date);
    }
}

void CheckInfoDialog::showEvent(QShowEvent *event) {
    QDialog::showEvent(event);

    if (hasPlayedOpenAnimation) {
        return;
    }

    hasPlayedOpenAnimation = true;
    setWindowOpacity(0.0);

    auto *animation = new QPropertyAnimation(this, "windowOpacity", this);
    animation->setDuration(220);
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);
    animation->setEasingCurve(QEasingCurve::OutCubic);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void CheckInfoDialog::buildUi() {
    setWindowTitle(QString::fromUtf8("Статистика"));
    resize(780, 520);
    setMinimumSize(680, 430);
    setObjectName(QStringLiteral("statisticsDialog"));
    setStyleSheet(R"(
        QDialog#statisticsDialog {
            background: #EAFBFF;
        }

        QFrame#statsSidebar,
        QFrame#summaryPanel,
        QFrame#heatmapPanel {
            background-color: rgba(255, 255, 255, 225);
            border: 1px solid #9ADFF6;
            border-radius: 8px;
        }

        QLabel#panelTitle {
            color: #06445C;
            font-size: 12pt;
            font-weight: 700;
            padding: 0;
        }

        QLabel#metricValue {
            color: #0077B6;
            font-size: 22pt;
            font-weight: 800;
            padding: 0;
        }

        QLabel#metricCaption,
        QLabel#selectedDate {
            color: #5C7A89;
            font-size: 9.5pt;
            font-weight: 600;
            padding: 0;
        }

        QListWidget {
            background-color: white;
            border: 1px solid #BEEBFA;
            border-radius: 8px;
            padding: 4px;
            outline: 0;
        }

        QListWidget::item {
            border-radius: 6px;
            padding: 8px;
            margin: 2px;
        }

        QListWidget::item:selected {
            background-color: #00AEEF;
            color: white;
        }

        QQuickWidget {
            border: none;
            background: transparent;
        }
    )");

    auto *rootLayout = new QHBoxLayout(this);
    rootLayout->setContentsMargins(14, 14, 14, 14);
    rootLayout->setSpacing(12);

    auto *sidebar = new QFrame(this);
    sidebar->setObjectName(QStringLiteral("statsSidebar"));
    sidebar->setMinimumWidth(215);
    sidebar->setMaximumWidth(260);
    addPanelShadow(sidebar, 24, 7, 24);
    auto *sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setContentsMargins(12, 12, 12, 12);
    sidebarLayout->setSpacing(10);

    auto *dateTitle = new QLabel(QString::fromUtf8("Даты"), sidebar);
    dateTitle->setObjectName(QStringLiteral("panelTitle"));
    dateTitle->setAlignment(Qt::AlignLeft);
    sidebarLayout->addWidget(dateTitle);

    dateFilter = new QLineEdit(sidebar);
    dateFilter->setPlaceholderText(QString::fromUtf8("Фильтр по дате"));
    sidebarLayout->addWidget(dateFilter);

    dateList = new QListWidget(sidebar);
    sidebarLayout->addWidget(dateList, 1);
    rootLayout->addWidget(sidebar);

    auto *rightLayout = new QVBoxLayout();
    rightLayout->setSpacing(12);
    rootLayout->addLayout(rightLayout, 1);

    auto *summaryPanel = new QFrame(this);
    summaryPanel->setObjectName(QStringLiteral("summaryPanel"));
    addPanelShadow(summaryPanel);
    auto *summaryLayout = new QGridLayout(summaryPanel);
    summaryLayout->setContentsMargins(16, 14, 16, 14);
    summaryLayout->setHorizontalSpacing(24);
    summaryLayout->setVerticalSpacing(8);

    selectedDateLabel = new QLabel(summaryPanel);
    selectedDateLabel->setObjectName(QStringLiteral("selectedDate"));
    selectedDateLabel->setAlignment(Qt::AlignLeft);
    summaryLayout->addWidget(selectedDateLabel, 0, 0, 1, 2);

    auto *consumedCaption = new QLabel(QString::fromUtf8("Выпито за день"), summaryPanel);
    consumedCaption->setObjectName(QStringLiteral("metricCaption"));
    consumedCaption->setAlignment(Qt::AlignLeft);
    consumedValue = new QLabel(summaryPanel);
    consumedValue->setObjectName(QStringLiteral("metricValue"));
    consumedValue->setAlignment(Qt::AlignLeft);
    summaryLayout->addWidget(consumedCaption, 1, 0);
    summaryLayout->addWidget(consumedValue, 2, 0);

    auto *percentCaption = new QLabel(QString::fromUtf8("Выполнение цели"), summaryPanel);
    percentCaption->setObjectName(QStringLiteral("metricCaption"));
    percentCaption->setAlignment(Qt::AlignLeft);
    percentValue = new QLabel(summaryPanel);
    percentValue->setObjectName(QStringLiteral("metricValue"));
    percentValue->setAlignment(Qt::AlignLeft);
    summaryLayout->addWidget(percentCaption, 1, 1);
    summaryLayout->addWidget(percentValue, 2, 1);

    completionProgress = new QProgressBar(summaryPanel);
    completionProgress->setRange(0, 100);
    completionProgress->setTextVisible(false);
    summaryLayout->addWidget(completionProgress, 3, 0, 1, 2);
    rightLayout->addWidget(summaryPanel);

    auto *heatmapPanel = new QFrame(this);
    heatmapPanel->setObjectName(QStringLiteral("heatmapPanel"));
    auto *heatmapLayout = new QVBoxLayout(heatmapPanel);
    heatmapLayout->setContentsMargins(16, 14, 16, 14);
    heatmapLayout->setSpacing(10);

    auto *heatmapTitle = new QLabel(QString::fromUtf8("Аналитика выполнения"), heatmapPanel);
    heatmapTitle->setObjectName(QStringLiteral("panelTitle"));
    heatmapTitle->setAlignment(Qt::AlignLeft);
    heatmapLayout->addWidget(heatmapTitle);

    auto *ambientView = createAmbientView(heatmapPanel, 72);
    heatmapLayout->addWidget(ambientView);

    contributionGrid = new ContributionGrid(heatmapPanel);
    heatmapLayout->addWidget(contributionGrid, 1);
    rightLayout->addWidget(heatmapPanel, 1);

    connect(dateFilter, &QLineEdit::textChanged, this, &CheckInfoDialog::applyDateFilter);
    connect(dateList, &QListWidget::currentItemChanged, this, [this](QListWidgetItem *current) {
        if (current == nullptr) {
            return;
        }
        selectDate(QDate::fromString(current->data(Qt::UserRole).toString(), Qt::ISODate));
    });
}

void CheckInfoDialog::loadStatistics() {
    statistics.clear();

    const QDir dataDirectory(QString::fromStdString(Storage::getDataDirectory()));
    const QFileInfoList files = dataDirectory.entryInfoList(
        {QStringLiteral("????-??-??.json")},
        QDir::Files | QDir::Readable,
        QDir::Name
    );

    for (const QFileInfo &file : files) {
        DailyStatistic statistic = statisticForFile(file.absoluteFilePath(), dailyGoalMl);
        if (statistic.date.isValid()) {
            statistics.append(statistic);
        }
    }

    std::sort(statistics.begin(), statistics.end(), [](const DailyStatistic &left, const DailyStatistic &right) {
        return left.date > right.date;
    });

    contributionGrid->setStatistics(statistics, dailyGoalMl);
}

void CheckInfoDialog::applyDateFilter(const QString &filterText) {
    dateList->clear();
    const QLocale locale(QLocale::Russian, QLocale::Russia);
    const QString normalizedFilter = filterText.trimmed();

    for (const DailyStatistic &statistic : statistics) {
        const QString dateText = statistic.date.toString(Qt::ISODate);
        const QString prettyDateText = locale.toString(statistic.date, QStringLiteral("d MMMM yyyy"));
        if (!normalizedFilter.isEmpty()
            && !dateText.contains(normalizedFilter, Qt::CaseInsensitive)
            && !prettyDateText.contains(normalizedFilter, Qt::CaseInsensitive)) {
            continue;
        }

        const QString itemText = QString::fromUtf8("%1   %2%")
                                     .arg(prettyDateText)
                                     .arg(statistic.completionPercent());
        auto *item = new QListWidgetItem(itemText, dateList);
        item->setData(Qt::UserRole, dateText);
        item->setToolTip(QString::fromUtf8("Выпито: %1 / %2")
                             .arg(formatMl(statistic.consumedMl))
                             .arg(formatMl(statistic.dailyGoalMl)));
    }

    if (dateList->count() > 0) {
        dateList->setCurrentRow(0);
    } else {
        DailyStatistic empty;
        empty.dailyGoalMl = dailyGoalMl;
        updateSummary(empty);
    }
}

void CheckInfoDialog::selectDate(const QDate &date) {
    for (const DailyStatistic &statistic : statistics) {
        if (statistic.date == date) {
            updateSummary(statistic);
            return;
        }
    }
}

void CheckInfoDialog::updateSummary(const DailyStatistic &statistic) {
    const QLocale locale(QLocale::Russian, QLocale::Russia);
    selectedDateLabel->setText(statistic.date.isValid()
                                   ? locale.toString(statistic.date, QStringLiteral("d MMMM yyyy"))
                                   : QString::fromUtf8("Нет данных за выбранный период"));
    consumedValue->setText(QString::fromUtf8("%1 / %2")
                               .arg(formatMl(statistic.consumedMl))
                               .arg(formatMl(statistic.dailyGoalMl)));
    percentValue->setText(QStringLiteral("%1%").arg(statistic.completionPercent()));
    completionProgress->setValue(qMin(100, statistic.completionPercent()));
}
