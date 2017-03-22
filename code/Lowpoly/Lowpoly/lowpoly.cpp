#include "lowpoly.h"
#include "qx_basic.h"
#include <algorithm>
#include <time.h>
//#include <highgui\highgui.hpp>
//#include <opencv2\opencv.hpp>


//#pragma comment( lib, "opencv_core244.lib" )
//#pragma comment( lib, "opencv_highgui244.lib" )

//using namespace cv;

bool Light_Compare(const COLOR &a, const COLOR &b)
{
	return a.L<b.L?true:false;
}

void LAB2RGB(double L, double a, double b, int* R, int* G, int* B)
{
	double T1 = 0.008856;
	double T2 = 0.206893;

	double fY = pow((L + 16.0) / 116.0, 3);
	bool YT = fY > T1;
	fY = (!YT) * (L / 903.3) + YT * fY;
	double Y = fY;

	// Alter fY slightly for further calculations
	fY = YT * pow(fY,1.0/3.0) + (!YT) * (7.787 * fY + 16.0/116.0);

	double fX = a / 500.0 + fY;
	bool XT = fX > T2;
	double X = (XT * pow(fX, 3) + (!XT) * ((fX - 16.0/116.0) / 7.787));

	double fZ = fY - b / 200.0;
	bool ZT = fZ > T2;
	double Z = (ZT * pow(fZ, 3) + (!ZT) * ((fZ - 16.0/116.0) / 7.787));

	X = X * 0.950456 * 255.0;
	Y = Y * 255.0;
	Z = Z * 1.088754 * 255.0;

	*R = (int) (3.240479*X - 1.537150*Y - 0.498535*Z);
	*G = (int) (-0.969256*X + 1.875992*Y + 0.041556*Z);
	*B = (int) (0.055648*X - 0.204043*Y + 1.057311*Z); 
}

int RGB2LAB(const int& r, const int& g, const int& b, double* lval, double* aval, double* bval)
{
	double xVal = 0.412453 * r + 0.357580 * g + 0.180423 * b;
	double yVal = 0.212671 * r + 0.715160 * g + 0.072169 * b;
	double zVal = 0.019334 * r + 0.119193 * g + 0.950227 * b;

	xVal /= (255.0 * 0.950456);
	yVal /=  255.0;
	zVal /= (255.0 * 1.088754);

	double fY, fZ, fX;
	double lVal, aVal, bVal;
	double T = 0.008856;

	bool XT = (xVal > T);
	bool YT = (yVal > T);
	bool ZT = (zVal > T);

	fX = XT * pow(xVal,1.0/3.0) + (!XT) * (7.787 * xVal + 16.0/116.0);

	// Compute L
	double Y3 = pow(yVal,1.0/3.0); 
	fY = YT*Y3 + (!YT)*(7.787*yVal + 16.0/116.0);
	lVal  = YT * (116 * Y3 - 16.0) + (!YT)*(903.3*yVal);

	fZ = ZT*pow(zVal,1.0/3.0) + (!ZT)*(7.787*zVal + 16.0/116.0);

	// Compute a and b
	aVal = 500 * (fX - fY);
	bVal = 200 * (fY - fZ);

	*lval = lVal;
	*aval = aVal;
	*bval = bVal;

	return 1; //LAB2bin(lVal, aVal, bVal);
}


LOWPOLY::LOWPOLY(int h,int w)
{
	m_h = h;
	m_w = w;
	image = qx_allocu_3( m_h, m_w, 3 );
	m_ErrArr = NULL;
	infinite = 1000000000.0;
}

LOWPOLY::~LOWPOLY()
{
	qx_freeu_3( image ); image = NULL;
	delete m_ErrArr; m_ErrArr = NULL;
}


