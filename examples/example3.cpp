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

    ZeroStorageCaptcha::setOnlyNumbersMode(true);
    auto captcha = ZeroStorageCaptcha::getCaptcha();
    qInfo() << captcha.token() << captcha.answer();
    QFile f("captcha.png");
    if (not f.open(QIODevice::WriteOnly))
    {
        return 1;
    }
    f.write(captcha.picture());
    f.close();
    qInfo() << "Validation: " << ZeroStorageCaptcha::validate(captcha.answer(), captcha.token());

    return a.exec();
}
