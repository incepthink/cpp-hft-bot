#pragma once

#include <iostream>
#include <string>
#include <thread>
#include <atomic>

class HyperliquidConnection
{
private:
    std::thread ws_thread;
    std::atomic<bool> connected{false};

public:
    HyperliquidConnection();
    bool connect();
    void disconnect();
};