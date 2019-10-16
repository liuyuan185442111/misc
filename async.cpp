#include <iostream>
#include <future>
#include <chrono>

bool is_prime(int x)
{
    std::cout << "task thread id is " << std::this_thread::get_id() << std::endl;
    for (int i = 2; i < x; ++i)
        if (x % i == 0)
            return false;
    return true;
}

int main()
{
    std::cout << "main thread id is " << std::this_thread::get_id() << std::endl;
    // call function asynchronously:
    //std::future<bool> fut = std::async(std::launch::async, is_prime, 1610612741);
    //std::future<bool> fut = std::async(std::launch::deferred, is_prime, 1610612741);
    std::future<bool> fut = std::async(is_prime, 1610612741);
    /**async����������ͬ�ڴ�std::launch::async|std::launch::deferred
    std::launch::deferredʵ��һ���߳�
    */

    // do something while waiting for function to set future:
    std::cout << "checking, please wait\n";
    std::chrono::milliseconds span(100);
    /** ��ָ��std::launch::deferred, fut.wait_for(span)Ӧ����std::future_status::deferred,
    ��gcc version 4.8.2ȴ����std::future_status::timeout */
    while(fut.wait_for(span) == std::future_status::timeout)
        std::cout << '.';

    std::cout << std::endl;
    bool x = fut.get(); // retrieve return value
    std::cout << (x ? "is" : "is not") << " prime.\n";

    return 0;
}
