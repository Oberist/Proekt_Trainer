#ifndef EXERCISES_PAGE_H
#define EXERCISES_PAGE_H

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QComboBox>
#include "ExerciseManager.h"

class ExercisesPage : public QWidget {
    Q_OBJECT
private:
    ExerciseManager manager;
    QListWidget* list;
    QLineEdit* searchBox;
    QComboBox* difficultyFilter;
    QComboBox* typeFilter;
    QComboBox* muscleFilter;
    QComboBox* equipmentFilter;

public:
    explicit ExercisesPage(const std::string& dbPath, QWidget* parent = nullptr);

private slots:
    void onSearchChanged(const QString& text);
    void onDifficultyChanged(int index);
    void onTypeChanged(int index);
    void onMuscleChanged(int index);
    void onEquipmentChanged(int index);
};

#endif
