#include "ExercisesPage.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QBrush>
#include <QFont>
#include <QMessageBox>
#include <QPixmap>
#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QCoreApplication>
#include <QDir>

ExercisesPage::ExercisesPage(const std::string& dbPath, QWidget* parent)
    : QWidget(parent), manager(dbPath)
{
    this->setStyleSheet("background-color: #1a1a1a;");
    QVBoxLayout* layout = new QVBoxLayout(this);

    // Метка для поиска
    QLabel* label = new QLabel("Поиск по имени:", this);
    label->setStyleSheet("color: white; font-size: 14px;");
    layout->addWidget(label);

    // Поле поиска
    searchBox = new QLineEdit(this);
    searchBox->setPlaceholderText("Введите имя упражнения...");
    searchBox->setFixedHeight(28);
    searchBox->setStyleSheet(
        "QLineEdit {"
        "    font-size: 14px;"
        "    color: white;"
        "    background-color: #2a2a2a;"
        "    border: 1px solid #444;"
        "    border-radius: 5px;"
        "    padding-left: 8px;"
        "}"
        "QLineEdit::placeholder { color: #888; }"
        );
    layout->addWidget(searchBox);
    connect(searchBox, &QLineEdit::textChanged, this, &ExercisesPage::onSearchChanged);

    // Фильтры
    auto setupCombo = [](QComboBox* combo) {
        combo->setStyleSheet(
            "QComboBox {"
            "    color: white;"
            "    font-size: 14px;"
            "    background-color: #2a2a2a;"
            "    border: 1px solid #444;"
            "    border-radius: 5px;"
            "    padding: 2px 8px;"
            "}"
            "QComboBox QAbstractItemView {"
            "    color: white;"
            "    background-color: #2a2a2a;"
            "    selection-background-color: #444;"
            "}"
            );
    };

    // Сложность
    difficultyFilter = new QComboBox(this);
    difficultyFilter->addItem("Все", -1);
    difficultyFilter->addItem("Легко", 1);
    difficultyFilter->addItem("Средне", 2);
    difficultyFilter->addItem("Тяжело", 3);
    setupCombo(difficultyFilter);
    layout->addWidget(difficultyFilter);
    connect(difficultyFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ExercisesPage::onDifficultyChanged);

    // Оборудование
    equipmentFilter = new QComboBox(this);
    equipmentFilter->addItem("Все");
    equipmentFilter->addItem("гантели");
    equipmentFilter->addItem("штанга");
    equipmentFilter->addItem("свой вес");
    equipmentFilter->addItem("тренажер");
    setupCombo(equipmentFilter);
    layout->addWidget(equipmentFilter);
    connect(equipmentFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ExercisesPage::onEquipmentChanged);

    // Тип
    typeFilter = new QComboBox(this);
    typeFilter->addItem("Все");
    typeFilter->addItem("Кардио");
    typeFilter->addItem("Растяжка");
    typeFilter->addItem("Базовое");
    typeFilter->addItem("Изолирующее");
    setupCombo(typeFilter);
    layout->addWidget(typeFilter);
    connect(typeFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ExercisesPage::onTypeChanged);

    // Мышечная группа
    muscleFilter = new QComboBox(this);
    muscleFilter->addItem("Все");
    muscleFilter->addItem("Грудные");
    muscleFilter->addItem("Квадрицепсы");
    muscleFilter->addItem("Широчайшие");
    muscleFilter->addItem("Плечи");
    muscleFilter->addItem("Трицепсы");
    muscleFilter->addItem("Бицепсы");
    muscleFilter->addItem("Трапеция");
    setupCombo(muscleFilter);
    layout->addWidget(muscleFilter);
    connect(muscleFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ExercisesPage::onMuscleChanged);

    // Список упражнений
    list = new QListWidget(this);
    list->setStyleSheet(
        "QListWidget {"
        "    background-color: #2a2a2a;"
        "    border: 1px solid #444;"
        "    border-radius: 5px;"
        "}"
        "QListWidget::item { padding: 5px; }"
        );
    layout->addWidget(list);

    // Загрузка всех упражнений при старте
    auto exercises = manager.getAllExercises();
    for (const auto& ex : exercises) {
        QListWidgetItem* item = new QListWidgetItem(QString::fromUtf8(ex.name.c_str()));
        item->setForeground(QBrush(Qt::white));
        QFont font = item->font();
        font.setPointSize(16);
        item->setFont(font);
        list->addItem(item);
    }

    // Обработка клика на упражнение
    connect(list, &QListWidget::itemClicked, this, [=](QListWidgetItem* item){
        QString name = item->text();
        auto results = manager.searchByName(std::string(name.toUtf8().constData()));

        if (results.empty()) {
            QMessageBox::warning(this, "Ошибка", "Упражнение не найдено.");
            return;
        }

        Exercise ex = results.front();

        QDialog dlg(this);
        dlg.setWindowTitle(QString::fromUtf8(ex.name.c_str()));
        dlg.setStyleSheet("background-color: #1a1a1a; color: white;");

        QVBoxLayout* dlgLayout = new QVBoxLayout(&dlg);

        // Фото
        QLabel* imageLabel = new QLabel(&dlg);

        QString imagePath = ":/images/exercises/" + QString::fromStdString(ex.image_path);
        QPixmap pix(imagePath);

        if (pix.isNull()) {
            imageLabel->setText("Изображение не найдено");
            imageLabel->setStyleSheet("color: white; font-style: italic;");
            imageLabel->setAlignment(Qt::AlignCenter);
        } else {
            int maxWidth = 500;
            int maxHeight = 400;
            imageLabel->setPixmap(pix.scaled(maxWidth, maxHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }

        dlgLayout->addWidget(imageLabel, 0, Qt::AlignHCenter);

        // Техника
        QLabel* techLabel = new QLabel("Техника выполнения:", &dlg);
        techLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
        dlgLayout->addWidget(techLabel);

        QTextEdit* techText = new QTextEdit(QString::fromUtf8(ex.description.c_str()), &dlg);
        techText->setReadOnly(true);
        techText->setStyleSheet(
            "QTextEdit {"
            "    background-color: #2a2a2a;"
            "    color: white;"
            "    border: 1px solid #444;"
            "    border-radius: 5px;"
            "}"
            );
        dlgLayout->addWidget(techText);

        // Основные мышцы
        QLabel* musclePrimalLabel = new QLabel("Основные мышцы:", &dlg);
        musclePrimalLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
        dlgLayout->addWidget(musclePrimalLabel);

        QTextEdit* musclePrimalText = new QTextEdit(QString::fromUtf8(ex.muscle_group_primary.c_str()), &dlg);
        musclePrimalText->setReadOnly(true);
        musclePrimalText->setStyleSheet(
            "QTextEdit {"
            "    background-color: #2a2a2a;"
            "    color: white;"
            "    border: 1px solid #444;"
            "    border-radius: 5px;"
            "}"
            );
        dlgLayout->addWidget(musclePrimalText);

        // Доп. мышцы
        QLabel* muscleSecondLabel = new QLabel("Дополнительные мышцы:", &dlg);
        muscleSecondLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
        dlgLayout->addWidget(muscleSecondLabel);

        QTextEdit* muscleSecondText = new QTextEdit(QString::fromUtf8(ex.muscle_group_secondary.c_str()), &dlg);
        muscleSecondText->setReadOnly(true);
        muscleSecondText->setStyleSheet(
            "QTextEdit {"
            "    background-color: #2a2a2a;"
            "    color: white;"
            "    border: 1px solid #444;"
            "    border-radius: 5px;"
            "}"
            );
        dlgLayout->addWidget(muscleSecondText);

        // Оборудование
        QLabel* equipmentLabel = new QLabel("Оборудование:", &dlg);
        equipmentLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
        dlgLayout->addWidget(equipmentLabel);

        QTextEdit* equipmentText = new QTextEdit(QString::fromUtf8(ex.equipment.c_str()), &dlg);
        equipmentText->setReadOnly(true);
        equipmentText->setStyleSheet(
            "QTextEdit {"
            "    background-color: #2a2a2a;"
            "    color: white;"
            "    border: 1px solid #444;"
            "    border-radius: 5px;"
            "}"
            );
        dlgLayout->addWidget(equipmentText);

        // Кнопки закрытия
        QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok, &dlg);
        dlgLayout->addWidget(buttons);
        connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);

        dlg.exec();
    });
}

