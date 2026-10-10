#include "ui/MainWindow.hpp"
#include <QApplication>
#include <QMessageBox>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
int main(int argc, char **argv) {
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("PRIMOW");
    QCoreApplication::setApplicationName("PRIMOW STREAM");
    QSettings::setDefaultFormat(QSettings::IniFormat);
    const auto settingsPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(settingsPath);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settingsPath);
    app.setStyleSheet("QWidget{background:#17191e;color:#eee;font-family:Segoe UI;font-size:13px;}"
        "QLineEdit,QComboBox,QSpinBox{background:#25282f;padding:7px;border:1px solid #41444c;border-radius:4px;}"
        "QPushButton{background:#30343c;padding:10px;border:0;border-radius:5px;}"
        "QPushButton:hover{background:#454a55;} QPushButton:disabled{color:#888;}");
    try { primow::MainWindow window; window.show(); return app.exec(); }
    catch (const std::exception &e) {
        QMessageBox::critical(nullptr, "PRIMOW STREAM", QString::fromUtf8(e.what())); return 1;
    }
}
