// 2022 (c) GPLv3, acetone at i2pmail.org
// Zero Storage Captcha

#include "zerostoragecaptchacrypto.h"
#include "timetoken.h"

#include <QVector>
#include <random>
#include <QDebug>
#include <openssl/sha.h>

constexpr int TIME_TOKEN_SIZE = 10;

namespace ZeroStorageCaptchaCrypto {

QTimer* TimeToken::m_updater = nullptr;
QString TimeToken::m_current;
QString TimeToken::m_prev;
bool KeyHolder::m_caseSensitive = false;
uint8_t KeyHolder::m_key[KEYSIZE] {0};

void TimeToken::init()
{
    if (m_updater) return;

    m_updater = new QTimer;
    m_current = ZeroStorageCaptchaCrypto::random(TIME_TOKEN_SIZE);
    m_updater->setInterval(2000); // 2 minutes
    QObject::connect(m_updater, &QTimer::timeout, [&]() {
        m_prev = m_current;
        m_current = ZeroStorageCaptchaCrypto::random(TIME_TOKEN_SIZE);
    });
    m_updater->start();
}

QString KeyHolder::captchaSecretLine(const QString &captchaAnswer, bool prevTimeToken)
{
    if (m_key[0] == 0)
    {
        auto noize = ZeroStorageCaptchaCrypto::random(KEYSIZE);
        for (int i = 0; i < KEYSIZE; ++i)
        {
            m_key[i] = noize[i];
        }
    }

    QString hashedAnswer = ZeroStorageCaptchaCrypto::hash((m_caseSensitive ? captchaAnswer : captchaAnswer.toUpper()) +
                                                          (prevTimeToken ? TimeToken::prevToken() : TimeToken::currentToken()) );

    uint8_t signature[SIGSIZE];
    sign(reinterpret_cast<const uint8_t *>(hashedAnswer.toStdString().c_str()), hashedAnswer.size(), signature, m_key);

    QByteArray rawResultArray;
    for(int i = 0; i < SIGSIZE; ++i)
    {
        rawResultArray += signature[i];
    }

    return compact(rawResultArray.toBase64(QByteArray::Base64Option::Base64UrlEncoding));
}

bool KeyHolder::validateCaptchaAnswer(const QString &answer, const QString &secretLine)
{
    if (captchaSecretLine(answer) == secretLine) return true;
    return captchaSecretLine(answer, true) == secretLine;
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

void KeyHolder::sign(const uint8_t *buf, int len, uint8_t *signature, const uint8_t *privateKey)
{
    auto MDCtx = EVP_MD_CTX_create ();
    auto PKey = EVP_PKEY_new_raw_private_key (EVP_PKEY_ED25519, NULL, privateKey, KEYSIZE);
    EVP_DigestSignInit (MDCtx, NULL, NULL, NULL, PKey);

    size_t l = SIGSIZE;

    EVP_DigestSign (MDCtx, signature, &l, buf, len);

    EVP_PKEY_free (PKey);
    EVP_MD_CTX_destroy (MDCtx);
}

QString hash(const QString &str)
{
    // Partially bit-inverted SHA256
    QVector<uint8_t> in;
    for(auto c: str)
    {
        in.push_back(c.unicode());
    }

    QVector<uint8_t> out(SHA256_DIGEST_LENGTH);
    SHA256(in.data(), in.size(), out.data());

    QByteArray rawResult;
    for (auto b: out)
    {
        rawResult.push_back(b);
    }

    short count = 0;
    for (auto it = rawResult.begin(); it != rawResult.end(); ++it)
    {
        if (++count % 2 == 0)
        {
            *it = ~*it;
        }
    }

    return rawResult.toBase64(QByteArray::Base64Option::Base64UrlEncoding);
}

QByteArray random(int length)
{
    constexpr char randomtable[60] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                                      'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
                                      'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
                                      'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D',
                                      'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
                                      'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X'};

    QByteArray random_value;

    std::random_device rd;
    std::uniform_int_distribution<int> dist(0, 59);

    while(random_value.size() < length)
    {
        random_value += randomtable[dist(rd)];
    }

    return random_value;
}

} // namespace
