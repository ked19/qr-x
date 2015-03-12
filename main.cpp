#include "main.h"

using namespace std;
using namespace cv;

#include <vector>
#include <iostream>
#include <zbar.h>

#include "define.h"
#include "imageIO.h"
#include "layer.h"
#include "denseMatrix.h"

using namespace std;
using namespace zbar;

int DecodeQR(string &strOut, Image &qrImg, ImageScanner &scan)
{
	int n = scan.scan(qrImg);
	if (n <= 0) {
		return -1;
	} else {}

	// extract results
	Image::SymbolIterator sIt = qrImg.symbol_begin();
	unsigned count = 0;
	zbar_symbol_type_t type;
	while (sIt != qrImg.symbol_end()) {
		type = sIt->get_type();
		strOut = sIt->get_data();
		++sIt;
		count++;
	}

	if (count != 1) {
		return -1;
	} else {}

	if (type != ZBAR_QRCODE) {
		return -1;
	} else {}
	return 0;
}

/*
int main (int argc, char **argv)
{
    if(argc < 2) return(1);

    // create a reader
    ImageScanner scanner;

    // configure the reader
    scanner.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 1);

    // obtain image data
	MyImg *pImg = imgIO.Read(argv[1]);
	Mtx *pMtxG = pImg->ConvertGray()->GetMtx(0);
	delete pImg;

	
	Vect2D<unsigned> dim = pMtxG->GetDim();
	int width = dim.m_x;
	int height = dim.m_y;
	unsigned char *pG = new unsigned char [width * height];
	unsigned loc = 0;
	for (unsigned y = 0; y < (unsigned)height; y++) {
		for (unsigned x = 0; x < (unsigned)width; x++) {
			pG[loc++] = (unsigned char)pMtxG->CellVal(x, y);
		}
	}
	const void *raw = (void *)pG;
	
    // wrap image data
    Image image(width, height, "Y800", raw, width * height);

    // scan the image for barcodes
    int n = scanner.scan(image);



    // clean up
    image.set_data(NULL, 0);

	delete []pG;

	system("pause");
    return(0);
}
*/

/*
void EqualHis(unsigned char *pIn, unsigned char *pMsk)
{
	static Mtx mIn(100, 100);
	static Mtx mMsk(100, 100);
	unsigned loc = 0;
	for (unsigned y = 0; y < 100; y++) {
		for (unsigned x = 0; x < 100; x++) {
			mIn.CellRef(x, y) = pIn[loc];
			mMsk.CellRef(x, y) = pMsk[loc];		
			loc++;
		}
	}
	mtxOp.hisEqual.Gen(mIn, 255, &mMsk);
	//mtxOp.hisEqual.Gen(mIn, 256, &mMsk);
	unsigned loc2 = 0;
	for (unsigned y = 0; y < 100; y++) {
		for (unsigned x = 0; x < 100; x++) {
			pIn[loc2++] = mIn.CellRef(x, y);
		}
	}
}
*/

