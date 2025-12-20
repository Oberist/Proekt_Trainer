#include "mainwindow.h"

#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QMessageBox>
#include <QDate>
#include <QApplication>
#include <QIcon>
#include <QKeySequence>
#include <QPushButton>
#include <QFrame>
#include <QCalendarWidget>
#include <QTextCharFormat>
#include <QLineEdit>
#include <QListWidget>
#include <QInputDialog>
#include <QDebug>
#include <QTimer>
#include <QTime>
#include <QSettings>
#include <QTextEdit>
#include <QProcess>
#include <QDir>
#include <ExercisesPage.h>
#include "ProgramsPage.h"
#include "ProgressPage.h"
#include "TrainingDayManager.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , stackedWidget(nullptr)
    , dashboardPage(nullptr)
    , exercisesPage(nullptr)
    , programsPage(nullptr)
    , progressPage(nullptr)
    , aiCoachPage(nullptr)
    , sidebar(nullptr)
    , sidebarLayout(nullptr)
    , dashboardBtn(nullptr)
    , exercisesBtn(nullptr)
    , programsBtn(nullptr)
    , progressBtn(nullptr)
    , aiCoachBtn(nullptr)
    , workoutTimer(nullptr)
    , elapsedTime(nullptr)
    , isTimerRunning(false)
    , totalWeeklySeconds(0)
    , trainingDayManager("TrainingDiary.db")
{
    // Настройка главного окна
    setWindowTitle("🏋️ GymTracker - AI Fitness Coach");
    resize(1200, 800);

    // Минимальные стили
    this->setStyleSheet("QMainWindow { background-color: #1a1a1a; }");

    // Инициализация таймера (НЕ используем глобальный таймер для избежания конфликтов)
    // workoutTimer и elapsedTime оставляем как nullptr - таймер будет локальным в Dashboard

    // Загружаем сохранённое время
    loadWeeklyTime();
    checkWeeklyReset();

    // Настройка интерфейса
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    setupCentralWidget();
    setupConnections();

    // Показываем Dashboard по умолчанию
    showDashboard();
}

MainWindow::~MainWindow()
{
    // Сохраняем время перед выходом
    saveWeeklyTime();

    delete elapsedTime; // Безопасно удаляем, даже если nullptr
    // Остальные виджеты удалятся автоматически
}

// ==================== НАСТРОЙКА МЕНЮ ====================
void MainWindow::setupMenuBar()
{
    QMenu *fileMenu = menuBar()->addMenu("&File");

    QAction *newAction = new QAction("&New Workout", this);
    newAction->setShortcut(QKeySequence::New);
    fileMenu->addAction(newAction);

    QAction *openAction = new QAction("&Open Workout...", this);
    openAction->setShortcut(QKeySequence::Open);
    fileMenu->addAction(openAction);

    QAction *saveAction = new QAction("&Save Workout", this);
    saveAction->setShortcut(QKeySequence::Save);
    fileMenu->addAction(saveAction);

    fileMenu->addSeparator();

    QAction *exitAction = new QAction("E&xit", this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &MainWindow::exitApp);
    fileMenu->addAction(exitAction);

    QMenu *viewMenu = menuBar()->addMenu("&View");

    QAction *dashboardAction = new QAction("&Dashboard", this);
    dashboardAction->setShortcut(Qt::CTRL | Qt::Key_1);
    connect(dashboardAction, &QAction::triggered, this, &MainWindow::showDashboard);
    viewMenu->addAction(dashboardAction);

    QAction *exercisesAction = new QAction("&Exercises", this);
    exercisesAction->setShortcut(Qt::CTRL | Qt::Key_2);
    connect(exercisesAction, &QAction::triggered, this, &MainWindow::showExercises);
    viewMenu->addAction(exercisesAction);

    QAction *programsAction = new QAction("&Programs", this);
    programsAction->setShortcut(Qt::CTRL | Qt::Key_3);
    connect(programsAction, &QAction::triggered, this, &MainWindow::showPrograms);
    viewMenu->addAction(programsAction);

    QAction *progressAction = new QAction("&Progress", this);
    progressAction->setShortcut(Qt::CTRL | Qt::Key_4);
    connect(progressAction, &QAction::triggered, this, &MainWindow::showProgress);
    viewMenu->addAction(progressAction);

    QAction *aiCoachAction = new QAction("&AI Coach", this);
    aiCoachAction->setShortcut(Qt::CTRL | Qt::Key_5);
    connect(aiCoachAction, &QAction::triggered, this, &MainWindow::showAICoach);
    viewMenu->addAction(aiCoachAction);

    QMenu *helpMenu = menuBar()->addMenu("&Help");

    QAction *aboutAction = new QAction("&About GymTracker", this);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::aboutApp);
    helpMenu->addAction(aboutAction);

    QAction *aboutQtAction = new QAction("About &Qt", this);
    connect(aboutQtAction, &QAction::triggered, this, &MainWindow::aboutQt);
    helpMenu->addAction(aboutQtAction);
}

