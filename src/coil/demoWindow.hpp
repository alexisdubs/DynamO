#pragma once#include <time.h>#include "clWindow.hpp"class demoWindow : public CLGLWindow{ public:  demoWindow(GlutMaster& gMaster,	     int setWidth, int setHeight,	     int setInitPositionX, int setInitPositionY,	     const char * title,	     cl::Platform& plat);    virtual void CallBackDisplayFunc(void);    void clTick();   protected:};