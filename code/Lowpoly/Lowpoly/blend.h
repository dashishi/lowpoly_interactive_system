#pragma once
#ifndef BLEND_H
#define BLEND_H

#include "linearSolver.h"
#include "lowpoly.h"

#include <vector>

class Blend : public implicitMatrix
{
public:
	Blend(vector<POLYCONTENT>& poly_tmp);
	~Blend();
	void matVecMult(const Areal x[], Areal r[]) const;
	void blendPoly(vector<POLYCONTENT>& poly_contents);
	void setRVector(vector<POLYCONTENT>& poly_contents, Areal *rL, Areal *rA, Areal *rB);
	void setXVector(vector<POLYCONTENT>& poly_contents, Areal *xL, Areal *xA, Areal *xB);
	Areal *xL, *xA, *xB;
private:
	vector<POLYCONTENT> polyContents;


};
#endif