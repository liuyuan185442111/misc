#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <random>

static const int kItemRepositorySize = 5; // Item buffer size
static const int kItemsToProducePerThread = 1000;
static const int M = 40; //生产线程数量
static const int N = 20; //消费线程数量

std::mt19937 gen;

struct ItemRepository
{
	int nextid = 1;
	std::queue<int> item_buffer;
	std::mutex mtx;
	std::condition_variable repo_not_full; // 条件变量, 指示产品缓冲区不为满
	std::condition_variable repo_not_empty; // 条件变量, 指示产品缓冲区不为空
} gItemRepository;

void ProduceItem(int taskid, ItemRepository *ir)
{
	std::unique_lock<std::mutex> lock(ir->mtx);
	while(ir->item_buffer.size() >= kItemRepositorySize)
	{
		std::cout << "Producer " << taskid << " is waiting for an empty slot...\n";
		ir->repo_not_full.wait(lock); // 生产者等待"产品库不为满"这一条件发生
	}
	std::cout << taskid << " produce the " << ir->nextid << "th item\n";
	ir->item_buffer.push(ir->nextid);
	++ir->nextid;
	ir->repo_not_empty.notify_all(); // 通知消费者产品库不为空
}

bool ConsumeItem(int taskid, ItemRepository *ir)
{
	std::unique_lock<std::mutex> lock(ir->mtx);
	while(ir->item_buffer.empty())
	{
		std::cout << "Consumer " << taskid << " is waiting for items...\n";
		// 消费者等待"产品库不为空"这一条件发生, 超时表示没有生产者了
		if(ir->repo_not_empty.wait_for(lock, std::chrono::seconds(2)) == std::cv_status::timeout)
			return false;
	}
	std::cout << taskid << " consume the " << ir->item_buffer.front() << "th item\n";
	ir->item_buffer.pop();
	ir->repo_not_full.notify_all(); // 通知生产者产品库不为满
	return true;
}

void ProducerTask(int id)
{
	std::uniform_int_distribution<> dis(10, 20);
	for(int i = 1; i <= kItemsToProducePerThread; ++i)
	{
		ProduceItem(id, &gItemRepository);
		std::this_thread::sleep_for(std::chrono::milliseconds(dis(gen)));
	}
	std::cout << "producer " << id << " exit...\n";
}

void ConsumerTask(int id)
{
	std::uniform_int_distribution<> dis(0, 10);
	while(true)
	{
		if(!ConsumeItem(id, &gItemRepository)) break;
		std::this_thread::sleep_for(std::chrono::milliseconds(dis(gen)));
	}
	std::cout << "consumer " << id << " exit...\n";
}

int main()
{
	std::random_device rd;
	gen = std::mt19937(rd());
	for(int i=1; i<=M; ++i)
		std::thread(ProducerTask, i).detach();
	std::thread consumers[N];
	for(int i=0; i<N; ++i)
		consumers[i] = std::thread(ConsumerTask, i+1);
	for(auto &th : consumers)
		th.join();
	return 0;
}
