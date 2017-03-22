#ifndef EDGE_DRAWING
#define EDGE_DRAWING

#include "EdgeMap.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "EDLineDetector.h"
//#include "EDLib.h"
#include "Timer.h"
#include <cv.h>
#include <vector>
using namespace cv;

class EdgeDrawing
{
public:
	EdgeDrawing(void);
	~EdgeDrawing(void);

	char* inputImageStr;
    Mat inputImage;
	EdgeMap* outputMap;
    void createMapFromImage();
	int getHeight() { return m_height; }   
	int getWidth() { return m_width; }
	
protected:
	int m_height;
	int m_width;

	int ReadImagePGM(char *filename, char **pBuffer, int *pWidth, int *pHeight);
	void SaveImagePGM(char *filename, char *buffer, int width, int height);
};

#endif

