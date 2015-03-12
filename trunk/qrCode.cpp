#include "qrCode.h"
#include "define.h"

#include <vector>

using namespace std;
using namespace cv;

//*************************************************************************************************
// QR_recognize
//*************************************************************************************************
//const Vect2D<unsigned> QR_recognize::DIM_FBO(600, 600);
const Vect2D<unsigned> QR_recognize::DIM_FBO(600, 600);
const Vect2D<unsigned> QR_recognize::DIM_QR_IN(1000, 1000);
const Vect4D<float> QR_recognize::BG_COLOR(1.f, 1.f, 1.f, 1.f);
const DATA QR_recognize::BOUND_SCL = 0.3F;

QR_recognize::QR_recognize()
	: m_bInitTx(false), m_txName(0)
{
	bool bDebug = true;

	//*******************************************
	// gpu
	//*******************************************
	if (bDebug) {
		std::cout << "h0" << endl;
	} else {}
	int e_fInit = glfwInit();
	MyAssert(e_fInit == GL_TRUE);

	m_pWin = glfwCreateWindow(DIM_FBO.m_x, DIM_FBO.m_y, "", 0, 0);
	MyAssert(m_pWin != 0);
	glfwMakeContextCurrent(m_pWin);
	glfwHideWindow(m_pWin);

	GLenum e_eInit = glewInit();
	MyAssert(e_eInit == GLEW_OK);

	m_pTxBuf = new unsigned char [DIM_QR_IN.m_x * DIM_QR_IN.m_y];

	//*******************************************
	// texture
	//*******************************************
	// RGBA 2D texture, 24 bit depth texture, 
	if (bDebug) {
		cout << "h1" << endl;
	} else {}
	glGenTextures(1, &m_txColor);
	glBindTexture(GL_TEXTURE_2D, m_txColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// 0 means reserve texture memory, but texels are undefined
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, DIM_FBO.m_x, DIM_FBO.m_y, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	//cout << "h0" << endl;
	glGenFramebuffers(1, &m_frameBuf);
	//cout << "h1" << endl;
	glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuf);
	// attach 2D texture to this FBO
	//cout << "h2" << endl;
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_txColor, 0);

	glGenRenderbuffers(1, &m_depthBuf);
	glBindRenderbuffer(GL_RENDERBUFFER, m_depthBuf);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, DIM_FBO.m_x, DIM_FBO.m_y);
	// attach depth buffer to FBO
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthBuf);

	// does the GPU support current FBO configuration?
	GLenum fboOk = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	MyAssert(fboOk == GL_FRAMEBUFFER_COMPLETE);
}

QR_recognize::~QR_recognize()
{}