void LOWPOLY::lowpoly_prepare()
{
	cout<<"Number of points: "<<m_VtxArr.size()<<endl;
	CreateMesh( m_VtxArr, &m_Mesh );
	cout<<"Mesh Created!"<<endl;

	init_polys_array();
	cout<<"Polys array initialized!"<<endl;

	init_error_array();
	cout<<"Error array initialized!"<<endl;

	int vtxArrSize = m_VtxArr.size();
	ranks.resize( vtxArrSize );
	for( int i=0; i<vtxArrSize; i++) { ranks[i]=1; }

}



void LOWPOLY::update_mesh(int rank)
{
	double min_err = infinite;
	int min_err_idx(-1);
	int errArrSize = m_VtxArr.size();
	int weightSize = weights.size();

	//for( int i=0; i<weightSize; i++) {
	//	m_ErrArr[i] *= m_ErrArr[i]*weights[i];
	//}
	

	for( int i=0; i<errArrSize; i++ )
	{
		if( !isRemoved[i] && m_ErrArr[i]<min_err ) {
			min_err = m_ErrArr[i];
			min_err_idx = i;
		}	
	}

	if( min_err_idx == -1 ) {
		cout<<"There is no point can be removed!"<<endl;
		return;
	}

	vector<int> update_candidates;
	clock_t t1 = clock();
	remove_node( min_err_idx, update_candidates );
	clock_t t2 = clock();
	//cout << "remove node:" << t2-t1 <<endl;
	isRemoved[min_err_idx] = true;
	ranks[min_err_idx] = rank;

	for( int i=0; i<update_candidates.size(); i++ )
	{
		int dst = update_candidates[i];
		if( isRemoved[dst] != true) {
			double error = infinite;		
			remove_node_error( dst, error );
			m_ErrArr[dst] = error;
		}
	}

	t1 = clock();
	//cout << "compute error:"<< t1-t2 <<endl;
}


void LOWPOLY::remove_node( int idx, vector<int>& update_candidates  )
{
	vector<POLYCONTENT> cell_polys;
	vector<int> cell_boundary_idx;
	vector<VERTEX2D> cell_boundary;

	find_local_cell( idx, cell_polys, cell_boundary_idx );
	delete_local_cell( idx ); // Delete the polys in local cell
	MESH local_mesh;

	for( int i=0; i<cell_boundary_idx.size(); i++ ) {
		cell_boundary.push_back( m_VtxArr[cell_boundary_idx[i]] );
		update_candidates.push_back( cell_boundary_idx[i] );
	}

	CreateMesh( cell_boundary, &local_mesh );
	remove_bad_triangles( cell_polys, &local_mesh );

	vector<POLYCONTENT> new_polycontents;
	TRIANGLE_PTR pTri = local_mesh.pTriArr;
	while( pTri != NULL )
	{
		POLYCONTENT t_content;
		t_content.poly.i1 = cell_boundary_idx[pTri->i1-3];
		t_content.poly.i2 = cell_boundary_idx[pTri->i2-3];
		t_content.poly.i3 = cell_boundary_idx[pTri->i3-3];
		t_content.averL=0; t_content.averA=0; t_content.averB=0;
		new_polycontents.push_back( t_content );
	
		pTri = pTri->pNext;
	}

	unsigned char** mask;
	mask = qx_allocu( m_h, m_w );
	memset( mask[0], 1, sizeof(unsigned char)*m_h*m_w );
	for( int i=0; i<cell_polys.size(); i++ ) {
		for( int j=0; j<cell_polys[i].pixels.size(); j++ ) {
			int t_x = cell_polys[i].pixels[j].x;
			int t_y = cell_polys[i].pixels[j].y;
			mask[t_y][t_x] = 0;
		}
	}
	find_pixels_of_each_poly( mask, new_polycontents );
	get_base_color( new_polycontents );

	// Update polycontent array
	for( int i=0; i<new_polycontents.size(); i++ ) {
		m_PolyContents.push_back( new_polycontents[i] );
	}

	qx_freeu( mask ); mask = NULL;
	UnInitMesh( &local_mesh );
	delete pTri;
}


