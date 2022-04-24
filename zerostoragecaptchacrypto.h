// 2022 (c) GPLv3, acetone at i2pmail.org
// Zero Storage Captcha

#ifndef ZEROSTORAGECAPTCHACRYPTO_H
#define ZEROSTORAGECAPTCHACRYPTO_H

#include <QString>
#include <QTimer>
#include <QMutex>
#include <QMultiMap>
#include <openssl/evp.h>

constexpr int KEYSIZE = 32;
constexpr int SIGSIZE = 64;

namespace ZeroStorageCaptchaCrypto {

QString hash(const QString& str);
QByteArray random(int length, bool onlyNumbers = false);

/////////

class TimeToken
{
public:
    TimeToken() = delete;

    static void init();
    static bool inited() { return m_updater; }
    static const QString currentToken() { return m_current; }
    static const QString prevToken()    { return m_prev; }

private:
    static QTimer* m_updater;
    static QString m_current;
    static QString m_prev;
};

/////////

class KeyHolder
{
public:
    KeyHolder() = delete;

    static QString captchaSecretLine(const QString& captchaAnswer, bool prevTimeToken = false);
    static bool validateCaptchaAnswer(const QString& answer, const QString& secretLine);
    static void setCaseSensitive(bool enabled = false) { m_caseSensitive = enabled; }
    static void setMaxSizeOfUsedTokensCache(size_t size) { m_maximalSizeOfUsedMap = size; }

    friend TimeToken;

private:
    static QString compact(const QString& str);
    static void sign(const uint8_t * buf, size_t len, uint8_t * signature, const uint8_t * privateKey);

    static void warningLog();
    static void removeOldToken(const QString& oldPrevToken);
    static QMutex m_usedTokensMtx;
    static QMultiMap<QString, QString> m_usedTokens;
    static int m_maximalSizeOfUsedMap;
    static bool m_caseSensitive;
    static uint8_t m_key[KEYSIZE];
};

} // namespace

#endif // ZEROSTORAGECAPTCHACRYPTO_H 
