#include "algo_recognize.h"

//*************************************************************************************************
// FaceFeature
//*************************************************************************************************
const unsigned FFeaLoc::FEA_NUM = 68;

FFeaLoc::FFeaLoc()
{
	m_x.resize(FEA_NUM);
	m_y.resize(FEA_NUM);
}

FFeaLoc::~FFeaLoc()
{}

//***********************************************
//const string FaceFeature::FMARK_NAME = "shape_predictor_68_face_landmarks.dat"; 
const string FaceFeature::FMARK_NAME = "landmarks.dat"; 
const Vect2D<unsigned> FaceFeature::DIM_FACE(500, 500);

FaceFeature::FaceFeature(string dir)
{
	m_arrGFace.set_size(DIM_FACE.m_x, DIM_FACE.m_y);

	deserialize(dir + FMARK_NAME) >> m_predictor;
}

FaceFeature::~FaceFeature()
{}

int FaceFeature::Detect(FFeaLoc &fFea, unsigned char *pImg, unsigned imgW, unsigned imgH, unsigned imgC,
						unsigned fL, unsigned fR, unsigned fB, unsigned fT, bool bRGB)
{
	MyAssert(fT <= fB);
	MyAssert(fR >= fL);
	unsigned fW = fR - fL + 1;
	unsigned fH = fB - fT + 1;
	//MyAssert(fW <= DIM_FACE.m_x &&
	//		 fH <= DIM_FACE.m_y);
	if (fW > DIM_FACE.m_x ||
		fH > DIM_FACE.m_y) {
		return -1;
	} else {}

	if (imgC == 1) {
		for (unsigned y = fT; y <= fB; y++) {
			unsigned yLoc = y * imgW;
			unsigned row = y - fT;
			for (unsigned x = fL; x <= fR; x++) {
				unsigned loc = yLoc + x;
				unsigned col = x - fL;
				m_arrGFace[row][col] = pImg[loc]; 
			}
		}	
	} else if (imgC == 3) {
		for (unsigned y = fT; y <= fB; y++) {
			unsigned yLoc = y * imgW;
			unsigned row = y - fT;
			for (unsigned x = fL; x <= fR; x++) {
				unsigned loc = (yLoc + x) * imgC;
				unsigned col = x - fL;
				if (bRGB) {
					m_arrGFace[row][col] = (unsigned char)(pImg[loc + 0] * 0.30f +
														   pImg[loc + 1] * 0.59f +
														   pImg[loc + 2] * 0.11f);
				} else {
					m_arrGFace[row][col] = (unsigned char)(pImg[loc + 2] * 0.30f +
														   pImg[loc + 1] * 0.59f +
														   pImg[loc + 0] * 0.11f);
				}
			}
		}
	} else {
		MyAssert(0);
	}

	static rectangle rect;
	rect.left() = 0;
	rect.right() = fW - 1;
	rect.bottom() = fH - 1;
	rect.top() = 0;
	full_object_detection ffTmp = m_predictor(m_arrGFace, rect);
	MyAssert(ffTmp.num_parts() == FFeaLoc::FEA_NUM);
	for (unsigned fea = 0; fea < FFeaLoc::FEA_NUM; fea++) {
		point pTmp = ffTmp.part(fea);
		fFea.m_x[fea] = pTmp.x() + fL;
		fFea.m_y[fea] = pTmp.y() + fT;
	}
	return 0;
}

//*************************************************************************************************
// HeadPose
//*************************************************************************************************
//const Vect2D<unsigned> HeadPose::DIM_FBO(400, 400);
const Vect2D<unsigned> HeadPose::DIM_FBO(100, 100);
const Vect3D<unsigned> HeadPose::DIM_FACE(500, 500, 3);
//const Vect3D<unsigned> HeadPose::DIM_FACE(100, 100, 3);
const Vect4D<float> HeadPose::BG_COLOR(0.5f, 0.5f, 0.5f, 0);
const Vect3D<DATA> HeadPose::EYE_POS(-1.F, 3.5F, -3.59F + 1.F);
const string HeadPose::SH_VERT_N = "flatSh.vert";
const string HeadPose::SH_FRAG_N = "flatSh.frag";
const string HeadPose::LABEL3D_N = "label.txt";
const unsigned HeadPose::WORLD_NUM = 35;
const unsigned HeadPose::REGIST_NUM = 25;
const unsigned HeadPose::MAPPING_NUM = 8;

const unsigned HeadPose::m_aIdxMod[] = {28, 32, 30, 29, 31,
										33,  0,  2,  3,  1,
										 8, 10, 11,  9, 18,
										19, 20, 21, 22, 23,
										24, 25, 26, 27, 34};
const unsigned HeadPose::m_aIdxImg[] = { 0,  1,  2, 16, 15,
										14, 17, 21, 22, 26,
										36, 39, 42, 45, 31, 
										35, 30, 33, 48, 54, 
										51, 62, 66, 55,  8};

void HeadPose::Get3DLabel(Mtx &mtxL3D, string fN) 
{
	ifstream ifs(fN);
	MyAssert(ifs.good());

	unsigned labNum;
	ifs >> labNum;
	MyAssert(labNum == WORLD_NUM);

	Vect2D<unsigned> dim = mtxL3D.GetDim();
	MyAssert(dim.m_y == WORLD_NUM);

	for (unsigned lab = 0; lab < dim.m_y; lab++) {
		Vect3D<DATA> vTmp;
		ifs >> vTmp.m_x >> vTmp.m_y >> vTmp.m_z;
		mtxL3D.CellRef(0, lab) = vTmp.m_x;
		mtxL3D.CellRef(1, lab) = vTmp.m_y;
		mtxL3D.CellRef(2, lab) = vTmp.m_z;
	}

	ifs.close();
}

