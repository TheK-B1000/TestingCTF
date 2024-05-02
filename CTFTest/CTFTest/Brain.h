#ifndef BRAIN_H
#define BRAIN_H

#include <QPointF>

enum class BrainDecision {
    Explore,
    GrabFlag,
    CaptureFlag,
    AvoidEnemy,
    RecoverFlag,
    DefendFlag,
    TagEnemy,
    ReturnToHomeZone
};

class Brain {
public:
    Brain();
    BrainDecision makeDecision(bool hasBlueFlag, bool hasRedFlag, bool inHomeZone, float distanceToFlag, bool isTagged, bool enemyHasFlag, float distanceToNearestEnemy, bool isTagging, bool isStuckInMiddle, bool inSide);

private:
    bool blueFlagCaptured;
    bool redFlagCaptured;
    int score;
    float proximityThreshold;
    float tagProximityThreshold;
};

#endif 