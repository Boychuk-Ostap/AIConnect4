#include "mcts.h"

#include <algorithm>
#include <cmath>
#include <memory>

namespace
{
	// UCB1 constant. Rewards are in the range -1 to +1, and sqrt(2) is the
	// usual exploration weight for that range.
	const double EXPLORATION = 1.41421356237;
}

struct MonteCarloTreeSearch::Node
{
	Board state;
	int columnPlayed = -1;
	int playerWhoMoved = Board::EMPTY;
	Node* parent = nullptr;
	std::vector<std::unique_ptr<Node>> children;
	std::vector<int> untriedColumns;
	int visits = 0;
	double totalReward = 0.0;
};

MonteCarloTreeSearch::MonteCarloTreeSearch(Difficulty difficulty)
	: iterationCount(SimulationCountFor(difficulty))
	, randomEngine(std::random_device{}())
{
}

int MonteCarloTreeSearch::ChooseColumn(const Board& board)
{
	if (board.IsGameOver())
		return -1;

	auto root = std::make_unique<Node>();
	root->state = board;
	root->untriedColumns = board.GetOpenColumns();
	ShuffleColumns(root->untriedColumns);
	if (root->untriedColumns.empty())
		return -1;

	for (int iteration = 0; iteration < iterationCount; iteration++)
	{
		Node* node = Select(root.get());
		if (!node->state.IsGameOver() && !node->untriedColumns.empty())
		{
			Node* expanded = Expand(node);
			if (expanded != nullptr)
				node = expanded;
		}

		int winner = Rollout(node->state);
		Backpropagate(node, winner);
	}

	// Most visits is steadier than the highest average. A single lucky
	// win would otherwise look perfect.
	Node* chosen = nullptr;
	for (int i = 0; i < (int)root->children.size(); i++)
	{
		Node* child = root->children[i].get();
		if (chosen == nullptr || child->visits > chosen->visits)
		{
			chosen = child;
			continue;
		}
		if (child->visits == chosen->visits && child->totalReward > chosen->totalReward)
			chosen = child;
	}

	if (chosen == nullptr)
		return root->untriedColumns[0];
	return chosen->columnPlayed;
}

MonteCarloTreeSearch::Node* MonteCarloTreeSearch::Select(Node* node)
{
	while (!node->state.IsGameOver() && node->untriedColumns.empty() && !node->children.empty())
	{
		Node* next = BestChild(node);
		if (next == nullptr)
			break;
		node = next;
	}
	return node;
}

MonteCarloTreeSearch::Node* MonteCarloTreeSearch::Expand(Node* node)
{
	while (!node->untriedColumns.empty())
	{
		int column = node->untriedColumns.back();
		node->untriedColumns.pop_back();

		Board next = node->state;
		if (!next.PlayTurn(column))
			continue;

		auto child = std::make_unique<Node>();
		child->state = std::move(next);
		child->columnPlayed = column;
		child->playerWhoMoved = node->state.GetCurrentPlayer();
		child->parent = node;
		if (!child->state.IsGameOver())
		{
			child->untriedColumns = child->state.GetOpenColumns();
			ShuffleColumns(child->untriedColumns);
		}

		Node* rawChild = child.get();
		node->children.push_back(std::move(child));
		return rawChild;
	}

	return nullptr;
}

int MonteCarloTreeSearch::Rollout(Board state)
{
	// The board never grows past its starting 42 cells, so this cap is
	// only a guard against a rules bug causing an endless loop.
	int guard = 64;
	while (!state.IsGameOver() && guard > 0)
	{
		std::vector<int> columns = state.GetOpenColumns();
		if (columns.empty())
			break;
		int column = columns[RandomIndex((int)columns.size())];
		if (!state.PlayTurn(column))
			break;
		guard--;
	}
	return state.GetWinner();
}

void MonteCarloTreeSearch::Backpropagate(Node* node, int winner)
{
	while (node != nullptr)
	{
		node->visits++;
		if (node->playerWhoMoved != Board::EMPTY)
		{
			if (winner == node->playerWhoMoved)
				node->totalReward += 1.0;
			else if (winner != Board::EMPTY)
				node->totalReward -= 1.0;
		}
		node = node->parent;
	}
}

MonteCarloTreeSearch::Node* MonteCarloTreeSearch::BestChild(Node* node) const
{
	Node* best = nullptr;
	double bestScore = 0.0;
	double parentVisits = (double)node->visits;
	if (parentVisits < 1.0)
		parentVisits = 1.0;
	double logParentVisits = std::log(parentVisits);

	for (int i = 0; i < (int)node->children.size(); i++)
	{
		Node* child = node->children[i].get();
		if (child->visits <= 0)
			continue;

		double averageReward = child->totalReward / (double)child->visits;
		double exploration = EXPLORATION * std::sqrt(logParentVisits / (double)child->visits);
		double score = averageReward + exploration;
		if (best == nullptr || score > bestScore)
		{
			best = child;
			bestScore = score;
		}
	}

	if (best == nullptr && !node->children.empty())
		return node->children[0].get();
	return best;
}

void MonteCarloTreeSearch::ShuffleColumns(std::vector<int>& columns)
{
	std::shuffle(columns.begin(), columns.end(), randomEngine);
}

int MonteCarloTreeSearch::RandomIndex(int count)
{
	std::uniform_int_distribution<int> distribution(0, count - 1);
	return distribution(randomEngine);
}
