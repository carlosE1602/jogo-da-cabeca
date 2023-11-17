//
// Created by Lucas N. Ferreira on 26/10/23.
//

#include "DeadState.h"
#include "../Ghost.h"
#include "../../../Game.h"
#include "../../../Actors/Pacman.h"
#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Random.h"

DeadState::DeadState(FSMComponent *fsm)
        :GhostState(fsm, "dead")
{

}

void DeadState::Start()
{
    // TODO [Parte 7.1]: https://ufv-inf216.lucasnferreira.com/p5-pacman
    mGhost->SetTargetNode(mGhost->GetGame()->GetGhostPen());
    mGhost->SetForwardSpeed(125.0f);
}

PathNode* DeadState::FindNextNode()
{
    PathNode *nextNode = nullptr;

    // TODO [Parte 7.2]: https://ufv-inf216.lucasnferreira.com/p5-pacman
    std::set<PathNode*> ignoreNodes{ mGhost->GetPreviousNode() };
    std::set<PathNode::Type> ignoreTypes{ PathNode::Type::Default, PathNode::Type::Tunnel };
    nextNode = FindNearestNode(mGhost->GetNextNode()->GetAdjacents(),mGhost->GetTargetNode()->GetPosition(),&ignoreTypes,&ignoreNodes);

    if(nextNode == nullptr){
        ignoreTypes.erase(PathNode::Default);
        nextNode = FindNearestNode(mGhost->GetNextNode()->GetAdjacents(),mGhost->GetTargetNode()->GetPosition(),&ignoreTypes,&ignoreNodes);
    }

    if(nextNode == nullptr){
        ignoreTypes.clear();
        ignoreTypes.clear();
        nextNode = FindNearestNode(mGhost->GetNextNode()->GetAdjacents(),mGhost->GetTargetNode()->GetPosition(),&ignoreTypes,&ignoreNodes);
    }

    return nextNode;
}



void DeadState::HandleStateTransition(float stateTime)
{
    // TODO [Parte 7.3]: https://ufv-inf216.lucasnferreira.com/p5-pacman
    auto* penAABB = mGhost->GetGame()->GetGhostPen()->GetComponent<AABBColliderComponent>();
    if(mGhost->GetComponent<AABBColliderComponent>()->Intersect(*penAABB)){
        mFSM->SetState("scatter");
    }
}