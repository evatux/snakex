#ifndef CLIENT_H
#define CLIENT_H

#include <memory>

#include <QtCore/QObject>
#include <QtWebSockets/QWebSocket>

#include "render.h"

class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(const QUrl &url, bool debug = false, QObject *parent = nullptr);

Q_SIGNALS:
    void closed();

private Q_SLOTS:
    void onConnected();
    void sendMessage(QString message);
    void closeConnection();

private:
    QWebSocket m_webSocket;
    QUrl m_url;
    bool m_debug;

    std::unique_ptr<Render> render_;
};

#endif // CLIENT_H
