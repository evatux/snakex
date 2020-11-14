#ifndef CLIENT_H
#define CLIENT_H

#include <memory>

#include <QtCore/QObject>
#include <QtWebSockets/QWebSocket>

#include "render.h"

class Client : public QObject {
    Q_OBJECT
public:
    explicit Client(const QUrl &url, bool debug = false, QObject *parent = nullptr);

Q_SIGNALS:
    void closed();

private Q_SLOTS:
    void onConnected();
    void receiveMessage(QString message);
    void sendMessage(QString message);
    void closeConnection();

private:
    QWebSocket m_webSocket;
    QUrl m_url;
    bool debug_;

    bool initReady_ = false;
    int id_ = -1;

    std::unique_ptr<Render> render_;
};

#endif // CLIENT_H
