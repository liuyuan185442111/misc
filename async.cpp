#include <iostream>
#include <future>
#include <chrono>

bool is_prime(int x)
{
    for (int i = 2; i < x; ++i)
        if (x % i == 0)
            return false;
    return true;
}

int main()
{
    std::future<bool> fut = std::async(std::launch::deferred, is_prime, 1610612741);
    std::cout << "checking, please wait\n";
    /** fut.wait_for()应返回std::future_status::deferred,
    但gcc version 4.8.2却返回std::future_status::timeout */
    while(fut.wait_for(std::chrono::milliseconds(100)) == std::future_status::timeout)
    {
        std::cout << '.';
        fflush(stdout);
    }
    std::cout << "return of wait_for\n";
    bool x = fut.get();
    std::cout << (x ? "is" : "is not") << " prime.\n";

    return 0;
}
