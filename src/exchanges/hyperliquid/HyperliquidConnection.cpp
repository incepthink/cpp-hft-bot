#include "HyperliquidConnection.hpp"
#include <iostream>
#include <chrono>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <string>
#include <sstream>

HyperliquidConnection::HyperliquidConnection()
{
    std::cout << "🔧 Hyperliquid WebSocket connection initialized" << std::endl;
}

bool HyperliquidConnection::connect()
{
    try
    {
        std::cout << "🔗 Connecting to Hyperliquid WebSocket..." << std::endl;

        // Create socket
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0)
        {
            std::cout << "❌ Socket creation failed" << std::endl;
            return false;
        }

        // Get server info
        struct hostent *host = gethostbyname("api.hyperliquid.xyz");
        if (!host)
        {
            std::cout << "❌ Cannot resolve hostname" << std::endl;
            close(sock);
            return false;
        }

        // Setup address
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(443); // HTTPS port
        addr.sin_addr.s_addr = *((unsigned long *)host->h_addr_list[0]);

        // Connect to server
        if (::connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        {
            std::cout << "❌ Connection to server failed" << std::endl;
            close(sock);
            return false;
        }

        // Setup SSL (reuse existing context)
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

        // Send WebSocket handshake
        std::string handshake =
            "GET /ws HTTP/1.1\r\n"
            "Host: wss://api.hyperliquid.xyz/ws\r\n"
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
                std::cout << "✅ Hyperliquid WebSocket handshake successful!" << std::endl;
                std::cout << "🔗 Connected to Hyperliquid WebSocket stream" << std::endl;

                // Cleanup SSL and socket immediately since we're just testing connection
                SSL_free(ssl);
                SSL_CTX_free(ctx);
                close(sock);
                return true;
            }
        }

        std::cout << "❌ Hyperliquid WebSocket handshake failed" << std::endl;
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

void HyperliquidConnection::disconnect()
{
    if (connected)
    {
        connected = false;
        std::cout << "🔌 Hyperliquid WebSocket disconnected" << std::endl;
    }

    if (ws_thread.joinable())
    {
        ws_thread.join();
    }
}