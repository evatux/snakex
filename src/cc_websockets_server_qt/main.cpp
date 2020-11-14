#include <QtCore/QCoreApplication>
#include <QtCore/QCommandLineParser>
#include <QtCore/QCommandLineOption>

#include "server.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("Snakex Qt server (using websockets)");
    parser.addHelpOption();

    parser.addOption({{"d", "debug"}, "Debug output [default: off]."});
    parser.addOption({{"p", "port"}, "Port for server [default: 1234].", "num", "1234"});
    parser.addOption({{"n", "nplayers"}, "Number of players [default: 1].", "num", "1"});
    parser.process(a);

    bool debug = parser.isSet("debug");
    int port = parser.value("port").toInt();
    int nplayers = parser.value("nplayers").toInt();

    Server *server = new Server(port, nplayers, debug);
    QObject::connect(server, &Server::closed, &a, &QCoreApplication::quit);

    return a.exec();
}
