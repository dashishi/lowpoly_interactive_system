#include "mainwindow.h"
#include <time.h>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent), imgshow(NULL),paintMask(NULL),points_x(NULL),points_y(NULL),ranks(NULL), imgTemp(NULL),inputBackup(NULL)
{
	ui.setupUi(this);
	ui.slider->hide();
	ui.distValue->hide();
	ui.paint->hide();
	ui.clear->hide();
	ui.lineEdit->hide();
	ui.distSlider->hide();
	connect(ui.actionOpen, SIGNAL(triggered()), this, SLOT(open()));
	connect(ui.actionSave, SIGNAL(triggered()),this, SLOT(save()));
	connect(ui.actionOrigin, SIGNAL(triggered()),this, SLOT(original_image()));
	connect(ui.actionWeights, SIGNAL(triggered()),this, SLOT(weights()));
	connect(ui.actionLowpoly,SIGNAL(triggered()),this, SLOT(lowpoly()));
	connect(ui.paint, SIGNAL(clicked()), this, SLOT(paint()));
	connect(ui.clear, SIGNAL(clicked()),this, SLOT(clear()));
	connect(ui.slider, SIGNAL(valueChanged(int)), this, SLOT(setTextValue(int)));
	connect(ui.actionRendering,SIGNAL(triggered()), this, SLOT(render()));
	connect(ui.distSlider, SIGNAL(valueChanged(int)), this, SLOT(setDistValue(int)));
	dist = 0;
	node_size = 0;
	saturValue = 50;
}

MainWindow::~MainWindow()
{
	//delete ui;

}

bool isNeighborPoly( POLY& p1, POLY& p2 ) {
    int count = 0;
    if( p1.i1 == p2.i1 ) count++;
    if( p1.i1 == p2.i2 ) count++;
    if( p1.i1 == p2.i3 ) count++;
    if( p1.i2 == p2.i1 ) count++;
    if( p1.i2 == p2.i2 ) count++;
    if( p1.i2 == p2.i3 ) count++;
    if( p1.i3 == p2.i1 ) count++;
    if( p1.i3 == p2.i2 ) count++;
    if( p1.i3 == p2.i3 ) count++;

    return (count==2)?true:false; //如果两个三角形共用两个顶点，则认为相邻
}
void findPolyRelationship(vector<POLYCONTENT>& poly_contents) {
    int polyContentSize = poly_contents.size();
    for(int i=0; i<polyContentSize; i++) {
        for( int j=i; j<polyContentSize; j++) {
            if( isNeighborPoly(poly_contents[i].poly, poly_contents[j].poly) ) {
                poly_contents[i].neighborPolyIndex.push_back(j);  //统计相邻三角形，将相邻三角形index存入neighborPolyIndex中
            }
        }
    }
}
void RGBtoWeight(COLOR c1, COLOR c2, double& r, double& g, double& b)
{
    r = c1.L - c2.L;
    if(r>=0 && r<20) { r = 20; }
    if(r<0 && r >-20) { r = -20; }

    g = c1.a - c2.a;
    if(g>=0 && g<20) { g = 20; }
    if(g<0 && g>-20) { g = -20; }

    b = c1.b - c2.b;
    if(b>=0 && b<20) { b = 20; }
    if(b<0 && b>-20) { b = -20; }
}
void countWeight(vector<POLYCONTENT>& poly_contents, vector<double>& _weights)
{
    int count = 0;
    int polyContentSize = poly_contents.size();

    for( int i=0; i<polyContentSize; i++) {
        for(int j=0; j<poly_contents[i].neighborPolyIndex.size(); j++)
        {
            int index_ = poly_contents[i].neighborPolyIndex[j];
            double r(0), g(0), b(0);
            COLOR color_1_, color_2_;
            color_1_.L = poly_contents[i].averL;
            color_1_.a = poly_contents[i].averA;
            color_1_.b = poly_contents[i].averB;
            color_2_.L = poly_contents[index_].averL;
            color_2_.a = poly_contents[index_].averA;
            color_2_.b = poly_contents[index_].averB;
            RGBtoWeight( color_1_, color_2_, r, g, b );
            _weights[3*count] = r;
            _weights[3*count+1] = g;
            _weights[3*count+2] = b;
            count++;
        }
    }
}

