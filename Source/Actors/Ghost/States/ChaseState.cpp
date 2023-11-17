//
// Created by Lucas N. Ferreira on 26/10/23.
//

#include "ChaseState.h"
#include "../Ghost.h"
#include "../../../Game.h"
#include "../../../Actors/Pacman.h"
#include "../../../Components/AIComponents/FSMComponent.h"

ChaseState::ChaseState(FSMComponent *fsm)
            :GhostState(fsm, "chase")
{

}

void ChaseState::Start()
{
    // TODO [Parte 5.1]: https://ufv-inf216.lucasnferreira.com/p5-pacman
    mGhost->SetForwardSpeed(90.0f);
}

PathNode* ChaseState::FindNextNode()
{
    PathNode *targetNode = FindTargetState();
    mGhost->SetTargetNode(targetNode);

    // Select next node
    PathNode *nextNode = nullptr;

    // TODO [Parte 5.2]: https://ufv-inf216.lucasnferreira.com/p5-pacman
    std::set<PathNode*> ignoreNode{ mGhost->GetPreviousNode() };
    std::set<PathNode::Type> ignoreType{ PathNode::Type::Ghost };

    nextNode = FindNearestNode(mGhost->GetNextNode()->GetAdjacents(),targetNode->GetPosition(),&ignoreType,&ignoreNode);

    if(nextNode == nullptr){
        ignoreNode.clear();
        ignoreType.clear();
        nextNode = FindNearestNode(mGhost->GetNextNode()->GetAdjacents(),targetNode->GetPosition(),&ignoreType,&ignoreNode);
    }

    return nextNode;
}

PathNode* ChaseState::FindTargetState()
{
    PathNode *targetNode = nullptr;

    // TODO [Parte 5.3]: https://ufv-inf216.lucasnferreira.com/p5-pacman
    Ghost::Type type = mGhost->GetType();
    const auto player = mGhost->GetGame()->GetPlayer();
    std::set<PathNode*> ignoreNodes;
    std::set<PathNode::Type> ignoreTypes{ PathNode::Type::Ghost, PathNode::Type::Tunnel  };


    if(type == Ghost::Type::Blinky)
        return player->GetPrevNode();

    if(type == Ghost::Type::Pinky){
        auto goalPosition = player->GetPointInFrontOf(80.0f);
        return FindNearestNode(mGhost->GetGame()->GetPathNodes(),goalPosition,&ignoreTypes,&ignoreNodes);
    }

    if(type == Ghost::Type::Inky){
        Vector2 goalPosition = mGhost->GetGame()->GetPlayer()->GetPointInFrontOf(40.0f);
        Vector2 v  = goalPosition - mGhost->GetPosition();
        Vector2 q = v*2 + mGhost->GetPosition();
        return FindNearestNode(mGhost->GetGame()->GetPathNodes(),q,&ignoreTypes,&ignoreNodes);
    }

    if(type == Ghost::Type::Clyde) {
        float dist = (mGhost->GetPosition() - mGhost->GetGame()->GetPlayer()->GetPosition()).LengthSq();
        if (dist > 150)
            return mGhost->GetGame()->GetPlayer()->GetPrevNode();
        return mGhost->GetScatterNode();

    }
}


void ChaseState::HandleStateTransition(float stateTime)
{
    // TODO [Parte 5.4]: https://ufv-inf216.lucasnferreira.com/p5-pacman
    if(stateTime > 20.0f)   mFSM->SetState("scatter");
}