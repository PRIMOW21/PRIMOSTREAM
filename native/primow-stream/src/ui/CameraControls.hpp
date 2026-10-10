#pragma once
#include <QGroupBox>
namespace primow {
class Engine;
class CameraControls : public QGroupBox {
public:
    explicit CameraControls(Engine &engine, QWidget *parent = nullptr);
};
}
