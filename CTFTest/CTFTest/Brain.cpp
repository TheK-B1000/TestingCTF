#include "Brain.h"
#include "Agent.h"
#include <QPointF>
#include <cmath>

Brain::Brain() : flagCaptured(false), score(0), proximityThreshold(250.0f), tagProximityThreshold(200.0f) {}

BrainDecision Brain::makeDecision(bool hasFlag, bool inHomeZone, float distanceToFlag, bool isTagged, bool enemyHasFlag, float distanceToNearestEnemy, bool isTagging, bool isStuckInMiddle) {
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
                // If no enemy is nearby, move towards the home zone
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
                // If the enemy has the flag and the agent is not in the home zone, recover the flag
                return BrainDecision::RecoverFlag;
            }
        }
        else {
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
                if (isStuckInMiddle || distanceToFlag > proximityThreshold) {
                    // If the agent is stuck in the middle for too long or the flag is far away, explore the field
                    return BrainDecision::Explore;
                }
                else if (distanceToNearestEnemy < tagProximityThreshold) {
                    // If an enemy is nearby on the enemy side, avoid them
                    return BrainDecision::AvoidEnemy;
                }
                else {
                    // If no enemy is nearby and the flag is within proximity, grab the flag
                    return BrainDecision::GrabFlag;
                }
            }
        }
    }
}