void MainWindow::setupToolBar()
{
    QToolBar *toolBar = addToolBar("Main Toolbar");
    toolBar->setMovable(false);
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage("Ready");

    QLabel *userLabel = new QLabel("User: Pavel");
    statusBar()->addPermanentWidget(userLabel);

    QLabel *dateLabel = new QLabel(QDate::currentDate().toString("dd.MM.yyyy"));
    statusBar()->addPermanentWidget(dateLabel);
}

void MainWindow::setupSidebar()
{
    sidebar = new QWidget(this);
    sidebar->setFixedWidth(220);
    sidebar->setObjectName("sidebar");

    sidebarLayout = new QVBoxLayout(sidebar);
    sidebarLayout->setSpacing(8);
    sidebarLayout->setContentsMargins(10, 20, 10, 20);

    QLabel *appTitle = new QLabel("🏋️ GymTracker");
    appTitle->setAlignment(Qt::AlignCenter);
    sidebarLayout->addWidget(appTitle);

    QFrame *separator = new QFrame();
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);
    sidebarLayout->addWidget(separator);

    dashboardBtn = new QPushButton("🏠 Dashboard");
    exercisesBtn = new QPushButton("💪 Exercises");
    programsBtn = new QPushButton("📅 Programs");
    progressBtn = new QPushButton("📊 Progress");
    aiCoachBtn = new QPushButton("🤖 AI Coach");

    QList<QPushButton*> navButtons = {dashboardBtn, exercisesBtn, programsBtn, progressBtn, aiCoachBtn};
    for (auto *btn : navButtons) {
        btn->setFixedHeight(45);
        btn->setCursor(Qt::PointingHandCursor);
        styleSidebarButton(btn);
    }

    sidebarLayout->addWidget(dashboardBtn);
    sidebarLayout->addWidget(exercisesBtn);
    sidebarLayout->addWidget(programsBtn);
    sidebarLayout->addWidget(progressBtn);
    sidebarLayout->addWidget(aiCoachBtn);

    sidebarLayout->addStretch();

    QPushButton *settingsBtn = new QPushButton("⚙️ Settings");
    settingsBtn->setFixedHeight(45);
    settingsBtn->setCursor(Qt::PointingHandCursor);
    styleSidebarButton(settingsBtn);
    sidebarLayout->addWidget(settingsBtn);
}

void MainWindow::styleSidebarButton(QPushButton *button, bool active)
{
    // МИНИМАЛЬНЫЕ СТИЛИ ДЛЯ КНОПОК
    if (active) {
        button->setStyleSheet(
            "QPushButton {"
            "    background-color: #4a6fa5;"
            "    color: white;"
            "    font-weight: bold;"
            "    border: none;"
            "    border-radius: 6px;"
            "    padding: 12px 15px;"
            "    text-align: left;"
            "}"
            );
    } else {
        button->setStyleSheet(
            "QPushButton {"
            "    background-color: transparent;"
            "    color: #cccccc;"
            "    border: none;"
            "    border-radius: 6px;"
            "    padding: 12px 15px;"
            "    text-align: left;"
            "}"
            "QPushButton:hover {"
            "    background-color: #333;"
            "    color: white;"
            "}"
            );
    }
}

void MainWindow::setupCentralWidget()
{
    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    centralWidget->setStyleSheet("background-color: #1a1a1a; color: white;");
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    setupSidebar();

    stackedWidget = new QStackedWidget();
    stackedWidget->setObjectName("contentArea");

    dashboardPage = createDashboardPage();
    exercisesPage = createExercisesPage();
    programsPage = createProgramsPage();
    progressPage = createProgressPage();
    aiCoachPage = createAICoachPage();

    stackedWidget->addWidget(dashboardPage);
    stackedWidget->addWidget(exercisesPage);
    stackedWidget->addWidget(programsPage);
    stackedWidget->addWidget(progressPage);
    stackedWidget->addWidget(aiCoachPage);

    mainLayout->addWidget(sidebar);
    mainLayout->addWidget(stackedWidget, 1);

    setCentralWidget(centralWidget);
}

