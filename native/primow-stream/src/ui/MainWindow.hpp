#pragma once
#include <QMainWindow>
#include <QSet>
#include <functional>
#include "../streaming/Engine.hpp"
#include "../capture/Windows.hpp"
class QComboBox;
class QCheckBox;
class QLabel;
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
    QComboBox *sources, *methods, *audioMode;
    QCheckBox *follow;
    QLabel *status;
    QSet<QString> approved;
    QString current;
    void refresh();
    void select();
    void attempt(const std::function<void()> &fn);
};
}
