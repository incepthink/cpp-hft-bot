#include "exchanges/binance/BinanceConnection.hpp"
#include "exchanges/bitget/BitgetConnection.hpp"
#include <iostream>

int main()
{
    std::cout << "🤖 Starting Arbitrage Bot..." << std::endl;

    BinanceConnection binance;
    BitgetConnection bitget;

    std::cout << "🔗 Testing connections to both exchanges..." << std::endl;

    // Test Binance connection
    bool binance_ok = binance.connect();
    if (binance_ok)
    {
        std::cout << "✅ Binance connection successful!" << std::endl;
    }
    else
    {
        std::cout << "❌ Binance connection failed!" << std::endl;
    }

    // Test Bitget connection
    bool bitget_ok = bitget.connect();
    if (bitget_ok)
    {
        std::cout << "✅ Bitget connection successful!" << std::endl;
    }
    else
    {
        std::cout << "❌ Bitget connection failed!" << std::endl;
    }

    if (binance_ok && bitget_ok)
    {
        std::cout << "🚀 Both exchanges connected! Ready for arbitrage..." << std::endl;
        std::cout << "📊 Binance + Bitget arbitrage opportunities available" << std::endl;
    }
    else
    {
        std::cout << "⚠️  Some connections failed. Check network and try again." << std::endl;
    }

    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();

    binance.disconnect();
    bitget.disconnect();
    return 0;
}