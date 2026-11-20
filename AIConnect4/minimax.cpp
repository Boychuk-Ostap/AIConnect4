#include "minimax.h"

#include <vector>

namespace
{
	const int WIN_SCORE = 100000;
	const int SCORE_LIMIT = 1000000000;

	// Try the middle column first. Equal scores then keep the first column,
	// so a tied position prefers the centre. It also helps alpha-beta cut sooner.
	std::vector<int> ColumnsFromCentre(const Board& state)
	{
		std::vector<int> ordered;
		int columnCount = state.GetColumnCount();
		int centre = columnCount / 2;

		for (int distance = 0; distance < columnCount; distance++)
		{
			if (distance == 0)
			{
				if (state.IsColumnOpen(centre))
					ordered.push_back(centre);
				continue;
			}

			int left = centre - distance;
			int right = centre + distance;
			if (left >= 0 && state.IsColumnOpen(left))
				ordered.push_back(left);
			if (right < columnCount && state.IsColumnOpen(right))
				ordered.push_back(right);
		}

		return ordered;
	}
}

Minimax::Minimax(Difficulty difficulty)
	: depthLimit(SearchDepthFor(difficulty))
{
}

int Minimax::ChooseColumn(const Board& board) const
{
	std::vector<int> columns = ColumnsFromCentre(board);
	if (columns.empty())
		return -1;

	int rootPlayer = board.GetCurrentPlayer();
	int alpha = -SCORE_LIMIT;
	int beta = SCORE_LIMIT;
	int bestScore = -SCORE_LIMIT;
	int bestColumn = columns[0];

	for (int i = 0; i < (int)columns.size(); i++)
	{
		int column = columns[i];
		Board next = board;
		if (!next.PlayTurn(column))
			continue;

		int score = Search(next, depthLimit - 1, alpha, beta, rootPlayer);
		if (score > bestScore)
		{
			bestScore = score;
			bestColumn = column;
		}
		// alpha is the best score the player to move can already force
		if (score > alpha)
			alpha = score;
	}

	return bestColumn;
}

int Minimax::Search(const Board& state, int depth, int alpha, int beta, int rootPlayer) const
{
	if (state.IsGameOver())
		return TerminalScore(state, depth, rootPlayer);
	if (depth <= 0)
		return Evaluate(state, rootPlayer);

	std::vector<int> columns = ColumnsFromCentre(state);
	if (columns.empty())
		return Evaluate(state, rootPlayer);

	bool maximising = state.GetCurrentPlayer() == rootPlayer;
	int bestScore = maximising ? -SCORE_LIMIT : SCORE_LIMIT;
	bool playedMove = false;

	for (int i = 0; i < (int)columns.size(); i++)
	{
		Board next = state;
		if (!next.PlayTurn(columns[i]))
			continue;

		playedMove = true;
		int score = Search(next, depth - 1, alpha, beta, rootPlayer);
		if (maximising)
		{
			if (score > bestScore)
				bestScore = score;
			if (score > alpha)
				alpha = score;
		}
		else
		{
			if (score < bestScore)
				bestScore = score;
			if (score < beta)
				beta = score;
		}

		// beta is the best the opponent will allow.
		// A score outside [alpha, beta] cannot change the move above us.
		if (alpha >= beta)
			break;
	}

	if (!playedMove)
		return Evaluate(state, rootPlayer);
	return bestScore;
}

int Minimax::TerminalScore(const Board& state, int depthRemaining, int rootPlayer) const
{
	if (state.GetWinner() == rootPlayer)
		return WIN_SCORE + depthRemaining;
	if (state.GetWinner() == Board::EMPTY)
		return 0;
	return -WIN_SCORE - depthRemaining;
}

int Minimax::Evaluate(const Board& state, int rootPlayer) const
{
	int score = 0;
	int rowCount = state.GetRowCount();
	int columnCount = state.GetColumnCount();

	for (int row = 0; row < rowCount; row++)
	{
		for (int column = 0; column <= columnCount - 4; column++)
		{
			score += ScoreLine(
				state.GetCell(row, column),
				state.GetCell(row, column + 1),
				state.GetCell(row, column + 2),
				state.GetCell(row, column + 3),
				rootPlayer);
		}
	}

	for (int column = 0; column < columnCount; column++)
	{
		for (int row = 0; row <= rowCount - 4; row++)
		{
			score += ScoreLine(
				state.GetCell(row, column),
				state.GetCell(row + 1, column),
				state.GetCell(row + 2, column),
				state.GetCell(row + 3, column),
				rootPlayer);
		}
	}

	for (int row = 0; row <= rowCount - 4; row++)
	{
		for (int column = 0; column <= columnCount - 4; column++)
		{
			score += ScoreLine(
				state.GetCell(row, column),
				state.GetCell(row + 1, column + 1),
				state.GetCell(row + 2, column + 2),
				state.GetCell(row + 3, column + 3),
				rootPlayer);
		}
	}

	for (int row = 0; row <= rowCount - 4; row++)
	{
		for (int column = 3; column < columnCount; column++)
		{
			score += ScoreLine(
				state.GetCell(row, column),
				state.GetCell(row + 1, column - 1),
				state.GetCell(row + 2, column - 2),
				state.GetCell(row + 3, column - 3),
				rootPlayer);
		}
	}

	// A small nudge towards the current centre column. Window scores are
	// much larger, so this only breaks ties in the heuristic.
	int centre = columnCount / 2;
	for (int row = 0; row < rowCount; row++)
	{
		for (int column = 0; column < columnCount; column++)
		{
			int cell = state.GetCell(row, column);
			if (cell == Board::EMPTY)
				continue;

			int distance = column - centre;
			if (distance < 0)
				distance = -distance;

			int placement = centre - distance;
			if (cell == rootPlayer)
				score += placement;
			else
				score -= placement;
		}
	}

	return score;
}

int Minimax::ScoreLine(int cell0, int cell1, int cell2, int cell3, int rootPlayer) const
{
	int cells[4] = { cell0, cell1, cell2, cell3 };
	int rootPieceCount = 0;
	int opponentPieceCount = 0;

	for (int i = 0; i < 4; i++)
	{
		if (cells[i] == rootPlayer)
			rootPieceCount++;
		else if (cells[i] != Board::EMPTY)
			opponentPieceCount++;
	}

	return ScoreWindow(rootPieceCount, opponentPieceCount);
}

int Minimax::ScoreWindow(int rootPieceCount, int opponentPieceCount) const
{
	int emptyCount = 4 - rootPieceCount - opponentPieceCount;
	if (rootPieceCount == 4)
		return 5000;
	if (opponentPieceCount == 4)
		return -5000;
	if (rootPieceCount > 0 && opponentPieceCount > 0)
		return 0;

	if (rootPieceCount == 3 && emptyCount == 1)
		return 50;
	if (rootPieceCount == 2 && emptyCount == 2)
		return 10;
	if (rootPieceCount == 1 && emptyCount == 3)
		return 1;

	// An open three for the opponent is worse than an open three for us.
	// Leaving it there loses on the next move, and a short search may not
	// reach that loss yet.
	if (opponentPieceCount == 3 && emptyCount == 1)
		return -80;
	if (opponentPieceCount == 2 && emptyCount == 2)
		return -10;
	if (opponentPieceCount == 1 && emptyCount == 3)
		return -1;

	return 0;
}
