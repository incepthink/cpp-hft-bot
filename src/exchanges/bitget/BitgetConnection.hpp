#pragma once

#include <iostream>
#include <string>
#include <thread>
#include <atomic>

class BitgetConnection
{
private:
    std::thread ws_thread;
    std::atomic<bool> connected{false};

public:
    BitgetConnection();
    bool connect();
    void disconnect();
};