void main() 
{
	bool bDebug = false;

	//MyFace myf;
	//FaceFeature fFeat;
	QR_detect qrDetect;
	QR_recognize qrRecog;
	ImageScanner qrScan;
	qrScan.set_config(ZBAR_NONE, ZBAR_CFG_ENABLE, 0);
	qrScan.set_config(ZBAR_QRCODE, ZBAR_CFG_ENABLE, 1);
	qrScan.set_config(ZBAR_QRCODE, ZBAR_CFG_MIN_LEN, 500);
	qrScan.set_config(ZBAR_QRCODE, ZBAR_CFG_MAX_LEN, 600);
	qrScan.set_config(ZBAR_QRCODE, ZBAR_CFG_X_DENSITY, 1);
	qrScan.set_config(ZBAR_QRCODE, ZBAR_CFG_Y_DENSITY, 1);
//	qrScan.set_config(ZBAR_QRCODE, ZBAR_CFG_MAX_LEN, 200);

	//const unsigned normLen = 600; //200;
	const DATA boundScl = 0.25F;
	//Mtx mtxNorm(normLen, normLen);
	//Mtx mtxT(2, 2);
	//Mtx mtxV(1, 2);
	//Mtx mtxOut(1, 2);
	//unsigned char qrBuf[normLen * normLen];
	//Image qrImg(normLen, normLen, "Y800", (void *)&qrBuf[0], normLen * normLen);

	unsigned char *pQR_norm;
	unsigned nW;
	unsigned nH;
	unsigned nC;
	qrRecog.GetMemory(&pQR_norm, nW, nH, nC);
	MyAssert(nC == 1);
	//Mtx mtxNorm(nW, nH);
	cout << nW << " " << nH << " " << nC << endl;
	Image qrImg(nW, nH, "Y800", (void *)pQR_norm, nW * nH);
	Mtx mtxNorm(nW, nH);

	unsigned qrCount = 0;

	//*******************************************
	// window
	//*******************************************
	ifstream ifs;
	ifs.open("version.txt");
	if (ifs.fail()) {
		MyAssert(0);
	} else {}
	
	string sLine;
	getline(ifs, sLine);
	string title = "QR code " + sLine;
	
	getline(ifs, sLine);
	bool bSetDefault = (sLine.compare("set default") == 0) ?
		true : false;

	fl_message_icon()->hide();

	pMain_win = make_window();
	pMain_win->show();
	pMain_win->label(title.c_str());
	if (!bSetDefault) {
		pIP0_txtIn->value("");
		pIP1_txtIn->value("");
		pIP2_txtIn->value("");
		pIP3_txtIn->value("");
		pAccount_txtIn->value("");
		pPasswd_txtIn->value("");
	} else {}

	pTarget_win->show();

	//*******************************************
	// looping
	//*******************************************
	//while(Fl::wait(0.05)) {
	while(Fl::wait(0)) {
		if (!Fl::first_window()) {
			break;
		} else {}

#ifdef SURV
#else
		int bVi = GetImage_vi();
		if (bVi != 0) {
			continue;
		} else {}
#endif

		if (!bTx) {
			continue;
		} else {}

#ifdef SURV
		gMutex.lock();
		bool bContinue = (condition != 1) ? true : false;
		gMutex.unlock();
		if (bContinue) {
			continue;
		} else {}
#else
#endif

		static unsigned count = 0;
		static time_t tBeg;
		static time_t tEnd;
		if (count == 0) {
			tBeg = time(0);
		} else {}
		count++;
		if (count >= 50) {
			tEnd = time(0);
			//time_t tDiff = tEnd - tBeg;
			cout << "********************************************************************************" << endl;
			cout << "processing time: " << difftime(tEnd, tBeg) / 49.f * 1000 << "ms" << endl;
			cout << "********************************************************************************" << endl;

			count = 0;
		} else {}

		// P
		// | \
		// |  \
		// R---Q
		qrDetect.Gen(aP_g, aQ_g, aR_g, pArrGray, txW, txH);
		MyAssert(aP_g.size() == aQ_g.size());
		MyAssert(aP_g.size() == aR_g.size());
		/*
		for (unsigned a = 0; a < aP_g.size(); a++) {
			cout << "(" << aP_g[a].m_x << "," << aP_g[a].m_y << ") "
				 << "(" << aQ_g[a].m_x << "," << aQ_g[a].m_y << ") "
				 << "(" << aR_g[a].m_x << "," << aR_g[a].m_y << ")" << endl; 
		}
		cout << "**************************************" << endl;
		*/


		for (unsigned no = 0; no < aP_g.size(); no++) {
			Vect2D<DATA> RP((DATA)aP_g[no].m_x - aR_g[no].m_x,
							(DATA)aP_g[no].m_y - aR_g[no].m_y);
			Vect2D<DATA> RQ((DATA)aQ_g[no].m_x - aR_g[no].m_x,
							(DATA)aQ_g[no].m_y - aR_g[no].m_y);
			Vect2D<DATA> org(aR_g[no].m_x, aR_g[no].m_y);

			qrRecog.Gen(pQR_norm, pArrGray, txW, txH, aR_g[no], aP_g[no], aQ_g[no]);
			mtxNorm.CopyFrom(pQR_norm);			

			imgIO.Write("o2.png", MyImg(mtxNorm));
			//getchar();

			/*
			mtxT.CellRef(0, 0) = RP.m_x;	mtxT.CellRef(0, 1) = RP.m_y;
			mtxT.CellRef(1, 0) = RQ.m_x;	mtxT.CellRef(1, 1) = RQ.m_y;
			for (unsigned y = 0; y < normLen; y++) {
				mtxV.CellRef(0, 1) = (DATA)y / normLen;
				mtxV.CellRef(0, 1) = mtxV.CellVal(0, 1) * (1.F + 2.F * boundScl) - boundScl;
				for (unsigned x = 0; x < normLen; x++) {
					mtxV.CellRef(0, 0) = (DATA)x / normLen;					
					mtxV.CellRef(0, 0) = mtxV.CellVal(0, 0) * (1.F + 2.F * boundScl) - boundScl;

					mtxOp.mul.Gen(mtxOut, mtxT, mtxV);
					mtxOut.CellRef(0, 0) += org.m_x;
					mtxOut.CellRef(0, 1) += org.m_y;
					unsigned xx = (unsigned)(mtxOut.CellVal(0, 0) + 0.5F);
					unsigned yy = (unsigned)(mtxOut.CellVal(0, 1) + 0.5F);

					unsigned loc = txW * yy + xx;
					mtxNorm.CellRef(x, y) = pArrGray[loc];
				}
			}
			*/
			//imgIO.Write("o2.png", MyImg(mtxNorm));
			//mtxNorm.CopyTo((unsigned char *)&qrBuf);
			
			string strOut;
			int dErr = DecodeQR(strOut, qrImg, qrScan);	
			cout << "#" << qrCount << ": " << strOut << endl;
			qrCount++;
		}

		pTarget_win->redraw();

#ifdef SURV
		gMutex.lock();
		condition = 0;
		gMutex.unlock();
#else
#endif

	}

	//Fl::run();
	CloseConnect();	
}