#pragma once
#include "Events.hpp"
#include <libgraphics/ws/Connection.hpp>
#include <libgraphics/ws/wrappers/CreateWindow.hpp>
#include <wisterialib/Function.hpp>

class Connection : public WS::ServerConnection {
public:
    Connection(const Function<void(WS::WSProtocol&)>& callback)
    : WS::ServerConnection()
    , m_callback(callback)
    {
        EventLoop::the().register_fd_for_select([this]{
            recieve_message();
            auto recieved_messages = take_over_massages();
            for (auto& message : recieved_messages) {
                m_callback(message);
            }
        }, m_socket_fd);
    }

    private:
        Function<void(WS::WSProtocol&)> m_callback;
};