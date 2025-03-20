#include "passwordmanager.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QTextStream>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/Key.ico"));

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOption(QCommandLineOption(QStringList() << "c" << "cli", "Run in CLI mode"));
    parser.process(app);

    QByteArray encryptionKey = QByteArrayLiteral("sdajywfgya215895@€$]£{|.-");

    if (parser.isSet("cli")) {
        PasswordManager pm(nullptr, encryptionKey, true);
        pm.cliMode();
        return 0;
    }

    PasswordManager w(nullptr, encryptionKey);
    w.setWindowTitle("CipherBox");
    w.resize(400, 350);
    w.show();

    return app.exec();
}