void MainWindow::setupConnections()
{
    connect(dashboardBtn, &QPushButton::clicked, this, &MainWindow::showDashboard);
    connect(exercisesBtn, &QPushButton::clicked, this, &MainWindow::showExercises);
    connect(programsBtn, &QPushButton::clicked, this, &MainWindow::showPrograms);
    connect(progressBtn, &QPushButton::clicked, this, &MainWindow::showProgress);
    connect(aiCoachBtn, &QPushButton::clicked, this, &MainWindow::showAICoach);
}
//Главный экран
QWidget* MainWindow::createDashboardPage()
{
    // Создаем скроллируемую область
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setStyleSheet(
        "QScrollArea {"
        "    border: none;"
        "    background-color: transparent;"
        "}"
        "QScrollBar:vertical {"
        "    border: none;"
        "    background-color: #2a2a2a;"
        "    width: 10px;"
        "    border-radius: 5px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background-color: #4a4a4a;"
        "    border-radius: 5px;"
        "    min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background-color: #5a5a5a;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "    border: none;"
        "    background: none;"
        "    height: 0px;"
        "}"
        );

    // Основной виджет для контента
    QWidget *contentWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(contentWidget);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 15, 20, 25);  // Увеличил нижний отступ

    // ЗАГОЛОВОК
    QLabel *title = new QLabel("🏠 Dashboard");
    title->setStyleSheet("font-size: 26px; font-weight: bold; color: white; margin-bottom: 10px;");
    mainLayout->addWidget(title);

    // СТАТИСТИКА КАРТОЧКИ
    QHBoxLayout *statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(15);

    QFrame *cardCalories = createStatCard("🔥 Ккал", "1,250", "За сегодня", "caloriesCard");
    QFrame *cardTotalTime = createStatCard("⏱️ Общее время", "0 ч 0 мин", "За неделю", "totalTimeCard");
    QFrame *cardWorkouts = createStatCard("💪 Тренировки", "0", "За неделю", "workoutsCard");

    cardCalories->setFixedWidth(180);
    cardTotalTime->setFixedWidth(180);
    cardWorkouts->setFixedWidth(180);

    statsLayout->addWidget(cardCalories);
    statsLayout->addWidget(cardTotalTime);
    statsLayout->addWidget(cardWorkouts);
    statsLayout->addStretch();
    mainLayout->addLayout(statsLayout);

    // РАЗДЕЛИТЕЛЬ
    QFrame *separator1 = new QFrame();
    separator1->setFrameShape(QFrame::HLine);
    separator1->setStyleSheet("background-color: #444; margin: 10px 0;");
    mainLayout->addWidget(separator1);

    // ОСНОВНОЙ КОНТЕНТ: КАЛЕНДАРЬ + СЕГОДНЯ
    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(15);
    contentLayout->setContentsMargins(0, 0, 0, 0);

    // ========== КАЛЕНДАРЬ ==========
    QFrame *calendarFrame = new QFrame();
    calendarFrame->setStyleSheet(
        "QFrame {"
        "    background-color: #2a2a2a;"
        "    border-radius: 10px;"
        "    padding: 12px;"
        "    border: 1px solid #444;"
        "}"
        );
    calendarFrame->setFixedWidth(380);

    QVBoxLayout *calendarLayout = new QVBoxLayout(calendarFrame);
    calendarLayout->setSpacing(8);
    calendarLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *calendarTitle = new QLabel("📅 Календарь тренировок");
    calendarTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: white; margin-bottom: 8px;");
    calendarLayout->addWidget(calendarTitle);

    QCalendarWidget *calendar = new QCalendarWidget();
    calendar->setFixedSize(350, 280);
    calendar->setStyleSheet(
        "QCalendarWidget {"
        "    background-color: #252525;"
        "    color: white;"
        "    border: 1px solid #444;"
        "    font-size: 11px;"
        "}"
        "QCalendarWidget QAbstractItemView:enabled {"
        "    color: white;"
        "    background-color: #252525;"
        "    selection-background-color: #4a6fa5;"
        "    font-size: 10px;"
        "}"
        "QCalendarWidget QToolButton {"
        "    font-size: 12px;"
        "    padding: 3px 8px;"
        "}"
        );

    calendar->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    calendar->setHorizontalHeaderFormat(QCalendarWidget::ShortDayNames);

    // Отмечаем дни тренировок
    QDate today = QDate::currentDate();
    QTextCharFormat workoutFormat;
    workoutFormat.setBackground(QBrush(QColor(76, 175, 80, 70)));
    workoutFormat.setFontWeight(QFont::Bold);
    calendar->setDateTextFormat(today.addDays(-1), workoutFormat);
    calendar->setDateTextFormat(today.addDays(-3), workoutFormat);

    connect(calendar, &QCalendarWidget::clicked, this, [this, calendar](const QDate &date) {
        // Получаем все тренировочные дни
        auto allDays = trainingDayManager.getAllTrainingDays();
        if (allDays.empty()) {
            QMessageBox::information(this, "Нет дней", "Список тренировочных дней пуст. Сначала добавьте дни в Programs.");
            return;
        }

        // Формируем список имён для выбора
        QStringList names;
        for (const auto& td : allDays) {
            // Показываем читаемое имя; при необходимости можно добавить id в скобках
            names << QString::fromStdString(td.name) + " (id:" + QString::number(td.id) + ")";
        }

        bool ok;
        QString picked = QInputDialog::getItem(this, "Выберите тренировочный день",
                                               QString("Дата: %1\nВыберите день:").arg(date.toString("dd.MM.yyyy")),
                                               names, 0, false, &ok);
        if (!ok || picked.isEmpty()) return;

        // Определяем id выбранного дня (парсим строку с "(id:NNN)")
        int idStart = picked.lastIndexOf("(id:");
        int idEnd = picked.lastIndexOf(")");
        int dayId = -1;
        if (idStart != -1 && idEnd != -1 && idEnd > idStart) {
            QString idStr = picked.mid(idStart + 4, idEnd - (idStart + 4));
            dayId = idStr.toInt();
        }

        // Визуально помечаем дату в календаре
        QTextCharFormat format;
        format.setBackground(QBrush(QColor(76, 175, 80, 120)));
        format.setFontWeight(QFont::Bold);
        calendar->setDateTextFormat(date, format);

        // Информируем пользователя
        QString dayName = picked;
        QMessageBox::information(this, "Привязка", QString("Дата %1 привязана к дню: %2").arg(date.toString("dd.MM.yyyy")).arg(dayName));

        // TODO: если хочешь сохранять привязку в БД, вызови здесь метод CalendarDAO или ProgramManager:
        // calendarManager.saveAssignment(date.toString("yyyy-MM-dd").toStdString(), dayId);
    });


    calendarLayout->addWidget(calendar, 0, Qt::AlignHCenter);

    // КНОПКИ КАЛЕНДАРЯ
    QHBoxLayout *calendarButtons = new QHBoxLayout();
    calendarButtons->setSpacing(8);

    QPushButton *addWorkoutBtn = new QPushButton("➕ Добавить");
    QPushButton *clearDayBtn = new QPushButton("🗑️ Очистить");

    addWorkoutBtn->setFixedHeight(32);
    clearDayBtn->setFixedHeight(32);

    addWorkoutBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #4a6fa5;"
        "    color: white;"
        "    padding: 5px 10px;"
        "    border-radius: 6px;"
        "    font-size: 12px;"
        "}"
        "QPushButton:hover { background-color: #5a7fb5; }"
        );

    clearDayBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #f44336;"
        "    color: white;"
        "    padding: 5px 10px;"
        "    border-radius: 6px;"
        "    font-size: 12px;"
        "}"
        "QPushButton:hover { background-color: #ff5555; }"
        );

    connect(addWorkoutBtn, &QPushButton::clicked, [calendar, this]() {
        QDate selectedDate = calendar->selectedDate();
        QString workoutType = QInputDialog::getText(this, "Тип тренировки", "Введите тип тренировки:");
        if (!workoutType.isEmpty()) {
            QTextCharFormat format;
            format.setBackground(QBrush(QColor(76, 175, 80, 70)));
            format.setFontWeight(QFont::Bold);
            calendar->setDateTextFormat(selectedDate, format);
            QMessageBox::information(this, "Успех", QString("Тренировка '%1' добавлена на %2").arg(workoutType).arg(selectedDate.toString("dd.MM.yyyy")));
        }
    });

    connect(clearDayBtn, &QPushButton::clicked, [calendar, this]() {
        QDate selectedDate = calendar->selectedDate();
        calendar->setDateTextFormat(selectedDate, QTextCharFormat());
        QMessageBox::information(this, "Очищено", "Тренировка удалена");
    });

    calendarButtons->addWidget(addWorkoutBtn);
    calendarButtons->addWidget(clearDayBtn);
    calendarButtons->addStretch();
    calendarLayout->addLayout(calendarButtons);

    contentLayout->addWidget(calendarFrame);

    // ========== СЕГОДНЯ ==========
    QFrame *todayFrame = new QFrame();
    todayFrame->setStyleSheet(
        "QFrame {"
        "    background-color: #2a2a2a;"
        "    border-radius: 10px;"
        "    padding: 15px;"
        "    border: 1px solid #555;"
        "}"
        );
    todayFrame->setFixedWidth(380);

    QVBoxLayout *todayLayout = new QVBoxLayout(todayFrame);
    todayLayout->setSpacing(12);
    todayLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *todayTitle = new QLabel("🎯 Сегодня");
    todayTitle->setStyleSheet("font-size: 18px; font-weight: bold; color: white; margin-bottom: 8px;");
    todayLayout->addWidget(todayTitle);

    QLabel *workoutLabel = new QLabel("Тренировка:");
    workoutLabel->setStyleSheet("font-size: 13px; color: white;");
    todayLayout->addWidget(workoutLabel);

    QLineEdit *workoutInput = new QLineEdit();
    workoutInput->setPlaceholderText("Например: Силовая - Грудь");
    workoutInput->setFixedHeight(35);
    workoutInput->setStyleSheet(
        "QLineEdit {"
        "    background-color: #333;"
        "    color: white;"
        "    border: 1px solid #555;"
        "    border-radius: 6px;"
        "    padding: 8px;"
        "    font-size: 13px;"
        "}"
        );
    todayLayout->addWidget(workoutInput);

    QLabel *exercisesLabel = new QLabel("Упражнения:");
    exercisesLabel->setStyleSheet("font-size: 13px; color: white; margin-top: 8px;");
    todayLayout->addWidget(exercisesLabel);

    QListWidget *exercisesList = new QListWidget();
    exercisesList->setFixedHeight(120);
    exercisesList->setStyleSheet(
        "QListWidget {"
        "    background-color: #333;"
        "    color: white;"
        "    border: 1px solid #555;"
        "    border-radius: 6px;"
        "    font-size: 12px;"
        "}"
        );

    QStringList sampleExercises = {
        "Жим лежа - 4x10",
        "Разводки гантелей - 3x12",
        "Отжимания на брусьях - 3x15",
        "Французский жим - 4x10",
        "Разгибания на блоке - 3x12"
    };

    for (const QString &exercise : sampleExercises) {
        QListWidgetItem *item = new QListWidgetItem("• " + exercise);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        exercisesList->addItem(item);
    }

    todayLayout->addWidget(exercisesList);

    // ТАЙМЕР