//GLEWContext gc;

/*
GLEWContext*	g_pGLEWContext;

static GLEWContext* glewGetContext()
{
    //static GLEWContext* myGLEWCONTEXT = NULL;

    //myGLEWCONTEXT = (GLEWContext*)glfwGetCurrentContext();   

    //return myGLEWCONTEXT;

	return g_pGLEWContext;
}
*/

HeadPose::HeadPose(string dir)
	: m_pTriModel(0), m_pWin(0) //, m_txFace(DIM_FACE.m_x, DIM_FACE.m_y, DIM_FACE.m_z)//, m_shader(SH_VERT_N, SH_FRAG_N)
	, m_mtxAMod(MAPPING_NUM, REGIST_NUM * 2), m_mtxBImg(1, REGIST_NUM * 2), m_mtxXTrans(1, MAPPING_NUM)
	, m_pTxBuf(0), m_txId(0), m_dimFace(0, 0)
	, m_bLock(false)
	, m_bInitTx(false), m_txName(0)
	, m_ffDetector(dir)
{
	bool bDebug = false;

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

	m_pTxBuf = new unsigned char [DIM_FACE.m_x * DIM_FACE.m_y * DIM_FACE.m_z];

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

	//*******************************************
	// 3D model
	//*******************************************
	if (bDebug) {
		cout << "h2" << endl;
	} else {}
	delete m_pTriModel;
	m_pTriModel = new TRIModel;
	string fN = dir + "faith_all.tri";
	if (bDebug) {
		cout << fN << endl;
	} else {}
	m_pTriModel->loadFromFile(fN.c_str());
	//cout << "ab" << endl;

	//*******************************************
	// shader
	//*******************************************
	//m_shader.Load();

	//*******************************************
	// matrix
	//*******************************************
	Mtx mtxL3D(3, WORLD_NUM);
	Get3DLabel(mtxL3D, dir + LABEL3D_N);

	mtxOp.zero.Gen(m_mtxAMod);
	for (unsigned lab = 0; lab < REGIST_NUM; lab++) {
		for (unsigned c = 0; c < 3; c++) {
			m_mtxAMod.CellRef(c + 0, lab * 2 + 0) = mtxL3D.CellVal(c, m_aIdxMod[lab]);
			m_mtxAMod.CellRef(c + 4, lab * 2 + 1) = mtxL3D.CellVal(c, m_aIdxMod[lab]);
		}
		m_mtxAMod.CellRef(3 + 0, lab * 2 + 0) = 1.F;
		m_mtxAMod.CellRef(3 + 4, lab * 2 + 1) = 1.F;
	}

}

HeadPose::~HeadPose() 
{}

void HeadPose::GetMemory(unsigned char **ppMemory, unsigned outC, unsigned &mW, unsigned &mH, unsigned &mC)
{
	mW = DIM_FBO.m_x;
	mH = DIM_FBO.m_y;
	mC = outC;
	*ppMemory = new unsigned char [mW * mH * mC];
}

void HeadPose::GetMask(unsigned char *pMsk) 
{
	/*
	//*******************************************
	// texture
	//*******************************************
	m_dimFace.m_x = 1;
	m_dimFace.m_y = 1;

	unsigned loc = 0;
	 for (unsigned y = 0; y < m_dimFace.m_y; y++) {
		 for (unsigned x = 0; x < m_dimFace.m_x; x++) {
			 for (unsigned c = 0; c < DIM_FACE.m_z; c++) {
				 m_pTxBuf[loc++] = 0;
			 }
		 }
	 }
	 */
	 
	//*******************************************
	// window
	//*******************************************
	glfwMakeContextCurrent(m_pWin);
	//glfwHideWindow(m_pWin);

	//*******************************************
	// rendering
	//*******************************************
	glClearColor(BG_COLOR.m_r, BG_COLOR.m_g, BG_COLOR.m_b, BG_COLOR.m_a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// my projection matrix
	DATA r = (DATA)DIM_FBO.m_y / DIM_FBO.m_x;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-0.013F, 0.013F, -0.013F * r, 0.013F * r, 0.03F, 2.F);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	DATA aENow[] = {-1.F, 3.5F, -3.59F + 1.F};
	gluLookAt(EYE_POS.m_x, EYE_POS.m_y, EYE_POS.m_z,
			  EYE_POS.m_x, EYE_POS.m_y, EYE_POS.m_z - 1.F,
			  0, 1.F, 0);	

	glViewport(0, 0, DIM_FBO.m_x, DIM_FBO.m_y);

	glEnable(GL_DEPTH_TEST);
	//glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuf);
	Render();

	glDisable(GL_DEPTH_TEST);

	glReadPixels(0, 0, DIM_FBO.m_x, DIM_FBO.m_y, GL_ALPHA, GL_UNSIGNED_BYTE, pMsk);
}

bool HeadPose::IsLock()
{
	return m_bLock;
}

