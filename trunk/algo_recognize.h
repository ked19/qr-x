#ifndef _ALGO_RECOGNIZE_H
#define _ALGO_RECOGNIZE_H

#define GA

#include <dlib/image_processing/frontal_face_detector.h>

#include "denseMatrix.h"
#include "layer.h"
#include "define.h"
#include "TRIModel.h"
#include "matrixOperation.h"

//#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
//#include <dlib/image_processing/frontal_face_detector.h>
#ifdef GA
#include <svm.h>
#endif
#include <vector>
#include <sstream>

using namespace std;
using namespace dlib;

//***********************************************

class FFeaLoc
{
public:
	FFeaLoc();
	~FFeaLoc();

	std::vector<unsigned> m_x;
	std::vector<unsigned> m_y;
	static const unsigned FEA_NUM;
};

class FaceFeature
{
public:
	FaceFeature(string dir = "");
	~FaceFeature();

	int Detect(FFeaLoc &fFea, unsigned char *pImg, unsigned imgW, unsigned imgH, unsigned imgC,
				unsigned fL, unsigned fR, unsigned fB, unsigned fT, bool bRGB = true);

private:
	shape_predictor m_predictor;
	array2d<unsigned char> m_arrGFace;
	//rectangle m_rect;

	static const string FMARK_NAME;
	static const Vect2D<unsigned> DIM_FACE;
};

//***********************************************

class HeadPose
{
public:
	HeadPose(string dir = "");
	~HeadPose();

	void GetMemory(unsigned char **ppMemory, unsigned outC, unsigned &mW, unsigned &mH, unsigned &mC);
	void GetMask(unsigned char *pMsk);
	int Normalize(unsigned char *pOut, unsigned outC,
				  unsigned char *pImg, unsigned imgW, unsigned imgH, unsigned imgC,
				  unsigned fL_in, unsigned fR_in, unsigned fB_in, unsigned fT_in, 
				  bool bRGB = true, bool bDebug = false);
	bool IsLock();

	int GetTrans(Mtx &mtxX, 
				 unsigned char *pImg, unsigned imgW, unsigned imgH, unsigned imgC,
				 unsigned fL_in, unsigned fR_in, unsigned fB_in, unsigned fT_in, 
				 bool bRGB = true, bool bDebug = false);

private:
	void Render();
	void Get3DLabel(Mtx &mtxL3d, string fN);

	TRIModel *m_pTriModel;
	GLFWwindow *m_pWin;
	unsigned char *m_pTxBuf;
	GLuint m_txId;
	Vect2D<unsigned> m_dimFace;
	FaceFeature m_ffDetector;
	FFeaLoc m_fFea;
	Mtx m_mtxAMod;
	Mtx m_mtxBImg;
	Mtx m_mtxXTrans;

	GLuint m_txColor;
	GLuint m_frameBuf;
	GLuint m_depthBuf;

	bool m_bLock;

	bool m_bInitTx;
	GLuint m_txName;

	static const Vect2D<unsigned> DIM_FBO; //(400, 400);
	static const Vect3D<unsigned> DIM_FACE;
	static const Vect4D<float> BG_COLOR;
	static const Vect3D<DATA> EYE_POS;
	static const string SH_VERT_N;
	static const string SH_FRAG_N;
	static const string LABEL3D_N;
	static const unsigned WORLD_NUM;
	static const unsigned REGIST_NUM;
	static const unsigned MAPPING_NUM;

	static const unsigned m_aIdxMod[];
	static const unsigned m_aIdxImg[];
};

//*************************************************************************************************
// gender/age
//*************************************************************************************************
#ifdef GA
class GenderAge
{
public:
	GenderAge();
	~GenderAge();
	
	void SetMsk(Mtx &mtxMsk);
	void SetMsk(unsigned char *pMsk, unsigned mW, unsigned mH);

	DATA Detect(bool &bM, unsigned &ageB, unsigned &ageE,
			    Mtx &mtxFIn, bool bHEqual, bool bDebug = false);
	DATA Detect(bool &bM, unsigned &ageB, unsigned &ageE,
		 	    unsigned char *pFace, unsigned fW, unsigned fH, unsigned fC, 
			    bool bHEqual, bool bDebug = false);

private:
	static const unsigned F_NORM_LEN;
	static const unsigned AGE_NUM;

	DATA Decide(bool &bM, unsigned &ageB, unsigned &ageE, bool bDebug);

	Mtx m_mtxFNorm;
	Mtx m_mtxMsk;

	std::vector<svm_model*> m_apMSvm;
	std::vector<svm_model*> m_apFSvm;
	std::vector<DATA> m_aMProb;
	std::vector<DATA> m_aFProb;
	std::vector<DATA> m_aPDiff;
	svm_node *m_pNode;
	
};
#endif
#endif