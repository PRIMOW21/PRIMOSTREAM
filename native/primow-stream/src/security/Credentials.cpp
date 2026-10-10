#include "Credentials.hpp"
#include <string>
#include <windows.h>
#include <wincred.h>
namespace primow {
static std::wstring targetFor(const QString &service) {
    return QString("NERON-STREAM-PROJECT/%1").arg(service).toStdWString();
}
bool saveKey(const QString &key, const QString &service) {
    QByteArray bytes = key.toUtf8();
    if (bytes.isEmpty() || bytes.size() > CRED_MAX_CREDENTIAL_BLOB_SIZE) return false;
    const auto target = targetFor(service);
    CREDENTIALW c{};
    c.Type = CRED_TYPE_GENERIC;
    c.TargetName = const_cast<LPWSTR>(target.c_str());
    c.CredentialBlobSize = DWORD(bytes.size());
    c.CredentialBlob = reinterpret_cast<LPBYTE>(bytes.data());
    c.Persist = CRED_PERSIST_LOCAL_MACHINE;
    bool ok = CredWriteW(&c, 0);
    SecureZeroMemory(bytes.data(), bytes.size());
    return ok;
}
QString loadKey(const QString &service) {
    const auto target = targetFor(service);
    PCREDENTIALW c = nullptr;
    if (!CredReadW(target.c_str(), CRED_TYPE_GENERIC, 0, &c)) {
        // Read-only compatibility with keys saved before the product rename.
        const auto legacy = QString("PRIMOW-STREAM/%1").arg(service).toStdWString();
        if (!CredReadW(legacy.c_str(), CRED_TYPE_GENERIC, 0, &c)) return {};
    }
    QString key = QString::fromUtf8(reinterpret_cast<char *>(c->CredentialBlob), c->CredentialBlobSize);
    SecureZeroMemory(c->CredentialBlob, c->CredentialBlobSize);
    CredFree(c);
    return key;
}
}
