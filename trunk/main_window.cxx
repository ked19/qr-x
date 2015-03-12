// generated by Fast Light User Interface Designer (fluid) version 1.0302

#include "main_window.h"

Fl_Double_Window *pMain_win=(Fl_Double_Window *)0;

Fl_Group *pCamera_group=(Fl_Group *)0;

Fl_Input *pIP0_txtIn=(Fl_Input *)0;

Fl_Input *pIP1_txtIn=(Fl_Input *)0;

Fl_Input *pIP2_txtIn=(Fl_Input *)0;

Fl_Input *pIP3_txtIn=(Fl_Input *)0;

Fl_Input *pPort_txtIn=(Fl_Input *)0;

Fl_Input *pAccount_txtIn=(Fl_Input *)0;

Fl_Input *pPasswd_txtIn=(Fl_Input *)0;

Fl_Group *pShooter_group=(Fl_Group *)0;

Fl_Button *pConnect_bttn=(Fl_Button *)0;

Fl_Light_Button *pClose_litBttn=(Fl_Light_Button *)0;

Fl_Light_Button *pCalculate_litBttn=(Fl_Light_Button *)0;

MyGlWindow *pTarget_win=(MyGlWindow *)0;

Fl_Double_Window* make_window() {
  { pMain_win = new Fl_Double_Window(1234, 684, "Face recognition");
    { pCamera_group = new Fl_Group(943, 20, 289, 110, "Camera setting");
      pCamera_group->box(FL_DOWN_BOX);
      pCamera_group->align(Fl_Align(FL_ALIGN_TOP_LEFT));
      { pIP0_txtIn = new Fl_Input(1048, 26, 39, 24, "IP address:");
      } // Fl_Input* pIP0_txtIn
      { pIP1_txtIn = new Fl_Input(1095, 26, 39, 24, "*");
      } // Fl_Input* pIP1_txtIn
      { pIP2_txtIn = new Fl_Input(1142, 26, 39, 24, "*");
      } // Fl_Input* pIP2_txtIn
      { pIP3_txtIn = new Fl_Input(1189, 26, 39, 24, "*");
      } // Fl_Input* pIP3_txtIn
      { pPort_txtIn = new Fl_Input(1048, 50, 100, 24, "HTTP port:");
      } // Fl_Input* pPort_txtIn
      { pAccount_txtIn = new Fl_Input(1048, 76, 100, 24, "Account name:");
      } // Fl_Input* pAccount_txtIn
      { pPasswd_txtIn = new Fl_Input(1048, 101, 100, 24, "Password:");
      } // Fl_Input* pPasswd_txtIn
      pCamera_group->end();
    } // Fl_Group* pCamera_group
    { pShooter_group = new Fl_Group(943, 160, 214, 60, "User");
      pShooter_group->box(FL_DOWN_BOX);
      pShooter_group->align(Fl_Align(FL_ALIGN_TOP_LEFT));
      { pConnect_bttn = new Fl_Button(952, 195, 195, 20, "Connect");
        pConnect_bttn->callback((Fl_Callback*)RunConnect);
      } // Fl_Button* pConnect_bttn
      { pClose_litBttn = new Fl_Light_Button(952, 170, 68, 20, "Close");
        pClose_litBttn->callback((Fl_Callback*)Disconnect);
        pClose_litBttn->deactivate();
      } // Fl_Light_Button* pClose_litBttn
      pShooter_group->end();
    } // Fl_Group* pShooter_group
    { pCalculate_litBttn = new Fl_Light_Button(943, 230, 215, 40, "Calculate");
      pCalculate_litBttn->callback((Fl_Callback*)ComputeScore);
      pCalculate_litBttn->align(Fl_Align(FL_ALIGN_CENTER|FL_ALIGN_INSIDE));
      pCalculate_litBttn->deactivate();
    } // Fl_Light_Button* pCalculate_litBttn
    { pTarget_win = new MyGlWindow(5, 5, 935, 675);
      pTarget_win->box(FL_BORDER_BOX);
      pTarget_win->color(FL_BACKGROUND_COLOR);
      pTarget_win->selection_color(FL_BACKGROUND_COLOR);
      pTarget_win->labeltype(FL_NORMAL_LABEL);
      pTarget_win->labelfont(0);
      pTarget_win->labelsize(14);
      pTarget_win->labelcolor(FL_FOREGROUND_COLOR);
      pTarget_win->align(Fl_Align(FL_ALIGN_TOP));
      pTarget_win->when(FL_WHEN_RELEASE);
      pTarget_win->end();
    } // MyGlWindow* pTarget_win
    pMain_win->end();
  } // Fl_Double_Window* pMain_win
  //initialization
  pIP0_txtIn->value("172");
  pIP1_txtIn->value("16");
  pIP2_txtIn->value("15");
  pIP3_txtIn->value("225"); //208");
  
  pPort_txtIn->value("80");
  pAccount_txtIn->value("admin"); //Admin");
  pPasswd_txtIn->value("123456"); //ivs123456");
  return pMain_win;
}
