#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <random>

static const int kItemRepositorySize = 5; // Item buffer size
static const int kItemsToProduce = 1000;   // How many items we plan to produce

struct ItemRepository
{
    std::queue<int> item_buffer;
    std::mutex mtx; // ������, ������Ʒ������
    std::condition_variable repo_not_full; // ��������, ָʾ��Ʒ��������Ϊ��
    std::condition_variable repo_not_empty; // ��������, ָʾ��Ʒ��������Ϊ��
} gItemRepository;

void ProduceItem(ItemRepository *ir, int item)
{
    std::unique_lock<std::mutex> lock(ir->mtx);
    while(ir->item_buffer.size() >= kItemRepositorySize)
    {
        std::cout << "Producer is waiting for an empty slot...\n";
        ir->repo_not_full.wait(lock); // �����ߵȴ�"��Ʒ�ⲻΪ��"��һ��������
    }
    ir->item_buffer.push(item);
    std::cout << "Produce the " << item << "th item...\n";
    ir->repo_not_empty.notify_one(); // ֪ͨ�����߲�Ʒ�ⲻΪ��
}

void ConsumeItem(ItemRepository *ir)
{
    std::unique_lock<std::mutex> lock(ir->mtx);
    while(ir->item_buffer.empty())
    {
        std::cout << "Consumer is waiting for items...\n";
        ir->repo_not_empty.wait(lock); // �����ߵȴ�"��Ʒ�ⲻΪ��"��һ��������
    }
    std::cout << "Consume the " << ir->item_buffer.front() << "th item\n";
    ir->item_buffer.pop();
    ir->repo_not_full.notify_one(); // ֪ͨ�����߲�Ʒ�ⲻΪ��
}

void ProducerTask()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(100, 500);
    for (int i = 1; i <= kItemsToProduce; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(dis(gen)));
        ProduceItem(&gItemRepository, i);
    }
}

void ConsumerTask()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(100, 300);
    for (int i = 0; i < kItemsToProduce; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(dis(gen)));
        ConsumeItem(&gItemRepository);
    }
}

int main()
{
    std::thread producer(ProducerTask);
    std::thread consumer(ConsumerTask);
    producer.join();
    consumer.join();
}
