#ifndef BRAIN_H
#define BRAIN_H

#include <QString>
#include <QPointF>
#include "Agent.h"

enum class BrainDecision {
    Explore,
    GrabFlag,
    CaptureFlag,
    ReturnToHomeZone
};

class Brain {
public:
    Brain();
    BrainDecision makeDecision(bool hasFlag, bool inHomeZone, float distanceToFlag);

private:
    bool flagCaptured;
    int score;
    float proximityThreshold;
};

#endif