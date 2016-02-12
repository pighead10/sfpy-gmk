#ifndef PROJECTION_H
#define PROJECTION_H

/*
Projection:
Information about a projection of a shape onto a given axis.
For use with Separating Axis Theorem.
*/

class Projection{ //Projection of a shape onto a given axis, for use with the Separating Axis Theorem
public:
	Projection(double mi,double ma):min(mi),max(ma){}
	~Projection(){}
	double min;
	double max;
	bool overlap(const Projection& other) const;
	double getOverlap(const Projection& other) const;
};

#endif