#include "lowpoly_thread.h"
//#include "generate.h"
#include "lowpoly.h"
#include "SLIC_corner.h"
#include "get_weights.h"
#include "EdgeDrawing.h"
#include "simplifyPath.h"
#include <time.h>
#include <fstream>
using namespace std;
LowpolyThread::LowpolyThread(QObject *parent) :
    QThread(parent)
{
	height = 0;
	width = 0;
	nr_points = 0;
}
//void drawCircleFromLine(unsigned char ***img, point_ p1, point_ p2, QRgb color , int height, int width)
//{
//    point_ center;

//    center.x = floor( 1.0 *(p1.x + p2.x)/2 + 0.5 );
//    center.y = floor( 1.0 *(p1.y + p2.y)/2 + 0.5 );

//    double radius = sqrt(1.0 * ((p1.x - center.x)*(p1.x - center.x) + (p1.y - center.y)*(p1.y - center.y)) );
//    int r_int = floor(radius) + 1;

//    for (int y = -r_int; y<=r_int; y++)
//    {
//        for (int x = -r_int; x < r_int; x++)
//        {
//            if( (center.x + x >= 0) && (center.x + x < width) && (center.y + y >= 0) && (center.y + y <height) && (x*x + y*y < floor(radius*radius)+1))
//            {
//                img[center.y+y][center.x+x][0] = qRed(color);
//                img[center.y+y][center.x+x][1] = qGreen(color);
//                img[center.y+y][center.x+x][2] = qBlue(color);
//            }
//        }
//    }
//}
//void drawRestrictedRegion(Mat restrictedRegionImage , vector<vector<point_>> & polygon ,int height, int width)
//{
//    for(int y = 0; y < width; y++)
//    {
//        for(int x = 0; x < height; x++)
//        {
//            restrictedRegionImage[y][x][1] = 0;
//            restrictedRegionImage[y][x][2] = 0;
//            restrictedRegionImage[y][x][3] = 0;
//        }
//    }
//    QRgb color = (255,255,255);
//    for(unsigned int i = 0; i < polygon.size();i++)
//    {
//        for (unsigned int j = 0; j < polygon[i].size();i++)
//        {
//            drawCircleFromLine(restrictedRegionImage,polygon[i][j-1],polygon[i][j],color,height,width);
//        }
//    }
//}
void drawCircleFromLine( Mat &img, const point_ p1, const point_ p2, const Vec3b color )
{
    point_ center;
    center.x = floor( 1.0*(p1.x+p2.x)/2+0.5 );
    center.y = floor( 1.0*(p1.y+p2.y)/2+0.5 );

    double radius = sqrt( 1.0*((p1.x-center.x)*(p1.x-center.x)+(p1.y-center.y)*(p1.y-center.y)) );
    int r_int = floor(radius)+1;

    for(int y=-r_int; y<=r_int; y++ )
        for(int x=-r_int; x<r_int; x++) {
            if( (center.x+x>=0) && (center.x+x<img.cols) && (center.y+y>=0) && (center.y+y<img.rows) && (x*x+y*y<floor(radius*radius)+1) ) {
                img.at<Vec3b>(center.y+y,center.x+x) = color;
            }
        }
}

// 画限制采点区域
void drawRestrictedRegion( Mat &restrictedRegionImage, vector<vector<point_>>& polygon)
{
    for(int y=0;y<restrictedRegionImage.rows;y++)
        for(int x=0;x<restrictedRegionImage.cols;x++) {
            restrictedRegionImage.at<Vec3b>(y,x) = Vec3b(0,0,0);
        }

    for(unsigned int i=0; i<polygon.size(); i++ )
        for( unsigned int j=1; j<polygon[i].size(); j++ ) {
            drawCircleFromLine( restrictedRegionImage, polygon[i][j-1], polygon[i][j], Vec3b(255,255,255) );
        }
}

