#include <iostream>
#include <fstream>

#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

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
#include <boost/chrono/thread_clock.hpp>

using namespace boost::chrono;
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

 

int make_curl_call(string postthis_, string url) {
	CURL *curl;
  	CURLcode res;

  	//static const char *postthis = "moo mooo moo moo";

	const char *postthis = postthis_.c_str();
  	curl = curl_easy_init();
  	if(curl) {
		std::string s(postthis);
		std::cout << "reached here " << s << "\n";
    		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postthis);

    		/* if we don't provide POSTFIELDSIZE, libcurl will strlen() by
       			itself */
    		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(postthis));

    		/* Perform the request, res will get the return code */
    		res = curl_easy_perform(curl);
    		/* Check for errors */
    		if(res != CURLE_OK)
      			fprintf(stderr, "curl_easy_perform() failed: %s\n",
              		curl_easy_strerror(res));

    		/* always cleanup */
    		curl_easy_cleanup(curl);
  	}	
  	return 0;
}


int main() {
	// Few variable intialization
    	long count = 4000000;
    	string key;
    	string value, out, url;
    	std::time_t result;
    	double rate;
    	int type;
    	double get, put, insert;
    	int req_type;
    	double req_wait_time;
		
		// Creating DB Instance
		DB_Instance db(true);
		ofstream myfile ("example_get.txt");
		ofstream myfile1 ("example_put.txt");

		
    	// Initializing curl
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
		vector<double> results_put;
		vector<double> results_get;		

		// "Number of operations: "
		cout << "Number of insert operations: ";
		scanf("%d", &num);

		// Vector for the thread
		std::vector<boost::thread *> z; 
      	const clock_t begin_time = clock();
 
		// Started the operations now
		result = std::time(nullptr);
		cout << "Started performing operations at : ";
		cout << std::asctime(std::localtime(&result));
		cout << "\n";

		string URL = "http://cse-bacheli01.cse.psu.edu:9080";


		for (int i = 0; i < num; i++) {
			req_type = _requestType.getNext();
			req_wait_time = _workload.nextArrival(); 
		//	usleep(req_wait_time);
			
			if (req_type == 1) {
				key = get_random_key(count);
				auto begin = std::chrono::high_resolution_clock::now();
				
				db.db_get(key, &value);
				
				auto end = std::chrono::high_resolution_clock::now();
				myfile << duration_cast<std::chrono::nanoseconds>(end - begin).count()  << "\n";
			}

			else if (req_type == 2) {
				key = get_random_key(count);
				value = generate_string(size - 6);
				auto begin = std::chrono::high_resolution_clock::now();
				
				db.db_put(key, value);
				
				auto end = std::chrono::high_resolution_clock::now();
				
				myfile1 << duration_cast<std::chrono::nanoseconds>(end - begin).count()  << "\n";
				//results_put.push_back(duration_cast<std::chrono::nanoseconds>(end - begin).count());
				
				count ++;
			}

			else {
				key = "key" + std::to_string(count);
				value = generate_string(size - 6);
				
//				auto begin = std::chrono::high_resolution_clock::now();
				
				db.db_put(key, value);
//				cout << key << value << "\n";
				
//				auto end = std::chrono::high_resolution_clock::now();
				
//				myfile1 << duration_cast<std::chrono::nanoseconds>(end - begin).count()  << "\n";
				//results_put.push_back(duration_cast<std::chrono::nanoseconds>(end - begin).count());
				count ++;
			}

		}

		cout << "Operation finished operations \n";
		myfile.flush();
		myfile1.flush();
/*		std::sort (results_get.begin(), results_get.end());
		std::sort (results_put.begin(), results_put.end());

		ofstream myfile ("example_get.txt");
		for (int i = 0; i < results_get.size(); i ++) {
		}

		ofstream myfile1("example_put.txt");
		for (int i = 0; i < results_put.size(); i ++) {
			myfile1 << results_put[i] << "\n";
		}
*/			
//		myfile.close();	
//		myfile1.close();

		cout << "\n";
    }
}
