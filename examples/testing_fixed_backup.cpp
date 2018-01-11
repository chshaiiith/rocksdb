#include <iostream>
#include <fstream>

#include "rocksdb/db.h"
#include "rocksdb/utilities/backupable_db.h"

#include <vector>

#include <ctime>
#include <thread>
#include <cstdlib>
#include <string>

#include <map>
#include <iostream>
#include <cassert>
#include <boost/foreach.hpp> 
#include <boost/filesystem.hpp>
#include <time.h>

#include "workload.h"
#include "RequestType.h"
#include <assert.h>
#include "DB_Instance.h"
 

using namespace rocksdb;
using namespace std;

namespace fs = boost::filesystem; 


static const char alphanum[] =
"0123456789"
"!@#$%^&*"
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz";

// int stringLength = sizeof(alphanum) - 1;

// Map to maintain the log files
std::map<std::string, bool> logMap;

#define DTTMFMT "%Y-%m-%d %H:%M:%S "
#define DTTMSZ 21

static char *getDtTm (char *buff) {
    time_t t = time (0);
    strftime (buff, DTTMSZ, DTTMFMT, localtime (&t));
    return buff;
}

// XXX: TODO: DO it based on yuquan's modeling
bool take_backup(int timeToSleep) {
    std::this_thread::sleep_for(std::chrono::seconds(timeToSleep));
    return true;
}

// META DATA file craeted for system
std::ofstream outfile;


void dynamic_backup(DB *db, int timeToWait) {

	// XXX: ToDo:: Read a file to maintain the version numbers 
    Status s;
    std::time_t result;

    BackupEngine* backup_engine;
    s = BackupEngine::Open(Env::Default(), BackupableDBOptions("/tmp/own_backup_8"), &backup_engine);
    assert(s.ok());

    string startTime;
    int backupNumber;
    string logFileString;
    map<string,bool>::const_iterator it;    
    string fileName;

    while (1) {
        while (take_backup(timeToWait) == false);

        result = std::time(nullptr);
        cout << "Started taking backup at : ";
		startTime = std::asctime(std::localtime(&result));
	
		char buff[DTTMSZ]; 
		outfile.open("META", std::ios_base::app);
        outfile << getDtTm (buff); 
    	outfile.close();
        
		cout << std::asctime(std::localtime(&result));
        cout << "\n";
        s = backup_engine->CreateNewBackup(db, true /* with flush*/);
        assert(s.ok());

        result = std::time(nullptr);
        cout << "Taking backup finished at : ";
        cout << std::asctime(std::localtime(&result));

		
        cout << "\n";
	
        // write in file backup number and time to take that backup
		string path = "/tmp/chetan/archive";
		if (boost::filesystem::is_directory(path) == false) {
			std::cout << "Log directory path is wrong";
			return;
		}

		std::ifstream current("CURRENT");

		current >> backupNumber;
        backupNumber ++;
	
		current.close();

		std::ofstream current1("CURRENT");
		current1 << backupNumber;

		current.close();
		logFileString = "";
    	for (auto & p : fs::directory_iterator(path))
        {  
			fileName = p.path().filename().string();
			it = logMap.find(fileName);
			if (it != logMap.end()) {
				continue;
			}
			logMap[fileName] = true;
                     
			cout << "FileName" << fileName << "\n";
			logFileString = logFileString + fileName + ":";
			cout << logFileString << "\n";
        }

		outfile.open("META", std::ios_base::app);
        outfile << " " + std::to_string(backupNumber) + " " + logFileString + "\n"; 
    	outfile.close();
     }

}

/****
char genRandom()  // Random string generator function.
{   
    return alphanum[rand() % stringLength];
}


string generate_string(int byte_size) {
    std::string Str = "";
    for (int i = 0; i < byte_size; i++) {
        Str += genRandom();
    }
    return Str;
}
****/

	
DB_Instance::DB_Instance(bool random) {
		
	// Options configurations
	options.create_if_missing = true;
	options.wal_dir = "/tmp/chetan/";
	options.WAL_ttl_seconds = 24 * 60 * 60;
	options.WAL_size_limit_MB = 4000;

	// Mention the db folder in which data is stored
	Status s = DB::Open(options, "/tmp/testing_db_2", &db);
	assert(s.ok());

	// Specifying the write option	
	write_options.sync = true;

	// Mentiones the time to take backup : UserInput
	cout << "Time to wait before backup: ";
	int timeTowait;
	scanf("%d", &timeTowait);
	
	std::thread t1(dynamic_backup, db, timeTowait);

}


void DB_Instance::db_get (string key, string *value) {
 
	// Started the operations now
	std::time_t result;
	result = std::time(nullptr);
	cout << "Started performing operations at : ";
	cout << std::asctime(std::localtime(&result));
	cout << "\n";

	Status s = db->Get(rocksdb::ReadOptions(), key, value);

	result = std::time(nullptr);
	cout << "Finished operations at : ";
	cout << std::asctime(std::localtime(&result));
	cout << "\n";
}

void DB_Instance::db_put(string key, string value) {
		
	// Started the operations now
	std::time_t result;
	result = std::time(nullptr);
	cout << "Started performing operations at : ";
	cout << std::asctime(std::localtime(&result));
	cout << "\n";

	Status s = db->Put(write_options, key, value);

	result = std::time(nullptr);
	cout << "Finished operations at : ";
	cout << std::asctime(std::localtime(&result));
	cout << "\n";

}