void DP(EdgeDrawing& ed, vector<vector<point_>>& polygon)
{
    int nr_constrained_points = 0;
    int height = ed.getHeight();
    int width = ed.getWidth();
    int interval = 2*floor( 0.02 *(height+width) + 0.5);

    for (int i = 0; i < ed.outputMap->noSegments; i++)
    {
        vector<point_> polygon_unit;
        vector<point_> contours;
        vector<point_> result;
        for (int j = 0 ; j < ed.outputMap->segments[i].noPixels; j++)
        {
            int r = ed.outputMap->segments[i].pixels[j].r;
            int c = ed.outputMap->segments[i].pixels[j].c;
            point_ tmp;
            tmp.x = c;
            tmp.y = r;
            contours.push_back( tmp);
        }
        simplifyPath dptmp;
        result = dptmp.simplifyWithRDP(contours,1.0);
        vector<int> resIdx;
        int count = 0;
        for (unsigned int j = 0; j < contours.size();j++)
        {
            if(contours[j].x == result[count].x && contours[j].y == result[count].y)
            {
                resIdx.push_back(j);
                count++;
            }
        }
        sort(resIdx.begin(), resIdx.end(), less<int>());
        vector<int> nResIdx;
        for(int j=0; j<resIdx.size()-1; j++) {
            int t_dist = resIdx[j+1]-resIdx[j];
            if( t_dist>interval ) {
                int t_step = t_dist/interval+1;
                double t_intervel = 1.0*t_dist/t_step;
                for(int k=0; k<t_step; k++) {
                    nResIdx.push_back( resIdx[j]+floor(k*t_intervel+0.5) );
                }
            }
            else {
                nResIdx.push_back( resIdx[j] );
            }
        }
        nResIdx.push_back( resIdx[resIdx.size()-1] );

        for(unsigned int j=0; j<nResIdx.size(); j++) {
            polygon_unit.push_back( contours[nResIdx[j]] );
        }

        nr_constrained_points += polygon_unit.size();
        polygon.push_back( polygon_unit );
    }
    std::cout<<nr_constrained_points<<std::endl;
        //vector<int> nRes
}

