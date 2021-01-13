#include <iostream>
#include "storage.h"
#include <signal.h>
using namespace std;
using namespace lcore;

static bool stoped = false;

static void sigusr1_handler(int signum)
{
	puts("shutdown db by SIGUSR1");
	stoped = true;
}

static bool add_signal()
{
	struct sigaction act;
	act.sa_handler = sigusr1_handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	return sigaction(SIGUSR1, &act, NULL) == 0;
}

int main(int argc, char **argv)
{
	if(!Storage::open("game.conf")) return 1;
	if(!add_signal()) return 2;
	pthread_t th;
	pthread_create(&th, NULL, &Storage::BackupThread, NULL);

	for(int i=0; !stoped ;++i)
	{
		Transaction txn;
		Table *user = Storage::get_table("user", txn);
		user->insert((OctetsStream() << i), (OctetsStream() << "value"), txn, 0);
	}

	while(!stoped)
		sleep(1);

	Storage::checkpoint();
	Storage::close();

	return 0;
}
