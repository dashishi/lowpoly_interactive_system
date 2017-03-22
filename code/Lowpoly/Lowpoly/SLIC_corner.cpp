#include "SLIC_corner.h"
#include "stdafx.h"
#include "SLIC.h"
#include "qx_basic.h"
#include <ctime>
#include <fstream>
#include <map>


bool isNoCornerArround( unsigned char** corner, const int& height, const int& width, const int& x, const int& y )
{
	if(x==0 || x==width-1 || y==0 || y==height-1) return true;

	if( corner[y+1][x+1] == 255 ) return false;
	if( corner[y+1][x] == 255 ) return false;
	if( corner[y+1][x-1] == 255 ) return false;
	if( corner[y][x-1] == 255 ) return false;
	if( corner[y-1][x-1] == 255 ) return false;
	if( corner[y-1][x] == 255 ) return false;
	if( corner[y-1][x+1] == 255 ) return false;
	if( corner[y][x+1] == 255 ) return false;
	
	return true;
}

void SLIC_corner( unsigned char*** image_in, const int& height, const int& width, unsigned char** corner )
{
    int nr_superpixels = 3000;
	unsigned int* ubuff = NULL;
	//unsigned int* ubuff_ = NULL;
	int sz = width*height;
	ubuff = new unsigned int[sz];
	//ubuff_ = new unsigned int[sz];
	for( int i = 0; i < height; i++ )
		for( int j = 0; j < width; j++ )
		{
			ubuff[i*width+j] = image_in[i][j][2]*65536 + image_in[i][j][1]*256 + image_in[i][j][0]; 
			//ubuff_[i*width+j] = image_in[i][j][2]*65536 + image_in[i][j][1]*256 + image_in[i][j][0]; 
		}

	int* labels = new int[sz];
	int numlabels(0);
	SLIC slic;
	clock_t begin = clock();
	
	//slic.PerformSLICO_ForGivenK(ubuff, width, height, labels, numlabels, nr_superpixels, 10.0);//for a given number K of superpixels
	slic.DoSuperpixelSegmentation_ForGivenNumberOfSuperpixels(ubuff, width, height, labels, numlabels,nr_superpixels, 10.0);
	clock_t end = clock();
	double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
	printf( "[%d superpixels],[time: %f sec]\n", nr_superpixels, elapsed_secs );

	int dxt[8] = { -1, -1, 0, 1, 1, 1, 0, -1 };
	int dyt[8] = { 0, -1, -1, -1, 0, 1, 1, 1 };

	bool* istaken = (bool*)malloc(sizeof(bool)*height*width);

	unsigned char** zeros = qx_allocu( height, width );
	memset( zeros[0], 0, sizeof(unsigned char)*height*width );

	memset(istaken,false,sizeof(bool)*height*width);	
	for(int i=0;i<height;i++) for(int j=0;j<width;j++) 
	{
		int np = 0;
		for( int k=0; k<8; k++ )
		{
			int x = j + dxt[k];
			int y = i + dyt[k];
			if( x>=0 && x<width && y>=0 && y<height ) {
				if( false==istaken[y*width+x] && labels[y*width+x]!=labels[i*width+j] )
					np++;
			}
		}
		if( np > 1 )
		{
			istaken[i*width+j] = true;
			zeros[i][j] = 255;
		}
	}

	for(int y=0;y<height;y++) for(int x=0;x<width;x++) {
		if( zeros[y][x]==255 ) {
			map<int,int> map;
			int count = 1;
			for(int p=0; p<=1; p++) for(int q=0; q<=1; q++) {
				int ny = y + p;
				int nx = x + q;
				if( nx>=0 && nx<width && ny>=0 && ny<height ) {
					if( map.find(labels[ny*width+nx])==map.end() ) {
						map.insert(pair<int,int>(labels[ny*width+nx],count));
						count++;
					}
				}
			}
			if( count > 3 ) {
				if( isNoCornerArround(corner,height,width,x,y) ) {
					corner[y][x] = 255;
				}
			}
		}
	}

	corner[0][0] = 255;
	corner[height-1][0] = 255;
	corner[0][width-1] = 255;
	corner[height-1][width-1] = 255;
	for(int y=0; y<height; y++) {
		if( zeros[y][0]==255 ) {
			corner[y][0] = 255;
		}
		if( zeros[y][width-1]==255 ) {
			corner[y][width-1] = 255;
		}
	}

	for(int x=0; x<width; x++) {
		if( zeros[0][x]==255 ) {
			corner[0][x] = 255;
		}
		if( zeros[height-1][x]==255 ) {
			corner[height-1][x] = 255;
		}
	}
}
