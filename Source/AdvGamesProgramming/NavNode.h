#pragma once

#include "CoreMinimal.h"

/**
 * NavNode is a substitution of ANavigationNode class
 * as Actors take significantly more memory, but bring
 * more flexibility when nodes are manually edited
 * for the case of node generation it is inefficient.
 */
class ADVGAMESPROGRAMMING_API NavNode
{
public:
	NavNode();
	NavNode(FVector Location);

	FVector Location;
	float GScore;
	float HScore;
	float FScore();

	TMap <NavNode*, float> ConnectedNodes;

	NavNode* CameFrom;

	~NavNode();
};
