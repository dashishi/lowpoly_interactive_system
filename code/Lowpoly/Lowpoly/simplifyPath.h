#ifndef SIMPLIFYPATH_H
#define SIMPLIFYPATH_H
#include <iostream>
#include <cstdlib>
#include <vector>
#include <cmath>

//"Point" struct stand for each GPS coordinates(x,y). Methods related are used only for simplification of calculation in implementation.

typedef struct point_{
    int x;
    int y;

    double operator*(point_ rhs)const{
        return (this->x * rhs.y - rhs.x * this->y);
    }

    point_ operator-(point_ rhs)const{
        point_ p;
        p.x=this->x-rhs.x;
        p.y=this->y-rhs.y;
        return p;
    }

    double Norm()const{
        return sqrt(double(this->x) *double( this->x )+ double(this->y) * double(this->y));
    }

}point_;

class simplifyPath{
//"findMaximumDistance" used as part of implementation for RDP algorithm.
private:
    const std::pair<int, double> findMaximumDistance(const std::vector<point_>& Points)const;

//"simplifyWithRDP" returns the simplified path with a Point vector. The function takes in the paths to be simplified and a customerized thresholds for the simplication.
public:
    std::vector<point_> simplifyWithRDP(std::vector<point_>& Points, double epsilon)const;
};

#endif // SIMPLIFYPATH_H
