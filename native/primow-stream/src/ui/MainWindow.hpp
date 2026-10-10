#pragma once
#include <QMainWindow>
#include <QSet>
#include <functional>
#include "../streaming/Engine.hpp"
#include "../capture/Windows.hpp"
class QComboBox;
class QCheckBox;
class QLabel;
class QLineEdit;
class QSpinBox;
namespace primow {
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow();
protected:
    void closeEvent(QCloseEvent *) override;
private:
    Engine engine;
    QVector<Window> available;
    QComboBox *sources = nullptr, *methods = nullptr, *audioMode = nullptr;
    QComboBox *platform = nullptr, *profile = nullptr;
    QCheckBox *follow = nullptr, *showKey = nullptr;
    QLabel *status = nullptr, *metrics = nullptr, *platformHelp = nullptr;
    QLineEdit *server = nullptr, *key = nullptr;
    QSpinBox *bitrate = nullptr;
    QSet<QString> approved;
    QString current;
    void refresh();
    void select();
    void loadPlatformSettings();
    QString credentialService() const;
    void attempt(const std::function<void()> &fn);
};
}