void QR_recognize::Render(unsigned char *pIn, unsigned inW, unsigned inH,
						  Vect2D<unsigned> &vRIn, Vect2D<unsigned> &vPIn, Vect2D<unsigned> &vQIn)
{	
	glDisable(GL_DEPTH_TEST);

	//static bool bInitTx = false;
	//static GLuint txName;
	if (!m_bInitTx) {
		m_bInitTx = true;

		glEnable(GL_TEXTURE_2D);
		glGenTextures(1, &m_txName);
		glBindTexture(GL_TEXTURE_2D, m_txName);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //GL_NEAREST);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		//unsigned char pTxBuf[] = {255, 255, 0};	
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, pTxBuf);	
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, DIM_QR_IN.m_x, DIM_QR_IN.m_y, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_pTxBuf);	
	} else {}

	Vect2D<int> aRP((int)vPIn.m_x - vRIn.m_x, 
					(int)vPIn.m_y - vRIn.m_y);
	Vect2D<int> aRQ((int)vQIn.m_x - vRIn.m_x, 
					(int)vQIn.m_y - vRIn.m_y);
	Vect2D<int> vR((int)(vRIn.m_x - aRP.m_x * BOUND_SCL - aRQ.m_x * BOUND_SCL),
				   (int)(vRIn.m_y - aRP.m_y * BOUND_SCL - aRQ.m_y * BOUND_SCL));
	Vect2D<int> vP((int)(vR.m_x + aRP.m_x * (1.F + 2.F * BOUND_SCL)),
				   (int)(vR.m_y + aRP.m_y * (1.F + 2.F * BOUND_SCL)));
	Vect2D<int> vQ((int)(vR.m_x + aRQ.m_x * (1.F + 2.F * BOUND_SCL)),
				   (int)(vR.m_y + aRQ.m_y * (1.F + 2.F * BOUND_SCL))); 
	Vect2D<int> aRP2((int)vP.m_x - vR.m_x, 
					 (int)vP.m_y - vR.m_y);
	Vect2D<int> vR2(aRP2.m_x + vQ.m_x,
					aRP2.m_y + vQ.m_y);
	int xL = vR2.m_x;		int xR = vR2.m_x;
	int yB = vR2.m_y;		int yT = vR2.m_y;
	unsigned aX[] = {vR.m_x, vP.m_x, vQ.m_x};
	unsigned aY[] = {vR.m_y, vP.m_y, vQ.m_y};
	for (unsigned i = 0; i < 3; i++) {
		if ((int)aX[i] < xL) {xL = aX[i];} else {}
		if ((int)aX[i] > xR) {xR = aX[i];} else {}
		if ((int)aY[i] < yB) {yB = aY[i];} else {}
		if ((int)aY[i] > yT) {yT = aY[i];} else {}
	}
	if (xL < 0) {
		xL = 0;
	} else {}
	if (yB < 0) {
		yB = 0;
	} else {}
	if (xR >= (int)inW) {
		xR = inW - 1;
	} else {}
	if (yT >= (int)inH) {
		yT = inH - 1;
	} else {}

	cout << inW << " " << inH << endl;
	cout << xL << " " << xR << " " << yB << " " << yT << endl;
	unsigned xLen = xR - xL + 1;
	unsigned yLen = yT - yB + 1;
	unsigned locTo = 0;
	for (unsigned y = yB; (int)y <= yT; y++) {
		//unsigned yTo = (y - yB) * DIM_QR_IN.m_x;
		unsigned yFrom = y * inW;
		for (unsigned x = xL; (int)x <= xR; x++) {
			//unsigned locTo = yTo + x - xL;
			unsigned locFrom = yFrom + x;
			m_pTxBuf[locTo] = pIn[locFrom];
			locTo++;
		}
	}

	Vect2D<DATA> vPNew((DATA)(vP.m_x - xL) / DIM_QR_IN.m_x,
					   (DATA)(vP.m_y - yB) / DIM_QR_IN.m_y);
	Vect2D<DATA> vQNew((DATA)(vQ.m_x - xL) / DIM_QR_IN.m_x,
					   (DATA)(vQ.m_y - yB) / DIM_QR_IN.m_y);
	Vect2D<DATA> vRNew((DATA)(vR.m_x - xL) / DIM_QR_IN.m_x,
					   (DATA)(vR.m_y - yB) / DIM_QR_IN.m_y);
	Vect2D<DATA> vR2New((DATA)(vR2.m_x - xL) / DIM_QR_IN.m_x,
						(DATA)(vR2.m_y - yB) / DIM_QR_IN.m_y);

	glBindTexture(GL_TEXTURE_2D, m_txName);
	//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, DIM_QR_IN.m_x, DIM_QR_IN.m_y, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_pTxBuf);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, xLen, yLen, GL_LUMINANCE, GL_UNSIGNED_BYTE, m_pTxBuf);
	float xRT = (float)(xLen - 1) / (DIM_QR_IN.m_x - 1);
	float yRT = (float)(yLen - 1) / (DIM_QR_IN.m_y - 1);
	float rt = (xRT > yRT) ? xRT : yRT;
cout << xLen << " " << yLen << endl;
	MyAssert(xRT <= 1.f && yRT <= 1.f);

	glEnable(GL_TEXTURE_2D);
		//glActiveTexture(m_txFace.GetTexID());
	glBindTexture(GL_TEXTURE_2D, m_txName);
	//glBindTexture(GL_TEXTURE_2D, m_pTxFace->GetTexID());

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//const vector<Triangle> &vTri = worldModel.triangleList;
	glBegin(GL_QUADS);
		//glNormal3dv(vTri[i].normal[0]);		
		//glTexCoord2f(0, 0);
		glTexCoord2f((float)vPNew.m_x, (float)vPNew.m_y);
		glVertex2f(-1.f, -1.f);		
			
		//glTexCoord2f(rt, 0);
		glTexCoord2f((float)vRNew.m_x, (float)vRNew.m_y);
		glVertex2f(1.f, -1.f);		
	
		//glTexCoord2f(rt, rt);
		glTexCoord2f((float)vQNew.m_x, (float)vQNew.m_y);
		glVertex2f(1.f, 1.f);		

		//glTexCoord2f(0, rt);
		glTexCoord2f((float)vR2New.m_x, (float)vR2New.m_y);
		glVertex2f(-1.f, 1.f);
	glEnd();

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
}