QLabel *timerTitle = new QLabel("⏱️ Таймер тренировки:");
timerTitle->setStyleSheet("font-size: 14px; font-weight: bold; color: white; margin-top: 12px;");
todayLayout->addWidget(timerTitle);

QHBoxLayout *timerDisplayLayout = new QHBoxLayout();
QLabel *timerLabel = new QLabel("Время:");
timerLabel->setStyleSheet("font-size: 13px; color: white;");
timerDisplayLayout->addWidget(timerLabel);

// timerDisplay теперь член класса
timerDisplay = new QLabel("00:00:00");
timerDisplay->setObjectName("timerDisplay");
timerDisplay->setStyleSheet(
    "QLabel {"
    "    font-size: 22px;"
    "    font-weight: bold;"
    "    color: #4CAF50;"
    "    background-color: #222;"
    "    border-radius: 8px;"
    "    padding: 10px;"
    "    min-width: 120px;"
    "    text-align: center;"
    "    border: 2px solid #444;"
    "}"
);
timerDisplayLayout->addWidget(timerDisplay);
todayLayout->addLayout(timerDisplayLayout);

// КНОПКИ ТАЙМЕРА (члены класса)
QHBoxLayout *timerButtonsLayout = new QHBoxLayout();
timerButtonsLayout->setSpacing(8);

