#include "board.h"

Board::Board()
{
	Reset();
}

void Board::Reset()
{
	rowCount = START_ROW_COUNT;
	columnCount = START_COLUMN_COUNT;
	currentPlayer = PLAYER_ONE;
	winner = EMPTY;
	movesPlayed = 0;
	gameOver = false;
	shiftPending = false;
	gravityPending = false;
	grid.assign(rowCount, std::vector<int>(columnCount, EMPTY));
}

bool Board::DropPiece(int column)
{
	if (gameOver || !IsColumnOpen(column)) return false;

	int landingRow = 0;
	for (int row = 0; row < rowCount; row++) {
		if (grid[row][column] == EMPTY) {
			landingRow = row;
		}
	}

	grid[landingRow][column] = currentPlayer;
	movesPlayed++;

	if (HasFourInARow(currentPlayer)) {
		winner = currentPlayer;
		gameOver = true;
		return true;
	}

	if (movesPlayed % MOVES_PER_SHIFT == 0)
	{
		shiftPending = true;
		return true;
	}

	if (!HasOpenColumn()) {
		gameOver = true;
		return true;
	}

	currentPlayer = OtherPlayer(currentPlayer);

	return true;
}

int Board::GetCell(int row, int column) const
{
	return grid[row][column];
}

int Board::GetRowCount() const
{
	return rowCount;
}

int Board::GetColumnCount() const
{
	return columnCount;
}

int Board::GetCurrentPlayer() const
{
	return currentPlayer;
}

int Board::GetWinner() const
{
	return winner;
}

int Board::GetMovesUntilShift() const
{
	return MOVES_PER_SHIFT - (movesPlayed % MOVES_PER_SHIFT);
}

bool Board::IsGameOver() const
{
	return gameOver;
}

bool Board::IsColumnOpen(int column) const
{
	if (column < 0 || column >= columnCount)
		return false;
	return grid[0][column] == EMPTY;
}

bool Board::IsShiftPending() const
{
	return shiftPending;
}

void Board::CommitRotation()
{
	if (!shiftPending)
		return;
	RotateClockwise();
	shiftPending = false;
	gravityPending = true;
}

void Board::CommitGravity()
{
	if (!gravityPending)
		return;
	ApplyGravity();
	gravityPending = false;
	if (HasFourInARow(currentPlayer))
	{
		winner = currentPlayer;
		gameOver = true;
		return;
	}
	if (HasFourInARow(OtherPlayer(currentPlayer)))
	{
		winner = OtherPlayer(currentPlayer);
		gameOver = true;
		return;
	}
	if (!HasOpenColumn())
	{
		gameOver = true;
		return;
	}
	currentPlayer = OtherPlayer(currentPlayer);
}

std::vector<FallingPiece> Board::PreviewGravity() const
{
	std::vector<FallingPiece> pieces;
	for (int column = 0; column < columnCount; column++)
	{
		std::vector<int> occupiedRows;
		for (int row = rowCount - 1; row >= 0; row--)
		{
			if (grid[row][column] != EMPTY)
				occupiedRows.push_back(row);
		}
		for (int index = 0; index < (int)occupiedRows.size(); index++)
		{
			FallingPiece piece;
			piece.player = grid[occupiedRows[index]][column];
			piece.column = column;
			piece.fromRow = occupiedRows[index];
			piece.toRow = rowCount - 1 - index;
			pieces.push_back(piece);
		}
	}
	return pieces;
}

bool Board::HasFourInARow(int player) const
{
	for (int row = 0; row < rowCount; row++)
	{
		for (int column = 0; column <= columnCount - 4; column++)
		{
			if (grid[row][column] == player &&
				grid[row][column + 1] == player &&
				grid[row][column + 2] == player &&
				grid[row][column + 3] == player)
				return true;
		}
	}
	for (int row = 0; row <= rowCount - 4; row++)
	{
		for (int column = 0; column < columnCount; column++)
		{
			if (grid[row][column] == player &&
				grid[row + 1][column] == player &&
				grid[row + 2][column] == player &&
				grid[row + 3][column] == player)
				return true;
		}
	}
	for (int row = 0; row <= rowCount - 4; row++)
	{
		for (int column = 0; column <= columnCount - 4; column++)
		{
			if (grid[row][column] == player &&
				grid[row + 1][column + 1] == player &&
				grid[row + 2][column + 2] == player &&
				grid[row + 3][column + 3] == player)
				return true;
		}
	}
	for (int row = 0; row <= rowCount - 4; row++)
	{
		for (int column = 3; column < columnCount; column++)
		{
			if (grid[row][column] == player &&
				grid[row + 1][column - 1] == player &&
				grid[row + 2][column - 2] == player &&
				grid[row + 3][column - 3] == player)
				return true;
		}
	}
	return false;
}

bool Board::HasOpenColumn() const
{
	for (int column = 0; column < columnCount; column++)
	{
		if (IsColumnOpen(column))
			return true;
	}
	return false;
}

void Board::RotateClockwise()
{
	int newRowCount = columnCount;
	int newColumnCount = rowCount;
	std::vector<std::vector<int>> rotated(newRowCount, std::vector<int>(newColumnCount, EMPTY));
	for (int row = 0; row < rowCount; row++)
	{
		for (int column = 0; column < columnCount; column++)
		{
			int newRow = column;
			int newColumn = rowCount - 1 - row;
			rotated[newRow][newColumn] = grid[row][column];
		}
	}
	grid = rotated;
	rowCount = newRowCount;
	columnCount = newColumnCount;
}

void Board::ApplyGravity()
{
	for (int column = 0; column < columnCount; column++)
	{
		std::vector<int> pieces;
		for (int row = rowCount - 1; row >= 0; row--)
		{
			if (grid[row][column] != EMPTY)
				pieces.push_back(grid[row][column]);
		}
		for (int row = rowCount - 1; row >= 0; row--)
		{
			int indexFromBottom = (rowCount - 1) - row;
			if (indexFromBottom < (int)pieces.size())
				grid[row][column] = pieces[indexFromBottom];
			else
				grid[row][column] = EMPTY;
		}
	}
}

int Board::OtherPlayer(int player) const
{
	if (player == PLAYER_ONE)
		return PLAYER_TWO;
	return PLAYER_ONE;
}
