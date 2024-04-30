#include "Brain.h"
#include "Agent.h"
#include <QPointF>
#include <cmath>

Brain::Brain() : flagCaptured(false), score(0), proximityThreshold(250.0f), tagProximityThreshold(200.0f) {}

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
            if (distanceToNearestEnemy < tagProximityThreshold) {
                // If an enemy is nearby while carrying the flag, try to avoid them
                return BrainDecision::AvoidEnemy;
            }
            else {
                return BrainDecision::ReturnToHomeZone;
            }
        }
    }
    else {
        if (enemyHasFlag) {
            if (inHomeZone) {
                // If the enemy has the flag and the agent is in the home zone, try to tag them
                return BrainDecision::TagEnemy;
            }
            else {
                return BrainDecision::RecoverFlag;
            }
        }

        if (inHomeZone) {
            if (distanceToNearestEnemy < tagProximityThreshold) {
                // If an enemy is nearby in the home zone, tag them
                return BrainDecision::TagEnemy;
            }
            else {
                // If no enemy is nearby in the home zone, defend the flag
                return BrainDecision::DefendFlag;
            }
        }
        else {
            if (distanceToNearestEnemy < tagProximityThreshold) {
                // If an enemy is nearby on the enemy side, avoid them
                return BrainDecision::AvoidEnemy;
            }
            else {
                if (distanceToFlag <= proximityThreshold) {
                    return BrainDecision::GrabFlag;
                }
                return BrainDecision::Explore;
            }
        }
    }
}