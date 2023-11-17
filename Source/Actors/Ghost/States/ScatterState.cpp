//
// Created by Lucas N. Ferreira on 26/10/23.
//

#include "ScatterState.h"
#include "../Ghost.h"
#include "../../../Components/AIComponents/FSMComponent.h"

ScatterState::ScatterState(FSMComponent *fsm)
            :GhostState(fsm, "scatter")
{

}

void ScatterState::Start()
{
    // TODO [Parte 4.1]: https://ufv-inf216.lucasnferreira.com/p5-pacman
    mGhost->SetTargetNode(mGhost->GetScatterNode());
    mGhost->SetForwardSpeed(90.0f);
}

PathNode* ScatterState::FindNextNode()
{
    PathNode *nextNode = nullptr;

    // TODO [Parte 4.2]: https://ufv-inf216.lucasnferreira.com/p5-pacman
    std::set<PathNode*> ignoreNode{ mGhost->GetPreviousNode() };
    std::set<PathNode::Type> ignoreType{ PathNode::Type::Ghost, PathNode::Type::Tunnel };

    nextNode = FindNearestNode(mGhost->GetNextNode()->GetAdjacents(),mGhost->GetTargetNode()->GetPosition(),&ignoreType,&ignoreNode);

    if(nextNode == nullptr){
        ignoreType.erase(PathNode::Type::Tunnel);
        nextNode = FindNearestNode(mGhost->GetNextNode()->GetAdjacents(),mGhost->GetTargetNode()->GetPosition(),&ignoreType,&ignoreNode);
    }

    if(nextNode == nullptr){
        ignoreType.clear();
        ignoreNode.erase(mGhost->GetPreviousNode());
        nextNode = FindNearestNode(mGhost->GetNextNode()->GetAdjacents(),mGhost->GetTargetNode()->GetPosition(),&ignoreType,&ignoreNode);
    }

    return nextNode;
}

void ScatterState::HandleStateTransition(float stateTime)
{
    // TODO [Parte 4.3]: https://ufv-inf216.lucasnferreira.com/p5-pacman
    if(stateTime > 5) mFSM->SetState("chase");
}