void HeadPose::Render()
{
	//glUseProgram(m_shader.GetProg());

	float fVs = 1.f;
	float aaLightPos[][4] = {{ 1.f/fVs,  1.f/fVs,  -1.f/fVs,  1.f},		
							 {-1.f/fVs, -1.f/fVs,  -1.f/fVs,  1.f},		
							 {-1.f/fVs, -1.f/fVs,  1.f/fVs,  1.f}};
	float aaLightAmb[][4] = {{0.01f, 0.01f, 0.01f, 1.f},		
							 {0.01f, 0.01f, 0.01f, 1.f},		
							 {0.01f, 0.01f, 0.01f, 1.f}};
	float aaLightDif[][4] = {{0.6f, 0.6f, 0.6f, 1.f},		
							 {0.6f, 0.6f, 0.6f, 1.f},		
							 {0.6f, 0.6f, 0.6f, 1.f}};
	float aaLightSpc[][4] = {{0.2f, 0.2f, 0.2f, 1.f},		
							 {0.2f, 0.2f, 0.2f, 1.f},		
							 {0.2f, 0.2f, 0.2f, 1.f}};

	glDisable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH_TEST);


	const std::vector<Triangle> &vTri = m_pTriModel->triangleList;
	//*******************************************
	// texture mapping
	//*******************************************
	/*
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_txFace.GetTexID());  //pTxF->GetTexID());
	glUniform1i(m_shader.GetUniLoc("smpFace"), 0);

	glUniform3f(m_shader.GetUniLoc("v3Cnt"), -1.0119, -1.0119, -1.0119);
	glUniform1i(m_shader.GetUniLoc("tfNo"), 0);
	glUniform4f(m_shader.GetUniLoc("v4TfX"), m_mtxXTrans.CellVal(0, 0), m_mtxXTrans.CellVal(1, 0), m_mtxXTrans.CellVal(2, 0), m_mtxXTrans.CellVal(3, 0));
	glUniform4f(m_shader.GetUniLoc("v4TfY"), m_mtxXTrans.CellVal(4, 0), m_mtxXTrans.CellVal(5, 0), m_mtxXTrans.CellVal(6, 0), m_mtxXTrans.CellVal(7, 0));
	glUniform4f(m_shader.GetUniLoc("v4Tf0X"), m_mtxXTrans.CellVal(0, 0), m_mtxXTrans.CellVal(1, 0), m_mtxXTrans.CellVal(2, 0), m_mtxXTrans.CellVal(3, 0));
	glUniform4f(m_shader.GetUniLoc("v4Tf0Y"), m_mtxXTrans.CellVal(4, 0), m_mtxXTrans.CellVal(5, 0), m_mtxXTrans.CellVal(6, 0), m_mtxXTrans.CellVal(6, 0));
	glUniform4f(m_shader.GetUniLoc("v4Tf1X"), m_mtxXTrans.CellVal(0, 0), m_mtxXTrans.CellVal(1, 0), m_mtxXTrans.CellVal(2, 0), m_mtxXTrans.CellVal(3, 0));
	glUniform4f(m_shader.GetUniLoc("v4Tf1Y"), m_mtxXTrans.CellVal(4, 0), m_mtxXTrans.CellVal(5, 0), m_mtxXTrans.CellVal(6, 0), m_mtxXTrans.CellVal(6, 0));

	glUniform3f(m_shader.GetUniLoc("v3Eye"), 0, 0, 0);
	glUniform1i(m_shader.GetUniLoc("lightNum"), 3);

	bool bSymetry = false;
	float sym = (bSymetry) ? 1.f : 0;
	glUniform1f(m_shader.GetUniLoc("symetry"), sym), 
	*/

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
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, DIM_FACE.m_x, DIM_FACE.m_y, 0, GL_RGB, GL_UNSIGNED_BYTE, m_pTxBuf);	
	} else {}

	glBindTexture(GL_TEXTURE_2D, m_txName);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, DIM_FACE.m_x, DIM_FACE.m_y, GL_RGB, GL_UNSIGNED_BYTE, m_pTxBuf);
	//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_dimFace.m_x, m_dimFace.m_y, GL_RGB, GL_UNSIGNED_BYTE, m_pTxBuf);	

	glEnable(GL_TEXTURE_2D);
		//glActiveTexture(m_txFace.GetTexID());
	glBindTexture(GL_TEXTURE_2D, m_txName);
	//glBindTexture(GL_TEXTURE_2D, m_pTxFace->GetTexID());

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//const vector<Triangle> &vTri = worldModel.triangleList;
	for(unsigned i=0; i<vTri.size(); i++)
	{
		
		//const int *pFC = vTri[i].foreColor;
		//const int *pBC = vTri[i].backColor;		
		//glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		//glColor3ub(pFC[0], pFC[1], pFC[2]);
		
		/*
		double aNAvg[3] = {0};
		for(unsigned v=0; v<3; v++)
		{
			myMath.Add3V(aNAvg, vTri[i].normal[v]);
		}
		myMath.Normal3V(aNAvg);

		double aVAvg[3] = {0};
		for(unsigned v=0; v<3; v++)
		{
			myMath.Add3V(aVAvg, vTri[i].vertex[v]);
		}
		myMath.Mul3V(aVAvg, 1.F/3.F);
		*/

		//if(eRenderMethod == FLAT_SH)
		{
			//glUniform3f(m_shader.GetUniLoc("v3Pos"), (float)aVAvg[0], (float)aVAvg[1], (float)aVAvg[2]);
			//glUniform3f(m_shader.GetUniLoc("v3Norm"), (float)aNAvg[0], (float)aNAvg[1], (float)aNAvg[2]);
		}
		//else {}
		
		glBegin(GL_TRIANGLES);
			//glNormal3dv(vTri[i].normal[0]);		
			glTexCoord2f(vTri[i].normal[0][0], vTri[i].normal[0][1]);
			glVertex3dv(vTri[i].vertex[0]);		
			//glNormal3dv(vTri[i].normal[1]);		
			glTexCoord2f(vTri[i].normal[1][0], vTri[i].normal[1][1]);
			glVertex3dv(vTri[i].vertex[1]);		
			//glNormal3dv(vTri[i].normal[2]);	
			glTexCoord2f(vTri[i].normal[2][0], vTri[i].normal[2][1]);
			glVertex3dv(vTri[i].vertex[2]);		
		glEnd();
	}

	glDisable(GL_TEXTURE_2D);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHTING);
	
	//glUseProgram(0);
}

