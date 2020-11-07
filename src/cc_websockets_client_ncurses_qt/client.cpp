#include "client.h"
#include <QtCore/QDebug>

QT_USE_NAMESPACE

Client::Client(const QUrl &url, bool debug, QObject *parent)
    : QObject(parent)
    , m_url(url)
    , m_debug(debug) {
    if (m_debug) qDebug() << "WebSocket server:" << url;
    connect(&m_webSocket, &QWebSocket::connected, this, &Client::onConnected);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &Client::closeConnection);
    m_webSocket.open(QUrl(url));
}

void Client::onConnected() {
    if (m_debug) qDebug() << "WebSocket connected";

    render_.reset(new Render(0));
    connect(&m_webSocket, &QWebSocket::textMessageReceived, render_.get(), &Render::receiveMessage);
    connect(render_.get(), &Render::messageSent, this, &Client::sendMessage);
    connect(render_.get(), &Render::gameFinished, this, &Client::closeConnection);

    render_->start();
}

void Client::sendMessage(QString message) {
    if (m_debug) qDebug() << "Message received:" << message;
    if (m_webSocket.isValid()) m_webSocket.sendTextMessage(message);
}

void Client::closeConnection() {
    if (render_) {
        render_->stopRendering();
        render_->wait();
    }
    m_webSocket.close();
    render_.reset();
    emit closed();
}
