#include "Credentials.hpp"
#include <windows.h>
#include <wincred.h>
namespace primow {
static wchar_t target[] = L"PRIMOW-STREAM/Kick";
bool saveKey(const QString &key) {
    QByteArray bytes = key.toUtf8();
    if (bytes.isEmpty() || bytes.size() > CRED_MAX_CREDENTIAL_BLOB_SIZE) return false;
    CREDENTIALW c{};
    c.Type = CRED_TYPE_GENERIC;
    c.TargetName = target;
    c.CredentialBlobSize = DWORD(bytes.size());
    c.CredentialBlob = reinterpret_cast<LPBYTE>(bytes.data());
    c.Persist = CRED_PERSIST_LOCAL_MACHINE;
    bool ok = CredWriteW(&c, 0);
    SecureZeroMemory(bytes.data(), bytes.size());
    return ok;
}
QString loadKey() {
    PCREDENTIALW c = nullptr;
    if (!CredReadW(target, CRED_TYPE_GENERIC, 0, &c)) return {};
    QString key = QString::fromUtf8(reinterpret_cast<char *>(c->CredentialBlob), c->CredentialBlobSize);
    SecureZeroMemory(c->CredentialBlob, c->CredentialBlobSize);
    CredFree(c);
    return key;
}
}
