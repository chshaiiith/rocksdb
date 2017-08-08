#include <iostream>
#include <fstream>
#include <experimental/filesystem>
#include <string>
#include <sstream>
//#include <filesystem>
#include <boost/filesystem.hpp>
#include <unistd.h>

#include <ctime>
#include <ratio>
#include <chrono>
#include <fstream>

#include "rocksdb/db.h"
#include "rocksdb/utilities/backupable_db.h"
#include <boost/algorithm/string.hpp>

namespace fs = std::experimental::filesystem;

using namespace rocksdb;
using namespace std;
using namespace std::chrono;

void get_all_required_files(std::string s, std::string backup_number, vector<fs::path> &files,
                            vector<std::string> &checkSum, int &manNumber) {
    string fileName = s + "meta/" + backup_number;
    if (boost::filesystem::exists(fileName) == false) {
        cout << "Backup doesn't exist or is corrupted";
        return;
    }

    std::string line;
    ifstream file;
    file.open(fileName.c_str());
    // Intial 3 lines are garbage
    for (int i = 0; i < 3; i++) {
        getline(file, line);
    }

    while (getline(file, line)) {
        std::vector<std::string> strs;
        boost::split(strs, line, boost::is_any_of(" "));

        // Just incase if there is any garbage
        if (strs.size() < 3) {
            continue;
        }

        if (strs.at(0).find("MANIFEST")) {
            manNumber = files.size();
        //  cout << manNumber;
        }
        fs::path p(s + strs.at(0));
        files.push_back(p);
        checkSum.push_back(strs.at(2));
    }

    return;
}


int main() {

    std::string s = "/tmp/own_backup_1/";
    std::string backup_number;
    cout << "Insert backup number: \n";
    std::cin >> backup_number;

    time_t result = std::time(nullptr);
    cout << "Time to restore backup starts: ";
    cout << std::asctime(std::localtime(&result));
    cout << "\n";

    vector<fs::path> files;
    vector<std::string> checkSum;
    int manNumber = 0;
    get_all_required_files(s, backup_number, files, checkSum, manNumber);
    // delete if directory exists 
    std::string restore_dir = "/tmp/backup_restore"; // + std::to_string(i);   
    fs::path ran(restore_dir);
    if (fs::is_directory(ran)) {
        fs::remove_all(ran);
    }

    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    fs::create_directories(restore_dir);
    uint32_t check_sum;

    for (int i = 0; i < files.size(); i++) {

        BackupEngine::myCheckSum(files.at(i).string(), Env::Default(), 0, &check_sum);

        if (std::to_string(check_sum) != checkSum.at(i)) {
            cout << "data is corrupted. Exiting...";
            return 0;
        }

        fs::path some1(restore_dir + "/" + files.at(i).filename().string());
//        if (symlink(files.at(i).string().c_str(), some1.string().c_str()) == -1) {
//          cout << "Error: Unable to create symlink\n";
//          return 0;
//	}
        create_hard_link(files.at(i), some1);
    }

    Options options;
    options.create_if_missing = true;
    DB* db;

    Status stat = DB::Open(options, restore_dir, &db);
    assert(stat.ok());
    delete db;
/*  std::string value;                                                                              
    stat = db->Get(rocksdb::ReadOptions(), "key1", &value);
    cout << value;
*/                                                                                                                                                                                                           
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    duration<double, std::milli> time_span = t2 - t1;
    result = std::time(nullptr);
    cout << "Backup restored at : ";
    cout << std::asctime(std::localtime(&result));

    cout << "\n";
    cout << "total time with high resolution clock is : " << time_span.count() << "\n";
//}
/*  BackupEngineReadOnly* backup_engine;
    stat = BackupEngineReadOnly::Open(Env::Default(), BackupableDBOptions(s), &backup_engine);
    assert(stat.ok());
    
    backup_engine->RestoreDBFromBackup(std::stoi(backup_number), "/tmp/rocksdb_restore", "/tmp/rocksdb_restore");

    high_resolution_clock::time_point t3 = high_resolution_clock::now();
    time_span = t3 - t2;

    cout << "total normal time with high resolution clock is : " << time_span.count() << "\n";


//  std::ofstream outfile;
//  outfile.open("results.txt", std::ios_base::app);

//  outfile << files.size();
//    outfile <<  " ";
//    outfile << time_span.count();
//    outfile << "\n";
 //   delete db;
*/
    return 0;
}

