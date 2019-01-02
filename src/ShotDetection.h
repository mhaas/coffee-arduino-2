#ifndef SHOT_DETECTION_H
#define SHOT_DETECTION_H

#include <Average.h>

#endif

class ShotDetection {


    public:
        ShotDetection(double* currentTemperature);
        void begin();
        void update();

    private:
        double* _currentTemperature;
};