//
// Created by Lucas N. Ferreira on 26/10/23.
//

#include "GhostState.h"
#include "../Ghost.h"
#include "../../../Game.h"
#include "../../../Components/AIComponents/FSMComponent.h"

GhostState::GhostState(class FSMComponent *fsm, const std::string &name)
        : FSMState(fsm, name)
{
    mGhost = dynamic_cast<Ghost *>(mFSM->GetOwner());
}

void GhostState::Update(float deltaTime)
{
    // TODO [Parte 3.1]: https://ufv-inf216.lucasnferreira.com/p5-pacman
    if(mGhost->GetNextNode() == nullptr)
        return;
    auto* currentAABBComponent = mGhost->GetComponent<AABBColliderComponent>();
    auto* nextAABBComponent = mGhost->GetNextNode()->GetComponent<AABBColliderComponent>();

    if (currentAABBComponent->Intersect(*nextAABBComponent)) {
        mGhost->SetPosition(mGhost->GetNextNode()->GetPosition());
        mGhost->GetComponent<RigidBodyComponent>()->SetVelocity(Vector2(0, 0));

        PathNode* nextNode = FindNextNode();
        if (nextNode != nullptr) {
            mGhost->SetPreviousNode(mGhost->GetNextNode());
            mGhost->SetNextNode(nextNode);
            UpdateDirection();
        }
    }
    mGhost->GetComponent<RigidBodyComponent>()->SetVelocity(mGhost->GetDirection() * mGhost->GetForwardSpeed());
}

void GhostState::UpdateDirection()
{
    // TODO [Parte 3.2]: https://ufv-inf216.lucasnferreira.com/p5-pacman
    auto previousNode = mGhost->GetPreviousNode();
    auto nextNode = mGhost->GetNextNode();

    Vector2 updatedDirection =  nextNode->GetPosition() - previousNode->GetPosition();
    updatedDirection.Normalize();

    if(nextNode->GetType() == PathNode::Type::Tunnel && previousNode->GetType() == PathNode::Type::Tunnel) updatedDirection *= -1;

    mGhost->SetDirection(updatedDirection);
}

PathNode *GhostState::FindNearestNode(std::vector<PathNode*>& nodes, const Vector2& targetPosition, const std::set<PathNode::Type>* ignoreTypes, const std::set<PathNode *>* ignoreNodes)
{
    PathNode *nearestNode = nullptr;
    float minDist = 999999999999999.9f; //a big number to compare
    // TODO [Parte 3.3]: https://ufv-inf216.lucasnferreira.com/p5-pacman
    for(auto *node: nodes){
        if(ignoreNodes->find(node) != ignoreNodes->end() || ignoreTypes->find(node->GetType())!= ignoreTypes->end()) continue;

        float dist = (node->GetPosition() - targetPosition).Length();
        if(dist < minDist){
            nearestNode = node;
            minDist = dist;
        }
    }

    return nearestNode;
}
