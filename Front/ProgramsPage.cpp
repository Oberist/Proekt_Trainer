#include "ProgramsPage.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QDateTime>

ProgramsPage::ProgramsPage(const std::string& dbPath, QWidget* parent)
    : QWidget(parent),
      programManager(dbPath),
      trainingDayManager(dbPath),
      exerciseEntryManager(dbPath),
      exerciseManager(dbPath)
{
    // Общий стиль (тёмная тема)
    setStyleSheet("background-color:#1a1a1a; color:white;");

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(8,8,8,8);
    root->setSpacing(8);

    // Верхняя панель: заголовок + кнопки
    auto* topRow = new QHBoxLayout();
    topRow->setSpacing(8);

    QLabel* title = new QLabel("📅 Programs");
    title->setStyleSheet("font-size:20px; font-weight:bold; color:white;");
    topRow->addWidget(title, 1);

    addProgramBtn = new QPushButton("➕ Add Program");
    deleteProgramBtn = new QPushButton("🗑 Delete Program");

    // Стили кнопок, чтобы были видны на тёмном фоне
    QString btnStyle = "QPushButton { background:#2e2e2e; color:white; padding:6px 10px; border-radius:4px; }"
                       "QPushButton:hover { background:#3a3a3a; }";
    addProgramBtn->setStyleSheet(btnStyle);
    deleteProgramBtn->setStyleSheet(btnStyle);

    topRow->addWidget(addProgramBtn, 0);
    topRow->addWidget(deleteProgramBtn, 0);

    root->addLayout(topRow);

    // Список программ (главный элемент)
    programsList = new QListWidget();
    programsList->setStyleSheet("background:#2a2a2a; color:white; font-size:14px; padding:6px;");
    programsList->setSelectionMode(QAbstractItemView::SingleSelection);
    root->addWidget(programsList, 1);

    // Подсказка
    hintLabel = new QLabel("Кликни на программу, чтобы управлять днями и упражнениями");
    hintLabel->setStyleSheet("color:#cccccc; font-size:12px;");
    root->addWidget(hintLabel);

    // Загрузка данных
    loadPrograms();

    // Сигналы

    // Клик по программе — открывает меню управления
    connect(programsList, &QListWidget::itemClicked, this, [this](QListWidgetItem* item){
        if (!item) return;
        int programId = item->data(Qt::UserRole).toInt();
        onProgramClicked(programId);
    });

    // Добавление программы
    connect(addProgramBtn, &QPushButton::clicked, this, [this]{
        bool ok;
        QString name = QInputDialog::getText(this, "Add Program", "Program name:", QLineEdit::Normal, "", &ok);
        if (!ok || name.trimmed().isEmpty()) return;

        Program p{};
        p.name = name.toUtf8().constData();
        if (!programManager.createProgram(p, {})) {
            QMessageBox::warning(this, "Error", "Failed to create program");
            return;
        }
        loadPrograms();
    });

    // Удаление выбранной программы
    connect(deleteProgramBtn, &QPushButton::clicked, this, [this]{
        auto* item = programsList->currentItem();
        if (!item) {
            QMessageBox::information(this, "Info", "Select a program first");
            return;
        }
        int id = item->data(Qt::UserRole).toInt();
        auto res = QMessageBox::question(this, "Delete Program", "Are you sure you want to delete this program?");
        if (res != QMessageBox::Yes) return;

        if (!programManager.deleteProgram(id)) {
            QMessageBox::warning(this, "Error", "Failed to delete program");
            return;
        }
        loadPrograms();
    });
}

// Загрузка списка программ
void ProgramsPage::loadPrograms() {
    programsList->clear();
    auto programs = programManager.getAllPrograms();
    for (const auto& p : programs) {
        auto* it = new QListWidgetItem(QString::fromStdString(p.name));
        it->setData(Qt::UserRole, p.id);
        programsList->addItem(it);
    }

    if (!programs.empty()) programsList->setCurrentRow(0);
}