void LOWPOLY::init_polys_array()
{
	TRIANGLE_PTR pTri = m_Mesh.pTriArr;
	while( pTri != NULL )
	{
		POLYCONTENT t_content;
		t_content.poly.i1 = pTri->i1-3;
		t_content.poly.i2 = pTri->i2-3;
		t_content.poly.i3 = pTri->i3-3;
		t_content.averL=0; t_content.averA=0; t_content.averB=0;
		m_PolyContents.push_back( t_content );
	
		pTri = pTri->pNext;
	}

	unsigned char** mask;
	mask = qx_allocu( m_h, m_w );
	memset( mask[0], 0, sizeof(unsigned char)*m_h*m_w );
	find_pixels_of_each_poly( mask, m_PolyContents );

	get_base_color( m_PolyContents );
}

void LOWPOLY::find_pixels_of_each_poly( unsigned char** mask, vector<POLYCONTENT>& poly_contents )
{
	int polySize = poly_contents.size();
	for( int i=0; i<polySize; i++ )
	{
		VERTEX2D v1 = m_VtxArr[poly_contents[i].poly.i1];
		VERTEX2D v2 = m_VtxArr[poly_contents[i].poly.i2];
		VERTEX2D v3 = m_VtxArr[poly_contents[i].poly.i3];

		int left = min( min(v1.x,v2.x), v3.x );
		int right = max( max(v1.x,v2.x), v3.x );
		int bottom = min( min(v1.y,v2.y), v3.y );
		int up = max( max(v1.y,v2.y), v3.y );

		for( int y=bottom; y<=up; y++ )
			for( int x=left; x<=right; x++ )
			{
				VERTEX2D dst;
				dst.x = x; dst.y = y;
				if( mask[y][x]==0 && in_triangle(v1,v2,v3,dst)==true ) {
					poly_contents[i].pixels.push_back( dst );
					mask[y][x] = 1;
				}
			}
	}
}

void LOWPOLY::get_base_color( vector<POLYCONTENT>& poly_contents )
{
	int polySize = poly_contents.size();

	for( unsigned int i=0; i<polySize; i++ )
	{
		int nr_pixels = poly_contents[i].pixels.size();

		if( 0 != nr_pixels ) {
			double sumL=0, sumA=0, sumB=0;
			for( unsigned int j=0; j<nr_pixels; j++ ) {
				int t_x = poly_contents[i].pixels[j].x;
				int t_y = poly_contents[i].pixels[j].y;
				sumL += image[t_y][t_x][0];
				sumA += image[t_y][t_x][1];
				sumB += image[t_y][t_x][2];
			}

			poly_contents[i].averL = sumL/nr_pixels;
			poly_contents[i].averA = sumA/nr_pixels;
			poly_contents[i].averB = sumB/nr_pixels;
		}
		else {
			poly_contents[i].averL = 0;
			poly_contents[i].averA = 0;
			poly_contents[i].averB = 0;
		}

	}
}


void LOWPOLY::init_error_array()
{
	int vtxArrSize = m_VtxArr.size();
 //   cout <<"ssss"<<endl;
    m_ErrArr = new double[vtxArrSize];
//    cout <<"aaaa"<<endl;
    for( int i=0; i<vtxArrSize; i++)
    {
        //cout<<i<<endl;
        double error = infinite;
        remove_node_error( i, error );
        m_ErrArr[i] = error;
        isRemoved.push_back( false );
    }
}


