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
#include "DB_Instance.h"

using namespace rocksdb;
using namespace std;

namespace fs = boost::filesystem; 


static const char alphanum[] =
"0123456789"
"!@#$%^&*"
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz";

int stringLength = sizeof(alphanum) - 1;

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


// GET RANDOM KEY
string get_random_key(long count) {
	srand (time(NULL));
	int random = rand() % count;
	return "key" + random;
}


int main() {
	// Few variable intialization
    long count = 0;
    string key;
    string value;
    std::time_t result;
	double rate;
	int type;
    double get, put, insert;
	int req_type;
	double req_wait_time;

	DB_Instance _dbInstance(true);

    while (1) {
        // Specify the arrival rate
		cout << "Arrival Rate for the backup: ";       
		scanf("%lf", &rate);

		// Specify the type
		cout << "Specify the type of workload ( 1 for det / 2 for exp): ";
		scanf("%d", &type);

		// Intializing the workload
		Workload _workload(rate, type);

		// Type of operations
		cout << "Specify the percentage of GET PUT INSERT operations: ";
		scanf("%lf %lf %lf", &get, &put, &insert);

		assert(get+put+insert == 1.0);

		RequestType _requestType(get, put, insert);

		int num, size;
		//Object size
		cout << "Size of object: ";
		scanf("%d", &size);

		// "Number of operations: "
		cout << "Number of insert operations: ";
		scanf("%d", &num);

		// Vector for the thread
		std::vector<boost::thread *> z; 
       
		// Started the operations now
		result = std::time(nullptr);
		cout << "Started performing operations at : ";
		cout << std::asctime(std::localtime(&result));
		cout << "\n";

		for (int i = 0; i < num; i++) {
			req_type = _requestType.getNext();
			req_wait_time = _workload.nextArrival(); 
			usleep(req_wait_time);

			// Get the key
			if (req_type == 1) {
				key = get_random_key(count);
				z.push_back(new boost::thread(&DB_Instance::db_get, _dbInstance, key, &value));
			}
			else if (req_type == 2) {
				key = get_random_key(count);
				value = generate_string(size - 6);
				z.push_back(new boost::thread(&DB_Instance::db_put, _dbInstance, key, value));
				count ++;
			}
			else {
				key = "key" + std::to_string(count);
				value = generate_string(size - 6);
				z.push_back(new boost::thread(&DB_Instance::db_put, _dbInstance, key, value));
				count ++;
			}

          /*key = "key" + std::to_string(count);
			value = generate_string(size - 6);
			db->Put(write_options, key, value);
			count++; */
		}

		// Waiting for each thread to response and then killing it
		for (int i = 0; i < num; i++){
			z[i]->join();
			delete z[i];
		}

		result = std::time(nullptr);
		cout << "Finished operations at : ";
		cout << std::asctime(std::localtime(&result));
		cout << "\n";
    }
}