int QR_recognize::Gen(unsigned char *pOut, unsigned char *pIn, unsigned inW, unsigned inH,
					  Vect2D<unsigned> &vR, Vect2D<unsigned> &vP, Vect2D<unsigned> &vQ)
{
	//m_bLock = true;

	//*******************************************
	// window
	//*******************************************
	glfwMakeContextCurrent(m_pWin);
	//glfwHideWindow(m_pWin);

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	//*******************************************
	// rendering
	//*******************************************
	glClearColor(BG_COLOR.m_r, BG_COLOR.m_g, BG_COLOR.m_b, BG_COLOR.m_a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// my projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glViewport(0, 0, DIM_FBO.m_x, DIM_FBO.m_y);

	glEnable(GL_DEPTH_TEST);
	Render(pIn, inW, inH, vR, vP, vQ);

	glDisable(GL_DEPTH_TEST);
	
	// read pixels
	glPixelTransferf(GL_RED_SCALE,		0.30f);
	glPixelTransferf(GL_GREEN_SCALE,	0.59f);
	glPixelTransferf(GL_BLUE_SCALE,		0.11f);

	glReadPixels(0, 0, DIM_FBO.m_x, DIM_FBO.m_y, GL_LUMINANCE, GL_UNSIGNED_BYTE, pOut);

	glPixelTransferf(GL_RED_SCALE,		1.f);
	glPixelTransferf(GL_GREEN_SCALE,	1.f);
	glPixelTransferf(GL_BLUE_SCALE,		1.f);

	return 0;
}

void QR_recognize::GetMemory(unsigned char **ppMemory, unsigned &mW, unsigned &mH, unsigned &mC)
{
	mW = DIM_FBO.m_x;
	mH = DIM_FBO.m_y;
	mC = 1;
	*ppMemory = new unsigned char [mW * mH * mC];
}

//*************************************************************************************************
// QR_detect
//*************************************************************************************************

//const vector<unsigned> DIM_BUF(1000, 1000);
const Vect2D<unsigned> QR_detect::DIM_MIN_DETECT(10, 10); //16, 16);
const float			   QR_detect::ANGULAR_TOLERANCE = 0.5f; //0.25f;
const float			   QR_detect::LINEAR_TOLERANCE = 0.5f; // 0.4f;

void stage2(vector<Vect2D<unsigned>> &aP, vector<Vect2D<unsigned>> &aQ, vector<Vect2D<unsigned>> &aR,
			Mat& img,double scale, vector<Rect>& stage1_res,float angular_tolerance, float linear_tolerance);

QR_detect::QR_detect()
	//: m_matBuf(DIM_BUF.m_y, DIM_BUF.m_x, CV_8UC1)
{
	m_ccClassf.load("qrD.xml");
	//m_ccClassf.load("qr2.xml");
}

QR_detect::~QR_detect()
{
	//m_matBuf.release();
}

void QR_detect::Gen(vector<Vect2D<unsigned>> &aP, vector<Vect2D<unsigned>> &aQ, vector<Vect2D<unsigned>> &aR, 
					unsigned char *pGray, unsigned inW, unsigned inH) 
{
	//MyAssert(inW <= DIM_BUF.m_x &&
	//		 inH <= DIM_BUF.m_y);

	//Rect subRect(0, 0, inH, inW);
	//Mat matIn(m_matBuf, subRect);
	//matIn.c

	Mat matIn(inH, inW, CV_8UC1, pGray);

    equalizeHist(matIn, matIn);
  
	vector<Rect> stage1_res;
    m_ccClassf.detectMultiScale(matIn, stage1_res,
        1.10,1, 0
        //|CV_HAAR_FIND_BIGGEST_OBJECT
        //|CV_HAAR_DO_CANNY_PRUNING
        //|CV_HAAR_DO_ROUGH_SEARCH
        //|CV_HAAR_SCALE_IMAGE
        |CASCADE_SCALE_IMAGE
        ,
		Size(DIM_MIN_DETECT.m_x, DIM_MIN_DETECT.m_y) ); // menor tamanho!
   
    stage2(aP, aQ, aR, matIn, 1.F, stage1_res, ANGULAR_TOLERANCE, LINEAR_TOLERANCE);
    
     //cv::imshow( "Processed Image", img );

     // waitKey(0);
     // cv::imshow( "Processed Image", smallImg );
  
    //imwrite("/tmp/output.jpg", img);
}

//*************************************************************************************************
// main detection below
//*************************************************************************************************

/*
void detectAndDraw( Mat& img,
                   CascadeClassifier& cascade, CascadeClassifier& nestedCascade,
                   double scale,
                   int minimum_size,
                   int minrep,float rang,float rlin,
                   CvVideoWriter* writer,
                   IplImage* ipl_img)
{
    double t = 0;
    vector<Rect> stage1_res;
    Size img_size = img.size();

    Mat gray, smallImg( cvRound (img.rows/scale), cvRound(img.cols/scale), CV_8UC1 );

    cvtColor( img, gray, CV_BGR2GRAY );
    resize( gray, smallImg, smallImg.size(), 0, 0, INTER_LINEAR );
    equalizeHist( smallImg, smallImg );


   
    
    t = (double)cvGetTickCount();
  
    cascade.detectMultiScale( smallImg, stage1_res,
        1.10,1, 0
        //|CV_HAAR_FIND_BIGGEST_OBJECT
        //|CV_HAAR_DO_CANNY_PRUNING
        //|CV_HAAR_DO_ROUGH_SEARCH
        //|CV_HAAR_SCALE_IMAGE
        |CASCADE_SCALE_IMAGE
        ,
        Size(minimum_size,minimum_size) ); // menor tamanho!
    t = (double)cvGetTickCount() - t;
    
    if (DEBUG){
    printf( "detection time = %g ms\n", t/((double)cvGetTickFrequency()*1000.) );
    }

    if (DEBUG){
    printf( "%d\tTime Elapsed (First Stage) = %g ms\t", minimum_size, t/((double)cvGetTickFrequency()*1000.) );
    }
    
    t = (double)cvGetTickCount();
    stage2(img, scale, stage1_res, rang, rlin);
    t = (double)cvGetTickCount() - t;

    if (DEBUG){
        printf( "Time Elapsed (Second Stage) = %g ms\n", t/((double)cvGetTickFrequency()*1000.) );
    }

    
    // ************************************************************************************************************************
    
     cv::imshow( "Processed Image", img );
     // waitKey(0);
     // cv::imshow( "Processed Image", smallImg );
  
    //imwrite("/tmp/output.jpg", img);
}
*/

#define MAX_FIPS 500
#define DEBUG false

bool scan_proportions(int scan_seg[], float tolerance){
   float average = 0;
   int i;
   int scan_lenght=6;
   int s[5];
   
   
   if (DEBUG){
   printf("\n[");
   }
   
   for(i=0;i<(scan_lenght-1); i++){ // calculates the lenght of the segments s0...s5
       s[i]= scan_seg[i+1]-scan_seg[i];
       average = average + s[i];
       
       if (DEBUG){
        printf("%d\t",s[i]);
       }
       
   }
   
   average= average / 7 ; // A FIP has 7 modules: 1 + 1 + 3 + 1 + 1 
   
   if (DEBUG){
        printf ("]\t M=%f, Tol=%f",average,average*tolerance);
   }
   
   tolerance = tolerance + (1.f/(average*0.9f)); // Here we try to compensate rounding error that is too big for small values.
   
   if (DEBUG){
        printf ("\tTolC=%f\n",average*tolerance);
   }
   
   
           
   for(i=0;i<(scan_lenght-1); i++){ // calculates the length of the segments s0...s5
       
       if (i==2) { // treats the middle segment as a special case because it should be 3 times larger
           if  (abs(average - (s[i]/3)) > abs(tolerance*average)){
               return false;
           }
           
       }else if (abs(average - s[i]) > abs(tolerance*average)){ 
           return false;
       }
       
   }
   
   
   return true;
   
   
   
}

void stage2(vector<Vect2D<unsigned>> &aP, vector<Vect2D<unsigned>> &aQ, vector<Vect2D<unsigned>> &aR,
			Mat& img,double scale, vector<Rect>& stage1_res,float angular_tolerance, float linear_tolerance){

    CvPoint pt1,pt2,/*pt3,*/pt_scan1,pt_scan2,point_a,point_b;
    int i,j,k,p,q,r,pivotp,pivotq,/*npivot,*/total_candidates,count,vertices,angle_type,vertex_to_look_for1,vertex_to_look_for2 = 0;
    const static Scalar colors[] =  { CV_RGB(0,0,255),
        CV_RGB(0,128,255),
        CV_RGB(0,255,255),
        CV_RGB(0,255,0),
        CV_RGB(255,128,0),
        CV_RGB(255,255,0),
        CV_RGB(255,0,0),
        CV_RGB(255,0,255)} ;

    float size_tolerance;
    float angle;
    float dotp,px,py,qx,qy,xcommon,ycommon,sizep,sizeq,min_ratio45,min_ratio90,max_ratio45, max_ratio90,min_ratio,max_ratio;
    float m_centers_sizes[MAX_FIPS][3];
    char string[20];
    int m_lines [MAX_FIPS][3];
    int line_length;
    int process_qr_code =1;
    int m_found_vertices[MAX_FIPS*5][3];
    int max_qrcode_version = 40;
    Vec3b intensity;
    int scan1,scan2,average;
    int scan_seg[6];
    CvFont font;
    cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 1.0, 1.0, 0, 1, CV_AA);


    
    angular_tolerance = (angular_tolerance) ? angular_tolerance : 0.15f ;       // Default values for angular and linear tolerances
    linear_tolerance = (linear_tolerance) ? linear_tolerance : 0.25f ;       // Default values for angular and linear tolerances

    size_tolerance = linear_tolerance; 
    min_ratio45=cos(0.78f+(angular_tolerance));
    max_ratio45=cos(0.78f-(angular_tolerance));
    min_ratio90=1-( (cos(0.78f-(angular_tolerance)) - cos(0.78f+(angular_tolerance)))/cos(0.78f) );
    max_ratio90=1.01f; 

    i=0;
    for( vector<Rect>::const_iterator r = stage1_res.begin(); r != stage1_res.end(); r++, i++ )
    {
        Mat smallImgROI;
        vector<Rect> nestedObjects;
        Point center;
        Scalar color = colors[i%8];
        int radius;
        int /*scan_x,*/scan_y,scan_points;
        center.x = cvRound((r->x + r->width*0.5)*scale);
        center.y = cvRound((r->y + r->height*0.5)*scale);
        radius = cvRound((r->width + r->height)*0.25*scale);
       
	pt1.x=cvRound((r->x)*scale);
	pt1.y=cvRound((r->y)*scale);
	pt2.x=cvRound((r->x + r->width)*scale);
	pt2.y=cvRound((r->y + r->height)*scale);
     
	
        
        // Scan through a line inside the detected rectangle, looking for black/white transitions in order to discard false positives

        pt_scan1.x=max(pt1.x+(pt1.x-pt2.x)/8,0); // begin the scan line 1/8L before the LxL detected square
        pt_scan1.y=scan_y=cvRound((pt1.y + pt2.y)/2);
        pt_scan2.x=pt_scan1.x;
        pt_scan2.y=pt_scan1.y;
        if (DEBUG){
                printf("\n");
        }
        
        average=0;
        
        
        
        // This loop only calculates the average of pixel intensities trough the scanline
        scan_points=0;
        while (pt_scan1.x<(pt2.x-(pt1.x-pt2.x)/8)){ // ends the scan line after 1/8L of the the LxL detected square
            
            
            pt_scan2.x=pt_scan1.x;
            pt_scan2.y=pt_scan1.y;

            pt_scan1.x++;
            
            scan1=cvRound(img.at<Vec3b>(pt_scan1)[0]*0.114 + img.at<Vec3b>(pt_scan1)[1]*0.587 + img.at<Vec3b>(pt_scan1)[2]*0.299 );
            scan2=cvRound(img.at<Vec3b>(pt_scan2)[0]*0.114 + img.at<Vec3b>(pt_scan2)[1]*0.587 + img.at<Vec3b>(pt_scan2)[2]*0.299 );
            // Y´ = 0.299 * R´ + 0.587 * G´ + 0.114 * B´
        if (DEBUG){
            printf("%d:\t|%d|\t%d\n",(pt_scan1.x-pt1.x),scan1,(scan1-scan2)) ;
        }
            
            
            average=average+scan1;
            scan_points++;
            
            
        }
        
        average = (int)floor(average / scan_points);
            
        if (DEBUG){
                printf("average:%d\n",average);
        }
        
        
    
        
        
        //int scan_vec[scan_points],it;
		vector<int> scan_vec(scan_points);
		int it;
        
        for (it=0;it<scan_points;it++){
            scan_vec[it]=0;
        }
        
        
        
        // This loop identifies transitions
        
        
        pt_scan1.x=max(pt1.x+(pt1.x-pt2.x)/8,0);
        pt_scan1.y=scan_y=cvRound((pt1.y + pt2.y)/2);
        pt_scan2.x=pt_scan1.x;
        pt_scan2.y=pt_scan1.y;
        
        
        
        it=0;
        while (pt_scan1.x<(pt2.x-(pt1.x-pt2.x)/8)){ // TODO: take care of image borders
            
            
            pt_scan2.x=pt_scan1.x;
            pt_scan2.y=pt_scan1.y;

            pt_scan1.x++;
            
            
            
            scan1=cvRound(img.at<Vec3b>(pt_scan1)[0]*0.114 + img.at<Vec3b>(pt_scan1)[1]*0.587 + img.at<Vec3b>(pt_scan1)[2]*0.299 );
            scan2=cvRound(img.at<Vec3b>(pt_scan2)[0]*0.114 + img.at<Vec3b>(pt_scan2)[1]*0.587 + img.at<Vec3b>(pt_scan2)[2]*0.299 );
     
        
            if ((scan1>=average && scan2<average)){ // black to white transition
            
                scan_vec[it]= +1;
                
                
            }else if ((scan1<average && scan2>=average)){ // white to black transition
            
                scan_vec[it]= -1;
                
            }
            it++;
    }
        // -------------------------------------------------
        
        
        
        it=cvRound(scan_points/2);
        int esq_OK=0,dir_OK=0;
        
        if (DEBUG){
                printf("\nLeft:");
        }
        
        
        while (it>=0 && esq_OK<3){ //looks for the first 3 transitions, from the midle to the left ( - + - )
        
            if (DEBUG){
                printf("%d",esq_OK);
            }
            
            if (esq_OK==0){
            
                if (scan_vec[it]==0){
                        it--;            
                }else if (scan_vec[it]<0){
                        esq_OK++;      
                        scan_seg[2]=it; 
                        it--; 
                }else if (scan_vec[it]>0){
                        it=-1;
                }
            
            }else if (esq_OK==1){
            
                if (scan_vec[it]==0){
                        it--;            
                }else if (scan_vec[it]>0){
                        esq_OK++;   
                        scan_seg[1]=it; 
                        it--; 
                }else if (scan_vec[it]<0){
                        it=-1;
                }
            
            }else if (esq_OK==2){
            
                if (scan_vec[it]==0){
                        it--;            
                }else if (scan_vec[it]<0){
                        esq_OK++;  
                        scan_seg[0]=it; 
                        it--; 
                }else if (scan_vec[it]>0){
                        it=-1;
                }
            }
                        
        }
        
        it=cvRound(scan_points/2);
        if (DEBUG){
            printf("\nRight:");
        }
        
        while (it<=scan_points && dir_OK<3){ //looks for the first 3 transitions, from the midle to the right ( + - + )
        
            if (DEBUG){
                printf("%d",dir_OK);
            }
             
            if (dir_OK==0){
            
                if (scan_vec[it]==0){
                        it++;            
                }else if (scan_vec[it]>0){
                        dir_OK++;    
                        scan_seg[3]=it; 
                        it++; 
                }else if (scan_vec[it]<0){
                        it=scan_points+1;
                }
            
            }else if (dir_OK==1){
            
                if (scan_vec[it]==0){
                        it++;            
                }else if (scan_vec[it]<0){
                        dir_OK++;     
                        scan_seg[4]=it;
                        it++; 
                }else if (scan_vec[it]>0){
                        it=scan_points+1;
                }
            
            }else if (dir_OK==2){
            
                if (scan_vec[it]==0){
                        it++;            
                }else if (scan_vec[it]>0){
                        dir_OK++; 
                        scan_seg[5]=it; 
                        it++; 
                }else if (scan_vec[it]<0){
                        it=scan_points+1;
                }
            }
                        
        }
        
        
        if (DEBUG){
                printf("\n%d:[",i);
      
                for (it=0;it<scan_points;it++){
                        printf("%d",scan_vec[it]);
                }
                printf("]\n");
          
        
                rectangle( img, pt1 , pt2, color, 1 ); 
        
         
                sprintf(string,"%d",i);
                putText(img, string, center, FONT_HERSHEY_SIMPLEX,1, color,3,1,0);

        }
        
        if (DEBUG && !(esq_OK==3 && dir_OK==3)){ //indicates that the candidate was discarded by not presenting the correct order of transitions
            
            point_a.x=pt1.x;
            point_a.y=pt1.y;
            point_b.x=pt2.x;
            point_b.y=pt2.y;
                                
            line( img,  point_a ,point_b , colors[4], 2,4,0 );
            
        }else if (DEBUG && !scan_proportions(scan_seg,0.4f)){ //indicates that the candidate was discarded by not presenting the correct proportions of transitions
            
            point_a.x=pt2.x; // line in the other diagonal
            point_a.y=pt1.y;
            point_b.x=pt1.x; // line in the other diagonal
            point_b.y=pt2.y;
                                
            line( img,  point_a ,point_b , colors[2], 2,4,0 ); //--> Beware! It changes the image to be processed!
            
        }else{ // process the candidate as it was approved in the transition criteria
       
        
        
        m_centers_sizes[i][0]=(float)center.x;
        m_centers_sizes[i][1]=(float)center.y;
        m_centers_sizes[i][2]=(float)radius;


        
        }
            
    }

    if (DEBUG){
        printf("Found FIPs: %d\nRang=%f\tRlin=%f\n",i,angular_tolerance,linear_tolerance);
    }
    

    if (process_qr_code) {

    total_candidates=i;
    count=0;
    for (i=0;i<total_candidates;i++){
        for(j=i+1;j<total_candidates;j++){

            if (DEBUG){
                printf("\ni=%d,j=%d,tami=%f;tamj=%f;",i,j,m_centers_sizes[i][2],m_centers_sizes[j][2]);
            }

            if (abs(m_centers_sizes[i][2] - m_centers_sizes[j][2]) < (m_centers_sizes[i][2]*size_tolerance)){ 
                pt1.x=cvRound(m_centers_sizes[i][0]);
                pt1.y=cvRound(m_centers_sizes[i][1]);
                pt2.x=cvRound(m_centers_sizes[j][0]);
                pt2.y=cvRound(m_centers_sizes[j][1]);

                line_length=(int)sqrt(pow((pt1.x-pt2.x),2)+pow((pt1.y-pt2.y),2));
                
                if ((line_length < (((max_qrcode_version-1)*4 -7 +21)/9)*sqrt(2) * m_centers_sizes[i][2]) && (line_length > (1.6) * m_centers_sizes[i][2])  ) { 

                    m_lines[count][0]=i;
                    m_lines[count][1]=j;
                    m_lines[count][2]=line_length;
                    
                    if (DEBUG){
                        line(img,pt1,pt2,colors[0],1,4,0);
                    }
                    
                    count++;
                }
            }

        }
    }

    vertices=0;
    for (p=0;p<count;p++){
        for (q=p+1;q<count;q++){
            // PIVOT CALCULATION
            pivotp=-1;
            if ((m_lines[p][0]==m_lines[q][0])){
                pivotp=0;
                pivotq=0;
            }else if (m_lines[p][0] == m_lines[q][1]) { // common point in the two lines
                pivotp=0;
                pivotq=1;
            }else if (m_lines[p][1] == m_lines[q][0]) { // common point in the two lines
                pivotp=1;
                pivotq=0;
            }else if (m_lines[p][1] == m_lines[q][1]) { // common point in the two lines
                pivotp=1;
                pivotq=1;
            }

            if (pivotp>=0){
                     
                xcommon=m_centers_sizes[m_lines[p][pivotp]][0];
                ycommon=m_centers_sizes[m_lines[p][pivotp]][1];

                px=m_centers_sizes[m_lines[p][1-pivotp]][0];
                py=m_centers_sizes[m_lines[p][1-pivotp]][1];
                qx=m_centers_sizes[m_lines[q][1-pivotq]][0];
                qy=m_centers_sizes[m_lines[q][1-pivotq]][1];
                sizep=(float)m_lines[p][2];
                sizeq=(float)m_lines[q][2];

               
                dotp=((px-xcommon)*(qx-xcommon)/(sizep*sizeq)+(py-ycommon)*(qy-ycommon)/(sizep*sizeq));

                if (dotp>1){ // sometimes a rounding problem may make the dotp to be larger than 1
                    dotp=1;
                }

                angle=acos(dotp);
                if (DEBUG){

                    printf("\n angulo = %f \t %d->%d->%d\t%f\t%f <t%d", angle,m_lines[p][1-pivotp],m_lines[p][pivotp],m_lines[q][1-pivotq],(sizep/sizeq),(sizeq/sizep),p);
                }
                  
                

                angle_type=0;
                if ((abs(angle)>(0.78 - angular_tolerance) && abs(angle)<(0.78 + angular_tolerance))){ 
                    min_ratio=min_ratio45;
                    max_ratio=max_ratio45;
                    angle_type = 45;
                }else if (angle>(1.57 - angular_tolerance) && angle<(1.57 + angular_tolerance)){
                    min_ratio=min_ratio90;
                    max_ratio=max_ratio90;
                   
                    angle_type=90;
                }


                if (angle_type>0){
                    if (((sizep/sizeq)>min_ratio && (sizep/sizeq)<max_ratio) || ((sizeq/sizep)>min_ratio && (sizeq/sizep)<max_ratio)){

                        m_found_vertices[vertices][0] = angle_type; // creates new vertex of 45 or 90 degrees
                        m_found_vertices[vertices][1] = min(p,q); // creates new vertex of 45 or 90 degrees
                        m_found_vertices[vertices][2] = max(p,q); // creates new vertex of 45 or 90 degrees

                        if (DEBUG){
                        printf("\nVertex[%d] = %d |\t%d\t%d\t(%f)",vertices,m_found_vertices[vertices][0],m_found_vertices[vertices][1],m_found_vertices[vertices][2], (angle*57.29));
                        }

                        vertices++;


                    }
            }
           }
        }

        }

	aP.clear();
	aQ.clear();
	aR.clear();
    for(i=0; i<vertices; i++){
    
        if (m_found_vertices[i][0] == 90){
            
            p=min(m_found_vertices[i][1],m_found_vertices[i][2]);
            q=max(m_found_vertices[i][1],m_found_vertices[i][2]);
            
            for(j=0;m_found_vertices[j][1]<=p;j++){ // For each pair of lines (45,p,R), search for (45,q,R). If it exists, then you found it!
                vertex_to_look_for1=-1;
                if ((m_found_vertices[j][1] == p ) && (m_found_vertices[j][0] == 45)){ // Found a (45,p,R)
                    r=m_found_vertices[j][2];
                    vertex_to_look_for1=min(p,r);
                    vertex_to_look_for2=max(p,r);
                }else if((m_found_vertices[j][2] == p ) && (m_found_vertices[j][0] == 45)){ // Found a (45,R,p)
                    r=m_found_vertices[j][1];
                    vertex_to_look_for1=min(p,r);
                    vertex_to_look_for2=max(p,r);
                }

                    if (vertex_to_look_for1>=0){
                    for(k=0;m_found_vertices[k][1]<=vertex_to_look_for1;k++){
                        if ((m_found_vertices[k][1]==vertex_to_look_for1) && (m_found_vertices[k][0]==45) && (m_found_vertices[k][2]==vertex_to_look_for2)){

                            
                            // The "if" verifies if R has a common extremity with P and Q
 
                            // PIVOT CALCULATION
                            pivotp=-1;
                            if ((m_lines[p][0]==m_lines[q][0])){
                                pivotp=0;
                                pivotq=0;
                            }else if (m_lines[p][0] == m_lines[q][1]) { 
                                pivotp=0;
                                pivotq=1;
                            }else if (m_lines[p][1] == m_lines[q][0]) { 
                                pivotp=1;
                                pivotq=0;
                            }else if (m_lines[p][1] == m_lines[q][1]) { 
                                pivotp=1;
                                pivotq=1;
                            }


                px=m_centers_sizes[m_lines[p][1-pivotp]][0];
                py=m_centers_sizes[m_lines[p][1-pivotp]][1];
                qx=m_centers_sizes[m_lines[q][1-pivotq]][0];
                qy=m_centers_sizes[m_lines[q][1-pivotq]][1];
								xcommon=m_centers_sizes[m_lines[p][pivotp]][0];
                                ycommon=m_centers_sizes[m_lines[p][pivotp]][1];
                                //printf("%d_%d_%d_%d_%d_%d ",(int)px,(int)py,(int)xcommon,(int)ycommon,(int)qx,(int)qy);
								Vect2D<unsigned> ptP((unsigned)px, (unsigned)py);				aP.push_back(ptP); 
								Vect2D<unsigned> ptQ((unsigned)qx, (unsigned)qy);				aQ.push_back(ptQ);
								Vect2D<unsigned> ptR((unsigned)xcommon,	(unsigned)ycommon);		aR.push_back(ptR);

                            if(   (    m_lines[r][0]==m_lines[p][1-pivotp]
                                    || m_lines[r][1]==m_lines[p][1-pivotp])
                               && (    m_lines[r][0]==m_lines[q][1-pivotq]
                                    || m_lines[r][1]==m_lines[q][1-pivotq]
                                    ))
                                   
                            {
                                xcommon=m_centers_sizes[m_lines[p][pivotp]][0];
                                ycommon=m_centers_sizes[m_lines[p][pivotp]][1];

                                if (DEBUG){
                                printf("\nQR code detected: %d %d %d\n",p,q,r);
                                }

                                //printf("%d_%d_%d_%d_%d_%d ",(int)px,(int)py,(int)xcommon,(int)ycommon,(int)qx,(int)qy);
								//Vect2D<unsigned> ptP((unsigned)px, (unsigned)py);				aP.push_back(ptP); 
								//Vect2D<unsigned> ptQ((unsigned)qx, (unsigned)qy);				aQ.push_back(ptQ);
								//Vect2D<unsigned> ptR((unsigned)xcommon,	(unsigned)ycommon);		aR.push_back(ptR);

								/*
                                //Draw the Triangle

                                
                                pt1.x=cvRound(m_centers_sizes[m_lines[p][0]][0]);
                                pt1.y=cvRound(m_centers_sizes[m_lines[p][0]][1]);

                                pt2.x=cvRound(m_centers_sizes[m_lines[p][1]][0]);
                                pt2.y=cvRound(m_centers_sizes[m_lines[p][1]][1]);

                                line(img,pt1,pt2,colors[6],3,4,0);

                                pt1.x=cvRound(m_centers_sizes[m_lines[q][0]][0]);
                                pt1.y=cvRound(m_centers_sizes[m_lines[q][0]][1]);

                                pt2.x=cvRound(m_centers_sizes[m_lines[q][1]][0]);
                                pt2.y=cvRound(m_centers_sizes[m_lines[q][1]][1]);

                                line(img,pt1,pt2,colors[6],3,4,0);

                                pt1.x=cvRound(m_centers_sizes[m_lines[r][0]][0]);
                                pt1.y=cvRound(m_centers_sizes[m_lines[r][0]][1]);

                                pt2.x=cvRound(m_centers_sizes[m_lines[r][1]][0]);
                                pt2.y=cvRound(m_centers_sizes[m_lines[r][1]][1]);

                                line(img,pt1,pt2,colors[6],3,4,0);
								*/
                            }

                        }
                    }

                    
                }
            }
                
        }


     }



        }

    if (DEBUG){
     printf("rang=%f / rlin=%f\nmin45\tmax45\tmin90\tmax90\n%f\t%f\t%f\t%f",angular_tolerance,linear_tolerance,min_ratio45,max_ratio45,min_ratio90,max_ratio90);
    }
}

