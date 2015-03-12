#ifndef _QR_CODE_H
#define _QR_CODE_H

#include "define.h"
//#include "main_winApp.h"

#include <opencv2\opencv.hpp>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class QR_detect
{
public:
	QR_detect();
	~QR_detect();

	void Gen(std::vector<Vect2D<unsigned>> &aP, std::vector<Vect2D<unsigned>> &aQ, std::vector<Vect2D<unsigned>> &aR,
			 unsigned char *pGray, unsigned inW, unsigned inH);

private:
	cv::CascadeClassifier m_ccClassf;

	static const Vect2D<unsigned> DIM_MIN_DETECT;
	static const float ANGULAR_TOLERANCE;
	static const float LINEAR_TOLERANCE;
};

class QR_recognize
{
public:
	QR_recognize();
	~QR_recognize();

	void GetMemory(unsigned char **ppMemory, unsigned &mW, unsigned &mH, unsigned &mC);
	int Gen(unsigned char *pArrOut, unsigned char *pArrIn, unsigned inW, unsigned inH,
		Vect2D<unsigned> &vR, Vect2D<unsigned> &vP, Vect2D<unsigned> &vQ);

private:
	void Render(unsigned char *pArrIn, unsigned inW, unsigned inH,
		Vect2D<unsigned> &vR, Vect2D<unsigned> &vP, Vect2D<unsigned> &vQ);

	GLFWwindow *m_pWin;
	GLuint m_txColor;
	GLuint m_frameBuf;
	GLuint m_depthBuf;

	bool m_bInitTx;
	GLuint m_txName;
	unsigned char *m_pTxBuf;

	static const Vect2D<unsigned> DIM_FBO; //(400, 400);
	static const Vect2D<unsigned> DIM_QR_IN;
	static const Vect4D<float> BG_COLOR;
	static const DATA BOUND_SCL;
};

#endif