void LOWPOLY::remove_node_error( int idx, double& error )
{
	vector<POLYCONTENT> cell_polys;
	vector<int> cell_boundary_idx;
	vector<VERTEX2D> cell_boundary;

	find_local_cell( idx, cell_polys, cell_boundary_idx );
	MESH local_mesh;
	for( int i=0; i<cell_boundary_idx.size(); i++ ) {
		cell_boundary.push_back( m_VtxArr[cell_boundary_idx[i]] );
	}
	CreateMesh( cell_boundary, &local_mesh );
	remove_bad_triangles( cell_polys, &local_mesh );

	vector<POLYCONTENT> new_polycontents;
	TRIANGLE_PTR pTri = local_mesh.pTriArr;
	while( pTri != NULL )
	{
		POLYCONTENT t_content;
		t_content.poly.i1 = cell_boundary_idx[pTri->i1-3];
		t_content.poly.i2 = cell_boundary_idx[pTri->i2-3];
		t_content.poly.i3 = cell_boundary_idx[pTri->i3-3];
		t_content.averL=0; t_content.averA=0; t_content.averB=0;
		new_polycontents.push_back( t_content );
	
		pTri = pTri->pNext;
	}
	unsigned char** mask;
	mask = qx_allocu( m_h, m_w );
	memset( mask[0], 1, sizeof(unsigned char)*m_h*m_w );
	for( int i=0; i<cell_polys.size(); i++ ) {
		for( int j=0; j<cell_polys[i].pixels.size(); j++ ) {
			int t_x = cell_polys[i].pixels[j].x;
			int t_y = cell_polys[i].pixels[j].y;
			mask[t_y][t_x] = 0;
		}
	}
	find_pixels_of_each_poly( mask, new_polycontents );
	get_base_color( new_polycontents );

	int sum_pixels_1(0), sum_pixels_2(0);
	for( int i=0; i<cell_polys.size(); i++ ) {
		sum_pixels_1 += cell_polys[i].pixels.size();
	}
	for( int i=0; i<new_polycontents.size(); i++ ) {
		sum_pixels_2 += new_polycontents[i].pixels.size();
	}

	if( sum_pixels_1 == sum_pixels_2 ) {
		error = compute_error(new_polycontents);// - compute_error(cell_polys);
	}

	qx_freeu( mask ); mask = NULL;
	UnInitMesh( &local_mesh );
	delete pTri;
}


double LOWPOLY::compute_error( vector<POLYCONTENT>& pcs )
{
	double error(0.0);
	int nr_pixels(0);

	for( int i=0; i<pcs.size(); i++ )
	{
		VERTEX2D v1 = m_VtxArr[pcs[i].poly.i1];
		VERTEX2D v2 = m_VtxArr[pcs[i].poly.i2];
		VERTEX2D v3 = m_VtxArr[pcs[i].poly.i3];

		for( int j=0; j<pcs[i].pixels.size(); j++ )
		{
			int t_x = pcs[i].pixels[j].x;
			int t_y = pcs[i].pixels[j].y;
			error = error + pow(pcs[i].averA-image[t_y][t_x][1],2.0)+pow(pcs[i].averB-image[t_y][t_x][2],2.0)+pow(pcs[i].averL-image[t_y][t_x][0],2.0);
			nr_pixels++;
		}
	}

	if( nr_pixels != 0 ) {
		return sqrt(error);
	}
	else {
		return error;
	}
}


