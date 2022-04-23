// 2022 (c) GPLv3, acetone at i2pmail.org
// Zero Storage Captcha

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

#include "zerostoragecaptchacrypto.h"

#include <QFont>
#include <QImage>

class ZeroStorageCaptchaContainer
{
public:
    ZeroStorageCaptchaContainer(const QByteArray& pic, const QString& token, const QString& answer) :
        m_picture(pic),
        m_token(token),
        m_answer(answer)
    {}

    const QByteArray& picture() const { return m_picture; }
    const QString& token() const { return m_token; }
    const QString& answer() const { return m_answer; }

private:
    QByteArray m_picture;
    QString m_token;
    QString m_answer;
};

class ZeroStorageCaptcha
{
public:
    ZeroStorageCaptcha();
    static bool validate(const QString& answer, const QString& token);
    static ZeroStorageCaptchaContainer getCaptcha(int length = 5, int difficulty = 3);

    QString captchaText() const   { return m_captchaText; }
    QString captchaToken() const;
    QByteArray captchaPngByteArray() const;

    QImage captchaImage() const   { return m_captchaImage; }
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
    void generateText(int length = 5);
    void updateCaptcha();

private:
    qreal m_hmod1;
    qreal m_hmod2;

    qreal m_vmod1;
    qreal m_vmod2;

    QFont m_font;
    QImage m_captchaImage;
    QString m_captchaText;
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
};

#endif // ZEROSTORAGECAPTCHA_H
