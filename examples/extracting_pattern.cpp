/*
* functions that are needed to generate workload 
* according to a pattern file, as well as an example 
* of using them.
*/
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

// a dummy DB class for debug
class DB {
public:
  void Put(std::string& key, std::string& value) {
    std::cout << "Put: (" << key << ", " << value << ")" << std::endl;
  }
  void Delete(std::string& key) {
    std::cout << "Del: " << key << std::endl;
  }
  void DeleteRange(std::string& startkey, std::string& endkey) {
    std::cout << "Range_Del: (" << startkey << ", " << endkey << ")"
              << std::endl;
  }
};


struct pattern {
  float put;	// percentage of put
  float del;	// percentage of del
  float rdel;	// percentage of range delete
  float unikey;	// percentage of hot keys
  int nkeys = 0;
  int nvals = 0;
  int keysize[50];
  int valsize[50];
  float keyprob[50];
  float valprob[50];
  void print() {
    std::cout << "percentage of PUT: " << put << std::endl;
    std::cout << "percentage of DEL: " << del << std::endl;
    std::cout << "percentage of RANGE DEL: " << rdel << std::endl;
    std::cout << "percentage of unique keys: " << unikey << std::endl;
    std::cout << "keys:" << std::endl;
    for (int i = 0; i < nkeys; i++) {
      std::cout << keysize[i] << " " << keyprob[i] << std::endl;
    }
    std::cout << "vals:" << std::endl;
    for (int i = 0; i < nvals; i++) {
      std::cout << valsize[i] << " " << valprob[i] << std::endl;
    }
  }
};

typedef struct pattern Pattern;

Pattern extract_pattern(char const *fname) {
  Pattern res;
  std::ifstream infile(fname);
  std::string line;
  while (std::getline(infile, line)) {
    std::stringstream ss(line);
    std::vector<std::string> vec;
    std::string buf;
    while (ss >> buf) vec.push_back(buf);
    if (vec[0] == "PUT") {
      res.put = std::stof(vec[1]);
    }
    if (vec[0] == "DEL") {
      res.del = std::stof(vec[1]);
    }
    if (vec[0] == "RDEL") {
      res.rdel = std::stof(vec[1]);
    }
    if (vec[0] == "unikey") {
      res.unikey = std::stof(vec[1]);
    }
    if (vec[0] == "key") {
      res.keysize[res.nkeys] = std::stoi(vec[1]);
      res.keyprob[res.nkeys] = std::stof(vec[2]);
      res.nkeys++;
    }
    if (vec[0] == "val") {
      res.valsize[res.nvals] = std::stoi(vec[1]);
      res.valprob[res.nvals] = std::stof(vec[2]);
      res.nvals++;
    }
  }
  return res;
}

