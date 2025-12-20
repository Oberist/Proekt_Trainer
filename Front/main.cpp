#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QFont>
#include <QDir>
#include <QDebug>

int main(int argc, char *argv[])
{
    // ✅ Сначала создаём QApplication — это обязательно
    QApplication app(argc, argv);

    // ✅ Теперь можно получить путь к .exe
    QString exeDir = QCoreApplication::applicationDirPath();

    // ✅ Устанавливаем рабочую директорию на папку, где лежит .exe
    QDir::setCurrent(exeDir);

    // ✅ Отладочный вывод (увидишь в PowerShell)
    qDebug() << "applicationDirPath:" << exeDir;
    qDebug() << "currentPath:" << QDir::currentPath();

    qDebug() << "🔥 GymApp1 build 2025-12-19 🔥";

    // Шрифт для macOS
#ifdef Q_OS_MAC
    QFont defaultFont("SF Pro Text", 11);
#else
    QFont defaultFont("Arial", 10);
#endif
    app.setFont(defaultFont);

    MainWindow window;
    window.setWindowTitle("🏋️ GymTracker - AI Fitness Coach");
    window.show();

    return app.exec();
}