void LowpolyThread::run()
{

	emit message(tr("Start Lowpoly"),0);

    ofstream out;
	clock_t t1 = clock();
 //   EdgeDrawing ed;
 //   ed.inputImage = img_mat;
 //   ed.createMapFromImage();
	//clock_t t2 = clock();
	//cout << "edge drawing: "<<t2-t1<<endl;
 //   vector<vector<point_>> polygon ; 
 //   DP( ed, polygon );
	//Mat triangulation;
	//triangulation.create(img_mat.rows,img_mat.cols,CV_8UC3);
	////img_mat.copySize(triangulation);
	//for (int i = 0; i < polygon.size(); i++)
	//{
	//	Point p1 = Point(polygon[i][0].x, polygon[i][0].y);
	//	Point p2 = Point(polygon[i][1].x, polygon[i][1].y);
	//	Point p3 = Point(polygon[i][2].x, polygon[i][2].y);
	//	line(triangulation, p1,p2,Scalar(0,0,255));
	//	line(triangulation, p1,p3,Scalar(0,0,255));
	//	line(triangulation, p3,p2,Scalar(0,0,255));
	//}
	//imwrite("result/triangulation.jpg",triangulation);
	//cout <<"DP:" <<t1-t2<<endl;
    //Mat edges(height,width,CV_8UC3);
    //for(int y=0;y<height;y++)
    //    for(int x=0;x<width;x++) {
    //        edges.at<Vec3b>(y,x) = Vec3b(0,0,0);
    //    }
    //for (int i=0; i<ed.outputMap->noSegments; i++)
    //{
    //    for (int j=0; j<ed.outputMap->segments[i].noPixels; j++){
    //        int r = ed.outputMap->segments[i].pixels[j].r;
    //        int c = ed.outputMap->segments[i].pixels[j].c;
    //        edges.at<Vec3b>(r,c) = Vec3b(255,255,255);
    //    }
    //}
 //   imwrite("result/edge_drawing.png", edges);
    //Mat anchorPoints;
    //inputImage.copyTo( anchorPoints );
    //anchorPoints.create( height, width, CV_8UC3 );
    //for(int y=0;y<height;y++) for(int x=0;x<width;x++) { anchorPoints.at<Vec3b>(y,x) = Vec3b(255,255,255); }

    //vector<BY_POINT> initPoints;
    //for(unsigned int i=0; i<polygon.size(); i++)
    //    for(unsigned int j=0; j<polygon[i].size(); j++) {
    //        int r = polygon[i][j].y;
    //        int c = polygon[i][j].x;

    //        for(int p=0;p<=0;p++) for(int q=0;q<=0;q++) {
    //            int x_ = p+c;
    //            int y_ = q+r;
    //            if(x_>=0 && x_<width && y_>=0 && y_<height) {
    //                anchorPoints.at<Vec3b>(y_,x_) = Vec3b(0,0,0);
    //            }
    //        }
    //    }

 //   imwrite("result/anchorPoints.png", anchorPoints);
	//t1 = clock();
 //   Mat restrictedRegionImage;
 //   restrictedRegionImage.create( height, width, CV_8UC3 );
 //   drawRestrictedRegion( restrictedRegionImage, polygon );
	//imwrite("result/restrictedRegionImage.jpg",restrictedRegionImage);
	unsigned char** corner = qx_allocu( height,width );
	memset( corner[0], 0, sizeof(unsigned char)*height*width );
	SLIC_corner( image_in, height, width, corner );
	clock_t t3 = clock();
	out.open("time.txt");
	out << "SLIC: " << t3 - t1 <<endl; 
	/////////////////////////////////////////////////////////////write corner image///////////////////////////////////////////////////
    //Mat cornerImg(height,width,CV_8UC3);
    //for(int y=0;y<height;y++)
    //    for(int x=0;x<width; x++) {
    //        cornerImg.at<Vec3b>(y,x) = Vec3b(255,255,255);
    //    }
    //for(int y=0;y<height;y++)
    //    for(int x=0;x<width; x++) {
    //        if(corner[y][x]>0) {
    //            for(int i=-1;i<=1;i++) for(int j=-1;j<=1;j++) {
    //                int x_ = x+i;
    //                int y_ = y+j;
    //                if(x_>=0 && x_<width && y_>=0 && y_<height) {
    //                    cornerImg.at<Vec3b>(y_,x_) = Vec3b(0,0,0);
    //                }
    //            }							
    //        }
    //    }
   // imwrite("result/corner.jpg",cornerImg);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    emit message(tr("Get Initial points"),0);
  //  Mat combinationImg ;
   // anchorPoints.copyTo( combinationImg );



   // for(int y=0;y<height;y++) for(int x=0;x<width;x++)
   // {
   //     if( corner[y][x] > 0 && restrictedRegionImage.at<Vec3b>(y,x).val[0]!=255 && anchorPoints.at<Vec3b>(y,x).val[0]!= 0)
   //     {
   //         points_x->push_back(x);
   //         points_y->push_back(y);
   //         combinationImg.at<Vec3b>(y,x) = Vec3b(255,0,0);
   //     }

   //      if( anchorPoints.at<Vec3b>(y,x).val[0] == 0 )
   //      {
   //          points_x->push_back(x);
   //          points_y->push_back(y);
   //          combinationImg.at<Vec3b>(y,x) = Vec3b(0,0,255);
   //      }
		 //if( corner[y][x] >0 && restrictedRegionImage.at<Vec3b>(y,x).val[0]==255 && anchorPoints.at<Vec3b>(y,x).val[0]!= 0 )
		 //{ 
			// combinationImg.at<Vec3b>(y,x) = Vec3b(0,255,0);
		 //}
		 //if ( corner[y][x] >0 && anchorPoints.at<Vec3b>(y,x).val[0]== 0)
		 //{
			// combinationImg.at<Vec3b>(y,x) = Vec3b(0,0,0);
		 //}
   //  }
	//t2 = clock();
	//cout <<"get initial points:" << t2-t1<<endl;
    for(int y=0;y<height;y++) for(int x=0;x<width;x++) {
        if( corner[y][x]>0) {
            points_x->push_back(x);
            points_y->push_back(y);

        }
    }
//    for(int y=0;y<height;y++) for(int x=0;x<width;x++) {
//        if( corner[y][x]==255 && restrictedRegionImage.at<Vec3b>(y,x).val[0]!=255 && anchorPoints.at<Vec3b>(y,x).val[0]!=255 ) {
//                BY_POINT point_unit;
//                point_unit.x=x; point_unit.y=y;
//                initPoints.push_back( point_unit );

//                for(int i=0;i<=0;i++) for(int j=0;j<0;j++) {
//                    int x_ = x+i;
//                    int y_ = y+j;
//                    if(x_>=0 && x_<width && y_>=0 && y_<height) {
//                        combinationImg.at<Vec3b>(y_,x_) = Vec3b(0,0,255);
//                    }
//                }
//            }
//        }
    //imwrite("result/combination.png",combinationImg);
	nr_points = points_x->size();
//    std::cout<<"anchor point"<<nr_points;
	//vector<double> weights(nr_points);

	//for(unsigned int i=0; i<nr_points; i++) { 
	//	weights[i] = m_weights[(*points_y)[i]][(*points_x)[i]];
	//}

	emit message(tr("Get Weights"),0);

	LOWPOLY lowpoly(height, width);

	for( int y=0; y<height; y++ )
		for( int x=0; x<width; x++ )
		{
			lowpoly.image[y][x][0] = image_in[y][x][0];
			lowpoly.image[y][x][1] = image_in[y][x][1];
			lowpoly.image[y][x][2] = image_in[y][x][2];
		}

	for( int i=0; i<nr_points; i++ ) {
		VERTEX2D vertex_unit;
		vertex_unit.x = (*points_x)[i];
		vertex_unit.y = (*points_y)[i];
		lowpoly.m_VtxArr.push_back( vertex_unit );
	}

	//for( unsigned int i=0; i<weights.size(); i++ ) {
	//	lowpoly.weights.push_back( weights[i] );
	//}

	emit message(tr("Prepare Adaptive Thinning"),0);
//    std::cout <<lowpoly.m_VtxArr.size()<<" "<<lowpoly.weights.size()<<std::endl;
	t1 = clock();
    lowpoly.lowpoly_prepare();
//	t2 = clock();
//	cout << "adaptive thinning:" << t2-t1 <<endl;

    for(int i=nr_points; i>200; i-- ) {
        lowpoly.update_mesh(i);  //find the ranks of each point
        if(i%100 == 0) {
            QString msg = QString("Remain Nodes: %1").arg(i);
            emit message(msg,0);
        }
    }
	//Mat outRank(height,width,CV_8UC3);
 //   for(int y=0;y<height;y++)
 //       for(int x=0;x<width; x++) {
 //           outRank.at<Vec3b>(y,x) = Vec3b(255,255,255);
 //       }
	//ofstream file("rank.txt",ios::out);
    (*ranks).resize(nr_points);
    for(int i=0; i<nr_points; i++) {
        (*ranks)[i] = lowpoly.ranks[i];
		//file <<lowpoly.m_VtxArr[i].x<<" "<<lowpoly.m_VtxArr[i].y<<" "<<lowpoly.ranks[i]<<endl;
		//Point center = Point(lowpoly.m_VtxArr[i].x, lowpoly.m_VtxArr[i].y);
		//int radius = floor(((3000 - lowpoly.ranks[i])/500.0));
		//if (radius <= 1)
		//{
		//	radius = 1;
		//}
		//circle(outRank, center, radius, Scalar(255,0,0), -1);
    }
	//imwrite("outRank.jpg", outRank);
	//file.close();
	//Mat top500 = Mat(height,width,CV_8UC3);
	//    for(int y=0;y<height;y++)
 //       for(int x=0;x<width;x++) {
 //           top500.at<Vec3b>(y,x) = Vec3b(255,255,255);
 //       }
	// vector<VERTEX2D> draw_points;
	//for (int i = 0; i < nr_points; i++)
	//{
	//	if((*ranks)[i] <1000 )
	//	{
	//		VERTEX2D vertex_unit;
	//		vertex_unit.x = lowpoly.m_VtxArr[i].x;
	//		vertex_unit.y = lowpoly.m_VtxArr[i].y;
	//		draw_points.push_back(vertex_unit);
	//		//top500.at<Vec3b>(lowpoly.m_VtxArr[i].y,lowpoly.m_VtxArr[i].x) = Vec3b(255,255,255);
	//	}
	//}
	//MESH mesh;
 //   CreateMesh(draw_points, &mesh);

 //   vector<POLYCONTENT> polyContents;

 //   TRIANGLE_PTR pTri = mesh.pTriArr;
 //   while (pTri != NULL)
 //   {
 //       POLYCONTENT content_unit;
 //       content_unit.poly.i1 = pTri->i1 - 3;
 //       content_unit.poly.i2 = pTri->i2 - 3;
 //       content_unit.poly.i3 = pTri->i3 - 3;
 //       content_unit.averL = 0; content_unit.averA = 0; content_unit.averB = 0;
 //       polyContents.push_back(content_unit);

 //       pTri = pTri->pNext;
 //   }
	//for (int i = 0; i < polyContents.size(); i++)
	//{
	//	VERTEX2D ptmp1 = draw_points[polyContents[i].poly.i1];
	//	VERTEX2D ptmp2 = draw_points[polyContents[i].poly.i2];
	//	VERTEX2D ptmp3 = draw_points[polyContents[i].poly.i3];
	//	line(top500, Point(ptmp1.x, ptmp1.y), Point(ptmp2.x, ptmp2.y) , Scalar(255,0,0));
	//	line(top500, Point(ptmp1.x, ptmp1.y), Point(ptmp3.x, ptmp3.y) , Scalar(255,0,0));
	//	line(top500, Point(ptmp3.x, ptmp3.y), Point(ptmp2.x, ptmp2.y) , Scalar(255,0,0));
	//}
	//imwrite("top500.jpg",top500);
	clock_t t2 = clock();
	
	out << "adaptive thinning: " << t2 - t1 <<endl;
	out.close();
//	cout <<"delete points"<<t1-t2<<endl;
	emit message(tr("Finished"),0);
}