int HeadPose::GetTrans(Mtx &mtxX, 
					   unsigned char *pImg, unsigned imgW, unsigned imgH, unsigned imgC,
					   unsigned fL_in, unsigned fR_in, unsigned fB_in, unsigned fT_in, 
					   bool bRGB, bool bDebug)
{
	//*******************************************
	// mapping coefficient
	//*******************************************
	float fScl = 1.f; // 1.7F;

	int bFD = m_ffDetector.Detect(m_fFea, pImg, imgW, imgH, imgC, fL_in, fR_in, fB_in, fT_in, bRGB);
	if (bFD != 0) {
		return -1;
	} else {}

	for (unsigned lab = 0; lab < REGIST_NUM; lab++) {
		m_mtxBImg.CellRef(0, lab * 2 + 0) = m_fFea.m_x[m_aIdxImg[lab]];
		m_mtxBImg.CellRef(0, lab * 2 + 1) = m_fFea.m_y[m_aIdxImg[lab]];
	}
	//mtxOp.out << m_mtxAMod; 
	//cout << endl << endl;
	//mtxOp.out << m_mtxBImg ;
	//cout << endl << endl;
	//getchar();

	Vect2D<unsigned> dimA = m_mtxAMod.GetDim();
	Mtx mtxAC(dimA.m_x / 2, 1);
	mtxOp.zero.Gen(mtxAC);
	for (unsigned y = 0; y < dimA.m_y; y += 2) {
		for (unsigned x = 0; x < dimA.m_x / 2 - 1; x++) {
			mtxAC.CellRef(x, 0) += m_mtxAMod.CellVal(x, y);
		}
	}
	for (unsigned x = 0; x < dimA.m_x / 2; x++) {
		mtxAC.CellRef(x, 0) /= (dimA.m_y / 2);
	}

	Mtx mtxA2(dimA.m_x, dimA.m_y);
	mtxOp.zero.Gen(mtxA2);
	for (unsigned y = 0; y < dimA.m_y / 2; y++) {
		for (unsigned x = 0; x < dimA.m_x / 2; x++) {
			mtxA2.CellRef(x, y * 2) = m_mtxAMod.CellVal(x, y * 2) - mtxAC.CellVal(x, 0);
			mtxA2.CellRef(x + dimA.m_x / 2, y * 2 + 1) = mtxA2.CellVal(x, y * 2);
		}
	}
	//mtxOp.out << mtxA2; 
	//cout << endl << endl;
	//mtxOp.out << m_mtxBImg ;
	//cout << endl << endl;
	//getchar();

	static Mtx mtxAA(MAPPING_NUM, MAPPING_NUM);
	static Mtx mtxAB(1, MAPPING_NUM);
	static unsigned aIdx[MAPPING_NUM];
	//mtxOp.leastSquare.Gen(m_mtxXTrans, m_mtxAMod, m_mtxBImg, mtxAA, mtxAB, aIdx);
	mtxOp.leastSquare.Gen(m_mtxXTrans, mtxA2, m_mtxBImg, mtxAA, mtxAB, aIdx);
	if (bDebug) {
		mtxOp.out << m_mtxXTrans;
		cout << endl << endl;
	} else {}

	Vect2D<unsigned> dimOut = mtxX.GetDim();
	MyAssert(dimOut.m_x == 1 && dimOut.m_y == MAPPING_NUM);
	mtxX.CopyFrom(m_mtxXTrans);
	return 0;
}

