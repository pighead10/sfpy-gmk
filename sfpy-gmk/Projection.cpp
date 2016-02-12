#include "Projection.h"
#include <algorithm>

bool Projection::overlap(const Projection& other) const{
	return (max > other.min && other.max > min);
}

double Projection::getOverlap(const Projection& other) const{
	//Returns magnitude of overlap between projections, if any
	if(overlap(other)){
		return std::min(max,other.max) - std::max(min,other.min);
	}
	return NULL;
}