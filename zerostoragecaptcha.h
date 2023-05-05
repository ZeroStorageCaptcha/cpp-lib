// GPLv3 (c) acetone, 2022-2023
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

#ifndef ZEROSTORAGECAPTCHA_H
#define ZEROSTORAGECAPTCHA_H

#include <QFont>
#include <QImage>
#include <QString>
#include <QTimer>
#include <QMutex>
#include <QSet>
#include <QMap>
#include <QSharedPointer>

class ZeroStorageCaptcha;

namespace ZeroStorageCaptchaService {

using IdType = size_t;

QByteArray random(int length, bool onlyNumbers = false);

class TimeToken
{
public:
    TimeToken() = delete;

    static void init();
    static const QString currentToken() { return m_current; }
    static const QString prevToken()    { return m_prev; }
    static bool exists(const QString& some) { return m_current == some or m_prev == some; }

private:
    static QTimer* m_updater;
    static QString m_current;
    static QString m_prev;
};

class IdCounter
{
public:
    IdCounter() = delete;

    static IdType get();

private:
    static std::atomic<IdType> m_counter;
};

class TokenManager
{
    friend TimeToken;

public:
    TokenManager() = delete;

    static QString get(const QString& captchaAnswer, IdType id = 0, bool prevTimeToken = false);
    static bool validateAnswer(const QString& answer, const QString& token);
    static QByteArray numberToBytes(IdType number);
    static IdType bytesToNumber(const QByteArray& bytes);
    static void setCaseSensitive(bool enabled = false) { m_caseSensitive = enabled; }
    static bool caseSensitive() { return m_caseSensitive; }

private:
    static void removeAllTokensExceptPassed(const QString& current, const QString& prev);
    static QMutex m_usedTokensMtx;
    static QMap<QString, QSet<IdType>> m_usedTokens;
    static bool m_caseSensitive;
};

class Cache
{
    friend TokenManager;
public:
    static void setAnswerLength(int length = 5);
    static int answerLength() { return m_length; }
    static void setDifficulty(int difficulty) { m_difficulty = difficulty; }
    static int difficulty() { return m_difficulty; }
    static void setMaxCapacity(qsizetype value) { m_capacity = value; }
    static qsizetype maxCapacity() { return m_capacity; }
    static qsizetype size() { return m_cache.size(); }
    static QSharedPointer<ZeroStorageCaptcha> get();

private:
    static void remove(const QString& token);

    static QList< QPair<QString /* time token */, QSharedPointer<ZeroStorageCaptcha>> > m_cache;
    static QMutex m_cacheMtx;
    static qsizetype m_capacity;
    static int m_length;
    static int m_difficulty;
};

} // namespace

///////////////////////////////

class ZeroStorageCaptcha
{
    friend ZeroStorageCaptchaService::Cache; // for dropToken()
public:
    ZeroStorageCaptcha();
    ZeroStorageCaptcha(const QString& answer, int difficulty = ZeroStorageCaptchaService::Cache::difficulty());
    static QSharedPointer<ZeroStorageCaptcha> cached();
    static bool validate(const QString& answer, const QString& token);
    static void setCacheMaxCapacity(qsizetype value);
    static qsizetype cacheMaxCapacity();
    static qsizetype cacheSize();
    static void setDefaultAnswerLength(int length);
    static int defaultAnswerLength();
    static void setDefaultDifficulty(int difficulty);
    static int defaultDifficulty();
    static void setNumbersOnlyMode(bool enabled = false) { m_onlyNumbers = enabled; }
    static bool numbersOnlyMode() { return m_onlyNumbers; }
    static void setCaseSensitive(bool enabled = false);
    static bool caseSensitive();

    QString answer() const        { return m_captchaText; }
    QString token() const;
    QByteArray picturePng() const;

    QImage qimage() const         { return m_captchaImage; }
    QFont font() const            { return m_font; }
    QColor fontColor() const      { return m_fontColor; }
    QColor backColor() const      { return m_backColor; }
    bool drawLines() const        { return m_drawLines; }
    bool drawEllipses() const     { return m_drawLines; }
    bool drawNoise() const        { return m_drawLines; }
    int noiseCount() const        { return m_noiseCount; }
    int lineCount() const         { return m_lineCount; }
    int ellipseCount() const      { return m_ellipseCount; }
    int lineWidth() const         { return m_lineWidth; }
    int ellipseMinRadius() const  { return m_ellipseMinRadius; }
    int ellipseMaxRadius() const  { return m_ellipseMaxRadius; }
    int noisePointSize() const    { return m_noisePointSize; }

    void setFont(const QFont& arg)    { m_font = arg; }
    void setCaptchaText(QString arg)  { m_captchaText = arg; }
    void setFontColor(QColor arg)     { m_fontColor = arg; }
    void setBackColor(QColor arg)     { m_backColor = arg; }
    void setDrawLines(bool arg)       { m_drawLines = arg; }
    void setDrawEllipses(bool arg)    { m_drawEllipses = arg; }
    void setDrawNoise(bool arg)       { m_drawNoise = arg; }
    void setNoiseCount(int arg)       { m_noiseCount = arg; }
    void setLineCount(int arg)        { m_lineCount = arg; }
    void setEllipseCount(int arg)     { m_ellipseCount = arg; }
    void setLineWidth(int arg)        { m_lineWidth = arg; }
    void setEllipseMinRadius(int arg) { m_ellipseMinRadius = arg; }
    void setEllipseMaxRadius(int arg) { m_ellipseMaxRadius = arg; }
    void setNoisePointSize(int arg)   { m_noisePointSize = arg; }
    void setSinDeform(qreal hAmplitude, qreal hFrequency, qreal vAmplitude, qreal vFrequency);
    void setDifficulty(int val);
    void setAnswer(const QString& answer);
    void generateAnswer(int length = 5);
    void render();

private:
    void dropToken() { m_token.clear(); } // for Cache
    void init();
    static bool m_onlyNumbers;

    qreal m_hmod1;
    qreal m_hmod2;

    qreal m_vmod1;
    qreal m_vmod2;

    QFont m_font;
    QImage m_captchaImage;
    QString m_captchaText = "empty";
    QColor m_fontColor;
    QColor m_backColor;
    qreal m_padding;
    bool m_drawLines;
    bool m_drawEllipses;
    bool m_drawNoise;
    int m_noiseCount;
    int m_lineCount;
    int m_ellipseCount;
    int m_lineWidth;
    int m_ellipseMinRadius;
    int m_ellipseMaxRadius;
    int m_noisePointSize;

    mutable QString m_token;
};

#endif // ZEROSTORAGECAPTCHA_H 
