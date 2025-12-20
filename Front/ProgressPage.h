#ifndef PROGRESSPAGE_H
#define PROGRESSPAGE_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

#include "ProgressManager.h"


class ProgressPage : public QWidget {
    Q_OBJECT

public:
    explicit ProgressPage(const std::string& dbPath, QWidget* parent = nullptr);

private:
    ProgressManager manager;

    QListWidget* list;
    QLabel* statsLabel;
    QPushButton* addBtn;
    QPushButton* deleteBtn;

    QChartView* chartView;

    void loadProgress();
    void loadStats();
    void loadChart();
};

#endif