int HeadPose::Normalize(unsigned char *pOut, unsigned outC,
						unsigned char *pImg, unsigned imgW, unsigned imgH, unsigned imgC,
						unsigned fL_in, unsigned fR_in, unsigned fB_in, unsigned fT_in, 
						bool bRGB, bool bDebug)
{
	m_bLock = true;

	//*******************************************
	// mapping coefficient
	//*******************************************
	float fScl = 1.7f;

	int bFD = m_ffDetector.Detect(m_fFea, pImg, imgW, imgH, imgC, fL_in, fR_in, fB_in, fT_in, bRGB);
	if (bFD != 0) {
		return -1;
	} else {}

	int fXC = (fL_in + fR_in) / 2;	int fXLen = (fR_in - fXC) * fScl;
	int fYC = (fT_in + fB_in) / 2;	int fYLen = (fB_in - fYC) * fScl;
	if (fXLen > (int)DIM_FACE.m_x / 2) {fXLen = DIM_FACE.m_x / 2;} else {}
	if (fYLen > (int)DIM_FACE.m_y / 2) {fYLen = DIM_FACE.m_y / 2;} else {}
	int fL = (int)(fXC - fXLen);		int fR = fL + 2 * fXLen - 1; //(int)(fXC + fXLen);
	int fT = (int)(fYC - fYLen);		int fB = fT + 2 * fYLen - 1; //(int)(fYC + fYLen);
	if (fL < 0) {fL = 0;} else {}
	if (fT < 0) {fT = 0;} else {}
	if (fR >= (int)imgW) {fR = imgW - 1;} else {}
	if (fB >= (int)imgH) {fB = imgH - 1;} else {}

	for (unsigned lab = 0; lab < REGIST_NUM; lab++) {
		m_mtxBImg.CellRef(0, lab * 2 + 0) = m_fFea.m_x[m_aIdxImg[lab]];
		//m_mtxBImg.CellRef(0, lab * 2 + 1) = imgH - 1 - m_fFea.m_y[m_aIdxImg[lab]];
		m_mtxBImg.CellRef(0, lab * 2 + 1) = m_fFea.m_y[m_aIdxImg[lab]];

		m_mtxBImg.CellRef(0, lab * 2 + 0) = (m_mtxBImg.CellVal(0, lab * 2 + 0) - fL) / DIM_FACE.m_x;
		m_mtxBImg.CellRef(0, lab * 2 + 1) = (m_mtxBImg.CellVal(0, lab * 2 + 1) - fT) / DIM_FACE.m_y;
	}
	//mtxOp.out << m_mtxAMod; 
	//cout << endl << endl;
	//mtxOp.out << m_mtxBImg ;
	//cout << endl << endl;

	static Mtx mtxAA(MAPPING_NUM, MAPPING_NUM);
	static Mtx mtxAB(1, MAPPING_NUM);
	static unsigned aIdx[MAPPING_NUM];
	mtxOp.leastSquare.Gen(m_mtxXTrans, m_mtxAMod, m_mtxBImg, mtxAA, mtxAB, aIdx);
	if (bDebug) {
		mtxOp.out << m_mtxXTrans;
		cout << endl << endl;
	} else {}

	unsigned idxL = 10 * 2;
	unsigned idxR = 13 * 2;
	unsigned idxM = 17 * 2;
	unsigned idxN = 16 * 2;
	/*
	DATA eRX = m_mtxAMod.CellVal(0, idxL) * m_mtxXTrans.CellVal(0, 0)
			 + m_mtxAMod.CellVal(1, idxL) * m_mtxXTrans.CellVal(0, 1)
			 + m_mtxAMod.CellVal(2, idxL) * m_mtxXTrans.CellVal(0, 2)
			 + m_mtxXTrans.CellVal(0, 3);
	DATA eLX = m_mtxAMod.CellVal(0, idxR) * m_mtxXTrans.CellVal(0, 0)
			 + m_mtxAMod.CellVal(1, idxR) * m_mtxXTrans.CellVal(0, 1)
			 + m_mtxAMod.CellVal(2, idxR) * m_mtxXTrans.CellVal(0, 2)
			 + m_mtxXTrans.CellVal(0, 3);
	DATA diffL = fabs(eLX - m_mtxBImg.CellVal(0, idxL));
	DATA diffR = fabs(eRX - m_mtxBImg.CellVal(0, idxR));
	cout << diffL << " " << diffR << ": " << m_mtxBImg.CellVal(0, idxL) << " " << m_mtxBImg.CellVal(0, idxR) << endl;
	bool bPsuL = (diffL - diffR > 0.1) ?
			true : false;
	bool bPsuR = (diffR - diffL > 0.1) ?
			true : false;
	if (bPsuL || bPsuR) {
		cout << "got" << endl;
	} else {}
	//bPsuL = false;
	//bPsuR = false;
	*/
	DATA eImgS = DIM_FACE.m_x / (fR - fL + 1.F);
	DATA eImgLX = m_mtxBImg.CellVal(0, idxL) * eImgS;		DATA eImgLY = m_mtxBImg.CellVal(0, idxL + 1) * eImgS;
	DATA eImgRX = m_mtxBImg.CellVal(0, idxR) * eImgS;		DATA eImgRY = m_mtxBImg.CellVal(0, idxR + 1) * eImgS;
	DATA eImgMX = m_mtxBImg.CellVal(0, idxM) * eImgS;		DATA eImgMY = m_mtxBImg.CellVal(0, idxM + 1) * eImgS;
	DATA eImgNX = m_mtxBImg.CellVal(0, idxN) * eImgS;		DATA eImgNY = m_mtxBImg.CellVal(0, idxN + 1) * eImgS;
	DATA xC = eImgRX - eImgLX;
	DATA yC = eImgRY - eImgLY;
	DATA cC = -xC * eImgMX - yC * eImgMY;
	DATA lenN = (xC * eImgNX + yC * eImgNY + cC) / sqrt(xC * xC + yC * yC);

	//cout << lenN << endl;
	bool bPsuR = (lenN > 0.01) ?
			true : false;
	bool bPsuL = (lenN < -0.01) ?
			true : false;
	if (bPsuL || bPsuR) {
		//cout << "got" << endl;
	} else {}
	/*
	DATA eDiff = eR - eL;
	//cout << eDiff << endl;
	bool bPsuL = false;
	bool bPsuR = false;
	if (eDiff < 0.3F) {
		DATA eM = m_mtxBImg.CellVal(0, idxM) * eS;
		if (eM < 0.5F) {
			bPsuL = true;
		} else {
			bPsuR = true;
		}
	} else {}
	*/

	DATA xCnt = -1.0119F;
	for (unsigned t = 0; t < m_pTriModel->triangleList.size(); t++) {
		Triangle &tri = m_pTriModel->triangleList[t];
		double aV[][3] = {tri.vertex[0][0], tri.vertex[0][1], tri.vertex[0][2],
						  tri.vertex[1][0], tri.vertex[1][1], tri.vertex[1][2],
						  tri.vertex[2][0], tri.vertex[2][1], tri.vertex[2][2]};
		if (bPsuL) {
			for (unsigned v = 0; v < 3; v++) {
				if (aV[v][0] < xCnt) {
					aV[v][0] = 2 * xCnt - aV[v][0];
				} else {}
			}
		} else if (bPsuR) {
			for (unsigned v = 0; v < 3; v++) {
				if (aV[v][0] > xCnt) {
					aV[v][0] = 2 * xCnt - aV[v][0];
				} else {}
			}
		} else {}

		for (unsigned v = 0; v < 3; v++) {
			double txX = aV[v][0] * m_mtxXTrans.CellVal(0, 0)
					   + aV[v][1] * m_mtxXTrans.CellVal(0, 1)
					   + aV[v][2] * m_mtxXTrans.CellVal(0, 2) 
					   + m_mtxXTrans.CellVal(0, 3);
			double txY = aV[v][0] * m_mtxXTrans.CellVal(0, 4)
					   + aV[v][1] * m_mtxXTrans.CellVal(0, 5)
					   + aV[v][2] * m_mtxXTrans.CellVal(0, 6) 
					   + m_mtxXTrans.CellVal(0, 7);
			tri.normal[v][0] = txX;
			tri.normal[v][1] = txY;
			tri.normal[v][2] = 0;

			//cout << "(" << txX << "," << txY << ")  ";
		}
	}
	//getchar();
	//return;

	//*******************************************
	// texture
	//*******************************************
	unsigned fW = fR - fL + 1;
	unsigned fH = fB - fT + 1; 
	m_dimFace.m_x = fW;
	m_dimFace.m_y = fH;

	//cout << fW << " " << fH << endl;
	MyAssert(fW <= DIM_FACE.m_x &&
			 fH <= DIM_FACE.m_y);
	MyAssert(imgC == DIM_FACE.m_z);
	 
	 /*
	 unsigned ww = 0;
	 for (unsigned y = 0; y < m_dimFace.m_y; y++) {
		 for (unsigned x = 0; x < m_dimFace.m_x; x++) {
			 for (unsigned c = 0; c < imgC; c++) {
				m_pTxBuf[ww++] = 255;
			 }
		 }
	 }
	 */

	 for (unsigned y = 0; y < m_dimFace.m_y; y++) {
		 unsigned yy = y + fT;
		 unsigned yImgLoc = yy * imgW;
		 unsigned yBufLoc = y * DIM_FACE.m_x;
		 for (unsigned x = 0; x < m_dimFace.m_x; x++) {
			 unsigned xx = x + fL;
			 unsigned locImg = (yImgLoc + xx) * imgC;
			 unsigned locBuf = (yBufLoc + x) * imgC;
			 if (bRGB) {
				for (unsigned c = 0; c < imgC; c++) {
					m_pTxBuf[locBuf + c] = pImg[locImg + c];
				}
			 } else {
				 //cout << "bgr" << endl;
			 	for (unsigned c = 0; c < imgC; c++) {
					m_pTxBuf[locBuf + imgC - c] = pImg[locImg + c];
				}
			 }
		 }
	 }
	 
	//*******************************************
	// window
	//*******************************************
	glfwMakeContextCurrent(m_pWin);
	//glfwHideWindow(m_pWin);

	//*******************************************
	// rendering
	//*******************************************
	glClearColor(BG_COLOR.m_r, BG_COLOR.m_g, BG_COLOR.m_b, BG_COLOR.m_a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// my projection matrix
	DATA r = (DATA)DIM_FBO.m_y / DIM_FBO.m_x;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-0.013F, 0.013F, -0.013F * r, 0.013F * r, 0.03F, 2.F);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	DATA aENow[] = {-1.F, 3.5F, -3.59F + 1.F};
	gluLookAt(EYE_POS.m_x, EYE_POS.m_y, EYE_POS.m_z,
			  EYE_POS.m_x, EYE_POS.m_y, EYE_POS.m_z - 1.F,
			  0, 1.F, 0);	

	glViewport(0, 0, DIM_FBO.m_x, DIM_FBO.m_y);

	glEnable(GL_DEPTH_TEST);
	//glBindFramebuffer(GL_FRAMEBUFFER, m_frameBuf);
	Render();

	glDisable(GL_DEPTH_TEST);

	if (outC == 1) {
		glPixelTransferf(GL_RED_SCALE,		0.30f);
		glPixelTransferf(GL_GREEN_SCALE,	0.59f);
		glPixelTransferf(GL_BLUE_SCALE,		0.11f);

		glReadPixels(0, 0, DIM_FBO.m_x, DIM_FBO.m_y, GL_LUMINANCE, GL_UNSIGNED_BYTE, pOut);

		glPixelTransferf(GL_RED_SCALE,		1.f);
		glPixelTransferf(GL_GREEN_SCALE,	1.f);
		glPixelTransferf(GL_BLUE_SCALE,		1.f);

	} else if (outC == 3) {
		glReadPixels(0, 0, DIM_FBO.m_x, DIM_FBO.m_y, GL_RGB, GL_UNSIGNED_BYTE, pOut);
	} else {
		MyAssert(0);
	}

	/*
	GLubyte *pPixel = new GLubyte [DIM_FBO.m_x * DIM_FBO.m_y * 3];
	glReadPixels(0, 0, DIM_FBO.m_x, DIM_FBO.m_y, GL_RGB, GL_UNSIGNED_BYTE, pPixel);

	Layer lyrOut(DIM_FBO.m_x, DIM_FBO.m_y, 3);
	Vect3D<unsigned> dimOut = lyrOut.GetDim();
	for (unsigned y = 0; y < DIM_FBO.m_y; y++) {
		for (unsigned x = 0; x < DIM_FBO.m_x; x++) {
			for (unsigned c = 0; c < dimOut.m_z; c++) {
				lyrOut.CellRef(x, y, c) = pPixel[(y * DIM_FBO.m_x + x) * 3 + c];
			}
		}
	}
	imgIO.Write("out.jpg", MyImg(lyrOut));
	//getchar();
	*/

	m_bLock = false;
	return 0;
}

