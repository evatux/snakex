#include <QtCore/QCoreApplication>
#include <QtCore/QCommandLineParser>
#include <QtCore/QCommandLineOption>

#include "client.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("Snakex Qt client (using websockets)");
    parser.addPositionalArgument("address", "Server address. Default localhost:1234");
    parser.addOption({{"d", "debug"}, "Debug output [default: off]."});
    parser.addHelpOption();
    parser.process(a);

    const QStringList args = parser.positionalArguments();
    QString address = args.isEmpty() ? QString("localhost:1234") : args.first();
    if (!address.contains(':')) address += ":1234";
    if (!address.startsWith("ws://")) address = QString("ws://") + address;

    const bool debug = parser.isSet("debug");

    Client client(QUrl(address), debug);
    QObject::connect(&client, &Client::closed, &a, &QCoreApplication::quit);

    return a.exec();
}
