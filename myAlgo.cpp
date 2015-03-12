#include "myAlgo.h"

const unsigned MyFace::BUF_LEN = 2000;

MyFace::MyFace()
	: m_arrGray(BUF_LEN, BUF_LEN)
{
	m_fd = get_frontal_face_detector();
}

MyFace::~MyFace()
{}

/*
void MyFace::Detect(std::vector<rectangle> &aRect, Mtx &mtxIn)
{
	Vect2D<unsigned> dimIn = mtxIn.GetDim();
	MyAssert(dimIn.m_x <= BUF_LEN &&
			 dimIn.m_y <= BUF_LEN);

	m_arrGray.set_size(dimIn.m_y, dimIn.m_x);
	for (unsigned y = 0; y < dimIn.m_y; y++) {
		unsigned invY = dimIn.m_y - 1 - y;
		for (unsigned x = 0; x < dimIn.m_x; x++) {
			m_arrGray[invY][x] = mtxIn.CellVal(x, y);
		}
	}

	aRect = m_fd(m_arrGray);
}
*/
/*
void MyFace::Detect(std::vector<rectangle> &aRect, 
					unsigned char *pImg, unsigned imgW, unsigned imgH, unsigned imgC, bool bDouble)
{
	MyAssert(imgW <= BUF_LEN &&
			 imgH <= BUF_LEN && 
			 imgC == 3);

	m_arrGray.set_size(imgH, imgW);

	unsigned loc = 0;
	for (unsigned y = 0; y < imgH; y++) {
		for (unsigned x = 0; x < imgW; x++) {
			m_arrGray[y][x] = (unsigned char)(pImg[loc + 0] * 0.30F
											+ pImg[loc + 1] * 0.59F
											+ pImg[loc + 2] * 0.11F);
			loc += 3;
		}
	}

	if (bDouble) {
		pyramid_up(m_arrGray);
	} else {}
	
	aRect = m_fd(m_arrGray);
	
	if (bDouble) {
		for (unsigned r = 0; r < aRect.size(); r++) {
			aRect[r].left() /= 2;
			aRect[r].right() /= 2;
			aRect[r].bottom() /= 2;
			aRect[r].top() /= 2;
		}
	} else {}
}
*/
void MyFace::Detect(std::vector<rectangle> &aRect, 
					unsigned char *pBuf, unsigned bufW, unsigned bufH, unsigned bufC, 
					unsigned imgW, unsigned imgH, bool bRGB, bool bDouble)
{
	MyAssert(imgW <= BUF_LEN &&
			 imgH <= BUF_LEN);

	unsigned scl = 2;
	m_arrGray.set_size(imgH / scl, imgW / scl);

	if (bufC == 1) {
		unsigned yLoc = 0;
		for (unsigned y = 0; y < imgH / scl; y++) {
			unsigned xLoc = yLoc;
			for (unsigned x = 0; x < imgW / scl; x++) {
				m_arrGray[y][x] = pBuf[xLoc];
				xLoc += scl;
			}
			yLoc += bufW * scl;
		}
	} else if (bufC == 3) {
		unsigned yLen = bufW * bufC;
		unsigned yLoc = 0;
		for (unsigned y = 0; y < imgH / scl; y++) {
			unsigned xLoc = yLoc;
			for (unsigned x = 0; x < imgW / scl; x++) {
				if (bRGB) {
					m_arrGray[y][x] = (unsigned char)(pBuf[xLoc + 0] * 0.30F
													+ pBuf[xLoc + 1] * 0.59F
													+ pBuf[xLoc + 2] * 0.11F);
				} else {
					m_arrGray[y][x] = (unsigned char)(pBuf[xLoc + 2] * 0.30F
													+ pBuf[xLoc + 1] * 0.59F
													+ pBuf[xLoc + 0] * 0.11F);
				}
				xLoc += bufC * scl;
			}
			yLoc += yLen * scl;
		}
	} else {
		MyAssert(0);
	}

	if (scl == 1) {}
	else if (scl == 2) {
		pyramid_up(m_arrGray);
	} else {
		MyAssert(0);
	}
	
	aRect = m_fd(m_arrGray);
}

