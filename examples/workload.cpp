#include "workload.h"

Workload::Workload(double rate, int type) {
	_rate = rate;
	_type = type;
	return;
	
}



double Workload::nextArrival() {
	if (_type == 1) {
		return _rate;
	}

	else if (_type == 2) {
		return (-1 * log(uniform01())/_rate);
	}
	return 0;
}