// generating an operation according to a specified pattern
void generate_op(DB* db, Pattern& pat, uint64_t* uniquekey,  
  int64_t* keypool, int* sizepool, int poolsize, int64_t count) {  
  std::string key;
  std::string value;
  std::string fkey;

  float opacc[3];
  opacc[0] = pat.put;
  opacc[1] = opacc[0] + pat.del;
  opacc[2] = opacc[1] + pat.rdel;

  int ksize;
  int vsize;

  // determine the type of op, PUT, DEL or RANGE DEL
  int r = rand()%100;
  int opflag = 0;
  for (; opflag < 3; opflag++) {
    if (opacc[opflag]*100 >= r) break;
  }
  if (opflag == 3) opflag = 2;
  // unique key?
  if (count < poolsize) {
    int tmp = rand()%100;
    int i = 0;
    for (; i < pat.nkeys; i++) {
      if (pat.keyprob[i]*100 >= tmp) break;
    }
    if (i == pat.nkeys) i = pat.nkeys - 1;
    ksize = pat.keysize[i];
    key = std::to_string(*uniquekey);
    key.resize(ksize, '.');
    fkey = std::to_string((*uniquekey) - 10);
    fkey.resize(ksize, '.');
    keypool[(*uniquekey)%poolsize] = *uniquekey;
    sizepool[(*uniquekey)%poolsize] = ksize;

    *uniquekey = *uniquekey + 1;
  } else if (rand()%100 < 100*pat.unikey){
    int tmp = rand()%100;
    int i = 0;
    for (; i < pat.nkeys; i++) {
      if (pat.keyprob[i]*100 >= tmp) break;
    }
    if (i == pat.nkeys) i = pat.nkeys - 1;
    ksize = pat.keysize[i];
    key = std::to_string(*uniquekey);
    key.resize(ksize, '.');
    fkey = std::to_string(*uniquekey - 10);
    fkey.resize(ksize, '.');
    int toss = rand()%(*uniquekey);
    if (toss < poolsize) {  // welcome board!
      int rin = rand()%poolsize;
      keypool[rin] = *uniquekey;
      sizepool[rin] = ksize;
    }
    *uniquekey = *uniquekey + 1;
  }else{  // existing key then
    int ind = rand()%poolsize;
    int64_t k = keypool[ind];
    ksize = sizepool[ind];
    key = std::to_string(k);
    key.resize(ksize, '.');
    fkey = std::to_string(k - 10);
    fkey.resize(ksize, '.');
  }

  if (opflag == 0) {  // if PUT
    int tmp = rand()%100;
    int i = 0;
    for (; i < pat.nvals; i++) {
      if (pat.valprob[i]*100 > tmp) break;
    }
    if (i == pat.nvals) i = pat.nvals - 1;
    vsize = pat.valsize[i];
    value = std::to_string(*uniquekey);
    value.resize(vsize, '.');
    db->Put(key, value);
    // add statistics
    int ki;
    int vi;
    for (int i = 0; i < pat.nkeys; i++) {
      if (pat.keysize[i] == ksize) {
        ki = i; 
        break;
      }
    }
    for (int i = 0; i < pat.nvals; i++) {
      if (pat.valsize[i] == vsize) {
        vi = i;
        break;
      }
    }
  } else if (opflag == 1) {  // if DEL
    // add statistics
    int ki;
    for (int i = 0; i < pat.nkeys; i++) {
      if (pat.keysize[i] == ksize) {
        ki = i;
        break;
      }
    }
    db->Delete(key);
  } else {  // if RANGE DEL
    // add statistics
    int ki;
    for (int i = 0; i < pat.nkeys; i++) {
      if (pat.keysize[i] == ksize) {
        ki = i;
        break;
      }
    }
    db->DeleteRange(fkey, key);
  }
}

int main(int argc, char const *argv[]) {
  if (argc != 3) {
    std::cout << "./fill_wal_with_pattern <nops> <pattern_file>"
              << std::endl;
    exit(1);
  }

  Pattern pat = extract_pattern(argv[2]);

  DB* db = new DB();

  std::string nop_string(argv[1]);

  int nop = std::stoi(nop_string);

  uint64_t count = 0;
  uint64_t uniquekey = 0;

  int put_matrix[pat.nkeys][pat.nvals];
  int del_array[pat.nkeys];
  int rdel_array[pat.nkeys];

  for (int i = 0; i < pat.nkeys; i++) {
    for (int j = 0; j < pat.nvals; j++) put_matrix[i][j] = 0;
  }
 
  for (int i = 0; i < pat.nkeys; i++) {
    del_array[i] = 0;
    rdel_array[i] = 0;
  }

  // acc key probabilities
  int acc = 0;
  for (int i = 0; i < pat.nkeys; i++) {
    pat.keyprob[i] = acc + pat.keyprob[i];
    acc += pat.keyprob[i];
  }

  // acc value probabilities
  acc = 0;
  for (int i = 0; i < pat.nvals; i++) {
    pat.valprob[i] = acc + pat.valprob[i];
    acc += pat.valprob[i];
  }

  int poolsize = 5;
  int64_t keypool[poolsize];	// topped array which records the most recent 1000-modular numbers
  int sizepool[poolsize];

  srand(time(NULL));

  while(count < nop) {
    generate_op(db, pat, &uniquekey, keypool, sizepool, poolsize, count);
    count++;
  }
  // print statistics
  for (int i = 0; i < pat.nkeys; i++) {
    for (int j = 0; j < pat.nvals; j++) {
      if (put_matrix[i][j] > 0) {
        std::cout << "PUT key size: " << pat.keysize[i] << " val size: " 
                  << pat.valsize[j] << " counts: " << put_matrix[i][j] 
                  << std::endl;
      }
    }
  }
  for (int i = 0; i < pat.nkeys; i++) {
    if (del_array[i] > 0) {
      std::cout << "DEL key size: " << pat.keysize[i] << " counts: " 
                << del_array[i] << std::endl;
    }
  }
  for (int i = 0; i < pat.nkeys; i++) {
    if (rdel_array[i] > 0) {
      std::cout << "RANGE DEL key size: " << pat.keysize[i] << " counts: "
                << rdel_array[i] << std::endl;
    }
  }
  std::cout << "The number of unique keys is " << uniquekey << std::endl;
  delete db;
  return 0;
}
