#include "get_weights.h"
#include "math.h"

int color_diff(unsigned char*** image, int height, int width, int x1, int y1, int x2, int y2)
{
	if(x1>=width || x1<0 || y1>=height || y1<0) { return 0; }

	if(x2>=width || x2<0 || y2>=height || y2<0) { return 0; }

	int red_diff = image[y1][x1][0] - image[y2][x2][0];
	int green_diff = image[y1][x1][1] - image[y2][x2][1];
	int blue_diff = image[y1][x1][2] - image[y2][x2][2];

	int max_diff = red_diff>green_diff?red_diff:green_diff;

	return max_diff>blue_diff?max_diff:blue_diff;
}


int max_neighbor_diff(unsigned char*** image, int height, int width, int x, int y)
{
	int max_diff = 0;
	for(int i=-1; i<=1; i++)
		for(int j=-1; j<=1; j++) {
			int diff_ = color_diff(image,height,width,x,y,x+i,y+j);
			if(diff_ > max_diff) {
				max_diff = diff_;
			}
		}

	return max_diff;
}

void get_weights(unsigned char*** image, int height, int width, double** weights)
{
	for(int y=0; y<height; y++)
		for(int x=0; x<width; x++) {
			weights[y][x] = 1.0+1.0*max_neighbor_diff(image,height,width,x,y)/100;
		}
}