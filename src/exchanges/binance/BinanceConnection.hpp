#pragma once

#include <iostream>
#include <string>
#include <thread>
#include <atomic>

class BinanceConnection
{
private:
    std::thread ws_thread;
    std::atomic<bool> connected{false};

public:
    BinanceConnection();
    bool connect();
    void disconnect();
};