// Обработка клика по программе: диалог с вариантами
void ProgramsPage::onProgramClicked(int programId) {
    auto prog = programManager.getProgramById(programId);

    // Сформируем список дней
    QString daysText;
    if (prog.training_day_ids.empty()) {
        daysText = "No training days";
    } else {
        for (int id : prog.training_day_ids) {
            auto td = trainingDayManager.getTrainingDayById(id);
            daysText += QString::fromStdString(td.name) + " (id:" + QString::number(td.id) + ")\n";
        }
    }

    QString info = QString("Program: %1\n\nDays:\n%2").arg(QString::fromStdString(prog.name)).arg(daysText);

    QMessageBox msg(this);
    msg.setWindowTitle("Program actions");
    msg.setText(info);
    QPushButton* addDayBtn = msg.addButton("Add Day", QMessageBox::ActionRole);
    QPushButton* manageDayBtn = msg.addButton("Manage Day", QMessageBox::AcceptRole);
    QPushButton* delProgramBtn = msg.addButton("Delete Program", QMessageBox::DestructiveRole);
    msg.addButton("Cancel", QMessageBox::RejectRole);

    msg.exec();

    if (msg.clickedButton() == addDayBtn) {
        bool ok;
        QString name = QInputDialog::getText(this, "Add Training Day", "Day name:", QLineEdit::Normal, "", &ok);
        if (!ok || name.trimmed().isEmpty()) return;

        // Добавляем уникальность, чтобы потом найти id
        QString uniqueName = name + " [" + QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz") + "]";
        TrainingDay td{};
        td.name = uniqueName.toUtf8().constData();

        if (!trainingDayManager.createTrainingDay(td)) {
            QMessageBox::warning(this, "Error", "TrainingDayManager: failed to add training day");
            return;
        }

        int createdId = findCreatedTrainingDayIdByName(td.name);
        if (createdId <= 0) {
            QMessageBox::warning(this, "Error", "Cannot determine created training day id");
            return;
        }

        if (!programManager.addTrainingDayToProgram(programId, createdId)) {
            QMessageBox::warning(this, "Error", "Failed to attach day to program");
            return;
        }

        QMessageBox::information(this, "Done", "Training day added");
        loadPrograms();
    }
    else if (msg.clickedButton() == manageDayBtn) {
        if (prog.training_day_ids.empty()) {
            QMessageBox::information(this, "Info", "No training days to manage");
            return;
        }

        QStringList dayNames;
        for (int id : prog.training_day_ids) {
            auto td = trainingDayManager.getTrainingDayById(id);
            dayNames << QString::fromStdString(td.name) + " (id:" + QString::number(td.id) + ")";
        }

        bool ok;
        QString picked = QInputDialog::getItem(this, "Select Day", "Days:", dayNames, 0, false, &ok);
        if (!ok || picked.isEmpty()) return;

        int idStart = picked.lastIndexOf("(id:");
        int idEnd = picked.lastIndexOf(")");
        if (idStart == -1 || idEnd == -1 || idEnd <= idStart) return;
        QString idStr = picked.mid(idStart + 4, idEnd - (idStart + 4));
        int dayId = idStr.toInt();

        onTrainingDayAction(dayId, programId);
    }
    else if (msg.clickedButton() == delProgramBtn) {
        auto res = QMessageBox::question(this, "Delete Program", "Are you sure?");
        if (res != QMessageBox::Yes) return;
        if (!programManager.deleteProgram(programId)) {
            QMessageBox::warning(this, "Error", "Failed to delete program");
            return;
        }
        loadPrograms();
    }
}