//*************************************************************************************************
// GenderAge
//*************************************************************************************************
#ifdef GA
const unsigned GenderAge::F_NORM_LEN = 50;
const unsigned GenderAge::AGE_NUM = 7;

GenderAge::GenderAge()
	: m_mtxFNorm(F_NORM_LEN, F_NORM_LEN)
	, m_mtxMsk(F_NORM_LEN, F_NORM_LEN)
{
	stringstream ss;
	string sTmp;
	for (unsigned g = 0; g < 2; g++) {
		std::vector<svm_model*> &apSvm = (g == 0) ?
			m_apMSvm : m_apFSvm;
		string gS = (g == 0) ? "m_" : "f_";
		apSvm.resize(AGE_NUM);

		for (unsigned aNo = 0; aNo < apSvm.size(); aNo++) {
			ss.str("");		ss.clear();
			ss << aNo;
			ss >> sTmp;
			string fN = gS + sTmp + "_ga2.svm";
			cout << fN << endl;

			apSvm[aNo] = svm_load_model(fN.c_str());
			//svm_free_model_content(apSvm[aNo]);

			MyAssert(svm_get_nr_class(apSvm[aNo]) == 2);
		}
	}

	m_aMProb.resize(AGE_NUM);
	m_aFProb.resize(AGE_NUM);

	m_pNode = new svm_node [F_NORM_LEN * F_NORM_LEN + 1];
}

