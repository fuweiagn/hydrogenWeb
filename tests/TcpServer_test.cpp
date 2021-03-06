/*
 * File: TcpServer_test.cpp
 * ------------------------
 * @author: Fu Wei
 *
 */

#include "network/EventLoop.hpp"
#include "network/TcpConnection.hpp"
#include "network/TcpServer.hpp"
#include "network/InetAddr.hpp"
#include "core/HyBuffer.hpp"

#include <unistd.h>

int count = 0;
EventLoop* g_loop;

typedef std::shared_ptr<TcpConnection> pTcpConnection;

void onConnection(const pTcpConnection& conn)
{
    if (conn->connected())
    {
        printf("onConnection(): new connection [%s] from %s\n",
               conn->name().c_str(),
               conn->clientAddr().toString().c_str());
        if (++count > 5)
        {
            g_loop->quit();
        }
    }
    else
    {
        printf("onConnection(): connection [%s] is down\n",
               conn->name().c_str());
    }
}

void onMessage(const pTcpConnection& conn,
               HyBuffer* buffer,
               Timestamp receiveTime)
{
    printf("onMessage(): received %zd bytes from connection [%s] at %s\n",
           buffer->readableBytes(),
           conn->name().c_str(),
           receiveTime.toString().c_str());
    conn->send(buffer->readAsString());
}

int main(int argc, char* argv[])
{
    printf("in main(): pid = %d\n", getpid());

    InetAddr listenAddr(23456);
    EventLoop loop;
    g_loop = &loop;
    TcpServer server(&loop, listenAddr, 4);
    server.setConnectionCallback(onConnection);
    server.setMessageCallback(onMessage);
    server.run();
    loop.loop();

    return 0;
}
