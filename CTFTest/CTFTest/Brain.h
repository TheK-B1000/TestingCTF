#ifndef BRAIN_H
#define BRAIN_H

#include <QString>
#include <QPointF>
#include "Agent.h"

enum class BrainDecision {
    Explore,
    GrabFlag,
    CaptureFlag,
    ReturnToHomeZone,
    RecoverFlag,
    TagEnemy
};

class Brain {
public:
    Brain();
    BrainDecision makeDecision(bool hasFlag, bool inHomeZone, float distanceToFlag, bool isTagged, bool enemyHasFlag, float distanceToNearestEnemy, bool isTaggign);

private:
    bool flagCaptured;
    int score;
    float proximityThreshold;
    float tagProximityThreshold;
};

#endif