GenderAge::~GenderAge()
{
	delete []m_pNode;

	for (unsigned g = 0; g < 2; g++) {
		std::vector<svm_model*> &apSvm = (g == 0) ?
			m_apMSvm : m_apFSvm;

		for (unsigned aNo = 0; aNo < apSvm.size(); aNo++) {
			svm_free_and_destroy_model(&apSvm[aNo]);
		}
	}
}

void GenderAge::SetMsk(Mtx &mtxMsk)
{
	mtxOp.scaleDim.Gen(m_mtxMsk, mtxMsk);
	mtxOp.binThrd.Gen(m_mtxMsk, 128.F, true);
}
void GenderAge::SetMsk(unsigned char *pMsk, unsigned mW, unsigned mH)
{
	Mtx mtxMsk(mW, mH);
	unsigned loc = 0;
	for (unsigned y = 0; y < mH; y++) {
		for (unsigned x = 0; x < mW; x++) {
			mtxMsk.CellRef(x, y) = pMsk[loc++];
		}
	}
	SetMsk(mtxMsk);
}

DATA GenderAge::Decide(bool &bM, unsigned &ageB, unsigned &ageE, bool bDebug) 
{
	//bDebug = true;

	static std::vector<DATA> aPDiff;
	aPDiff.resize(AGE_NUM);

	DATA mPSum = 0;
	DATA fPSum = 0;
	for (unsigned aNo = 0; aNo < AGE_NUM; aNo++) {
		mPSum += m_aMProb[aNo];
		fPSum += m_aFProb[aNo];
	}
	DATA maxProb = fabs(mPSum - fPSum);

	for (unsigned aNo = 0; aNo < AGE_NUM; aNo++) {
		aPDiff[aNo] = (mPSum > fPSum) ?
			m_aMProb[aNo] - m_aFProb[aNo] :
			m_aFProb[aNo] - m_aMProb[aNo];

		if (bDebug) {
			cout << aNo << ": " << m_aMProb[aNo] << ", " << m_aFProb[aNo] << endl;
		} else {}
	}

	/*
	DATA maxDiff = 0;
	unsigned idMD = 0;
	for (unsigned aNo = 0; aNo < AGE_NUM; aNo++) {
		DATA abs = fabs(aPDiff[aNo]);
		if (abs > maxDiff) {
			maxDiff = abs;
			idMD = aNo;
		}
	}
	*/
	unsigned idMD = 0;
	for (unsigned aNo = 1; aNo < AGE_NUM; aNo++) {
		//if (m_aMProb[aNo] + m_aFProb[aNo] >= 0.5F) {
		//	idMD++;
		//} else {}
		if (mPSum > fPSum) {
			//if (m_aMProb[aNo] > 0.5F) {
			if (aPDiff[aNo] > 0.5F) {
				idMD++;
			} else {}
		} else {
			if (aPDiff[aNo] > 0.5F) {
				idMD++;
			} else {}
		}
	}

	//bM = (aPDiff[idMD] > 0) ? true : false;
	bM = (mPSum > fPSum) ? true : false;
	if (idMD == 0) {
		ageB = 0;
		ageE = 5;
		return maxProb;
	} else if (idMD == AGE_NUM - 1) {
		ageB = (idMD - 1) * 10 + 5;
		ageE = 100;
		return maxProb;
	} else {
		ageB = (idMD - 1) * 10 + 5;
		ageE = ageB + 10;
		if (mPSum > fPSum) {
			if (aPDiff[idMD + 1] < 0.5F && aPDiff[idMD + 1] > 0.25F) {
				ageB += 5;
			} else {
				ageE -= 5;
			}
		} else {
			if (aPDiff[idMD + 1] < 0.5F && aPDiff[idMD + 1] > 0.25F) {
				ageB += 5;
			} else {
				ageE -= 5;
			}	
		}
		return maxProb;
	}

	/*
	std::vector<DATA> &aGProb = (mPSum > fPSum) ? m_aMProb : m_aFProb;
	if (fPSum > mPSum) {
		for (unsigned aNo = 0; aNo < AGE_NUM; aNo++) {
			aPDiff[aNo] *= -1.F;
		}
	} else {}

	int idx = -1;
	DATA dMax = -1e10;
	for (unsigned aNo = 0; aNo < AGE_NUM; aNo++) {
		//if (aGProb[aNo] >= 0.5F) {
			if (aPDiff[aNo] > dMax) {
				idx = aNo;
				dMax = aPDiff[aNo];
			} else {}
		//} else {}
	}

	if (idx == -1) {
		for (unsigned aNo = 0; aNo < AGE_NUM; aNo++) {
			if (aPDiff[aNo] > dMax) {
				idx = aNo;
				dMax = aPDiff[aNo];
			} else {}
		}
	} else {}

	int iAdd = 0;
	if (idx == 0 || idx == AGE_NUM - 1) {
		iAdd = 0;
	} else if (aPDiff[idx + 1] > aPDiff[idx - 1]) {
		iAdd = 1;
	} else {
		iAdd = -1;
	}

	bM = (mPSum > fPSum) ? true : false;

	ageB = idx * 10;
	if (idx > 0) {
		ageB -= 5;
	} else {}
	ageE = (idx + 1) * 10 - 5;

	if (iAdd == 1) {
		ageB += 5;
	} else if (iAdd == -1) {
		ageE -= 5;
	} else {}
	*/
}

