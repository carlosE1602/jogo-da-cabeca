//
// Created by Lucas N. Ferreira on 26/10/23.
//

#include "FrightenedState.h"
#include "../Ghost.h"
#include "../../../Game.h"
#include "../../../Actors/Pacman.h"
#include "../../../Components/AIComponents/FSMComponent.h"
#include "../../../Random.h"

FrightenedState::FrightenedState(FSMComponent *fsm)
        :GhostState(fsm, "frightened")
{

}

void FrightenedState::Start()
{
    // TODO [Parte 6.1]: https://ufv-inf216.lucasnferreira.com/p5-pacman
    PathNode* next = mGhost->GetNextNode();
    mGhost->SetForwardSpeed(65.0f);
    mGhost->SetTargetNode(nullptr);
    mGhost->SetNextNode(mGhost->GetPreviousNode());
    mGhost->SetPreviousNode(next);
    UpdateDirection();
}

PathNode* FrightenedState::FindNextNode()
{
    PathNode *nextNode = nullptr;

    // TODO [Parte 6.2]: https://ufv-inf216.lucasnferreira.com/p5-pacman
    std::vector<PathNode*> adj;
    for(PathNode* node: mGhost->GetNextNode()->GetAdjacents()){
        if(node != mGhost->GetPreviousNode() && node->GetType() == PathNode::Type::Default){
            adj.emplace_back(node);
        }
    }

    if(adj.empty()){
        for(PathNode* node: mGhost->GetNextNode()->GetAdjacents()){
            if(node != mGhost->GetPreviousNode() && node->GetType() != PathNode::Type::Tunnel){
                adj.emplace_back(node);
            }
        }
    }

    if(adj.empty()){
        adj = mGhost->GetNextNode()->GetAdjacents();
    }

    nextNode = adj[std::rand()%adj.size()];
    return nextNode;
}

void FrightenedState::HandleStateTransition(float stateTime)
{
    // TODO [Parte 6.3]: https://ufv-inf216.lucasnferreira.com/p5-pacman
    if(stateTime > 7.0f)    mFSM->SetState("scatter");

}