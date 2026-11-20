#pragma once

#include "ai.h"
#include "board.h"

#include <random>
#include <vector>

// Monte Carlo Tree Search using the UCB1 rule.
// Each move is chosen by four steps: select, expand, rollout, backpropagate.
class MonteCarloTreeSearch
{
public:
	explicit MonteCarloTreeSearch(Difficulty difficulty);

	// Column to play for the player who is about to move.
	// Returns -1 when that player has no legal drop.
	int ChooseColumn(const Board& board);

private:
	struct Node;

	Node* Select(Node* node);
	Node* Expand(Node* node);
	int Rollout(Board state);
	void Backpropagate(Node* node, int winner);
	Node* BestChild(Node* node) const;
	void ShuffleColumns(std::vector<int>& columns);
	int RandomIndex(int count);

	int iterationCount;
	std::mt19937 randomEngine;
};
