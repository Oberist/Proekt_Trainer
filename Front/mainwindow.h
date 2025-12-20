#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTimer>
#include <QTime>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel> // добавлено для QLabel

// Добавляем include для ExercisesPage и других страниц
#include "ExercisesPage.h"
#include "ProgramsPage.h"  // <-- Добавляем для ProgramsPage
#include "ProgramManager.h"  // <-- Добавляем для работы с программами
#include "TrainingDayManager.h"  // <-- Добавляем для работы с днями тренировок


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void newWorkout();
    void openWorkout();
    void saveWorkout();
    void exitApp();

    void showDashboard();
    void showExercises();
    void showPrograms();  // <-- Добавляем слот для показа Programs
    void showProgress();
    void showAICoach();

    void aboutApp();
    void aboutQt();

private:
    // Перенёс все члены таймера сюда (убрал их из секции slots)
    // timer members
    QTimer *simpleTimer = nullptr;
    QLabel *timerDisplay = nullptr;
    QPushButton *startTimerBtn = nullptr;
    QPushButton *pauseTimerBtn = nullptr;
    QPushButton *stopTimerBtn = nullptr;

    QTime simpleStartTime;
    int simpleElapsedSeconds = 0;
    bool simpleTimerRunning = false;

    // Указатели на виджеты страниц
    QStackedWidget *stackedWidget;
    QWidget *dashboardPage;
    ExercisesPage *exercisesPage;   // <-- оставляем ExercisesPage
    ProgramsPage *programsPage;     // <-- добавляем указатель на ProgramsPage
    QWidget *progressPage;
    QWidget *aiCoachPage;
    TrainingDayManager trainingDayManager;

    // Боковая панель и её элементы
    QWidget *sidebar;
    QVBoxLayout *sidebarLayout;
    QPushButton *dashboardBtn;
    QPushButton *exercisesBtn;
    QPushButton *programsBtn;  // <-- Добавляем кнопку для Programs
    QPushButton *progressBtn;
    QPushButton *aiCoachBtn;

    // Таймер тренировки (оставлено, если используется отдельно)
    QTimer *workoutTimer = nullptr;
    QTime *elapsedTime = nullptr;
    bool isTimerRunning = false;
    int totalWeeklySeconds = 0;  // Общее время за неделю в секундах

    // Приватные методы настройки
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupSidebar();
    void setupCentralWidget();
    void setupConnections();

    // Методы создания страниц
    QWidget* createDashboardPage();
    ExercisesPage* createExercisesPage();  // <-- возвращаем ExercisesPage
    ProgramsPage* createProgramsPage();    // <-- возвращаем ProgramsPage
    QWidget* createProgressPage();
    QWidget* createAICoachPage();

    // Методы для таймера
    void startWorkoutTimer();
    void stopWorkoutTimer();
    void updateTimerDisplay();
    void resetWeeklyTimeIfNeeded();
    void checkWeeklyReset();
    void saveWeeklyTime();
    void loadWeeklyTime();

    // Вспомогательный метод для стилизации кнопок
    void styleSidebarButton(QPushButton *button, bool active = false);

    // Вспомогательные методы для Dashboard
    QFrame* createStatCard(const QString& title, const QString& value, const QString& subtitle, const QString& id = "");
    QHBoxLayout* createActivityItem(const QString& date, const QString& type, const QString& details, const QString& duration);
};

#endif // MAINWINDOW_H
