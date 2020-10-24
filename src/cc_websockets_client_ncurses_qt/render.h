#ifndef RENDER_H
#define RENDER_H

#include <QtCore/QObject>
#include <QtCore/QThread>

#include <mutex>
#include <optional>
#include <queue>

#include "cc_core/proto.hpp"

namespace detail {
template<typename T> class my_queue {
public:
    void push(const T& value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(value);
    }

    std::optional<T> pop() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty()) return std::nullopt;
        T t{m_queue.back()};
        m_queue.pop();
        return t;
    }

private:
    std::queue<T> m_queue;
    mutable std::mutex m_mutex;
};
}
using message_queue_t = detail::my_queue<proto::message_t>;

class Render : public QThread {
    Q_OBJECT
public:
    explicit Render(int id);

    void run() override;
    bool isReady() const;
    void stopRendering();
    void receiveMessage(const proto::message_t &message);

Q_SIGNALS:
    void messageSent(QString qstr);
    void gameFinished();

private:
    int id_ = -1;

    void init();

    message_queue_t message_queue_;

    void processWaitingMessages();
};

#endif // RENDER_H
