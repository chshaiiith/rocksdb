#include "RequestType.h"
#include "random_helper.h"


RequestType::RequestType(double get, double put, double insert) {
	_get = get;
	_put = put;
	_insert = insert;
}

int RequestType::getNext () {
	double rnd = uniform01();
	if (rnd < _get) {
		return 1;
	}

	rnd -= _get;

	if (rnd < _put) {
		return 2;
	}

	return 3;
}