startTimerBtn = new QPushButton("▶️ Старт");
pauseTimerBtn = new QPushButton("⏸️ Пауза");
stopTimerBtn = new QPushButton("⏹️ Стоп");

startTimerBtn->setFixedHeight(35);
pauseTimerBtn->setFixedHeight(35);
stopTimerBtn->setFixedHeight(35);

QString buttonStyle =
    "QPushButton {"
    "    padding: 8px 15px;"
    "    border-radius: 6px;"
    "    font-weight: bold;"
    "    font-size: 13px;"
    "    color: white;"
    "}"
    "QPushButton:disabled { background-color: #666; color: #999; }";

startTimerBtn->setStyleSheet(buttonStyle + "QPushButton { background-color: #4CAF50; } QPushButton:hover { background-color: #5CBF60; }");
pauseTimerBtn->setStyleSheet(buttonStyle + "QPushButton { background-color: #FF9800; } QPushButton:hover { background-color: #FFB74D; }");
stopTimerBtn->setStyleSheet(buttonStyle + "QPushButton { background-color: #f44336; } QPushButton:hover { background-color: #ff5555; }");

pauseTimerBtn->setEnabled(false);
stopTimerBtn->setEnabled(false);

timerButtonsLayout->addWidget(startTimerBtn);
timerButtonsLayout->addWidget(pauseTimerBtn);
timerButtonsLayout->addWidget(stopTimerBtn);
todayLayout->addLayout(timerButtonsLayout);

// Инициализация членов состояния
simpleElapsedSeconds = 0;
simpleTimerRunning = false;
simpleStartTime = QTime::fromString("00:00:00", "hh:mm:ss");

// Создаём таймер как член класса, привязанный к this
simpleTimer = new QTimer(this);
simpleTimer->setInterval(1000);

// Подключаем timeout — лямбда захватывает this, использует члены класса
connect(simpleTimer, &QTimer::timeout, this, [this]() {
    int totalSeconds = simpleElapsedSeconds;
    if (simpleTimerRunning) {
        totalSeconds += simpleStartTime.secsTo(QTime::currentTime());
    }
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int seconds = totalSeconds % 60;

    timerDisplay->setText(
        QString("%1:%2:%3")
            .arg(hours, 2, 10, QChar('0'))
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'))
    );
});

// Кнопки — тоже лямбды, захватываем this
connect(startTimerBtn, &QPushButton::clicked, this, [this]() {
    if (!simpleTimerRunning) {
        simpleStartTime = QTime::currentTime();
        simpleTimerRunning = true;
        startTimerBtn->setEnabled(false);
        pauseTimerBtn->setEnabled(true);
        stopTimerBtn->setEnabled(true);
        simpleTimer->start();
    }
});

connect(pauseTimerBtn, &QPushButton::clicked, this, [this]() {
    if (simpleTimerRunning) {
        simpleElapsedSeconds += simpleStartTime.secsTo(QTime::currentTime());
        simpleTimerRunning = false;
        startTimerBtn->setEnabled(true);
        pauseTimerBtn->setEnabled(false);
        simpleTimer->stop();
    }
});

