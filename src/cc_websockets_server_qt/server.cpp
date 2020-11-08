#include "QtWebSockets/qwebsocketserver.h"
#include "QtWebSockets/qwebsocket.h"
#include <QtCore/QDebug>

#include "cc_core/game.hpp"
#include "cc_core/proto.hpp"

#include "server.h"

QT_USE_NAMESPACE

Server::Server(quint16 port, int nplayers, bool debug, QObject *parent)
    : QObject(parent)
    , pWebSocketServer_(new QWebSocketServer(QStringLiteral("Server"), QWebSocketServer::NonSecureMode, this))
    , nplayers_(nplayers)
    , debug_(debug) {
    if (pWebSocketServer_->listen(QHostAddress::Any, port)) {
        if (debug_) qDebug() << "server listening on port " << port;
        connect(pWebSocketServer_, &QWebSocketServer::newConnection, this, &Server::onNewConnection);
        connect(pWebSocketServer_, &QWebSocketServer::closed, this, &Server::closed);
    }
}

Server::~Server() {
    pWebSocketServer_->close();
    qDeleteAll(clientList_.begin(), clientList_.end());
}

void Server::onNewConnection() {
    if (debug_) qDebug() << "new connection";
    QWebSocket *pSocket = pWebSocketServer_->nextPendingConnection();
    clientList_ << pSocket;

    if (clientList_.size() == nplayers_) {
        GameServer *gameServer = new GameServer(clientList_);
        clientList_.clear();
    }
}
