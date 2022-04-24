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

#include "zerostoragecaptcha.h"

#include <QTime>
#include <QBuffer>
#include <QDebug>
#include <QPainter>
#include <random>

bool ZeroStorageCaptcha::m_onlyNumbers = false;

ZeroStorageCaptcha::ZeroStorageCaptcha()
{
    if (not ZeroStorageCaptchaCrypto::TimeToken::inited()) ZeroStorageCaptchaCrypto::TimeToken::init();

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

    setDifficulty(3);
    m_captchaText = "NOTSET";

    qsrand(static_cast<uint>(QTime::currentTime().msec())); // randomize
}

bool ZeroStorageCaptcha::validate(const QString &answer, const QString &token)
{
    return ZeroStorageCaptchaCrypto::KeyHolder::validateCaptchaAnswer(answer, token);
}

ZeroStorageCaptchaContainer ZeroStorageCaptcha::getCaptcha(int length, int difficulty)
{
    ZeroStorageCaptcha c;
    c.setDifficulty(difficulty);
    c.generateText(length);
    return ZeroStorageCaptchaContainer (c.captchaPngByteArray(), c.captchaToken(), c.captchaText());
}

QString ZeroStorageCaptcha::captchaToken() const
{
    return ZeroStorageCaptchaCrypto::KeyHolder::captchaSecretLine(m_captchaText);
}

QByteArray ZeroStorageCaptcha::captchaPngByteArray() const
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

    path.addText(m_vmod2 + m_padding, m_hmod2 - m_padding + fm.height(), font(), captchaText());

    qreal sinrandomness = (static_cast<qreal>(qrand()) / RAND_MAX) * 5.0;

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
            int x1 = (static_cast<qreal>(qrand()) / RAND_MAX) * m_captchaImage.width();
            int y1 = (static_cast<qreal>(qrand()) / RAND_MAX) * m_captchaImage.height();
            int x2 = (static_cast<qreal>(qrand()) / RAND_MAX) * m_captchaImage.width();
            int y2 = (static_cast<qreal>(qrand()) / RAND_MAX) * m_captchaImage.height();
            painter.drawLine(x1, y1, x2, y2);
        }
        painter.setPen(Qt::NoPen);
    }

    if (m_drawEllipses)
    {
        for (int i = 0; i < m_ellipseCount; i++)
        {
            int x1 = static_cast<int>(m_ellipseMaxRadius / 2.0 + (static_cast<qreal>(qrand()) / RAND_MAX) * (m_captchaImage.width() - m_ellipseMaxRadius));
            int y1 = static_cast<int>(m_ellipseMaxRadius / 2.0 + (static_cast<qreal>(qrand()) / RAND_MAX) * (m_captchaImage.height() - m_ellipseMaxRadius));
            int rad1 = static_cast<int>(m_ellipseMinRadius + (static_cast<qreal>(qrand()) / RAND_MAX) * (m_ellipseMaxRadius - m_ellipseMinRadius));
            int rad2 = static_cast<int>(m_ellipseMinRadius + (static_cast<qreal>(qrand()) / RAND_MAX) * (m_ellipseMaxRadius - m_ellipseMinRadius));
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
            int x1 = static_cast<int>(static_cast<qreal>(qrand()) / RAND_MAX * m_captchaImage.width());
            int y1 = static_cast<int>(static_cast<qreal>(qrand()) / RAND_MAX * m_captchaImage.height());

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
    if (val < 0 or val > 5)
    {
        qInfo().noquote() << QString(__PRETTY_FUNCTION__) << "Min difficulty is 0, maximal is 5";
    }

    if (val < 1)
    {
        m_drawLines = false;
        m_drawEllipses = false;
        m_drawNoise = false;
        setSinDeform(10, 10, 5, 20);
    }
    else if (val == 1)
    {
        m_drawLines = true;
        m_lineWidth = 3;
        m_lineCount = 5;
        m_drawEllipses = false;
        m_drawNoise = false;
        setSinDeform(10, 15, 5, 20);
    }
    else if (val == 2)
    {
        m_drawLines = true;
        m_lineWidth = 2;
        m_lineCount = 5;
        m_drawEllipses = true;
        m_ellipseCount = 1;
        m_ellipseMinRadius = 20;
        m_ellipseMaxRadius = 40;
        m_drawNoise = false;
        setSinDeform(10, 15, 5, 15);
    }
    else if (val == 3)
    {
        m_drawLines = true;
        m_lineWidth = 2;
        m_lineCount = 3;
        m_drawEllipses = true;
        m_ellipseCount = 1;
        m_ellipseMinRadius = 20;
        m_ellipseMaxRadius = 50;
        m_drawNoise = true;
        m_noiseCount = 100;
        m_noisePointSize = 3;
        setSinDeform(8, 13, 5, 15);
    }
    else if (val == 4)
    {
        m_drawLines = true;
        m_lineWidth = 3;
        m_lineCount = 5;
        m_drawEllipses = true;
        m_ellipseCount = 2;
        m_ellipseMinRadius = 20;
        m_ellipseMaxRadius = 40;
        m_drawNoise = true;
        m_noiseCount = 100;
        m_noisePointSize = 3;
        setSinDeform(8, 13, 5, 15);
    }
    else
    {
        m_drawLines = true;
        m_lineWidth = 4;
        m_lineCount = 7;
        m_drawEllipses = true;
        m_ellipseCount = 1;
        m_ellipseMinRadius = 20;
        m_ellipseMaxRadius = 40;
        m_drawNoise = true;
        m_noiseCount = 200;
        m_noisePointSize = 3;
        setSinDeform(8, 10, 5, 10);
    }
}

void ZeroStorageCaptcha::generateText(int length)
{
    if (length <= 0)
    {
        qInfo() << QString(__PRETTY_FUNCTION__) << "Invalid number of characters. Set to 5.";
        length = 5;
    }

    m_captchaText = ZeroStorageCaptchaCrypto::random(length, m_onlyNumbers);

    updateCaptcha();
} 
