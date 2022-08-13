// GPLv3 (c) acetone, 2022
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

    ZeroStorageCaptcha c("myText");
    QFile pic("c.png");
    if (not pic.open(QIODevice::WriteOnly)) return 1;
    pic.write(c.picturePng());
    pic.close();

    qInfo() << c.token() << c.answer();
    qInfo() << "Validate" << ZeroStorageCaptcha::validate (c.answer(), c.token());
    qInfo() << "Validate" << ZeroStorageCaptcha::validate (c.answer(), c.token());

    return a.exec();
}
