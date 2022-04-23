// 2022 (c) GPLv3, acetone at i2pmail.org
// Zero Storage Captcha example

#include "zerostoragecaptcha.h"

#include <QApplication>
#include <QFile>
#include <QDebug>

int main(int argc, char *argv[])
{
    // To start QApplication without X-server (non-GUI system) should use:
    // "export QT_QPA_PLATFORM=offscreen" in plain shell
    // or
    // "Environment=QT_QPA_PLATFORM=offscreen" in systemd service ([Service] section)
    QApplication a(argc, argv);

    ZeroStorageCaptcha captcha;
    captcha.generateText();
    qInfo() << captcha.captchaToken();
    qInfo() << captcha.captchaText();
    qInfo() << captcha.captchaPngByteArray().toBase64();
    qInfo() << "Validation:" << ZeroStorageCaptcha::validate(captcha.captchaText(), captcha.captchaToken());

    return a.exec();
}
