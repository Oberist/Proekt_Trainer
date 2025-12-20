#ifndef PROGRAMSPAGE_H
#define PROGRAMSPAGE_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>

#include "ProgramManager.h"
#include "TrainingDayManager.h"
#include "ExerciseEntryManager.h"
#include "ExerciseManager.h"

class ProgramsPage : public QWidget {
    Q_OBJECT

public:
    explicit ProgramsPage(const std::string& dbPath, QWidget* parent = nullptr);

private:
    // Менеджеры
    ProgramManager programManager;
    TrainingDayManager trainingDayManager;
    ExerciseEntryManager exerciseEntryManager;
    ExerciseManager exerciseManager;

    // UI
    QListWidget* programsList;
    QLabel* hintLabel;
    QPushButton* addProgramBtn;
    QPushButton* deleteProgramBtn;

    // Методы
    void loadPrograms();

    // Действия
    void onProgramClicked(int programId);
    void onTrainingDayAction(int trainingDayId, int programId);

    // Вспомогательные
    int findCreatedTrainingDayIdByName(const std::string& name);
};

#endif // PROGRAMSPAGE_H