void LOWPOLY::find_local_cell( int idx, vector<POLYCONTENT>& cell_polys, vector<int>& cell_boundary_idx )
{
	int polySize = m_PolyContents.size();
	for( unsigned int i=0; i<polySize; i++ )
	{
		POLY t_poly = m_PolyContents[i].poly;
		if( idx == t_poly.i1 ) {
			if( find(cell_boundary_idx.begin(),cell_boundary_idx.end(),t_poly.i2) == cell_boundary_idx.end() ) {
				cell_boundary_idx.push_back(t_poly.i2);
			}
			if( find(cell_boundary_idx.begin(),cell_boundary_idx.end(),t_poly.i3) == cell_boundary_idx.end() ) {
				cell_boundary_idx.push_back(t_poly.i3);
			}
			cell_polys.push_back( m_PolyContents[i] );
		}

		if( idx == t_poly.i2 ) {
			if( find(cell_boundary_idx.begin(),cell_boundary_idx.end(),t_poly.i1) == cell_boundary_idx.end() ) {
				cell_boundary_idx.push_back(t_poly.i1);
			}
			if( find(cell_boundary_idx.begin(),cell_boundary_idx.end(),t_poly.i3) == cell_boundary_idx.end() ) {
				cell_boundary_idx.push_back(t_poly.i3);
			}
			cell_polys.push_back( m_PolyContents[i] );
		}

		if( idx == t_poly.i3 ) {
			if( find(cell_boundary_idx.begin(),cell_boundary_idx.end(),t_poly.i2) == cell_boundary_idx.end() ) {
				cell_boundary_idx.push_back(t_poly.i2);
			}
			if( find(cell_boundary_idx.begin(),cell_boundary_idx.end(),t_poly.i1) == cell_boundary_idx.end() ) {
				cell_boundary_idx.push_back(t_poly.i1);
			}
			cell_polys.push_back( m_PolyContents[i] );
		}
	}
}


void LOWPOLY::delete_local_cell( int idx )
{
	for( vector<POLYCONTENT>::iterator itr = m_PolyContents.begin(); itr != m_PolyContents.end(); )
	{
		POLY t_poly = itr->poly;
		if( idx==t_poly.i1 || idx==t_poly.i2 || idx==t_poly.i3 ) {
			*itr = m_PolyContents.back();
			m_PolyContents.pop_back();
			//itr = m_PolyContents.erase(itr);
		}
		else {
			itr++;
		}
	}
}


void LOWPOLY::remove_bad_triangles( vector<POLYCONTENT>& cell_polys, MESH_PTR pMesh )
{
	TRIANGLE_PTR pTri = pMesh->pTriArr;
	bool is_inside_the_boundingbox = true;
    //cout<<"tttt"<<endl;
	while( pTri != NULL )
	{
		is_inside_the_boundingbox = false;
		VERTEX2D center;
		center.x = ( (pMesh->pVerArr+pTri->i1)->x + (pMesh->pVerArr+pTri->i2)->x + (pMesh->pVerArr+pTri->i3)->x )/3;
		center.y = ( (pMesh->pVerArr+pTri->i1)->y + (pMesh->pVerArr+pTri->i2)->y + (pMesh->pVerArr+pTri->i3)->y )/3;
		for( unsigned int i=0; i<cell_polys.size(); i++ ) {
			VERTEX2D v1 = m_VtxArr[cell_polys[i].poly.i1];
			VERTEX2D v2 = m_VtxArr[cell_polys[i].poly.i2];
			VERTEX2D v3 = m_VtxArr[cell_polys[i].poly.i3];
			if( true == in_triangle(v1,v2,v3,center) ) {
				is_inside_the_boundingbox = true;
				break;
			}
		}

		if( false == is_inside_the_boundingbox )
		{
			TRIANGLE_PTR pTriNextTemp = pTri->pNext;
			RemoveTriangleNode( pMesh, pTri );
			pTri = pTriNextTemp;
			pTriNextTemp = NULL;
		}
		else {
			pTri = pTri->pNext;
		}
	}

	delete pTri;
}

// Related function
double counter_clock_wise( VERTEX2D pa, VERTEX2D pb, VERTEX2D pc )
{
	return ((pb.x - pa.x)*(pc.y - pb.y) - (pc.x - pb.x)*(pb.y - pa.y));
}

bool in_triangle( VERTEX2D s1, VERTEX2D s2, VERTEX2D s3, VERTEX2D dst )
{
	double ccw1 = counter_clock_wise( s1, s2, dst );
	double ccw2 = counter_clock_wise( s2, s3, dst );
	double ccw3 = counter_clock_wise( s3, s1, dst );
	
	if ( ccw1>=0.0 && ccw2>=0.0 && ccw3>=0.0 ) {
		return true;
	}
	else {
		return false;
	}
}