Mat QImage2cvMat(QImage image)
{
    cv::Mat mat;
    qDebug() << image.format();
    switch(image.format())
    {
    case QImage::Format_ARGB32:
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32_Premultiplied:
        mat = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
        break;
    case QImage::Format_RGB888:
        mat = cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
        cv::cvtColor(mat, mat, CV_BGR2RGB);
        break;
    case QImage::Format_Indexed8:
        mat = cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
        break;
    }
    return mat;
}
void MainWindow::open()
{
	QFileDialog *fileDialog = new QFileDialog(this);
    fileDialog->setWindowTitle(tr("Open Image"));
    fileDialog->setDirectory("."); // 设置初始路径？（"."可能是指当前路径）
    fileDialog->setFilter(QDir::Files	// 只列出文件
        | QDir::NoSymLinks);	// 不列出符号连接
    if (fileDialog->exec() == QDialog::Accepted)
    {
        path = fileDialog->selectedFiles()[0]; // 返回一个或者多个文件（绝对路径）
        QMessageBox::information(NULL, tr("Path"), tr("You selected ") + path);
    }
    else
    {
        QMessageBox::information(NULL, tr("Path"), tr("You didn't select any pictures"));
        return;
    }
    if(imgshow)
    {
        imgshow->deleteLater();
    }
    if (paintMask)
    {
        delete paintMask;
    }
	if (imgTemp)
	{
		delete imgTemp;
	}
	if (inputBackup)
	{
		delete inputBackup;
	}
    if(!points_x.empty())
    {
        points_x.swap(vector<int>());
    }
    if(!points_y.empty())
    {
         points_y.swap(vector<int>());;
    }
    if(!ranks.empty())
    {
         ranks.swap(vector<int>());;
    }
    imgshow = new PaintWidget(this);
    // 构建QImage对象读入图片
    imgInput = new QImage(path);
    Mat imgtmp = QImage2cvMat(*imgInput);
    cvtColor(imgtmp,image_mat,CV_BGR2GRAY);
 //   imshow("a",img_mat);
    width = imgInput->width();
    height = imgInput->height();

    image_in = qx_allocu_3(height, width, 3);
    m_weights = qx_allocd(height, width);
    for (int y = 0; y<height; y++) for (int x = 0; x<width; x++) {
        QRgb color_unit = imgInput->pixel(x, y);
        image_in[y][x][0] = qRed(color_unit);
        image_in[y][x][1] = qGreen(color_unit);
        image_in[y][x][2] = qBlue(color_unit);
    }
    get_weights(image_in, height, width, m_weights);

    paintMask = new QImage(imgInput->size(), QImage::Format_RGB888);
    paintMask->fill(Qt::white);

    imgTemp = new QImage(*imgInput);
    inputBackup = new QImage(*imgInput);

    // 用QLabel显示图片

    //imgshow->setPixmap(QPixmap::fromImage(*imgIn));
    imgshow->setGeometry(5, 27, width, height); // 该方法继承与Qwidget类，设置相对于父窗口的位置
    imgshow->paintMask = paintMask;
    imgshow->srcImg = imgTemp;
    imgshow->inputImg = imgInput;
    update();
    imgshow->show();

    this->resize(QSize(width + 10, height + 140)); // 主窗口大小由输入图像调整

    ui.slider->setVisible(true); // show()中调用了setVisible(true)
    ui.lineEdit->setVisible(true);
   // ui->saturSlider->setVisible(true);
    ui.paint->setVisible(true);
    ui.clear->setVisible(true);
	ui.distSlider->setVisible(true);
	ui.distValue->setVisible(true);
    ui.slider->setGeometry(20, height + 40, width / 2, 19);
	ui.distSlider->setGeometry(20, height + 60, width/2, 19);
   // ui->saturSlider->setGeometry(20, height + 60, width / 2 , 19);
    ui.lineEdit->setGeometry(width / 2 + 25, height + 40, 50, 20);
	ui.distValue->setGeometry(width / 2 + 25 , height + 60, 50, 20);
    ui.paint->setGeometry(width - 130, height + 40, 60, 20);
    //ui.paint->setGeometry(width-170,height+40,80,20);
    ui.clear->setGeometry(width - 65, height + 40, 60, 20);
}
void MainWindow::save()
{
    // 设置保存对话框
    QFileDialog *saveDialog = new QFileDialog(this);
    //QStringList filters;
    saveDialog->setWindowTitle("Save as");
    saveDialog->setAcceptMode(QFileDialog::AcceptSave);
    saveDialog->setFileMode(QFileDialog::AnyFile);
    saveDialog->setViewMode(QFileDialog::Detail);
    saveDialog->setDirectory(".");

    // 根据选择路径保存图片
    QString savePath;
    QPixmap out;
    out = this->grab(QRect(5, 27, width, height));
    if (saveDialog->exec() == QDialog::Accepted)
    {
        savePath = saveDialog->selectedFiles()[0];
        out.save(savePath);
    }
}
void MainWindow::weights()
{
    double max_weights = 0;

    for (int y = 0; y<height; y++)
        for (int x = 0; x<width; x++) {
        if (max_weights < m_weights[y][x]) {
            max_weights = m_weights[y][x];
        }
        }

    for (int y = 0; y<height; y++)
        for (int x = 0; x<width; x++) {
        int gray = qx_round(255.0*m_weights[y][x] / max_weights);
        if (qRed(paintMask->pixel(QPoint(x, y))) < 10) { gray = 255; }
        imgTemp->setPixel(QPoint(x, y), qRgb(gray, gray, gray));
        }

    imgshow->show();
}
void MainWindow::lowpoly()
{
	if (!points_x.empty()) {
    ui.slider->setValue(node_size);
    setTextValue(node_size);
    return;
    }

    //do lowpoly
    unsigned char** corner = qx_allocu(height, width);
    memset(corner[0], 0, sizeof(unsigned char)*height*width);

    LowpolyThread* lowpoly_thread = new LowpolyThread;
    lowpoly_thread->image_in = image_in;
    lowpoly_thread->m_weights = m_weights;
    lowpoly_thread->points_x = &points_x;
    lowpoly_thread->points_y = &points_y;
    lowpoly_thread->ranks = &ranks;
    lowpoly_thread->height = height;
    lowpoly_thread->width = width;
    lowpoly_thread->img_mat = image_mat;
   // imwrite("s.jpg",lowpoly_thread->img_mat);
    connect(lowpoly_thread, SIGNAL(finished()), this, SLOT(finish_lowpoly()));
    connect(lowpoly_thread, SIGNAL(message(QString, int)), ui.statusBar, SLOT(showMessage(QString, int)));

    lowpoly_thread->start(QThread::HighPriority);
}
void MainWindow::paint()
{
	if (imgshow->isPaintButtonClicked == false) {
    imgshow->isPaintButtonClicked = true;
    ui.paint->setText(tr("Done"));
    return;
    }

    if (imgshow->isPaintButtonClicked == true) {
        imgshow->isPaintButtonClicked = false;
        ui.paint->setText(tr("Paint"));
        imgshow->curPosition.setX(-50);
        imgshow->curPosition.setY(-50);
        if (!points_x.empty()) {
            generate(node_size, saturValue , dist);
        }
        return;
    }
}
void MainWindow::clear()
{
	paintMask->fill(Qt::white);
    *imgTemp = *imgInput;

    imgshow->curPosition.setX(-50);
    imgshow->curPosition.setY(-50);
    imgshow->show();
}
void MainWindow::setTextValue(int value)
{
	QString pos = QString("%1").arg(value); // 用arg中的内容替代%i来构建字符串
    ui.lineEdit->setText(pos);

    node_size = value;
    //if(node_size == ui.slider->maximum())
    //{
    //    imgshow->inputImg = imgInput;
    //    update();
    //    imgshow->show();
    //}
    //else{
    generate(value , saturValue, dist);
   // }
}
void MainWindow::original_image()
{
	imgTemp = new QImage(path);
    imgshow->srcImg = imgTemp;
    imgshow->show();
}
void MainWindow::setSaturValue(int value)
{
	saturValue = value;
    generate(node_size, saturValue, dist);
}
void MainWindow::finish_lowpoly()
{
    LowpolyThread* th = qobject_cast<LowpolyThread*>(sender()); // QObject::Sender()返回发送信号的对象的指针，返回类型为QObject*
    if (th)
    {
        if (th->isFinished())
        {
            ui.slider->setValue(500);
            setTextValue(500);
            th->deleteLater();
        }
    }
}
void find_pixels_of_each_poly(unsigned char** mask, vector<POLYCONTENT>& poly_contents, vector<VERTEX2D>& points)
{
    int polySize = poly_contents.size();
    for (int i = 0; i<polySize; i++)
    {
        VERTEX2D v1 = points[poly_contents[i].poly.i1];
        VERTEX2D v2 = points[poly_contents[i].poly.i2];
        VERTEX2D v3 = points[poly_contents[i].poly.i3];

        int left = min(min(v1.x, v2.x), v3.x);
        int right = max(max(v1.x, v2.x), v3.x);
        int bottom = min(min(v1.y, v2.y), v3.y);
        int up = max(max(v1.y, v2.y), v3.y);

        for (int y = bottom; y <= up; y++)
            for (int x = left; x <= right; x++)
            {
            VERTEX2D dst;
            dst.x = x; dst.y = y;
            if (mask[y][x] == 0 && in_triangle(v1, v2, v3, dst) == true) {
                poly_contents[i].pixels.push_back(dst);
                mask[y][x] = 1;
            }
            }
    }
}
void histogram_equalization(QImage* input, vector<POLYCONTENT>& poly_contents)
{
    int polyContentSize = poly_contents.size();
    int poly_rows = 0;
    //findPolyRelationship( poly_contents );
    for(int i=0; i<polyContentSize; i++)  {
        poly_rows += poly_contents[i].neighborPolyIndex.size();
    }
    for (int i = 0; i<polyContentSize; i++)
    {
        if (poly_contents[i].pixels.size() != 0) {
            vector<COLOR> colorArray;
            for (int j = 0; j<poly_contents[i].pixels.size(); j++)
            {
                int t_x = poly_contents[i].pixels[j].x;
                int t_y = poly_contents[i].pixels[j].y;
                COLOR color_unit;
                QRgb rgb = input->pixel(t_x, t_y);
                RGB2LAB(qRed(rgb), qGreen(rgb), qBlue(rgb), &color_unit.L, &color_unit.a, &color_unit.b);
                colorArray.push_back(color_unit);
            }
            sort(colorArray.begin(), colorArray.end(), Light_Compare);

            // 去L分量排序在0.4-0.6之间的值 作颜色平均
            int begin_value = floor(0.4*poly_contents[i].pixels.size());
            int end_value = floor(0.6*poly_contents[i].pixels.size()) + 1;
            int diff_value = end_value - begin_value;

            double sumL = 0, sumA = 0, sumB = 0;
            for (int j = begin_value; j<end_value; j++) {
                sumL += colorArray[j].L;
                sumA += colorArray[j].a;
                sumB += colorArray[j].b;
            }

            int exR, exG, exB;
            LAB2RGB(sumL / diff_value, sumA / diff_value, sumB / diff_value, &exR, &exG, &exB);
            poly_contents[i].averL = exR;
            poly_contents[i].averA = exG;
            poly_contents[i].averB = exB;
        }
    }
}
double count_(double a, double b  , double dis)
{
	double c = a - b;
	if(c < dis && c>=0) c = dis;
	if(c < 0 && c>-1.0*dis) c = -1.0*dis;
	return c;
}
void histogram_equalization_RGB(QImage* input, vector<POLYCONTENT>& poly_contents, double dis)
{
    int polyContentSize = poly_contents.size();
    int poly_rows = 0;
    findPolyRelationship( poly_contents );
    for(int i=0; i<polyContentSize; i++)  {
        poly_rows += poly_contents[i].neighborPolyIndex.size();
    }
	clock_t t1 = clock();
    for (int i = 0; i<polyContentSize; i++)
    {
        if (poly_contents[i].pixels.size() != 0) {
            vector<COLOR> colorArray;
            for (int j = 0; j<poly_contents[i].pixels.size(); j++)
            {
                int t_x = poly_contents[i].pixels[j].x;
                int t_y = poly_contents[i].pixels[j].y;
                COLOR color_unit;
                QRgb rgb = input->pixel(t_x, t_y);
                RGB2LAB(qRed(rgb), qGreen(rgb), qBlue(rgb), &color_unit.L, &color_unit.a, &color_unit.b);
                colorArray.push_back(color_unit);
            }
            sort(colorArray.begin(), colorArray.end(), Light_Compare);

            // 去L分量排序在0.4-0.6之间的值 作颜色平均
            int begin_value = floor(0.4*poly_contents[i].pixels.size());
            int end_value = floor(0.6*poly_contents[i].pixels.size()) + 1;
            int diff_value = end_value - begin_value;

            double sumL = 0, sumA = 0, sumB = 0;
            for (int j = begin_value; j<end_value; j++) {
                sumL += colorArray[j].L;
                sumA += colorArray[j].a;
                sumB += colorArray[j].b;
            }

   //         int exR, exG, exB;
   //         LAB2RGB(sumL / diff_value, sumA / diff_value, sumB / diff_value, &exR, &exG, &exB);
			//poly_contents[i].averL = exR;
   //         poly_contents[i].averA = exG;
   //         poly_contents[i].averB = exB;
            poly_contents[i].averL = sumL / diff_value;
            poly_contents[i].averA = sumA / diff_value;
            poly_contents[i].averB = sumB / diff_value;
        }
    }
	clock_t t2 = clock();
	int numOfPoly = poly_contents.size();
	myg::SparseMatrix<int, double> A(numOfPoly);
	vector<double> bL;
	bL.resize(numOfPoly);
	//vector<double> bR;
	//vector<double> bG;
	//vector<double> bB;
	for (int i = 0; i < numOfPoly; i++)
	{
		int numOfNeighbor = poly_contents[i].neighborPolyIndex.size();
		if( numOfNeighbor != 0)
		{	
			A(i,i,numOfNeighbor+1);
			double l = 0;
			//double r = 0;
			//double g = 0;
			//double b = 0;
			for (int j = 0; j < numOfNeighbor;j++)
			{
				A(i,poly_contents[i].neighborPolyIndex[j],-1);
				int index = poly_contents[i].neighborPolyIndex[j];
				//double dis = 10;
				double ltmp = count_(poly_contents[i].averL,poly_contents[index].averL , dis);
				//double rtmp = count(poly_contents[i].averL,poly_contents[index].averL);
				//double gtmp = count(poly_contents[i].averA,poly_contents[index].averA);
				//double btmp = count(poly_contents[i].averB,poly_contents[index].averB);
				l += ltmp;
				//r += rtmp;
				//g += gtmp;
				//b += btmp;
     		}
			l += poly_contents[i].averL;
			//r += poly_contents[i].averL;
			//g += poly_contents[i].averA;
			//b += poly_contents[i].averB;
			bL[i] = l;
			//bL.push_back(l);
			//bR.push_back(r);
			//bG.push_back(g);
			//bB.push_back(b);
		}
		else
		{
			bL[i] = poly_contents[i].averL;
			//bL.push_back(poly_contents[i].averL);
			//bR.push_back(poly_contents[i].averL);
			//bG.push_back(poly_contents[i].averA);
			//bB.push_back(poly_contents[i].averB);
		}
	
	}
	vector<double> xL = A.solve(bL);
	//vector<double> xR = A.solve(bR);
	//vector<double> xG = A.solve(bG);
	//vector<double> xB = A.solve(bB);
	for( int i=0; i<polyContentSize; i++ ) {
		int exR, exG, exB;
		LAB2RGB(xL[i], poly_contents[i].averA, poly_contents[i].averB, &exR, &exG, &exB);
		poly_contents[i].averL = min(max(0, int(exR)), 255);
        poly_contents[i].averA = min(max(0, int(exG)), 255);
        poly_contents[i].averB = min(max(0, int(exB)), 255);
		//poly_contents[i].averL = min(max(0, int(xR[i])), 255);
  //      poly_contents[i].averA = min(max(0, int(xG[i])), 255);
  //      poly_contents[i].averB = min(max(0, int(xB[i])), 255);
    }
//	Blend blender(poly_contents);
////	FILE *fp = fopen("renderout.txt","w");
//	blender.blendPoly(poly_contents);
//    for( int i=0; i<polyContentSize; i++ ) {
//		poly_contents[i].averL = min(max(0, int(blender.xL[i])), 255);
//        poly_contents[i].averA = min(max(0, int(blender.xA[i])), 255);
//        poly_contents[i].averB = min(max(0, int(blender.xB[i])), 255);
//    }
	t1 = clock();
	cout << "linear solver:"<<t1-t2<<endl;
}

