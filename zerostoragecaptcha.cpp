// GPLv3 (c) 2022, acetone
// Zero Storage Captcha

// PNG generation based on:

/*
* Copyright (c) 2014 Omkar Kanase
* QtCaptcha: https://github.com/omkar-developer/QtCaptcha
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#include "zerostoragecaptcha.h"

#include <QTime>
#include <QBuffer>
#include <QDebug>
#include <QPainter>
#include <QPainterPath>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QCryptographicHash>

bool ZeroStorageCaptcha::m_onlyNumbers = false;

ZeroStorageCaptcha::ZeroStorageCaptcha()
{
    ZeroStorageCaptchaService::TimeToken::init();

    m_hmod1 = 0.0;
    m_hmod2 = 0.0;

    m_vmod1 = 0.0;
    m_vmod2 = 0.0;

    m_font.setStyleStrategy(QFont::ForceOutline);
    m_font.setPointSize(50);
    m_font.setBold(true);
    m_font.setLetterSpacing(QFont::PercentageSpacing, QFont::SemiCondensed);

    m_captchaImage = QImage(200, 100, QImage::Format_RGB32);

    if (QTime::currentTime().msec() % 2 == 0)
    {
        m_backColor = Qt::GlobalColor::white;
        m_fontColor = Qt::GlobalColor::black;
    }
    else
    {
        m_backColor = Qt::GlobalColor::black;
        m_fontColor = Qt::GlobalColor::white;
    }

    m_padding = 5;

    setDifficulty(2);
    m_captchaText = "NOTSET";
}

bool ZeroStorageCaptcha::validate(const QString &answer, const QString &token)
{
    return ZeroStorageCaptchaService::TokenManager::validateAnswer(answer, token);
}

QString ZeroStorageCaptcha::token() const
{
    if (m_token.isEmpty())
    {
        m_token = ZeroStorageCaptchaService::TokenManager::get(m_captchaText);
    }
    return m_token;
}

QByteArray ZeroStorageCaptcha::picture() const
{
    QByteArray data;
    QBuffer buff(&data);
    m_captchaImage.save(&buff, "PNG");
    return data;
}

void ZeroStorageCaptcha::updateCaptcha()
{
    QPainterPath path;
    QFontMetrics fm(m_font);

    path.addText(m_vmod2 + m_padding, m_hmod2 - m_padding + fm.height(), font(), answer());

    qreal sinrandomness = QRandomGenerator::system()->generateDouble() * 5.0;

    for (int i = 0; i < path.elementCount(); ++i)
    {
        const QPainterPath::Element& el = path.elementAt(i);
        qreal y = el.y + sin(el.x / m_hmod1 + sinrandomness) * m_hmod2;
        qreal x = el.x + sin(el.y / m_vmod1 + sinrandomness) * m_vmod2;
        path.setElementPositionAt(i, x, y);
    }

    m_captchaImage = QImage(static_cast<int>(fm.horizontalAdvance(m_captchaText) + m_vmod2 * 2 + m_padding * 2),
                            static_cast<int>(fm.height() + m_hmod2 * 2 + m_padding * 2), QImage::Format_RGB32);

    m_captchaImage.fill(backColor());

    QPainter painter;
    painter.begin(&m_captchaImage);
    painter.setPen(Qt::NoPen);
    painter.setBrush(fontColor());
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawPath(path);

    if (m_drawLines)
    {
        painter.setPen(QPen(Qt::black, m_lineWidth));
        for (int i = 0; i < m_lineCount; i++)
        {
            int x1 = static_cast<int>(QRandomGenerator::system()->generateDouble() * m_captchaImage.width());
            int y1 = static_cast<int>(QRandomGenerator::system()->generateDouble() * m_captchaImage.height());
            int x2 = static_cast<int>(QRandomGenerator::system()->generateDouble() * m_captchaImage.width());
            int y2 = static_cast<int>(QRandomGenerator::system()->generateDouble() * m_captchaImage.height());
            painter.drawLine(x1, y1, x2, y2);
        }
        painter.setPen(Qt::NoPen);
    }

    if (m_drawEllipses)
    {
        for (int i = 0; i < m_ellipseCount; i++)
        {
            int x1 = static_cast<int>(m_ellipseMaxRadius / 2.0 + QRandomGenerator::system()->generateDouble() * (m_captchaImage.width() - m_ellipseMaxRadius));
            int y1 = static_cast<int>(m_ellipseMaxRadius / 2.0 + QRandomGenerator::system()->generateDouble() * (m_captchaImage.height() - m_ellipseMaxRadius));
            int rad1 = static_cast<int>(m_ellipseMinRadius + QRandomGenerator::system()->generateDouble() * (m_ellipseMaxRadius - m_ellipseMinRadius));
            int rad2 = static_cast<int>(m_ellipseMinRadius + QRandomGenerator::system()->generateDouble() * (m_ellipseMaxRadius - m_ellipseMinRadius));
            if (backColor() == Qt::GlobalColor::black)
            {
                painter.setBrush(fontColor());
                painter.setCompositionMode(QPainter::CompositionMode_Difference);
            } else {
                painter.setBrush(backColor());
                painter.setCompositionMode(QPainter::CompositionMode_Exclusion);
            }
            painter.drawEllipse(QPoint(x1, y1), rad1, rad2);
        }
    }

    if (m_drawNoise)
    {
        for (int i = 0; i < m_noiseCount; i++)
        {
            int x1 = static_cast<int>(QRandomGenerator::system()->generateDouble() * m_captchaImage.width());
            int y1 = static_cast<int>(QRandomGenerator::system()->generateDouble() * m_captchaImage.height());

            QColor col = backColor() == Qt::GlobalColor::black ? Qt::GlobalColor::white : Qt::GlobalColor::black;

            painter.setPen(QPen(col, m_noisePointSize));
            painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            painter.drawPoint(x1, y1);
        }
    }

    painter.end();
}

void ZeroStorageCaptcha::setSinDeform(qreal hAmplitude, qreal hFrequency, qreal vAmplitude, qreal vFrequency)
{
    m_hmod1 = hFrequency;
    m_hmod2 = hAmplitude;
    m_vmod1 = vFrequency;
    m_vmod2 = vAmplitude;
}

void ZeroStorageCaptcha::setDifficulty(int val)
{
    short variant = QRandomGenerator::system()->bounded(1, 3);

    if (val < 0 or val > 2)
    {
        qInfo().noquote() << QString(__PRETTY_FUNCTION__) << "Min difficulty is 0, maximal is 2";
    }

    if (val < 1)
    {
        m_drawLines = false;
        m_drawNoise = false;
        m_drawEllipses = true;
        m_ellipseCount = 1;
        m_ellipseMinRadius = 10;
        m_ellipseMaxRadius = 40;
        switch (variant) {
        case 0:
            setSinDeform(2, 5, 5, 15);
            break;
        case 1:
            setSinDeform(5, 7, 7, 15);
            break;
        default:
            setSinDeform(2, 1, 1, 10);
            break;
        }
    }
    else if (val == 1)
    {
        m_drawLines = true;
        m_lineWidth = 2;
        m_lineCount = 10;
        m_drawEllipses = true;
        m_ellipseCount = 2;
        m_ellipseMinRadius = 20;
        m_ellipseMaxRadius = 30;
        m_drawNoise = true;
        m_noiseCount = 30;
        m_noisePointSize = 5;
        switch (variant) {
        case 0:
            setSinDeform(2, 5, 5, 25);
            break;
        case 1:
            setSinDeform(6, 8, 8, 25);
            break;
        default:
            setSinDeform(3, 6, 6, 15);
            break;
        }
    }
    else
    {
        m_drawLines = true;
        m_lineWidth = 4;
        m_lineCount = 9;
        m_drawEllipses = true;
        m_ellipseCount = 1;
        m_ellipseMinRadius = 50;
        m_ellipseMaxRadius = 70;
        m_drawNoise = true;
        m_noiseCount = 150;
        m_noisePointSize = 4;
        setSinDeform(2, 5, 5, 15);
    }
}

void ZeroStorageCaptcha::generateText(int length)
{
    if (length <= 0)
    {
        qInfo() << QString(__PRETTY_FUNCTION__) << "Invalid number of characters. Set to 5.";
        length = 5;
    }

    m_captchaText = ZeroStorageCaptchaService::random(length, m_onlyNumbers);
} 

//////////////////////////

constexpr const int      TIME_TOKEN_SIZE = 5; // (symbols count) + secs since epoch
constexpr const size_t   DEFAULT_SIZE_OF_USED_TOKENS_CACHE = 10000000; // 10M
constexpr const int      TIMER_TO_CHANGE_TOKEN_MSECS = 90000; // 1,5 min
constexpr const int      KEY_STRING_SIZE = 32;

namespace ZeroStorageCaptchaService {

QTimer*                           TimeToken::m_updater = nullptr;
QString                           TimeToken::m_current;
QString                           TimeToken::m_prev;

size_t                            TokenManager::m_maximalSizeOfUsedMap = DEFAULT_SIZE_OF_USED_TOKENS_CACHE;
QMutex                            TokenManager::m_usedTokensMtx;
std::map<QString, QSet<quint64>>  TokenManager::m_usedTokens;
bool                              TokenManager::m_caseSensitive = false;
QString                           TokenManager::m_key = nullptr;

void TimeToken::init()
{
    if (m_updater) return;

    m_updater = new QTimer;
    m_current = ZeroStorageCaptchaService::random(TIME_TOKEN_SIZE) + QString::number(QDateTime::currentSecsSinceEpoch());
    m_updater->setInterval(TIMER_TO_CHANGE_TOKEN_MSECS);
    QObject::connect (
        m_updater, &QTimer::timeout,
        [&]() {
            TokenManager::removeToken( prevToken() );
            m_prev = m_current;
            m_current = ZeroStorageCaptchaService::random(TIME_TOKEN_SIZE) + QString::number(QDateTime::currentSecsSinceEpoch());
        }
    );
    m_updater->start();
}

std::atomic<size_t> IdCounter::counter = 0;

size_t IdCounter::get()
{
    size_t value = ++counter;
    if (value == 0)
    {
        value++;
    }
    return value;
}

QString TokenManager::get(const QString &captchaAnswer, size_t id, bool prevTimeToken)
{
    if (m_key.isEmpty())
    {
        m_key = random(KEY_STRING_SIZE); // init at first call
    }

    if (id == 0)
    {
        id = IdCounter::get();
    }

    // ANSWER + TIME_TOKEN + ID + SESSION_KEY
    // TIME_TOKEN - temporary marker for limiting captcha life circle
    // ID - size_t validation key for concrete captcha
    // SESSION_KEY - random run-time session key for unique hash value
    const QString base = (m_caseSensitive ? captchaAnswer : captchaAnswer.toUpper()) +
                         (prevTimeToken ? TimeToken::prevToken() : TimeToken::currentToken()) +
                         QString::number(id) + m_key;

    const QByteArray hash = QCryptographicHash::hash(base.toUtf8(), QCryptographicHash::Md5);
    QString b64Hash = hash.toBase64(QByteArray::Base64Option::Base64UrlEncoding);
    static QRegularExpression rgx_OnlyLetters("[^a-zA-Z]");
    b64Hash.remove(rgx_OnlyLetters);
    QString token = b64Hash + "_" + QString::number(id);
    return token;
}

bool TokenManager::validateAnswer(const QString &answer, const QString &token)
{
    QString idString {token};
    static QRegularExpression rgx_id("^.*_");
    idString.remove (rgx_id);
    bool idConvertingStatus = false;
    size_t id = idString.toULongLong(&idConvertingStatus);
    if (not idConvertingStatus or id == 0)
    {
        return false;
    }

    QString timeKey;
    if (TokenManager::get(answer, id) == token)
    {
        timeKey = TimeToken::currentToken();
    }
    else if (TokenManager::get(answer, id, true) == token)
    {
        timeKey = TimeToken::prevToken();
    }

    if (timeKey.isEmpty())
    {
        return false;
    }

    QMutexLocker lock (&m_usedTokensMtx);
    if (m_usedTokens.find(timeKey) != m_usedTokens.end())
    {
        if (m_usedTokens[timeKey].contains(id)) // already used
        {
            return false;
        }
    }

    size_t currentUsedSize = 0;
    for (const auto& timePoint: m_usedTokens)
    {
        currentUsedSize += timePoint.second.size();
    }
    if (currentUsedSize >= m_maximalSizeOfUsedMap)
    {
        limitWarningLog();
        return false;
    }

    m_usedTokens[timeKey].insert( id );
    return true;
}

void TokenManager::removeToken(const QString &oldPrevToken)
{
    QMutexLocker lock (&m_usedTokensMtx);
    auto iter = m_usedTokens.find(oldPrevToken);
    if (iter != m_usedTokens.end())
    {
        m_usedTokens.erase(iter);
    }
}

void TokenManager::limitWarningLog()
{
    qInfo().noquote() <<
        "<warning time=\"" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")+ "\">\n"
        "  Token cache is full (" + QString::number(m_maximalSizeOfUsedMap) + "). Service temporary unavailable.\n"
        "  You can increase maximal cache size via ZeroStorageCaptchaService::TokenManager::setMaxSizeOfUsedTokensCache(size_t)\n"
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

    while(random_value.size() < length)
    {
        random_value += randomtable[ QRandomGenerator::system()->bounded (
                                        onlyNumbers ? 0 : 1,
                                        onlyNumbers ? 9 : 59
                                     ) ];
    }

    return random_value;
}

} // namespace ZeroStorageCaptchaService
