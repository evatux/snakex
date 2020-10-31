#include "QtWebSockets/qwebsocketserver.h"
#include "QtWebSockets/qwebsocket.h"
#include <QtCore/QDebug>

#include "cc_core/game.hpp"
#include "cc_core/proto.hpp"

#include "server.h"

QT_USE_NAMESPACE

Server::Server(quint16 port, bool debug, QObject *parent) :
    QObject(parent),
    m_pWebSocketServer(new QWebSocketServer(QStringLiteral(" Server"), QWebSocketServer::NonSecureMode, this)),
    m_debug(debug)
{
    if (m_pWebSocketServer->listen(QHostAddress::Any, port)) {
        if (m_debug) qDebug() << "server listening on port" << port;
        connect(m_pWebSocketServer, &QWebSocketServer::newConnection,
                this, &Server::onNewConnection);
        connect(m_pWebSocketServer, &QWebSocketServer::closed, this, &Server::closed);
    }
}

Server::~Server() {
    m_pWebSocketServer->close();
    qDeleteAll(m_clients.begin(), m_clients.end());
}

void Server::onNewConnection() {
    if (m_debug) qDebug() << "new connection";
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();
    GameServer *gameServer = new GameServer(pSocket);
    m_clients << gameServer;
}

#if 0
void Server::socketDisconnected() {
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (m_debug) qDebug() << "socketDisconnected:" << pClient;

    int i = 0;
    for (; i < m_clients.size(); ++i)
        if (m_clients[i]->socket() == pClient)
            break;

    if (i == m_clients.size()) {
        qDebug() << "no client found?";
        return;
    }
    m_clients[i].socketDisconnected();
    m_clients.removeAt(i);
}
#endif
