#ifndef REQUEST_TYPE_PROCESS_H
#define REQUEST_TYPE_PROCESS_H

#include <cstdint>


class RequestType
{
public:
    RequestType(double get, double put, double insert);
    double _get, _put, _insert;
    // Returns next type
    int getNext();
};

#endif

