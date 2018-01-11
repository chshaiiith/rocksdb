#ifndef DB_INSTANCE_PROCESS_H
#define DB_INSTANCE_PROCESS_H

#include <cstdint>
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
#include <boost/thread.hpp>

using namespace rocksdb;
using namespace std;


class DB_Instance
{
public:
    DB_Instance(bool random);
	Options options;
	DB* db;

	rocksdb::WriteOptions write_options;

    // Returns next type
    void db_get(string key, string *value);
	void db_put(string key, string value);
};

#endif
