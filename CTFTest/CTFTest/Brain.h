#ifndef BRAIN_H
#define BRAIN_H

enum class BrainDecision {
    CaptureFlag,
    TagEnemy,
    RecoverFlag,
    ReturnToHomeZone,
    GrabFlag,
    Explore
};

class Brain {
public:
    Brain();
    BrainDecision makeDecision(bool hasFlag, bool opponentHasFlag, bool isTagged, bool inHomeZone, float distanceToFlag, float distanceToNearestEnemy);

private:
    bool flagCaptured;
    int score;
    float proximityThreshold;

    float evaluateFlagCapture(bool hasFlag, float distanceToFlag);
    float evaluateFlagRecovery(bool opponentHasFlag, float distanceToNearestEnemy);
    float evaluateExplore(bool hasFlag, bool opponentHasFlag, bool inHomeZone);
};

#endif