//////////////////////////////////////////////////////////////////                                                            //// glutWindow.h                                               //// beta version 0.3 - 9/9/97)                                 ////                                                            //// George Stetten and Korin Crawford                          //// copyright given to the public domain                       ////                                                            //// Please email comments to email@stetten.com                 ////                                                            //////////////////////////////////////////////////////////////////#ifndef __GLUT_WINDOW_H__#define __GLUT_WINDOW_H__class GlutWindow{protected:   int          windowID;public:   GlutWindow();   ~GlutWindow();   virtual void CallBackDisplayFunc() {}   virtual void CallBackIdleFunc() {}   virtual void CallBackKeyboardFunc(unsigned char key, int x, int y) {}   virtual void CallBackKeyboardUpFunc(unsigned char key, int x, int y) {}   virtual void CallBackMotionFunc(int x, int y) {}   virtual void CallBackMouseFunc(int button, int state, int x, int y) {}   virtual void CallBackMouseWheelFunc(int button, int dir, int x, int y) {}   virtual void CallBackPassiveMotionFunc(int x, int y) {}   virtual void CallBackReshapeFunc(int w, int h) {}   virtual void CallBackSpecialFunc(int key, int x, int y) {}   virtual void CallBackSpecialUpFunc(int key, int x, int y) {}   virtual void CallBackVisibilityFunc(int visible) {}   void    SetWindowID(const int newWindowID) { windowID = newWindowID; }   int     GetWindowID() { return windowID; }};#endif