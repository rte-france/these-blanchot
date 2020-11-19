#include "catch2.hpp"
#include <iostream>
#include <chrono>
#include <thread>

#include "Timer.h"

TEST_CASE("Test Timer.h"){
    std::cout << "Hello waiter" << std::endl;
    std::chrono::seconds dura( 10);
    std::this_thread::sleep_for( dura );
    std::cout << "Waited 10s\n";
}