// 2022 (c) GPLv3, acetone at i2pmail.org
// Zero Storage Captcha

#include "zerostoragecaptchacrypto.h"

#include <QDateTime>
#include <QVector>
#include <QDebug>
#include <random>
#include <openssl/sha.h>

constexpr int TIME_TOKEN_SIZE = 5; // + secs since epoch
constexpr int DEFAULT_SIZE_OF_USED_TOKENS_CACHE = 100000;
constexpr int TIMER_TO_CHANGE_TOKEN_MSECS = 90000; // 1,5 min

namespace ZeroStorageCaptchaCrypto {

QTimer* TimeToken::m_updater = nullptr;
QString TimeToken::m_current;
QString TimeToken::m_prev;
int KeyHolder::m_maximalSizeOfUsedMap = DEFAULT_SIZE_OF_USED_TOKENS_CACHE;
QMutex KeyHolder::m_usedTokensMtx;
QMultiMap<QString, QString> KeyHolder::m_usedTokens;
bool KeyHolder::m_caseSensitive = false;
uint8_t KeyHolder::m_key[KEYSIZE] {0};

void TimeToken::init()
{
    if (m_updater) return;

    m_updater = new QTimer;
    m_current = ZeroStorageCaptchaCrypto::random(TIME_TOKEN_SIZE) + QString::number(QDateTime::currentSecsSinceEpoch());
    m_updater->setInterval(TIMER_TO_CHANGE_TOKEN_MSECS);
    QObject::connect(m_updater, &QTimer::timeout, [&]() {
        KeyHolder::removeOldToken(m_prev);
        m_prev = m_current;
        m_current = ZeroStorageCaptchaCrypto::random(TIME_TOKEN_SIZE) + QString::number(QDateTime::currentSecsSinceEpoch());
    });
    m_updater->start();
}

QString KeyHolder::captchaSecretLine(const QString &captchaAnswer, bool prevTimeToken)
{
    if (m_usedTokens.size() > m_maximalSizeOfUsedMap)
    {
        warningLog();
        return QString();
    }

    if (m_key[0] == 0)
    {
        auto noize = ZeroStorageCaptchaCrypto::random(KEYSIZE);
        for (int i = 0; i < KEYSIZE; ++i)
        {
            m_key[i] = static_cast<uint8_t>(noize[i]);
        }
    }

    QString stringToSign = (m_caseSensitive ? captchaAnswer : captchaAnswer.toUpper()) +
                           (prevTimeToken ? TimeToken::prevToken() : TimeToken::currentToken());

    uint8_t signature[SIGSIZE];
    sign(reinterpret_cast<const uint8_t *>(stringToSign.toStdString().c_str()), static_cast<size_t>(stringToSign.size()), signature, m_key);

    QByteArray rawResultArray;
    for(int i = 0; i < SIGSIZE; ++i)
    {
        rawResultArray += static_cast<char>(signature[i]);
    }

    return compact(rawResultArray.toBase64(QByteArray::Base64Option::Base64UrlEncoding));
}

bool KeyHolder::validateCaptchaAnswer(const QString &answer, const QString &secretLine)
{
    QString timeKey;
    if (captchaSecretLine(answer) == secretLine)
    {
        timeKey = TimeToken::currentToken();
    }
    else if (captchaSecretLine(answer, true) == secretLine)
    {
        timeKey = TimeToken::prevToken();
    }

    if (not timeKey.isEmpty())
    {
        QMutexLocker lock (&m_usedTokensMtx);
        if (m_usedTokens.size() > m_maximalSizeOfUsedMap)
        {
            warningLog();
            return false;
        }
        if (m_usedTokens.find( timeKey ) == m_usedTokens.end())
        {
            m_usedTokens.insert(timeKey, secretLine);
            return true;
        }
    }

    return false;
}

void KeyHolder::removeOldToken(const QString &oldPrevToken)
{
    QMutexLocker lock (&m_usedTokensMtx);
    m_usedTokens.remove(oldPrevToken);
}

QString KeyHolder::compact(const QString &str)
{
    QString result;
    int counter = 0;
    for (const auto& c: str)
    {
        if (++counter % 3 == 0)
        {
            result += c;
        }
    }
    result.remove('=');
    result.remove('_');
    result.remove('-');
    return result;
}

void KeyHolder::sign(const uint8_t *buf, size_t len, uint8_t *signature, const uint8_t *privateKey)
{
    auto MDCtx = EVP_MD_CTX_create ();
    auto PKey = EVP_PKEY_new_raw_private_key (EVP_PKEY_ED25519, NULL, privateKey, KEYSIZE);
    EVP_DigestSignInit (MDCtx, NULL, NULL, NULL, PKey);

    size_t l = SIGSIZE;
    EVP_DigestSign (MDCtx, signature, &l, buf, len);

    EVP_PKEY_free (PKey);
    EVP_MD_CTX_destroy (MDCtx);
}

void KeyHolder::warningLog()
{
    qInfo().noquote() <<
        "<warning time=\"" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+ "\">\n"
        "  Token cache is full (" + QString::number(m_maximalSizeOfUsedMap) + "). Service temporary unavailable.\n"
        "  You can increase maximal cache size via ZeroStorageCaptchaCrypto::KeyHolder::setMaxSizeOfUsedTokensCache(size_t)\n"
        "</warning>";
}

QByteArray random(int length, bool onlyNumbers)
{
    constexpr char randomtable[60] =
         {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
          'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
          'k', 'l', 'm', 'n', 'k', 'p', 'q', 'r', 's', 't',
          'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D',
          'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
          'h', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X'};

    QByteArray random_value;

    std::random_device rd;
    std::uniform_int_distribution<int> dist(onlyNumbers ? 0 : 1, onlyNumbers ? 9 : 59);

    while(random_value.size() < length)
    {
        random_value += randomtable[dist(rd)];
    }

    return random_value;
}

} // namespace 
