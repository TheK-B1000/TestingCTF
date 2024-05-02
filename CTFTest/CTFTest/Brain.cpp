#include "Brain.h"
#include "Agent.h"
#include <QPointF>
#include <cmath>

Brain::Brain() : flagCaptured(false), score(0), proximityThreshold(250.0f), tagProximityThreshold(200.0f) {}

BrainDecision Brain::makeDecision(bool hasFlag, bool inHomeZone, float distanceToFlag, bool isTagged, bool enemyHasFlag, float distanceToNearestEnemy, bool isTagging, bool isStuckInMiddle, bool inSide) {
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
                // an enemy is nearby while carrying the flag
                return BrainDecision::AvoidEnemy;
            }
            else {
                // no enemy is nearby, move towards the home zone
                return BrainDecision::ReturnToHomeZone;
            }
        }
    }
    else {
        if (enemyHasFlag) {
            if (inSide) {
                // the enemy has the flag and the agent is in the home zone
                return BrainDecision::TagEnemy;
            }
            else {
                if (distanceToNearestEnemy < tagProximityThreshold) {
                    // an enemy is nearby while the enemy has the flag, avoid the enemy
                    return BrainDecision::AvoidEnemy;
                }
                else {
                    // the enemy has the flag and the agent is not in the home zone
                    return BrainDecision::RecoverFlag;
                }
            }
        }
        else {
            if (inSide) {
                if (distanceToNearestEnemy < tagProximityThreshold) {
                    // an enemy is nearby in the home zone
                    return BrainDecision::TagEnemy;
                }
                else {
                    // no enemy is nearby in the home zone
                    return BrainDecision::DefendFlag;
                }
            }
            else {
                if (isStuckInMiddle || distanceToFlag > proximityThreshold) {
                    // the agent is stuck in the middle for too long or the flag is far away
                    return BrainDecision::Explore;
                }
                else {
                    if (distanceToNearestEnemy < tagProximityThreshold) {
                        // an enemy is nearby on the enemy side
                        return BrainDecision::AvoidEnemy;
                    }
                    else {
                        // no enemy is nearby and the flag is within proximity
                        return BrainDecision::GrabFlag;
                    }
                }
            }
        }
    }
}