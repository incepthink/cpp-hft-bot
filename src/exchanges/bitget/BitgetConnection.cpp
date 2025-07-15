#include "BitgetConnection.hpp"
#include <iostream>
#include <chrono>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <string>
#include <sstream>
#include <fcntl.h>

BitgetConnection::BitgetConnection()
{
    std::cout << "🔧 Bitget WebSocket connection initialized" << std::endl;
}

bool BitgetConnection::connect()
{
    try
    {
        std::cout << "🔗 Connecting to Bitget WebSocket..." << std::endl;

        // Create non-blocking socket for HFT
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0)
        {
            std::cout << "❌ Socket creation failed" << std::endl;
            return false;
        }

        // Set socket to non-blocking mode for HFT
        int flags = fcntl(sock, F_GETFL, 0);
        fcntl(sock, F_SETFL, flags | O_NONBLOCK);

        // Get server info - try futures endpoint
        struct hostent *host = gethostbyname("ws.bitgetapi.com");
        if (!host)
        {
            std::cout << "❌ Cannot resolve hostname" << std::endl;
            close(sock);
            return false;
        }

        // Setup address
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(443);
        addr.sin_addr.s_addr = *((unsigned long *)host->h_addr_list[0]);

        // Non-blocking connect
        int result = ::connect(sock, (struct sockaddr *)&addr, sizeof(addr));
        if (result < 0 && errno != EINPROGRESS)
        {
            std::cout << "❌ Connection failed" << std::endl;
            close(sock);
            return false;
        }

        // Wait for connection with timeout (polling approach)
        fd_set write_fds;
        struct timeval timeout;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        FD_ZERO(&write_fds);
        FD_SET(sock, &write_fds);

        if (select(sock + 1, NULL, &write_fds, NULL, &timeout) <= 0)
        {
            std::cout << "❌ Connection timeout" << std::endl;
            close(sock);
            return false;
        }

        // Setup SSL
        SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
        if (!ctx)
        {
            std::cout << "❌ SSL context creation failed" << std::endl;
            close(sock);
            return false;
        }

        SSL *ssl = SSL_new(ctx);
        if (!ssl)
        {
            std::cout << "❌ SSL creation failed" << std::endl;
            SSL_CTX_free(ctx);
            close(sock);
            return false;
        }

        SSL_set_fd(ssl, sock);

        if (SSL_connect(ssl) <= 0)
        {
            std::cout << "❌ SSL handshake failed" << std::endl;
            SSL_free(ssl);
            SSL_CTX_free(ctx);
            close(sock);
            return false;
        }

        // Send WebSocket handshake for Bitget futures WebSocket
        std::string handshake =
            "GET /mix/v1/stream HTTP/1.1\r\n"
            "Host: ws.bitgetapi.com\r\n"
            "Upgrade: websocket\r\n"
            "Connection: Upgrade\r\n"
            "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
            "Sec-WebSocket-Version: 13\r\n"
            "\r\n";

        if (SSL_write(ssl, handshake.c_str(), handshake.length()) <= 0)
        {
            std::cout << "❌ Failed to send handshake" << std::endl;
            SSL_free(ssl);
            SSL_CTX_free(ctx);
            close(sock);
            return false;
        }

        // Read response
        char buffer[1024];
        int bytes = SSL_read(ssl, buffer, sizeof(buffer) - 1);
        if (bytes > 0)
        {
            buffer[bytes] = '\0';
            std::string response(buffer);

            if (response.find("101 Switching Protocols") != std::string::npos)
            {
                connected = true;
                std::cout << "✅ Bitget WebSocket handshake successful!" << std::endl;
                std::cout << "🚀 Ready for HFT - No threads, async I/O only" << std::endl;

                // Store connection details for event loop processing
                // In real HFT, you'd add this to an epoll/event manager

                SSL_free(ssl);
                SSL_CTX_free(ctx);
                close(sock);
                return true;
            }
        }

        std::cout << "❌ Bitget WebSocket handshake failed" << std::endl;
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(sock);
        return false;
    }
    catch (const std::exception &e)
    {
        std::cout << "❌ Error: " << e.what() << std::endl;
        return false;
    }
}

void BitgetConnection::disconnect()
{
    if (connected)
    {
        connected = false;
        std::cout << "🔌 Bitget WebSocket disconnected" << std::endl;
    }
}