//	cout<<"get average"<<t2-t2<<endl;
//    vector<double> _weights(3*poly_rows);
//    countWeight( poly_contents, _weights );
//	t1 = clock();
//	cout<<"count weight :"<<t1-t1<<endl;
//    int eigen_cols = polyContentSize;
//    int eigen_rows = poly_rows+eigen_cols;
//    MatrixXd A(eigen_rows, eigen_cols);
//    MatrixXd b_r(eigen_rows, 1);
//    MatrixXd b_g(eigen_rows, 1);
//    MatrixXd b_b(eigen_rows, 1);
//    int count = 0;
//    for(int i=0; i<polyContentSize; i++) {
//        for( int j=0; j<poly_contents[i].neighborPolyIndex.size(); j++) {
//            int index_ = poly_contents[i].neighborPolyIndex[j];
//            A(count, i) = 1; A(count, index_) = -1;
//            count++;
//        }
//    }
//    for(int i=0,j=poly_rows; i<polyContentSize; i++, j++ ) { A(j,i) = 1; }
//	t1 = clock();
//    // 对向量b赋值
//    for(int i=0; i<poly_rows; i++) {
//        b_r(i,0) = _weights[3*i];
//        b_g(i,0) = _weights[3*i+1];
//        b_b(i,0) = _weights[3*i+2];
//    }
//    for(int i=0,j=poly_rows; i<polyContentSize; i++, j++ ) {
//        b_r(j,0) = poly_contents[i].averL;
//        b_g(j,0) = poly_contents[i].averA;
//        b_b(j,0) = poly_contents[i].averB;
//    }
//	t2 = clock();
//	cout<<"get B: "<<t2-t1<<endl;
//    // 计算逆矩阵ATA-1，三组向量br bg bb共享同一个逆矩阵
//    MatrixXd AT(eigen_cols, eigen_rows);
//    MatrixXd ATA(eigen_cols, eigen_cols);
//    MatrixXd ATAinv(eigen_cols, eigen_cols);
//	t1 = clock();
//    AT = A.transpose();
//    ATA = AT*A;
//    ATAinv = ATA.inverse();
//	t2 = clock();
//	cout<<"compute ATA-1: "<<t2 - t1<<endl;
//    //  用逆矩阵分别计算三个通道的颜色
//    MatrixXd x_r(eigen_cols, 1);
//    MatrixXd x_g(eigen_cols, 1);
//    MatrixXd x_b(eigen_cols, 1);
//    MatrixXd ATbr(eigen_cols, 1);
//    MatrixXd ATbg(eigen_cols, 1);
//    MatrixXd ATbb(eigen_cols, 1);
//	t1 = clock();
//    ATbr = AT*b_r;
//    ATbg = AT*b_g;
//    ATbb = AT*b_b;
//    x_r = ATAinv*ATbr;
//    x_g = ATAinv*ATbg;
//    x_b = ATAinv*ATbb;
//	t2 = clock();
//	cout<<"get rgb: "<<t2-t1<<endl;
//    for( int i=0; i<polyContentSize; i++ ) {
//        poly_contents[i].averL = min(max(0, int(x_r(i,0))), 255);
//        poly_contents[i].averA = min(max(0, int(x_g(i,0))), 255);
//        poly_contents[i].averB = min(max(0, int(x_b(i,0))), 255);
//    }
//}
void MainWindow::generate(int value , int sValue , double dis)
{
    //绘制low poly 图
    //value：保存前value个点
    vector<VERTEX2D> draw_points;
    int nr_points = points_x.size();

    vector<int> ranks_backup;
    for (unsigned int i = 0; i<ranks.size(); i++) {
        ranks_backup.push_back(ranks[i]);
    }

    for (unsigned int i = 0; i<ranks.size(); i++) {
        int x_ = points_x[i];
        int y_ = points_y[i];
        if (qRed(paintMask->pixel(QPoint(x_, y_)))<10 && ranks[i]>value) {
            VERTEX2D vertex_unit;
            vertex_unit.x = x_;
            vertex_unit.y = y_;
            draw_points.push_back(vertex_unit);
            ranks_backup[i] = 1;
        }
    }

    int nr_remain_points = value - draw_points.size();

    for (int i = 0; i<nr_points; i++) {
        if (ranks_backup[i] <= nr_remain_points) {
            VERTEX2D vertex_unit;
            vertex_unit.x = points_x[i];
            vertex_unit.y = points_y[i];
            draw_points.push_back(vertex_unit);
        }
    }



    MESH mesh;
    CreateMesh(draw_points, &mesh);

    vector<POLYCONTENT> polyContents;

    TRIANGLE_PTR pTri = mesh.pTriArr;
    while (pTri != NULL)
    {
        POLYCONTENT content_unit;
        content_unit.poly.i1 = pTri->i1 - 3;
        content_unit.poly.i2 = pTri->i2 - 3;
        content_unit.poly.i3 = pTri->i3 - 3;
        content_unit.averL = 0; content_unit.averA = 0; content_unit.averB = 0;
        polyContents.push_back(content_unit);

        pTri = pTri->pNext;
    }
 //   QImage *trianglution = new QImage(*imgInput);
 //   QColor white = Qt::white;
 //   trianglution->fill(white);
 //   for (int i = 0 ; i < height ; i++)
 //   {
 //       for (int j = 0 ; j < width; j++)
 //       {
 //           trianglution->setPixel(QPoint(i,j),Qt::white);
 //       }
 //   }

 //   QPainter paintline(trianglution);
 //   paintline.setPen(Qt::black);
 //   //paintline.drawLine(0,0,height,width);
	//Mat out = Mat(height,width,CV_8UC3);
	//for(int y=0;y<height;y++)
 //       for(int x=0;x<width;x++) {
 //           out.at<Vec3b>(y,x) = Vec3b(255,255,255);
 //       }
 //   for (int i = 0; i<polyContents.size(); i++)
 //   {
 //       VERTEX2D ptmp1 , ptmp2, ptmp3;
 //       ptmp1 = draw_points[polyContents[i].poly.i1];
 //       ptmp2 = draw_points[polyContents[i].poly.i2];
 //       ptmp3 = draw_points[polyContents[i].poly.i3];
	//	line(out, Point(ptmp1.x , ptmp1.y) , Point(ptmp2.x , ptmp2.y), Scalar(0,0,0),1 );
	//	line(out, Point(ptmp1.x , ptmp1.y) , Point(ptmp3.x , ptmp3.y), Scalar(0,0,0),1 );
	//	line(out, Point(ptmp3.x , ptmp3.y) , Point(ptmp2.x , ptmp2.y), Scalar(0,0,0),1 );
 //       paintline.drawLine(QPoint(ptmp1.x , ptmp1.y) , QPoint(ptmp2.x , ptmp2.y));
 //       paintline.drawLine(QPoint(ptmp1.x , ptmp1.y) , QPoint(ptmp3.x , ptmp3.y));
 //       paintline.drawLine(QPoint(ptmp3.x , ptmp3.y) , QPoint(ptmp2.x , ptmp2.y));

 //   }
 //   trianglution->save("trianglution.jpg");
	//imwrite("out.bmp",out);
    unsigned char** mask;
    mask = qx_allocu(height, width);
    memset(mask[0], 0, sizeof(unsigned char)*height*width);
    find_pixels_of_each_poly(mask, polyContents, draw_points);
    histogram_equalization_RGB(inputBackup, polyContents , dis);

    for (int i = 0; i<polyContents.size(); i++)
    {
        for (int j = 0; j<polyContents[i].pixels.size(); j++)
        {
            int t_x = polyContents[i].pixels[j].x;
            int t_y = polyContents[i].pixels[j].y;
            QRgb rgb = qRgb(qx_round(polyContents[i].averL), qx_round(polyContents[i].averA), qx_round(polyContents[i].averB));
            imgTemp->setPixel(QPoint(t_x, t_y), rgb);
            imgInput->setPixel(QPoint(t_x, t_y), rgb);
        }
    }



    imgshow->show();
}


