#include "ProgressPage.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QDate>

ProgressPage::ProgressPage(const std::string& dbPath, QWidget* parent)
    : QWidget(parent),
      manager(dbPath)
{
    setStyleSheet("background-color:#1a1a1a; color:white;");

    auto* root = new QVBoxLayout(this);

    QLabel* title = new QLabel("📊 Progress");
    title->setStyleSheet("font-size:24px; font-weight:bold;");
    root->addWidget(title);

    statsLabel = new QLabel();
    statsLabel->setStyleSheet("font-size:14px; color:#cccccc;");
    root->addWidget(statsLabel);

    // === ГРАФИК ===
    chartView = new QChartView();
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumHeight(300);
    chartView->setStyleSheet("background-color:#2a2a2a;");
    root->addWidget(chartView);

    // === СПИСОК ЗАПИСЕЙ ===
    list = new QListWidget();
    list->setStyleSheet("background:#2a2a2a; color:white;");
    root->addWidget(list, 1);

    auto* btnRow = new QHBoxLayout();
    addBtn = new QPushButton("➕ Add Progress");
    deleteBtn = new QPushButton("🗑 Delete");

    QString btnStyle =
        "QPushButton { background:#333; color:white; padding:6px 10px; border-radius:4px; }"
        "QPushButton:hover { background:#444; }";

    addBtn->setStyleSheet(btnStyle);
    deleteBtn->setStyleSheet(btnStyle);

    btnRow->addWidget(addBtn);
    btnRow->addWidget(deleteBtn);
    root->addLayout(btnRow);

    loadProgress();
    loadStats();
    loadChart();

    // === Добавление записи ===
    connect(addBtn, &QPushButton::clicked, this, [this]{
        bool ok;

        double weight = QInputDialog::getDouble(this, "Weight", "Enter weight (kg):", 70, 0, 500, 1, &ok);
        if (!ok) return;

        int volume = QInputDialog::getInt(this, "Volume", "Total volume (kg):", 0, 0, 1000000, 1, &ok);
        if (!ok) return;

        Progress p{};
        p.user_id = 1;
        p.date = QDate::currentDate().toString("yyyy-MM-dd").toUtf8().constData();
        p.weight = weight;
        p.total_volume = volume;
        p.calendar_id = 0;

        if (!manager.saveProgress(p)) {
            QMessageBox::warning(this, "Error", "Failed to save progress");
            return;
        }

        loadProgress();
        loadStats();
        loadChart();
    });

    // === Удаление записи ===
    connect(deleteBtn, &QPushButton::clicked, this, [this]{
        auto* item = list->currentItem();
        if (!item) return;

        int id = item->data(Qt::UserRole).toInt();
        if (!manager.deleteProgress(id)) {
            QMessageBox::warning(this, "Error", "Failed to delete");
            return;
        }

        loadProgress();
        loadStats();
        loadChart();
    });
}

void ProgressPage::loadProgress() {
    list->clear();

    auto all = manager.getAllProgressForUser(1);
    for (auto& p : all) {
        QString text = QString("%1 — %2 kg, volume %3")
                           .arg(QString::fromStdString(p.date))
                           .arg(p.weight)
                           .arg(p.total_volume);

        auto* item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, p.id);
        list->addItem(item);
    }
}

void ProgressPage::loadStats() {
    auto stats = manager.computeStatistics(1);

    if (stats.count == 0) {
        statsLabel->setText("No progress data yet");
        return;
    }

    QString s;
    s += QString("Entries: %1\n").arg(stats.count);
    s += QString("Avg weight: %1 kg\n").arg(stats.avg_weight, 0, 'f', 1);
    s += QString("Min weight: %1 kg\n").arg(stats.min_weight);
    s += QString("Max weight: %1 kg\n").arg(stats.max_weight);
    s += QString("Weight stddev: %1\n").arg(stats.weight_stddev, 0, 'f', 2);
    s += QString("Total volume: %1 kg\n").arg(stats.total_volume);
    s += QString("Avg daily volume: %1 kg\n").arg(stats.avg_daily_volume, 0, 'f', 1);

    statsLabel->setText(s);
}

void ProgressPage::loadChart() {
    auto all = manager.getAllProgressForUser(1);

    QLineSeries* series = new QLineSeries();
    series->setColor(QColor("#4CAF50"));
    series->setName("Weight");

    int index = 0;
    for (auto& p : all) {
        series->append(index++, p.weight);
    }

    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Weight Progress");
    chart->setBackgroundBrush(QBrush(QColor("#2a2a2a")));
    chart->setTitleBrush(QBrush(Qt::white));

    QValueAxis* axisX = new QValueAxis();
    axisX->setTitleText("Entry #");
    axisX->setLabelsColor(Qt::white);
    axisX->setGridLineColor(QColor("#444"));

    QValueAxis* axisY = new QValueAxis();
    axisY->setTitleText("Weight (kg)");
    axisY->setLabelsColor(Qt::white);
    axisY->setGridLineColor(QColor("#444"));

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    series->attachAxis(axisX);
    series->attachAxis(axisY);

    chartView->setChart(chart);
}