//void DP( EdgeDrawing& ed, vector<vector<Point>>& polygon )
//{
//    int nr_constrained_points = 0;
//    int height = ed.getHeight();
//    int width = ed.getWidth();
//    int interval = 2*floor( 0.02*(height+width)+0.5 );

//    for (int i=0; i<ed.outputMap->noSegments; i++)
//    {
//        vector<Point> polygon_unit;
//        vector<Point> contours;
//        vector<Point> result;

//        for (int j=0; j<ed.outputMap->segments[i].noPixels; j++){
//            int r = ed.outputMap->segments[i].pixels[j].r;
//            int c = ed.outputMap->segments[i].pixels[j].c;
//            contours.push_back( Point(c,r) );
//        }
//        approxPolyDP( contours, result, 1.0, true );

//        vector<int> resIdx;
//        int count = 0;
//        for(unsigned int j=0; j<contours.size(); j++) {
//            if( contours[j].x==result[count].x && contours[j].y==result[count].y ) {
//                resIdx.push_back(j);
//                count++;
//            }
//        }

//        sort(resIdx.begin(), resIdx.end(),less<int>());

//        vector<int> nResIdx;
//        for(int j=0; j<resIdx.size()-1; j++) {
//            int t_dist = resIdx[j+1]-resIdx[j];
//            if( t_dist>interval ) {
//                int t_step = t_dist/interval+1;
//                double t_intervel = 1.0*t_dist/t_step;
//                for(int k=0; k<t_step; k++) {
//                    nResIdx.push_back( resIdx[j]+floor(k*t_intervel+0.5) );
//                }
//            }
//            else {
//                nResIdx.push_back( resIdx[j] );
//            }
//        }
//        nResIdx.push_back( resIdx[resIdx.size()-1] );

//        for(unsigned int j=0; j<nResIdx.size(); j++) {
//            polygon_unit.push_back( contours[nResIdx[j]] );
//        }

//        nr_constrained_points += polygon_unit.size();
//        polygon.push_back( polygon_unit );
//        printf("%f\n",ed.outputMap->segments[i].noPixels);
//    }

//    printf("Constrained points number: %d\n", nr_constrained_points);
//}