void MainWindow::render()
{
    vector<VERTEX2D> draw_points;
    int nr_points = points_x.size();
	clock_t t1 = clock();
    vector<int> ranks_backup;
    for (unsigned int i = 0; i<ranks.size(); i++) {
        ranks_backup.push_back(ranks[i]);
    }

    for (unsigned int i = 0; i<ranks.size(); i++) {
        int x_ = points_x[i];
        int y_ = points_y[i];
        if (qRed(paintMask->pixel(QPoint(x_, y_)))<10 && ranks[i]>node_size) {
            VERTEX2D vertex_unit;
            vertex_unit.x = x_;
            vertex_unit.y = y_;
            draw_points.push_back(vertex_unit);
            ranks_backup[i] = 1;
        }
    }

    int nr_remain_points = node_size - draw_points.size();

    for (int i = 0; i<nr_points; i++) {
        if (ranks_backup[i] <= nr_remain_points) {
            VERTEX2D vertex_unit;
            vertex_unit.x = points_x[i];
            vertex_unit.y = points_y[i];
            draw_points.push_back(vertex_unit);
        }
    }



    MESH mesh;
    CreateMesh(draw_points, &mesh);

    vector<POLYCONTENT> polyContents;

    TRIANGLE_PTR pTri = mesh.pTriArr;
    while (pTri != NULL)
    {
        POLYCONTENT content_unit;
        content_unit.poly.i1 = pTri->i1 - 3;
        content_unit.poly.i2 = pTri->i2 - 3;
        content_unit.poly.i3 = pTri->i3 - 3;
        content_unit.averL = 0; content_unit.averA = 0; content_unit.averB = 0;
        polyContents.push_back(content_unit);

        pTri = pTri->pNext;
    }
	clock_t t2 = clock();
	cout <<"get the top value points:"<<t2-t1<<endl;
    unsigned char** mask;
    mask = qx_allocu(height, width);
    memset(mask[0], 0, sizeof(unsigned char)*height*width);
	t1 = clock();
    find_pixels_of_each_poly(mask, polyContents, draw_points);
	t2 = clock();
	cout<<"find pixels of each poly:"<<t2-t1<<endl;
	double dis = 10;
    histogram_equalization_RGB(inputBackup, polyContents , dis);

    for (int i = 0; i<polyContents.size(); i++)
    {
        for (int j = 0; j<polyContents[i].pixels.size(); j++)
        {
            int t_x = polyContents[i].pixels[j].x;
            int t_y = polyContents[i].pixels[j].y;
            QRgb rgb = qRgb(qx_round(polyContents[i].averL), qx_round(polyContents[i].averA), qx_round(polyContents[i].averB));
            imgTemp->setPixel(QPoint(t_x, t_y), rgb);
            imgInput->setPixel(QPoint(t_x, t_y), rgb);
        }
    }
}
void MainWindow::setDistValue(int value)
{
	double value_ = double(value)/100.0;
	QString pos = QString("%1").arg(value_); // 用arg中的内容替代%i来构建字符串
	ui.distValue->setText(pos);

    dist = value/100;
	generate(node_size, saturValue, dist);
}