connect(stopTimerBtn, &QPushButton::clicked, this, [this]() {
    if (simpleTimerRunning) {
        simpleElapsedSeconds += simpleStartTime.secsTo(QTime::currentTime());
        simpleTimerRunning = false;
    }
    // сброс
    simpleElapsedSeconds = 0;
    timerDisplay->setText(QString("00:00:00"));

    startTimerBtn->setEnabled(true);
    pauseTimerBtn->setEnabled(false);
    stopTimerBtn->setEnabled(false);
    simpleTimer->stop();
});


    // КНОПКА СОХРАНИТЬ
QPushButton *saveWorkoutBtn = new QPushButton("💾 Сохранить тренировку");
saveWorkoutBtn->setFixedHeight(35);
saveWorkoutBtn->setStyleSheet(
    "QPushButton {"
    " background-color: #4a6fa5;"
    " color: white;"
    " padding: 8px 12px;"
    " border-radius: 6px;"
    " font-weight: bold;"
    " font-size: 13px;"
    " margin-top: 12px;"
    "}"
    "QPushButton:hover { background-color: #5a7fb5; }"
);

connect(saveWorkoutBtn, &QPushButton::clicked, [workoutInput, exercisesList, this]() {
    QString workout = workoutInput->text();
    if (workout.isEmpty()) {
        QMessageBox::warning(this, "Внимание", "Введите тип тренировки!");
        return;
    }

    QStringList selectedExercises;
    for (int i = 0; i < exercisesList->count(); ++i) {
        QListWidgetItem *item = exercisesList->item(i);
        if (item->checkState() == Qt::Checked) {
            selectedExercises << item->text().remove(0, 2);
        }
    }

    QMessageBox::information(this, "Сохранено", QString("Тренировка '%1' сохранена!\nУпражнения: %2").arg(workout).arg(selectedExercises.join(", ")));
});

todayLayout->addWidget(saveWorkoutBtn);
contentLayout->addWidget(todayFrame);
mainLayout->addLayout(contentLayout);


// КНОПКА СБРОСА СТАТИСТИКИ
QPushButton *resetStatsBtn = new QPushButton("🔄 Сбросить недельную статистику");
resetStatsBtn->setFixedHeight(35);
resetStatsBtn->setStyleSheet(
    "QPushButton {"
    " background-color: #9C27B0;"
    " color: white;"
    " padding: 8px 12px;"
    " border-radius: 6px;"
    " font-weight: bold;"
    " font-size: 13px;"
    " margin-top: 10px;"
    "}"
    "QPushButton:hover { background-color: #AB47BC; }"
);
connect(resetStatsBtn, &QPushButton::clicked, [this, cardTotalTime, cardWorkouts]() {
    totalWeeklySeconds = 0;
    QLabel *totalTimeValue = cardTotalTime->findChild<QLabel*>("valueLabel");
    if (totalTimeValue)
        totalTimeValue->setText("0 ч 0 мин");
    QLabel *workoutsValue = cardWorkouts->findChild<QLabel*>("valueLabel");
    if (workoutsValue)
        workoutsValue->setText("0");
    saveWeeklyTime();
    QMessageBox::information(this, "Сброс статистики", "Недельная статистика сброшена! Начинаем новую неделю. 🎯");
});
mainLayout->addWidget(resetStatsBtn);
mainLayout->addStretch();



    // РЕКОМЕНДАЦИЯ ИИ
    QLabel *aiLabel = new QLabel("🤖 Рекомендация AI Coach");
    aiLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: white; margin-top: 15px;");
    mainLayout->addWidget(aiLabel);

    QFrame *aiCard = new QFrame();
    aiCard->setStyleSheet(
        "QFrame {"
        "    background-color: #2a2a2a;"
        "    border-left: 5px solid #3498db;"
        "    border-radius: 8px;"
        "    padding: 12px;"
        "}"
        );

    QLabel *aiText = new QLabel("Увеличьте вес в жиме лежа на 2.5 кг. После завершения тренировки не забудьте сделать растяжку!");
    aiText->setStyleSheet("font-size: 13px; color: white;");
    aiText->setWordWrap(true);
    aiCard->setLayout(new QVBoxLayout());
    aiCard->layout()->addWidget(aiText);

    mainLayout->addWidget(aiCard);


    // Устанавливаем контент в скролл-область
    scrollArea->setWidget(contentWidget);

    // Возвращаем скролл-область вместо обычного виджета
    return scrollArea;
}

ExercisesPage* MainWindow::createExercisesPage()
{
    // Создаём ExercisesPage с путём к базе данных
    ExercisesPage* exercisesPage = new ExercisesPage("TrainingDiary.db", this);
    return exercisesPage;
}

ProgramsPage* MainWindow::createProgramsPage()
{
    ProgramsPage* programsPage = new ProgramsPage("TrainingDiary.db", this);
    stackedWidget->addWidget(programsPage);
    return programsPage;
}

QWidget* MainWindow::createProgressPage()
{
     return new ProgressPage("TrainingDiary.db", this);
}