void BeSkin(Mtx &mtxSkin, unsigned char *pRgb, unsigned w, unsigned h, unsigned scl, bool bRGB)
{
	unsigned loc = 0;
	for (unsigned y = 0; y < h / scl; y++) {
		for (unsigned x = 0; x < w / scl; x++) {
			unsigned char aV[3];  
			if (bRGB) {
				aV[0] = pRgb[loc + 0];
				aV[1] = pRgb[loc + 1];
				aV[2] = pRgb[loc + 2];
			} else {
				aV[2] = pRgb[loc + 0];
				aV[1] = pRgb[loc + 1];
				aV[0] = pRgb[loc + 2];
			}
			loc += 3 * scl;

			unsigned char vMin = (aV[0] < aV[1]) ? aV[0] : aV[1];
			unsigned char vMax = (aV[0] > aV[1]) ? aV[0] : aV[1];
			if (aV[2] < vMin) {vMin = aV[2];} else {}
			if (aV[2] > vMax) {vMax = aV[2];} else {}

			bool bSkin = true;
			if (aV[0] > 95 && aV[1] > 40 && aV[2] > 20 &&
				vMax - vMin > 15 &&
				abs(aV[0] - aV[1]) > 15 &&
				aV[0] > aV[1] &&
				aV[0] > aV[2]) {
				bSkin = true;
			} else {
				bSkin = false;
			}

			DATA sV = (bSkin) ? 255.F : 0;
			mtxSkin.CellRef(x, y) = sV;
		} 
		loc += 3 * (scl - 1) * w;
	}
}

void MyFace::DetectLocal(std::vector<rectangle> &aRect, 
					     unsigned char *pBuf, unsigned bufW, unsigned bufH,  
						 unsigned xL, unsigned yT, unsigned imgW, unsigned imgH)
{
	m_arrGray.set_size(imgH, imgW);

	unsigned locOrg = yT * bufW + xL;
	for (unsigned y = 0; y < imgH; y++) {
		for (unsigned x = 0; x < imgW; x++) {
			unsigned loc = locOrg + x;
			//unsigned loc = (yT + y) * bufW + xL + x;
			m_arrGray[y][x] = pBuf[loc];
		}
		locOrg += bufW;
	}

	unsigned minLen = (imgW < imgH) ? imgW : imgH;
	DATA scl = minLen / 90.F;
	//DATA scl = minLen / 100.F;
	
	
	static array2d<unsigned char> arrG_rs(200, 200);
	if (scl < 1.F) {
		//pyramid_up(m_arrGray);
		//aRect = m_fd(m_arrGray);
		arrG_rs.set_size(imgH * 2, imgW * 2);
		resize_image(m_arrGray, arrG_rs);
		aRect = m_fd(arrG_rs);
	} else if (scl > 4.F) {
		//pyramid_down<2>(m_arrGray);
		arrG_rs.set_size(imgH / 2, imgW / 2);
		resize_image(m_arrGray, arrG_rs);
		aRect = m_fd(arrG_rs);
	} else {
		aRect = m_fd(m_arrGray);
	}

	if (scl < 1.F) {
		for (unsigned r = 0; r < aRect.size(); r++) {
			aRect[r].left() /= 2;
			aRect[r].right() /= 2;
			aRect[r].bottom() /=2;
			aRect[r].top() /= 2;
		}
	} else if (scl > 4.F) {
		for (unsigned r = 0; r < aRect.size(); r++) {
			aRect[r].left() *= 2;
			aRect[r].right() *= 2;
			aRect[r].bottom() *=2;
			aRect[r].top() *= 2;
		}
	} else {}

	for (unsigned r = 0; r < aRect.size(); r++) {
		aRect[r].left() += xL;
		aRect[r].right() += xL;
		aRect[r].bottom() += yT;
		aRect[r].top() += yT;
	}
	
	/*	
	static array2d<unsigned char> arrG_rs(200, 200);
	if (scl < 1.F) {
		arrG_rs.set_size(imgH / scl, imgW / scl);
		resize_image(m_arrGray, arrG_rs);
		aRect = m_fd(arrG_rs);
	} else if (scl > 2.F) {
		scl /= 2.F;
		arrG_rs.set_size(imgH / scl, imgW / scl);
		resize_image(m_arrGray, arrG_rs);
		aRect = m_fd(arrG_rs);
	} else {
		scl = 1.F;
		aRect = m_fd(m_arrGray);
	}

	for (unsigned r = 0; r < aRect.size(); r++) {
		aRect[r].left() *= scl;
		aRect[r].right() *= scl;
		aRect[r].bottom() *= scl;
		aRect[r].top() *= scl;
	}

	for (unsigned r = 0; r < aRect.size(); r++) {
		aRect[r].left() += xL;
		aRect[r].right() += xL;
		aRect[r].bottom() += yT;
		aRect[r].top() += yT;
	}
	*/
}