/*
void detectAndDraw(vector<CvPoint> &aP, vector<CvPoint> &aQ, vector<CvPoint> &aR,
				   Mat& img,
                   CascadeClassifier& cascade, CascadeClassifier& nestedCascade,
                   double scale,
                   int minimum_size,
                   int minrep,float rang,float rlin,
                   CvVideoWriter* writer,
                   IplImage* ipl_img)
{
    double t = 0;
    vector<Rect> stage1_res;
    Size img_size = img.size();

    Mat gray, smallImg( cvRound (img.rows/scale), cvRound(img.cols/scale), CV_8UC1 );

    cvtColor( img, gray, CV_BGR2GRAY );
    resize( gray, smallImg, smallImg.size(), 0, 0, INTER_LINEAR );
    equalizeHist( smallImg, smallImg );


   
    
    t = (double)cvGetTickCount();
  
    cascade.detectMultiScale( smallImg, stage1_res,
        1.10,1, 0
        //|CV_HAAR_FIND_BIGGEST_OBJECT
        //|CV_HAAR_DO_CANNY_PRUNING
        //|CV_HAAR_DO_ROUGH_SEARCH
        //|CV_HAAR_SCALE_IMAGE
        |CASCADE_SCALE_IMAGE
        ,
        Size(minimum_size,minimum_size) ); // menor tamanho!
    t = (double)cvGetTickCount() - t;
    
    if (DEBUG){
    printf( "detection time = %g ms\n", t/((double)cvGetTickFrequency()*1000.) );
    }

    if (DEBUG){
    printf( "%d\tTime Elapsed (First Stage) = %g ms\t", minimum_size, t/((double)cvGetTickFrequency()*1000.) );
    }
    
    t = (double)cvGetTickCount();
    stage2(aP, aQ, aR, img, scale, stage1_res, rang, rlin);
    t = (double)cvGetTickCount() - t;

    if (DEBUG){
        printf( "Time Elapsed (Second Stage) = %g ms\n", t/((double)cvGetTickFrequency()*1000.) );
    }

    
    // ************************************************************************************************************************
    
     cv::imshow( "Processed Image", img );
     // waitKey(0);
     // cv::imshow( "Processed Image", smallImg );
  
    //imwrite("/tmp/output.jpg", img);
}
*/