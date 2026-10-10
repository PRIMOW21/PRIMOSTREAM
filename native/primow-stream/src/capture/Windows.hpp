#pragma once
#include <QString>
#include <QVector>
#include <windows.h>
namespace primow {
struct Window { HWND handle; QString title, executable, obsId; };
QVector<Window> windows();
}
