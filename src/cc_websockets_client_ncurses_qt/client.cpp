#include "client.h"
#include <QtCore/QDebug>

QT_USE_NAMESPACE

Client::Client(const QUrl &url, bool debug, QObject *parent) :
    QObject(parent),
    m_url(url),
    m_debug(debug)
{
    if (m_debug) qDebug() << "WebSocket server:" << url;
    connect(&m_webSocket, &QWebSocket::connected, this, &Client::onConnected);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &Client::closed);
    m_webSocket.open(QUrl(url));
}

void Client::onConnected() {
    if (m_debug) qDebug() << "WebSocket connected";

    render.reset(new Render());
    connect(&m_webSocket, &QWebSocket::textMessageReceived,
            render.get(), &Render::receiveMessage);
    connect(render.get(), &Render::messageSent,
            this, &Client::sendMessage);
    connect(render.get(), &Render::gameFinished,
            this, &Client::closeConnection);
}

void Client::sendMessage(QString message) {
    if (m_debug) qDebug() << "Message received:" << message;
    m_webSocket.sendTextMessage(message);
}

void Client::closeConnection() {
    m_webSocket.close();
    render.reset();
}
