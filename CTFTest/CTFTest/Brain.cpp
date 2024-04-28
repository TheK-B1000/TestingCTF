#include "Brain.h"
#include <QPointF>
#include <cmath>

Brain::Brain() : flagCaptured(false), score(0), proximityThreshold(150.0f) {}

BrainDecision Brain::makeDecision(bool hasFlag, bool inHomeZone, float distanceToFlag) {

    if (hasFlag) {
        if (inHomeZone) {
            if (!flagCaptured) {
                flagCaptured = true;
                score++;
            }
            return BrainDecision::CaptureFlag;
        }
        else {
            return BrainDecision::ReturnToHomeZone;
        }
    }
    else {
        if (distanceToFlag <= proximityThreshold) {
            return BrainDecision::GrabFlag;
        }
        else {
            return BrainDecision::Explore;
        }
    }
}
