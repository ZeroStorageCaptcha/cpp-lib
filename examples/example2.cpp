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
    captcha.updateCaptcha();
    qInfo() << captcha.token();
    qInfo() << captcha.answer();
    qInfo() << captcha.picture().toBase64();
    qInfo() << "Validation 1:" << ZeroStorageCaptcha::validate(captcha.answer(), captcha.token());
    qInfo() << "Validation 2:" << ZeroStorageCaptcha::validate(captcha.answer(), captcha.token());

    return a.exec();
}