// ----- Все обработчики поиска и фильтров -----

void ExercisesPage::onSearchChanged(const QString& text) {
    list->clear();
    QByteArray utf8 = text.toUtf8();
    auto results = manager.searchByName(std::string(utf8.constData(), utf8.size()));
    for (const auto& ex : results) {
        QListWidgetItem* item = new QListWidgetItem(QString::fromUtf8(ex.name.c_str()));
        item->setForeground(QBrush(Qt::white));
        QFont font = item->font();
        font.setPointSize(16);
        item->setFont(font);
        list->addItem(item);
    }
}

void ExercisesPage::onDifficultyChanged(int index) {
    int difficulty = difficultyFilter->itemData(index).toInt();
    list->clear();
    std::vector<Exercise> results;
    if (difficulty == -1)
        results = manager.getAllExercises();
    else
        results = manager.filterByDifficulty(difficulty);

    for (const auto& ex : results) {
        QListWidgetItem* item = new QListWidgetItem(QString::fromUtf8(ex.name.c_str()));
        item->setForeground(QBrush(Qt::white));
        QFont font = item->font();
        font.setPointSize(16);
        item->setFont(font);
        list->addItem(item);
    }
}

void ExercisesPage::onTypeChanged(int index) {
    QString type = typeFilter->itemText(index).toLower();
    list->clear();
    std::vector<Exercise> results;

    if (type == "все") {
        results = manager.getAllExercises();
    } else {
        if (type == "базовое") type = "basic";
        if (type == "изолирующее") type = "isolation";
        if (type == "кардио") type = "cardio";
        if (type == "растяжка") type = "stretch";

        QByteArray utf8 = type.toUtf8();
        results = manager.filterByType(std::string(utf8.constData(), utf8.size()));
    }

    for (const auto& ex : results) {
        QListWidgetItem* item = new QListWidgetItem(QString::fromUtf8(ex.name.c_str()));
        item->setForeground(QBrush(Qt::white));
        QFont font = item->font();
        font.setPointSize(16);
        item->setFont(font);
        list->addItem(item);
    }
}

