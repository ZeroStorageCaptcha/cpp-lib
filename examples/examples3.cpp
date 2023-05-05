// GPLv3 (c) acetone, 2023
// Zero Storage Captcha example (caching)

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

    qInfo() << "Global parameters:";
    qInfo() << "Case sensitive:" << ZeroStorageCaptcha::caseSensitive();
    qInfo() << "Numbers mode:" << ZeroStorageCaptcha::numbersOnlyMode();
    qInfo() << "Cache capacity:" << ZeroStorageCaptcha::cacheMaxCapacity();
    qInfo() << "Cache size:" << ZeroStorageCaptcha::cacheSize();
    qInfo() << "Default difficulty:" << ZeroStorageCaptcha::defaultDifficulty();
    qInfo() << "Default answer length:" << ZeroStorageCaptcha::defaultAnswerLength();

    auto c = ZeroStorageCaptchaService::Cache::get();
    QFile pic("c.png");
    if (not pic.open(QIODevice::WriteOnly)) return 1;
    pic.write(c->picturePng());
    pic.close();

    qInfo() << "";
    qInfo() << "Captcha generated and saved to c.png";
    qInfo() << "Cache size after generation:" << ZeroStorageCaptcha::cacheSize();
    qInfo() << "";

    qInfo() << "Token:" << c->token();
    qInfo() << "Answer:" << c->answer();
    qInfo() << "First validation:" << ZeroStorageCaptcha::validate (c->answer(), c->token()); // success
    qInfo() << "Second validation:" << ZeroStorageCaptcha::validate (c->answer(), c->token()); // failed

    qInfo() << "";
    qInfo() << "Captcha removed from cache at first successful validation";
    qInfo() << "Current cache size:" << ZeroStorageCaptcha::cacheSize();
    qInfo() << "";
    qInfo() << "Cached captcha will be reused after <=3 minutes, when is not answered and its token expires";

    return a.exec();
}
