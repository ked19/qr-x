#ifndef _MY_ALGO_H
#define _MY_ALGO_H

#include <dlib/image_processing/frontal_face_detector.h>
#include <vector>

#include "define.h"
#include "denseMatrix.h"
#include "matrixOperation.h"

using namespace std;
using namespace dlib;

class MyFace
{
public:
	MyFace();
	~MyFace();

	void Detect(std::vector<rectangle> &aRect, 
				unsigned char *pBuf, unsigned bufW, unsigned bufH, unsigned bufC, 
				unsigned imgW, unsigned imgH, 
				bool bRGB = true,
				bool bDouble = false);

	void Detect(std::vector<rectangle> &aRect, 
				unsigned char *pGray, unsigned char *pRgb, 
				Mtx &mtxSkin, Mtx &mtxLab, unsigned scl,
				unsigned imgW, unsigned imgH, 
				bool bRGB = true,
				bool bDouble = false);

	void DetectLocal(std::vector<rectangle> &aRect, 
				     unsigned char *pBuf, unsigned bufW, unsigned bufH,  
					 unsigned xL, unsigned yB, unsigned imgW, unsigned imgH);

private:
	static const unsigned BUF_LEN;

	frontal_face_detector m_fd;
	array2d<unsigned char> m_arrGray;
};

#endif