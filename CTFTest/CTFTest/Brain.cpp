#include "Brain.h"
#include "Agent.h"
#include <QPointF>
#include <cmath>

Brain::Brain() : flagCaptured(false), score(0), proximityThreshold(250.0f), tagProximityThreshold(100.0f) {}
BrainDecision Brain::makeDecision(bool hasFlag, bool inHomeZone, float distanceToFlag, bool isTagged, bool enemyHasFlag, float distanceToNearestEnemy, bool isTagging) {
    if (isTagged) {
        flagCaptured = false; // Reset flag captured status when tagged
        return BrainDecision::ReturnToHomeZone;
    }

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
        if (enemyHasFlag) {
            return BrainDecision::RecoverFlag;
        }

        if (distanceToNearestEnemy < tagProximityThreshold) {
            // If the nearest enemy can be tagged, make the tagging decision
            return BrainDecision::TagEnemy;
        }

        if (isTagging) {
            if (!inHomeZone || distanceToNearestEnemy > tagProximityThreshold) {
                // Exit tagging behavior if not in home zone or enemy is too far
                return BrainDecision::Explore;
            }
            else {
                return BrainDecision::TagEnemy;
            }
        }
        else {
            if (distanceToFlag <= proximityThreshold) {
                return BrainDecision::GrabFlag;
            }

            return BrainDecision::Explore;
        }
    }
}