#ifndef WATCHY_SLIDES999_H
#define WATCHY_SLIDES99_H

#include "Watchy_Base.h"
#include "PebblecoNumbers_MONO32pt7b.h"

class WatchySlides999 : public WatchyBase{
    public:
        WatchySlides999();
        void drawWatchFace();
        void getTime();
        void drawTime();
        void getDate();
        void updateCache();
        void drawDate();
        void getSteps();
        void drawSteps();
        void drawAnim();
        void getRand();
        void checkBattery();
};

#endif