void ExercisesPage::onMuscleChanged(int index) {
    QString muscle = muscleFilter->itemText(index);
    list->clear();
    std::vector<Exercise> results;
    if (muscle == "Все")
        results = manager.getAllExercises();
    else {
        QByteArray utf8 = muscle.toUtf8();
        results = manager.filterByMuscleGroup(std::string(utf8.constData(), utf8.size()));
    }

    for (const auto& ex : results) {
        QListWidgetItem* item = new QListWidgetItem(QString::fromUtf8(ex.name.c_str()));
        item->setForeground(QBrush(Qt::white));
        QFont font = item->font();
        font.setPointSize(16);
        item->setFont(font);
        list->addItem(item);
    }
}

void ExercisesPage::onEquipmentChanged(int index) {
    QString equipment = equipmentFilter->itemText(index)
                            .toLower()
                            .trimmed();

    equipment.replace("ё", "е");

    list->clear();
    std::vector<Exercise> results;

    // Если "Все" — просто выводим всё
    if (equipment == "все") {
        results = manager.getAllExercises();
    } else {
        auto all = manager.getAllExercises();

        for (const auto& ex : all) {
            QString dbEquip = QString::fromUtf8(ex.equipment.c_str())
                                  .toLower()
                                  .trimmed();

            dbEquip.replace("ё", "е");

            if (dbEquip.contains(equipment)) {
                results.push_back(ex);
            }
        }
    }

    // Отображение
    for (const auto& ex : results) {
        QListWidgetItem* item =
            new QListWidgetItem(QString::fromUtf8(ex.name.c_str()));
        item->setForeground(QBrush(Qt::white));

        QFont font = item->font();
        font.setPointSize(16);
        item->setFont(font);

        list->addItem(item);
    }
}








