#include "Windows.hpp"
#include <QFileInfo>
#include <dwmapi.h>
namespace primow {
static QString escape(QString s) {
    return s.replace("#", "#22").replace(":", "#3A");
}
QVector<Window> windows() {
    QVector<Window> result;
    EnumWindows([](HWND hwnd, LPARAM data) -> BOOL {
        if (!IsWindowVisible(hwnd) || GetWindow(hwnd, GW_OWNER)) return TRUE;
        DWORD pid = 0, cloaked = 0;
        GetWindowThreadProcessId(hwnd, &pid);
        if (pid == GetCurrentProcessId()) return TRUE;
        DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &cloaked, sizeof(cloaked));
        if (cloaked) return TRUE;
        wchar_t title[2048]{}, klass[256]{}, path[32768]{};
        if (!GetWindowTextW(hwnd, title, 2048)) return TRUE;
        GetClassNameW(hwnd, klass, 256);
        HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
        if (!process) return TRUE;
        DWORD size = 32768;
        const bool ok = QueryFullProcessImageNameW(process, 0, path, &size);
        CloseHandle(process);
        if (!ok) return TRUE;
        QString name = QFileInfo(QString::fromWCharArray(path)).fileName();
        QString text = QString::fromWCharArray(title);
        auto &out = *reinterpret_cast<QVector<Window> *>(data);
        out.push_back({hwnd, text, name, escape(text) + ":" +
            escape(QString::fromWCharArray(klass)) + ":" + escape(name)});
        return TRUE;
    }, reinterpret_cast<LPARAM>(&result));
    return result;
}
}
