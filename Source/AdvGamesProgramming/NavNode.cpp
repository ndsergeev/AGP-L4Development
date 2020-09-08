#include "NavNode.h"

NavNode::NavNode()
{
    GScore = 0;
    HScore = 0;
    CameFrom = nullptr;
    Location = FVector::ZeroVector;
}

NavNode::NavNode(FVector Location)
{
    NavNode();
    this->Location = Location;
}

NavNode::~NavNode()
{
    ConnectedNodes.Empty();
    delete this;
}

float NavNode::FScore() {
    return GScore + HScore;
}
