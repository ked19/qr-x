#ifndef _MAIN_WINAPP_H
#define _MAIN_WINAPP_H

#define SURV

#include "SDK10000.h"
#include "layer.h"
#include "imageIO.h"
#include "algo_recognize.h"
#include "layerOperation.h"
#include "videoInput.h"

#include <FL/Fl.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Light_Button.H>
#include <FL/Fl_ask.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/glut.H>
#include <iostream>
#include <vector>
#include <mutex>
#include <sstream>

using namespace std;

extern Fl_Input *pIP0_txtIn;
extern Fl_Input *pIP1_txtIn;
extern Fl_Input *pIP2_txtIn;
extern Fl_Input *pIP3_txtIn;
extern Fl_Input *pPort_txtIn;
extern Fl_Input *pAccount_txtIn;
extern Fl_Input *pPasswd_txtIn;
extern Fl_Input *pInID_txtIn;
extern Fl_Input *pSearchID_txtIn;
extern Fl_Light_Button *pClose_litBttn;
extern Fl_Light_Button *pCalculate_litBttn;
extern Fl_Button *pConnect_bttn;

extern bool bTx;
extern unsigned char *pTxBuf;
extern unsigned txW;
extern unsigned txH;
//extern unsigned sclLen;
extern unsigned txCh;

const unsigned NORM_NUM = 10;
extern unsigned char *pArrGray;
extern unsigned char *pArrRgb;
extern unsigned char *apNorm[NORM_NUM];
extern unsigned normW;
extern unsigned normH;
extern unsigned normC;

/*
class FF
{
public:
	DATA m_0;
	DATA m_1;
	DATA m_2;
	DATA m_3;
};
extern std::vector<FF> arrFace;
*/

extern std::vector<FFeaLoc> aFFLoc;
extern std::vector<bool> aBSmile;
extern std::vector<rectangle> aFace;
extern unsigned condition;
extern std::mutex gMutex;

//extern FaceCount fcount;

extern std::vector<Vect2D<unsigned>> aP_g;
extern std::vector<Vect2D<unsigned>> aQ_g;
extern std::vector<Vect2D<unsigned>> aR_g;

void RunConnect				(Fl_Button *w, void *v);
void ComputeScore			(Fl_Light_Button *w, void *v);
void Disconnect				(Fl_Light_Button *w, void *v);
void CloseConnect();

//#define SURV
#ifdef SURV
#else
int GetImage_vi();
#endif

class MyGlWindow : public Fl_Gl_Window
{
public:
	MyGlWindow(int x, int y, int w, int h, const char *l = 0);
	void draw();
	int handle(int event);
};
extern MyGlWindow *pTarget_win;

#endif