QWidget* MainWindow::createAICoachPage()
{
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);

    QLabel *title = new QLabel("🤖 AI Coach");
    title->setStyleSheet("font-size: 24px; font-weight: bold; color: white; margin: 20px;");
    layout->addWidget(title);

    QLineEdit *input = new QLineEdit();
    input->setPlaceholderText("Задай вопрос, например: \"Как улучшить жим лежа?\"");
    input->setStyleSheet("font-size: 14px; color: white; background-color: #2a2a2a; padding: 6px;");
    layout->addWidget(input);

    QPushButton *btn = new QPushButton("Получить рекомендацию");
    btn->setStyleSheet("font-size: 14px; padding: 8px;");
    layout->addWidget(btn);

    QTextEdit *output = new QTextEdit();
    output->setReadOnly(true);
    output->setStyleSheet("font-size: 14px; color: white; background-color: #1e1e1e;");
    layout->addWidget(output);

    QLabel *status = new QLabel("");
    status->setStyleSheet("font-size: 12px; color: #888;");
    layout->addWidget(status);

    layout->addStretch();

    connect(btn, &QPushButton::clicked, this, [=]() {
        QString question = input->text().trimmed();
        if (question.isEmpty()) {
            output->setText("Введите вопрос.");
            return;
        }

        status->setText("Генерация ответа...");

        QProcess *process = new QProcess(page);

        // ✅ Устанавливаем рабочую директорию (ОБЯЗАТЕЛЬНО!)
        QString workDir = QCoreApplication::applicationDirPath();
        process->setWorkingDirectory(workDir);

        // ✅ Правильные пути (PowerShell-friendly)
        QString pythonExe = QDir::toNativeSeparators(workDir + "/python/python.exe");
        QString scriptPath = QDir::toNativeSeparators(workDir + "/rag/ai_chat.py");

        // ✅ Логируем пути (очень полезно)
        qDebug() << "Python:" << pythonExe;
        qDebug() << "Script:" << scriptPath;
        qDebug() << "WorkDir:" << workDir;

        QStringList args;
        args << scriptPath << question;

        // ✅ Чтение stdout
        connect(process, &QProcess::readyReadStandardOutput, page, [=]() {
            QByteArray data = process->readAllStandardOutput();
            QString jsonText = QString::fromUtf8(data).trimmed();

            qDebug() << "PYTHON STDOUT:" << jsonText;

            if (jsonText.contains("\"answer\"")) {
                QString answer = jsonText;
                answer.remove("{").remove("}").remove("\"answer\":").remove("\"");
                answer.replace("\\n", "\n");
                output->setPlainText(answer.trimmed());
            } else {
                output->setPlainText(jsonText);
            }

            status->setText("Готово.");
        });

        // ✅ Чтение stderr
        connect(process, &QProcess::readyReadStandardError, page, [=]() {
            QByteArray err = process->readAllStandardError();
            if (!err.isEmpty()) {
                QString errText = QString::fromUtf8(err);
                qDebug() << "PYTHON ERROR:" << errText;
                output->append("\n[ERROR]\n" + errText);
            }
        });

        // ✅ Запуск
        process->start(pythonExe, args);

        // ✅ Проверка запуска
        if (!process->waitForStarted(2000)) {
            output->setText("[ERROR] Не удалось запустить Python.\nПроверь путь к python.exe");
            status->setText("Ошибка.");
            qDebug() << "FAILED TO START PYTHON";
        }
    });

    return page;
}



// ==================== ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ ====================
QFrame* MainWindow::createStatCard(const QString& title, const QString& value, const QString& subtitle, const QString& id)
{
    QFrame *card = new QFrame();
    card->setMinimumSize(140, 90);    // ИЗМЕНЕНО: для адаптивности
    card->setMaximumSize(170, 110);   // ИЗМЕНЕНО: для адаптивности
    card->setStyleSheet(
        "QFrame {"
        "    background-color: #2a2a2a;"
        "    border-radius: 10px;"
        "    border: 1px solid #444;"
        "}"
        );

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setSpacing(3);
    cardLayout->setContentsMargins(10, 10, 10, 10);

    QLabel *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("font-size: 11px; color: #aaa;");
    titleLabel->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(titleLabel);

    QLabel *valueLabel = new QLabel(value);
    valueLabel->setObjectName("valueLabel");
    valueLabel->setStyleSheet("font-size: 22px; font-weight: bold; color: white;");
    valueLabel->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(valueLabel);

    QLabel *subtitleLabel = new QLabel(subtitle);
    subtitleLabel->setStyleSheet("font-size: 10px; color: #4CAF50;");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(subtitleLabel);

    return card;
}

// ==================== МЕТОДЫ ДЛЯ ТАЙМЕРА ====================
void MainWindow::startWorkoutTimer()
{
    if (!isTimerRunning) {
        isTimerRunning = true;
        qDebug() << "Таймер начат";
    }
}

void MainWindow::stopWorkoutTimer()
{
    if (isTimerRunning) {
        isTimerRunning = false;
        qDebug() << "Таймер остановлен";
    }
}

