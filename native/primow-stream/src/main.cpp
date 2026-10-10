#include "ui/MainWindow.hpp"
#include <QApplication>
#include <QMessageBox>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
int main(int argc, char **argv) {
    QApplication app(argc, argv);
    // Keep the existing settings identity so upgrades retain local preferences.
    QCoreApplication::setOrganizationName("PRIMOW");
    QCoreApplication::setApplicationName("PRIMOW STREAM");
    QApplication::setApplicationDisplayName("NERON STREAM PROJECT");
    QSettings::setDefaultFormat(QSettings::IniFormat);
    const auto settingsPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(settingsPath);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settingsPath);
    app.setStyleSheet(
        "QMainWindow,QWidget#page{background:#0c1017;color:#edf0f7;font-family:'Segoe UI';font-size:13px;}"
        "QScrollArea{background:transparent;border:0;}"
        "QLabel#title{font-size:26px;font-weight:700;color:#f5f6fb;}"
        "QLabel#subtitle,QLabel#hint{color:#9aa4b5;}"
        "QLabel#status{background:#202733;color:#c5ccda;border:1px solid #354152;"
        "border-radius:12px;padding:7px 12px;font-weight:600;}"
        "QLabel#metrics{color:#b9c1ce;font-weight:600;padding:8px;}"
        "QGroupBox{background:#141a23;border:1px solid #293241;border-radius:12px;"
        "margin-top:12px;padding:22px 15px 15px;font-weight:600;}"
        "QGroupBox::title{subcontrol-origin:margin;left:14px;padding:0 6px;color:#dce2ed;}"
        "QLineEdit,QComboBox,QSpinBox{background:#0e141d;color:#f2f4f8;"
        "padding:9px 10px;border:1px solid #333e4e;border-radius:7px;min-height:20px;}"
        "QLineEdit:focus,QComboBox:focus,QSpinBox:focus{border:1px solid #a889ff;}"
        "QLineEdit:disabled,QComboBox:disabled,QSpinBox:disabled{color:#737e8d;background:#141a23;}"
        "QComboBox QAbstractItemView{background:#141a23;color:#f2f4f8;"
        "selection-background-color:#6244a5;border:1px solid #333e4e;}"
        "QPushButton{background:#252d39;color:#f3f5f8;padding:9px 12px;"
        "border:1px solid #354152;border-radius:7px;font-weight:600;}"
        "QPushButton:hover{background:#303b4b;border-color:#65748a;}"
        "QPushButton:disabled{color:#788393;background:#1a2029;}"
        "QPushButton#primaryButton{background:#e63d59;border:0;color:white;"
        "font-size:14px;font-weight:700;border-radius:8px;padding:13px 22px;}"
        "QPushButton#primaryButton:hover{background:#f04b67;}"
        "QCheckBox{spacing:8px;color:#dce2ed;}"
        "QCheckBox::indicator{width:16px;height:16px;border-radius:4px;"
        "border:1px solid #596579;background:#0e141d;}"
        "QCheckBox::indicator:checked{background:#a889ff;border-color:#a889ff;}"
        "QSlider::groove:horizontal{height:4px;background:#333e4e;border-radius:2px;}"
        "QSlider::handle:horizontal{width:14px;margin:-5px 0;background:#a889ff;border-radius:7px;}"
        "QScrollBar:vertical{background:#0c1017;width:10px;margin:2px;}"
        "QScrollBar::handle:vertical{background:#394556;border-radius:4px;min-height:24px;}"
        "QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical{height:0;}");
    try { primow::MainWindow window; window.show(); return app.exec(); }
    catch (const std::exception &e) {
        QMessageBox::critical(nullptr, "NERON STREAM PROJECT", QString::fromUtf8(e.what())); return 1;
    }
}
