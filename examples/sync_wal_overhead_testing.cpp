#include <iostream>
#include <fstream>

#include "rocksdb/db.h"
#include "rocksdb/utilities/backupable_db.h"

#include <vector>

#include <ctime>
#include <thread>
#include <cstdlib>
#include <string>


using namespace rocksdb;
using namespace std;

static const char alphanum[] =
"0123456789"
"!@#$%^&*"
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz";

int stringLength = sizeof(alphanum) - 1;

bool take_backup(int timeToSleep) {
    std::this_thread::sleep_for(std::chrono::seconds(timeToSleep));
    return true;
}

void dynamic_backup(DB *db, int timeToWait) {

	// XXX: ToDo:: Read a file to maintain the version numbers 
    Status s;
    std::time_t result;

    BackupEngine* backup_engine;
    s = BackupEngine::Open(Env::Default(), BackupableDBOptions("/tmp/own_backup_8"), &backup_engine);
    assert(s.ok());

    while (1) {
        while (take_backup(timeToWait) == false);

        result = std::time(nullptr);
        cout << "Started taking backup at : ";
        cout << std::asctime(std::localtime(&result));
        cout << "\n";
        s = backup_engine->CreateNewBackup(db, true /* with flush*/);
        assert(s.ok());

        result = std::time(nullptr);
        cout << "Taking backup finished at : ";
        cout << std::asctime(std::localtime(&result));
        cout << "\n";
    }
}


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


int main() {
    Options options;
    options.create_if_missing = true;
    DB* db;
    Status s = DB::Open(options, "/tmp/testing_db_1", &db);
    assert(s.ok());
    long count = 0;
    string key;
    string value;
    rocksdb::WriteOptions write_options;
    write_options.sync = true;
    std::time_t result;

    while (1) {
   
       cout << "Number of insert operations: ";
       int num, size;
       scanf("%d", &num);
       cout << "Size of object";
       scanf("%d", &size);

       result = std::time(nullptr);
       cout << "Started performing operations at : ";
       cout << std::asctime(std::localtime(&result));
       cout << "\n";

       for (int i = 0; i < num; i++) {
           key = "key" + std::to_string(count);
           value = generate_string(size - 6);
           db->Put(write_options, key, value);
           count++;
       }

       result = std::time(nullptr);
       cout << "Finished operations at : ";
       cout << std::asctime(std::localtime(&result));
       cout << "\n";
    }

}