// Действия для тренировочного дня (добавление/настройка/удаление упражнений)
void ProgramsPage::onTrainingDayAction(int trainingDayId, int /*programId*/) {
    auto td = trainingDayManager.getTrainingDayById(trainingDayId);

    auto entries = exerciseEntryManager.getEntriesByTrainingDay(trainingDayId);
    QString entriesText;
    if (entries.empty()) entriesText = "No entries";
    else {
        auto allExercises = exerciseManager.getAllExercises();
        for (const auto& e : entries) {
            QString exName = QString::number(e.exercise_id);
            for (const auto& ex : allExercises) {
                if (ex.id == e.exercise_id) { exName = QString::fromStdString(ex.name); break; }
            }
            entriesText += QString("%1 (entry id:%2) sets:%3 reps:%4 rest:%5s\n")
                           .arg(exName).arg(e.id).arg(e.sets).arg(e.reps).arg(e.rest_time_sec);
        }
    }

    QString info = QString("Day: %1\n\nEntries:\n%2").arg(QString::fromStdString(td.name)).arg(entriesText);

    QMessageBox msg(this);
    msg.setWindowTitle("Day actions");
    msg.setText(info);
    QPushButton* addExBtn = msg.addButton("Add Exercise", QMessageBox::ActionRole);
    QPushButton* editExBtn = msg.addButton("Edit Entry", QMessageBox::ActionRole);
    QPushButton* delDayBtn = msg.addButton("Delete Day", QMessageBox::DestructiveRole);
    msg.addButton("Cancel", QMessageBox::RejectRole);

    msg.exec();

    if (msg.clickedButton() == addExBtn) {
        auto all = exerciseManager.getAllExercises();
        if (all.empty()) { QMessageBox::information(this, "Info", "No exercises available"); return; }

        QStringList names;
        for (const auto& e : all) names << QString::fromStdString(e.name);

        bool ok;
        QString picked = QInputDialog::getItem(this, "Add Exercise", "Choose:", names, 0, false, &ok);
        if (!ok || picked.isEmpty()) return;

        int idx = names.indexOf(picked);
        if (idx < 0 || idx >= static_cast<int>(all.size())) return;

        int sets = QInputDialog::getInt(this, "Sets", "Sets:", 3, 0, 100, 1, &ok);
        if (!ok) return;
        int reps = QInputDialog::getInt(this, "Reps", "Reps:", 8, 0, 1000, 1, &ok);
        if (!ok) return;
        int rest = QInputDialog::getInt(this, "Rest (sec)", "Rest seconds:", 60, 0, 3600, 1, &ok);
        if (!ok) return;

        ExercisesEntry entry{};
        entry.training_day_id = trainingDayId;
        entry.exercise_id = all[idx].id;
        entry.sets = sets;
        entry.reps = reps;
        entry.rest_time_sec = rest;

        if (!exerciseEntryManager.createEntry(entry)) {
            QMessageBox::warning(this, "Error", "Failed to add exercise entry");
            return;
        }
        QMessageBox::information(this, "Done", "Exercise added");
    }
    else if (msg.clickedButton() == editExBtn) {
        if (entries.empty()) { QMessageBox::information(this, "Info", "No entries to edit"); return; }

        QStringList entryNames;
        auto allExercises = exerciseManager.getAllExercises();
        for (const auto& e : entries) {
            QString exName = QString::number(e.exercise_id);
            for (const auto& ex : allExercises) {
                if (ex.id == e.exercise_id) { exName = QString::fromStdString(ex.name); break; }
            }
            entryNames << QString("%1 (entry id:%2) sets:%3 reps:%4 rest:%5")
                          .arg(exName).arg(e.id).arg(e.sets).arg(e.reps).arg(e.rest_time_sec);
        }

        bool ok;
        QString picked = QInputDialog::getItem(this, "Select Entry", "Entries:", entryNames, 0, false, &ok);
        if (!ok || picked.isEmpty()) return;

        int idStart = picked.indexOf("(entry id:");
        int idEnd = picked.indexOf(")", idStart);
        if (idStart == -1 || idEnd == -1) return;
        QString idStr = picked.mid(idStart + 10, idEnd - (idStart + 10));
        int entryId = idStr.toInt();

        int sets = QInputDialog::getInt(this, "Sets", "Sets:", 3, 0, 100, 1, &ok);
        if (!ok) return;
        int reps = QInputDialog::getInt(this, "Reps", "Reps:", 8, 0, 1000, 1, &ok);
        if (!ok) return;
        int rest = QInputDialog::getInt(this, "Rest (sec)", "Rest seconds:", 60, 0, 3600, 1, &ok);
        if (!ok) return;

        if (!exerciseEntryManager.setSets(entryId, sets) ||
            !exerciseEntryManager.setReps(entryId, reps) ||
            !exerciseEntryManager.setRestTime(entryId, rest)) {
            QMessageBox::warning(this, "Error", "Failed to save entry parameters");
            return;
        }
        QMessageBox::information(this, "Done", "Entry updated");
    }
    else if (msg.clickedButton() == delDayBtn) {
        auto res = QMessageBox::question(this, "Delete Day", "Are you sure?");
        if (res != QMessageBox::Yes) return;

        if (!trainingDayManager.deleteTrainingDay(trainingDayId)) {
            QMessageBox::warning(this, "Error", "Failed to delete training day");
            return;
        }
        loadPrograms();
        QMessageBox::information(this, "Done", "Day deleted");
    }
}

// Поиск созданного дня по точному имени
int ProgramsPage::findCreatedTrainingDayIdByName(const std::string& name) {
    auto all = trainingDayManager.getAllTrainingDays();
    for (const auto& td : all) {
        if (td.name == name) return td.id;
    }
    return -1;
}