void MainWindow::updateTimerDisplay()
{
    qDebug() << "Обновление отображения таймера";
}

void MainWindow::resetWeeklyTimeIfNeeded()
{
    checkWeeklyReset();
}

void MainWindow::checkWeeklyReset()
{
    QSettings settings;
    QDate lastReset = settings.value("lastResetDate").toDate();
    QDate currentDate = QDate::currentDate();

    if (!lastReset.isValid() ||
        lastReset.daysTo(currentDate) >= 7 ||
        lastReset.weekNumber() != currentDate.weekNumber())
    {
        totalWeeklySeconds = 0;
        settings.setValue("lastResetDate", currentDate);
        qDebug() << "Недельная статистика сброшена";
    }
}

void MainWindow::saveWeeklyTime()
{
    QSettings settings;
    settings.setValue("weeklyTime", totalWeeklySeconds);
    settings.setValue("lastSaveDate", QDate::currentDate());
    qDebug() << "Время сохранено:" << totalWeeklySeconds << "секунд";
}

void MainWindow::loadWeeklyTime()
{
    QSettings settings;
    totalWeeklySeconds = settings.value("weeklyTime", 0).toInt();
    qDebug() << "Время загружено:" << totalWeeklySeconds << "секунд";
}

QHBoxLayout* MainWindow::createActivityItem(const QString& date, const QString& type,
                                            const QString& details, const QString& duration)
{
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setSpacing(10);
    layout->setContentsMargins(5, 5, 5, 5);

    QLabel *dateLabel = new QLabel(date);
    dateLabel->setStyleSheet("color: white;");

    QLabel *typeLabel = new QLabel(type);
    typeLabel->setStyleSheet("color: white; font-weight: bold;");

    QLabel *detailsLabel = new QLabel(details);
    detailsLabel->setStyleSheet("color: #ccc;");

    QLabel *durationLabel = new QLabel(duration);
    durationLabel->setStyleSheet("color: #4CAF50;");

    layout->addWidget(dateLabel);
    layout->addWidget(typeLabel);
    layout->addWidget(detailsLabel);
    layout->addWidget(durationLabel);
    layout->addStretch();

    return layout;
}

// ==================== СЛОТЫ ====================
void MainWindow::newWorkout() { QMessageBox::information(this, "New Workout", "Creating new workout..."); }
void MainWindow::openWorkout() { QMessageBox::information(this, "Open Workout", "Opening workout..."); }
void MainWindow::saveWorkout() { QMessageBox::information(this, "Save Workout", "Saving workout..."); }
void MainWindow::exitApp() { QApplication::quit(); }

void MainWindow::showDashboard()
{
    stackedWidget->setCurrentWidget(dashboardPage);
    statusBar()->showMessage("Dashboard view", 3000);
    styleSidebarButton(dashboardBtn, true);
    styleSidebarButton(exercisesBtn, false);
    styleSidebarButton(programsBtn, false);
    styleSidebarButton(progressBtn, false);
    styleSidebarButton(aiCoachBtn, false);
}

void MainWindow::showExercises()
{
    stackedWidget->setCurrentWidget(exercisesPage);
    statusBar()->showMessage("Exercises view", 3000);
    styleSidebarButton(dashboardBtn, false);
    styleSidebarButton(exercisesBtn, true);
    styleSidebarButton(programsBtn, false);
    styleSidebarButton(progressBtn, false);
    styleSidebarButton(aiCoachBtn, false);
}

void MainWindow::showPrograms()
{
    stackedWidget->setCurrentWidget(programsPage);
    statusBar()->showMessage("Programs view", 3000);
    styleSidebarButton(dashboardBtn, false);
    styleSidebarButton(exercisesBtn, false);
    styleSidebarButton(programsBtn, true);
    styleSidebarButton(progressBtn, false);
    styleSidebarButton(aiCoachBtn, false);
}

void MainWindow::showProgress()
{
    stackedWidget->setCurrentWidget(progressPage);
    statusBar()->showMessage("Progress view", 3000);
    styleSidebarButton(dashboardBtn, false);
    styleSidebarButton(exercisesBtn, false);
    styleSidebarButton(programsBtn, false);
    styleSidebarButton(progressBtn, true);
    styleSidebarButton(aiCoachBtn, false);
}

void MainWindow::showAICoach()
{
    stackedWidget->setCurrentWidget(aiCoachPage);
    statusBar()->showMessage("AI Coach view", 3000);
    styleSidebarButton(dashboardBtn, false);
    styleSidebarButton(exercisesBtn, false);
    styleSidebarButton(programsBtn, false);
    styleSidebarButton(progressBtn, false);
    styleSidebarButton(aiCoachBtn, true);
}

void MainWindow::aboutApp()
{
    QMessageBox::about(this, "About GymTracker",
                       "<h2>GymTracker v1.0</h2>"
                       "<p>AI-powered fitness tracking application</p>");
}

void MainWindow::aboutQt()
{
    QMessageBox::aboutQt(this, "About Qt");
}