void MyFace::Detect(std::vector<rectangle> &aRect, 
					unsigned char *pGray, unsigned char *pRgb, 
					Mtx &mtxSkin, Mtx &mtxLab, unsigned scl,
					unsigned imgW, unsigned imgH, 
					bool bRGB, bool bDouble)
{
	//MyAssert(bRGB);

	Vect2D<unsigned> dimSkin = mtxSkin.GetDim();
//cout << dimSkin.m_x << " " << dimSkin.m_y << " " << scl << " " << imgW << " " << imgH << endl;
	//MyAssert(dimSkin.m_x * scl == imgW &&
	//		 dimSkin.m_y * scl == imgH);
	MyAssert(dimSkin.m_x == imgW / scl &&
			 dimSkin.m_y == imgH / scl);

	BeSkin(mtxSkin, pRgb, imgW, imgH, scl, bRGB);

	static std::vector<unsigned> aIdx;
	mtxOp.regionLabel.Gen(mtxLab, mtxSkin, aIdx);
	//cout << aIdx.size() << endl;

	static std::vector<Vect4D<unsigned>> aBBox;
	//static std::vector<unsigned> aArea;
	aBBox.resize(aIdx.size());
	//aArea.resize(aIdx.size());
	for (unsigned i = 0; i < aIdx.size(); i++) {
		aBBox[i].m_r = imgW - 1; // l
		aBBox[i].m_g = imgH - 1; // b
		aBBox[i].m_b = 0; // r
		aBBox[i].m_a = 0; // t
		//aArea[i] = 0;
	}
	for (unsigned y = 0; y < imgH / scl; y++) {
		for (unsigned x = 0; x < imgW / scl; x++) {
			unsigned r = (unsigned)mtxLab.CellVal(x, y);
			if (r != 0) {
				if (x < aBBox[r].m_r) {aBBox[r].m_r = x;} else {}
				if (x > aBBox[r].m_b) {aBBox[r].m_b = x;} else {}
				if (y < aBBox[r].m_g) {aBBox[r].m_g = y;} else {}
				if (y > aBBox[r].m_a) {aBBox[r].m_a = y;} else {}				
				//aArea[r]++;
			} else {}
		} // x
	} // y

	aRect.clear();
	static std::vector<rectangle> aRTmp;
	for (unsigned i = 1; i < aBBox.size(); i++) {
		if (aIdx[i] != i) {
			continue; 
		} else {}

		int xLen = aBBox[i].m_b - aBBox[i].m_r;
		int yLen = aBBox[i].m_a - aBBox[i].m_g;
		MyAssert(xLen >= 0 && yLen >= 0);

		int minLen = (xLen < yLen) ? xLen : yLen;
		if (minLen < (int)(50 / scl)) {
			continue;
		} else {}

		//cout << i << ": " 
		//	 << aBBox[i].m_r << " " 
		//	 << aBBox[i].m_g << " "
		//	 << aBBox[i].m_b << " "
		//	 << aBBox[i].m_a << endl;
		//getchar();

		aRTmp.clear();
		DetectLocal(aRTmp, pGray, imgW, imgH, aBBox[i].m_r * scl, aBBox[i].m_g * scl, xLen * scl, yLen * scl);

		for (unsigned f = 0; f < aRTmp.size(); f++) {
			aRect.push_back(aRTmp[f]);
		}
	}
}