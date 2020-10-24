#include "client.h"

#include <string>

#include <QtCore/QDebug>

#include "cc_core/proto.hpp"

QT_USE_NAMESPACE

Client::Client(const QUrl &url, bool debug, QObject *parent)
    : QObject(parent)
    , m_url(url)
    , debug_(debug) {
    if (debug_) qDebug() << "WebSocket server:" << url;
    connect(&m_webSocket, &QWebSocket::connected, this, &Client::onConnected);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &Client::closeConnection);
    m_webSocket.open(QUrl(url));
}

void Client::onConnected() {
    if (debug_) qDebug() << "WebSocket connected";

    connect(&m_webSocket, &QWebSocket::textMessageReceived,
            this, &Client::receiveMessage);
}

void Client::receiveMessage(QString qstr) {
    std::string str = qstr.toStdString();
    auto message = proto::message_from_string(str);

    if (!initReady_) {
        assert(message.size() == 2);
        id_ = std::get<proto::id_t>(message[0]).id;
        // FIXME: check window size
        sendMessage(QString(proto::to_string(proto::message_t{proto::id_t{id_}}).c_str()));

        render_.reset(new Render(id_));
        connect(render_.get(), &Render::messageSent, this, &Client::sendMessage);
        connect(render_.get(), &Render::gameFinished, this, &Client::closeConnection);

        initReady_ = true;
        return;
    }

    if (render_->isFinished()) {
        qDebug() << "EEE receiveMessage: FIXME: render_->isFinished()";
        return; // FIXME: process
    }

    if (!render_->isRunning()) render_->start();
    while (!render_->isReady()); // FIXME: remove busy wait
    render_->receiveMessage(message);
}

void Client::sendMessage(QString qstr) {
    if (debug_) qDebug() << "Message received:" << qstr;
    if (m_webSocket.isValid()) m_webSocket.sendTextMessage(qstr);
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
