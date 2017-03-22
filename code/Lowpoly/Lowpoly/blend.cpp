#include "blend.h"

Blend::Blend(vector<POLYCONTENT>& poly_tmp)
{

	//for (int i = 0 ; i < poly_tmp.size(); i++)
	//{
	//	POLYCONTENT poly;
	//	poly.averA = poly_tmp[i].averA;
	//	poly.averL = poly_tmp[i].averL;
	//	poly.averB = poly_tmp[i].averB;
	//	poly.neighborPolyIndex

	//}
	polyContents = poly_tmp;
}
Blend::~Blend()
{
	free(xL);
	free(xA);
	free(xB);

}
double countWeight(double a, double b)
{
	double c = a - b;
	if(c < 20 && c>=0) c = 20;
	if(c < 0 && c>-20) c = -20;
	return c;
}
void Blend::setRVector(vector<POLYCONTENT>& poly_contents, Areal *rL, Areal *rA, Areal *rB)
{
	int center;
	int dist = 1000000;
	//int num = poly_contents.size();
	for (int i = 0 ; i < poly_contents.size(); i++)
	{
		//int c = poly_contents[i].poly.i1;
		int numOfNeighbor = poly_contents[i].neighborPolyIndex.size();
		//rL[i] = - numOfNeighbor * poly_contents[i].averL;
		//rA[i] = - numOfNeighbor * poly_contents[i].averA;
		//rB[i] = - numOfNeighbor * poly_contents[i].averB;
		for (int j = 0 ; j < numOfNeighbor; j++)
		{
			int index = poly_contents[i].neighborPolyIndex[j];
			double ltmp = countWeight(poly_contents[index].averL,poly_contents[i].averL);
			double atmp = countWeight(poly_contents[index].averA,poly_contents[i].averA);
			double btmp = countWeight(poly_contents[index].averB,poly_contents[i].averB);
			//rL[i] += poly_contents[index].averL;
			//rA[i] += poly_contents[index].averA;
			//rB[i] += poly_contents[index].averB;
			rL[i] += ltmp;
			rA[i] += atmp;
			rB[i] += btmp;
		}
		if ( i%5 == 0 && poly_contents[i].neighborPolyIndex.size()!=0)
		{
			//rL[i] = rL[i] - countWeight(poly_contents[poly_contents[i].neighborPolyIndex[0]].averL ,poly_contents[i].averL ) - poly_contents[i].averL;
			//rA[i] = rA[i] - countWeight(poly_contents[poly_contents[i].neighborPolyIndex[0]].averA ,poly_contents[i].averL ) - poly_contents[i].averA;
			//rB[i] = rB[i] - countWeight(poly_contents[poly_contents[i].neighborPolyIndex[0]].averB ,poly_contents[i].averL ) - poly_contents[i].averB;
			//rL[i] -= poly_contents[i].averL;
			//rA[i] -= poly_contents[i].averA;
			//rB[i] -= poly_contents[i].averB;
			rL[i] -= poly_contents[poly_contents[i].neighborPolyIndex[0]].averL;
			rA[i] -= poly_contents[poly_contents[i].neighborPolyIndex[0]].averA;
			rB[i] -= poly_contents[poly_contents[i].neighborPolyIndex[0]].averB;
		}
		//rL[i] += poly_contents[i].averL;
		//rA[i] += poly_contents[i].averA;
		//rB[i] += poly_contents[i].averB;
	}
	//for (int i = poly_contents.size()/5; i < poly_contents.size()/4; i++)
	//{
	//	rL[i] -= poly_contents[poly_contents[i].neighborPolyIndex[0]].averL;
	//	rA[i] -= poly_contents[poly_contents[i].neighborPolyIndex[0]].averA;
	//	rB[i] -= poly_contents[poly_contents[i].neighborPolyIndex[0]].averB;
	//}
	//for (int i = poly_contents.size(); i < 2*poly_contents.size(); i++)
	//{
	//	rL[i] = poly_contents[i-poly_contents.size()].averL;
	//	rA[i] = poly_contents[i-poly_contents.size()].averA;
	//	rB[i] = poly_contents[i-poly_contents.size()].averB;
	//}
}
void Blend::setXVector(vector<POLYCONTENT>& poly_contents, Areal *xL, Areal *xA, Areal *xB)
{
	for (int i = 0 ; i < poly_contents.size(); i++)
	{
		xL[i] = poly_contents[i].averL;
		xA[i] = poly_contents[i].averA;
		xB[i] = poly_contents[i].averB;
	}
}
void Blend::matVecMult(const Areal x[], Areal r[]) const
{
	for (int i = 0 ; i < polyContents.size(); i++)
	{
		int neighborSize = polyContents[i].neighborPolyIndex.size();
		r[i] = -neighborSize * x[i];
		for (int j = 0 ; j < neighborSize; j++)
		{
			int index = polyContents[i].neighborPolyIndex[j];
			//if ( i%10 !=0 )
			//{
			//	r[i] += x[index];
			//}
			//else if (j!=0)
			//{
			//	r[i] += x[index];
			//}
			r[i] += x[index];
			//double rtmp = countWeight(x[index],x[i]);
			//r[i] += rtmp;
			
		}
	   if ( i%5 == 0 && polyContents[i].neighborPolyIndex.size()!=0)
		{
			r[i] -= x[polyContents[i].neighborPolyIndex[0]];
			//r[i] = r[i] - countWeight(x[polyContents[i].neighborPolyIndex[0]],x[i]) - x[i];
		}
//		r[i] += x[i];
	}
}
void Blend::blendPoly(vector<POLYCONTENT>& poly_contents)
{
	xL = (Areal *) malloc (poly_contents.size() * sizeof(Areal));
	xA = (Areal *) malloc (poly_contents.size() * sizeof(Areal));
	xB = (Areal *) malloc (poly_contents.size() * sizeof(Areal));
	Areal *rL = (Areal *) malloc (poly_contents.size() * sizeof(Areal));
	Areal *rA = (Areal *) malloc (poly_contents.size() * sizeof(Areal));
	Areal *rB = (Areal *) malloc (poly_contents.size() * sizeof(Areal));
	setRVector(poly_contents, rL, rA, rB);
	setXVector(poly_contents, xL, xA, xB);

	Areal epsilon = 0.001;
	int steps = 2000;
	Areal errorL = ConjGrad(poly_contents.size(),this, xL, rL, epsilon, &steps);
	Areal errorA = ConjGrad(poly_contents.size(),this, xA, rA, epsilon, &steps);
	Areal errorB = ConjGrad(poly_contents.size(),this, xB, rB, epsilon, &steps);
	
	//for (int i = 0 ; i < poly_contents.size(); i++)
	//{
	//	cout << xL[i] <<endl; 
	//}

	free(rL);
	free(rA);
	free(rB);

	
}