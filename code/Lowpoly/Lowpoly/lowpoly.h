#ifndef BY_LOWPOLY
#define BY_LOWPOLY


#include "delaunay.h"
#include "qx_basic.h"

struct COLOR
{
	double L,a,b;
};

struct NODE
{
	VERTEX2D v;
	COLOR color;
	int idx;
};

struct POLY
{
	int i1, i2, i3;
};

struct POLYCONTENT
{
	POLY poly;
	vector<VERTEX2D> pixels;
	double averL, averA, averB;
    double L, A, B;
    vector<int> neighborPolyIndex;
};


class LOWPOLY
{
public:
	unsigned char*** image;
	vector<VERTEX2D> m_VtxArr;
	vector<double> weights;
	vector<int> ranks;
	void lowpoly_prepare();
	void update_mesh(int rank);
	LOWPOLY(int h,int w);
	~LOWPOLY();

private:
	void get_base_color( vector<POLYCONTENT>& poly_contents );
	void init_node(NODE &node,int x,int y);

	void init_polys_array();
	void find_pixels_of_each_poly( unsigned char** mask, vector<POLYCONTENT>& poly_contents );
	void init_error_array();
	void remove_node_error( int idx, double& error );
	void remove_node( int idx, vector<int>& update_candidates );
	void find_local_cell( int idx, vector<POLYCONTENT>& cell_polys, vector<int>& cell_boundary_idx );
	void delete_local_cell( int idx );
	void remove_bad_triangles( vector<POLYCONTENT>& cell_polys, MESH_PTR pMesh );
	double compute_error( vector<POLYCONTENT>& pcs );
	void show();

	int m_h, m_w;
	MESH m_Mesh;
	vector<bool> isRemoved;
	vector<POLYCONTENT> m_PolyContents;
	double *m_ErrArr;
	double infinite;
};


// Related function
double counter_clock_wise( VERTEX2D pa, VERTEX2D pb, VERTEX2D pc );
bool in_triangle( VERTEX2D s1, VERTEX2D s2, VERTEX2D s3, VERTEX2D dst );
bool Light_Compare(const COLOR &a, const COLOR &b);
void LAB2RGB(double L, double a, double b, int* R, int* G, int* B);
int RGB2LAB(const int& r, const int& g, const int& b, double* lval, double* aval, double* bval);


#endif
