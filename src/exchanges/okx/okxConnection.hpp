#pragma once

#include <iostream>
#include <string>
#include <thread>
#include <atomic>

class OKXConnection
{
private:
    std::thread ws_thread;
    std::atomic<bool> connected{false};

public:
    OKXConnection();
    bool connect();
    void disconnect();
};