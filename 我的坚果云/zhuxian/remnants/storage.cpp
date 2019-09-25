#include "storage.h"
namespace WDB
{
WDB::StorageEnv::StorageMap WDB::StorageEnv::storage_map;
WDB::StorageEnv::StorageVec WDB::StorageEnv::storage_vec;
GNET::DBCollection *WDB::StorageEnv::env;
pthread_key_t WDB::StorageEnv::ThreadContext::key;
std::string WDB::StorageEnv::homedir;
std::string WDB::StorageEnv::datadir;
std::string WDB::StorageEnv::logdir;
GNET::Thread::Mutex WDB::StorageEnv::checkpoint_locker;
};

#include <map>
#include <string>
#include <vector>
#include <iostream>
using namespace std;
using namespace WDB;
using namespace GNET;
int main()
{
	Conf::load("test.conf");
	if(!StorageEnv::Open())
	{
		cerr << "Initialize storage environment failed." << endl;
		exit(-1);
	}
	/*
	pthread_t th;  
	pthread_create(&th, NULL, &StorageEnv::BackupThread, NULL);
	StorageEnv::backup("backup");
	sleep(3600);
	*/
	{
		StorageEnv::Storage *puser = StorageEnv::GetStorage("user");
		StorageEnv::AtomTransaction txn;
		string str;
		for(int i=0;i<199998;++i) str+="0123456789";
		Octets key(str.c_str(), str.length()), val;
		if(puser->find(key, val, txn))
			cout << "find " << string((const char*)val.begin(),val.size()) << endl;
		puser->insert(key, Octets("xuanya3"), txn);
	}
	StorageEnv::checkpoint();
	StorageEnv::Close();
	cout << "sizeof PageLayout is " << sizeof(GNET::__db_helper::PageLayout) << endl;
	return 0;
}
