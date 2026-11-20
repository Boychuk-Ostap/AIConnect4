#pragma once

#include "ai.h"
#include "board.h"

// Depth-limited minimax with alpha-beta pruning.
// Alpha-beta skips branches that cannot change the chosen move.
// The move it returns is the same move plain minimax would return.
class Minimax
{
public:
	explicit Minimax(Difficulty difficulty);

	// Best column for the player who is about to move.
	// Returns -1 when that player has no legal drop.
	int ChooseColumn(const Board& board) const;

private:
	int Search(const Board& state, int depth, int alpha, int beta, int rootPlayer) const;
	int Evaluate(const Board& state, int rootPlayer) const;
	int ScoreLine(int cell0, int cell1, int cell2, int cell3, int rootPlayer) const;
	int ScoreWindow(int rootPieceCount, int opponentPieceCount) const;
	int TerminalScore(const Board& state, int depthRemaining, int rootPlayer) const;

	int depthLimit;
};
