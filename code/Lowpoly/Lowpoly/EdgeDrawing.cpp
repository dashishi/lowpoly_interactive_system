#include "EdgeDrawing.h"

//#pragma comment(lib, "cv249.lib")          // OpenCV
#pragma comment(lib, "opencv_core249.lib")      // OpenCV - cxcore
//#pragma comment(lib, "EDLib.lib")          // DetectEdgesByED, DetectEdgesByEDPF, DetectEdgesByCannySR, DetectEdgesByCannySRPF functions

EdgeDrawing::EdgeDrawing(void)
{
    inputImage = NULL;
	inputImageStr = NULL;
	outputMap = NULL;
}


EdgeDrawing::~EdgeDrawing(void)
{
	inputImageStr = NULL;
	delete outputMap;
}


void EdgeDrawing::createMapFromImage()
{
	int width, height;
	unsigned char *srcImg; 
//    imshow("s1",inputImage);
//    waitKey(0);
//	if (ReadImagePGM(inputImageStr, (char **)&srcImg, &width, &height) == 0){
//		printf("Failed opening <%s>\n", inputImageStr);
//		return;
//	} //end-if

//	printf("Working on %dx%d <%s> image\n\n", width, height, inputImageStr);

//	//-------------------------------- ED Test ------------------------------------
    Timer timer;
    timer.Start();
    EDLineParam para;
    para.ksize = 3;
    para.sigma = 1.0;
    para.gradientThreshold = 36;
    para.anchorThreshold = 8;
    para.minLineLen = 15;
    para.lineFitErrThreshold = 1.4;
    para.scanIntervals = 2;
    EDLineDetector lineDetector(para);
    EdgeChains edgeChains;
//    imshow("s",inputImage);

    imwrite("in.jpg",inputImage);
    lineDetector.EdgeDrawing(inputImage, edgeChains, false);
//    std::cout<< inputImage.cols<<std::endl;
//    std::cout<< inputImage.rows;
//    imshow("s",inputImage);
//    waitKey(0);
    int r = inputImage.rows;
    int c = inputImage.cols;
    outputMap = new EdgeMap(c,r);

    //outputMap->width = c;
   // outputMap->height = r;
//    outputMap->width = inputImage.cols;
    outputMap->noSegments = edgeChains.numOfEdges;
//    std::cout<<edgeChains.sId.size()<<std::endl;
 //   std::cout<<edgeChains.xCors.size()<<std::endl;
    for(int i = 0; i < edgeChains.sId.size(); i++)
    {
        EdgeSegment segment;
//        if (edgeChains.sId[i+1] <= edgeChains.sId[i])
//        {
//            std::cout<<edgeChains.sId[i]<<" "<<edgeChains.sId[i+1];
//        }
//        std::cout<<i<<std::endl;
        if (i == edgeChains.sId.size()-1)
        {
//            std::cout<<edgeChains.sId[i]<<std::endl;
            segment.pixels = new pixel[edgeChains.xCors.size()-edgeChains.sId[i]];
            segment.noPixels = int(edgeChains.xCors.size() - edgeChains.sId[i]);
            for(int j = edgeChains.sId[i]; j < edgeChains.xCors.size(); j++)
            {
//                std::cout<<j<<std::endl;
                pixel pixeltmp;
                pixeltmp.c = edgeChains.xCors[j];
                pixeltmp.r = edgeChains.yCors[j];
                segment.pixels[j-edgeChains.sId[i]] = pixeltmp;
            }
            outputMap->segments[i] = segment;

        }
        else
        {
            segment.pixels = new pixel[edgeChains.sId[i+1] - edgeChains.sId[i]];
            segment.noPixels = int(edgeChains.sId[i+1] - edgeChains.sId[i]);
            for (int j = int(edgeChains.sId[i]) ; j < int(edgeChains.sId[i+1]); j++)
            {
                pixel pixeltmp;
                pixeltmp.c = edgeChains.xCors[j];
                pixeltmp.r = edgeChains.yCors[j];
                segment.pixels[j-edgeChains.sId[i]] = pixeltmp;
            }
            outputMap->segments[i] = segment;
        }
//        outputMap->segments[i] = segment;
    }
    //std::cout<<"sfcafas";
//    //outputMap = DetectEdgesByED(srcImg, width, height, SOBEL_OPERATOR, 36, 8, 1.0);
//    m_height = height;
//    m_width = width;

    timer.Stop();
    std::cout << "ed over"<<std::endl;
    printf("ED detects <%d> edge segments in <%4.2lf> ms\n\n", outputMap->noSegments, timer.ElapsedTime());

    // This is how you access the pixels of the edge segments returned by ED
//    memset(outputMap->edgeImg, 0, width*height);
//    for (int i=0; i<outputMap->noSegments; i++){
//        for (int j=0; j<outputMap->segments[i].noPixels; j++){
//            int r = outputMap->segments[i].pixels[j].r;
//            int c = outputMap->segments[i].pixels[j].c;
//            outputMap->edgeImg[r*width+c] = 255;
//        } //end-for
//    } //end-for

   // SaveImagePGM("ED-EdgeMap.pgm", (char *)outputMap->edgeImg, width, height);
}