DATA GenderAge::Detect(bool &bM, unsigned &ageB, unsigned &ageE, 
					   Mtx &mtxFIn, bool bHEqual, bool bDebug)
{
	Vect2D<unsigned> dimFIn = mtxFIn.GetDim();
	if (dimFIn.m_x != F_NORM_LEN ||
		dimFIn.m_y != F_NORM_LEN) {
		mtxOp.scaleDim.Gen(m_mtxFNorm, mtxFIn);
	} else {
		m_mtxFNorm.CopyFrom(mtxFIn);
	}

	if (bHEqual) {
		mtxOp.hisEqual.Gen(m_mtxFNorm, 255, &m_mtxMsk);
		if (bDebug) {
			imgIO.Write("bb.jpg", MyImg(m_mtxFNorm));
		} else {}
	} else {}

	DATA mVal = 2.F / 255.F;
	unsigned loc = 0;
	for (unsigned y = 0; y < F_NORM_LEN; y++) {
		for (unsigned x = 0; x < F_NORM_LEN; x++) {
			DATA val = m_mtxFNorm.CellVal(x, y) * mVal - 1.F;
			m_pNode[loc].index = loc + 1;
			m_pNode[loc].value = val;
			loc++;
		} 
	}
	m_pNode[loc].index = -1;

	DATA aTmp[2];

	for (unsigned g = 0; g < 2; g++) {
		std::vector<DATA> &aProb = (g == 0) ? m_aMProb : m_aFProb;
		std::vector<svm_model*> &apSvm = (g == 0) ?
			m_apMSvm : m_apFSvm;

		for (unsigned aNo = 0; aNo < AGE_NUM; aNo++) {
			svm_predict_probability(apSvm[aNo], &m_pNode[0], &aTmp[0]);
			aProb[aNo] = aTmp[0];
		}
	}

	DATA prob = Decide(bM, ageB, ageE, bDebug);

	return prob;
}

DATA GenderAge::Detect(bool &bM, unsigned &ageB, unsigned &ageE, 
	 				   unsigned char *pFace, unsigned fW, unsigned fH, unsigned fC, 
					   bool bHEqual, bool bDebug)
{
	MyAssert(fC == 3 || fC == 1);
	Mtx mtxF(fW, fH);

	if (fC == 1) {
		mtxF.CopyFrom(pFace);
	} else if (fC == 3) {
		unsigned loc = 0;
		for (unsigned y = 0; y < fH; y++) {
			for (unsigned x = 0; x < fW; x++) {
				mtxF.CellRef(x, y) = pFace[loc + 0] * 0.30F
								   + pFace[loc + 1] * 0.59F
								   + pFace[loc + 2] * 0.11F;
				loc += 3;
			}
		}
	} else {
		MyAssert(0);
	}

	if (bDebug) {
		imgIO.Write("aa.jpg", MyImg(mtxF));
	} else {}
	return Detect(bM, ageB, ageE, mtxF, bHEqual, bDebug);
}
#endif