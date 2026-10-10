#pragma once
#include <QString>
namespace primow {
bool saveKey(const QString &key, const QString &service);
QString loadKey(const QString &service);
}