int EdgeDrawing::ReadImagePGM(char *filename, char **pBuffer, int *pWidth, int *pHeight)
{
    FILE *fp;
   char buf[71];
   int width, height;

   if ((fp = fopen(filename, "rb")) == NULL){
     fprintf(stderr, "Error reading the file %s in ReadImagePGM().\n", filename);
     return(0);
   } //end-if

   /***************************************************************************
   * Verify that the image is in PGM format, read in the number of columns
   * and rows in the image and scan past all of the header information.
   ***************************************************************************/
   fgets(buf, 70, fp);
   bool P2 = false;
   bool P5 = false;

   if      (strncmp(buf, "P2", 2) == 0) P2 = true;
   else if (strncmp(buf, "P5", 2) == 0) P5 = true;

   if (P2 == false && P5 == false){
      fprintf(stderr, "The file %s is not in PGM format in ", filename);
      fprintf(stderr, "ReadImagePGM().\n");
      fclose(fp);
      return 0;
   } //end-if

   do {fgets(buf, 70, fp);} while (buf[0] == '#');  /* skip all comment lines */
   sscanf_s(buf, "%d %d", &width, &height);
   fgets(buf, 70, fp);  // Skip max value (255)

   *pWidth = width;
   *pHeight = height;

   /***************************************************************************
   * Allocate memory to store the image then read the image from the file.
   ***************************************************************************/
   if (((*pBuffer) = (char *) malloc((*pWidth)*(*pHeight))) == NULL){
      fprintf(stderr, "Memory allocation failure in ReadImagePGM().\n");
      fclose(fp);
      return(0);
   } //end-if

   if (P2){
      int index=0;
      char *p = *pBuffer;
      int col = 0;
      int read = 0;

      while (1){
        int c;
        if (fscanf_s(fp, "%d", &c) < 1) break;
        read++;

        if (col < *pWidth) p[index++] = (unsigned char)c;

        col++;
        if (col == width) col = 0;
      } //end-while

      if (read != width*height){
        fprintf(stderr, "Error reading the image data in ReadImagePGM().\n");
        fclose(fp);
        free((*pBuffer));
        return(0);
      } //end-if

   } else if (P5){
      int index=0;
      char *p = *pBuffer;
      int col = 0;
      int read = 0;

      while (1){
        unsigned char c;
        if (fread(&c, 1, 1, fp) < 1) break;
        read++;

        if (col < *pWidth) p[index++] = c;

        col++;
        if (col == width) col = 0;
      } //end-while

     if (read != width*height){
        fprintf(stderr, "Error reading the image data in ReadImagePGM().\n");
        fclose(fp);
        free((*pBuffer));
        return(0);
     } //end-if
   } //end-else

   fclose(fp);
   return 1;
}

