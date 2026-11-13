#pragma once
#include <vector>

struct FallingPiece
{
    int player;
    int column;
    int fromRow;
    int toRow;
};

class Board {
public:
	static const int START_ROW_COUNT = 6;
	static const int START_COLUMN_COUNT = 7;
	static const int EMPTY = 0;
	static const int PLAYER_ONE = 1;
	static const int PLAYER_TWO = 2;
	static const int MOVES_PER_SHIFT = 4;

	Board();

	void Reset();

	bool DropPiece(int column);

	int GetCell(int row, int column) const;
	int GetRowCount() const;
    int GetColumnCount() const;
    int GetCurrentPlayer() const;
    int GetWinner() const;
    int GetMovesUntilShift() const;
    bool IsGameOver() const;
    bool IsColumnOpen(int column) const;
    bool IsShiftPending() const;
    void CommitRotation();
    void CommitGravity();
    std::vector<FallingPiece> PreviewGravity() const;
private:
    bool HasFourInARow(int player) const;
    bool HasOpenColumn() const;
    void RotateClockwise();
    void ApplyGravity();
    int OtherPlayer(int player) const;
    int rowCount;
    int columnCount;
    int currentPlayer;
    int winner;
    int movesPlayed;
    bool gameOver;
    bool shiftPending;
    bool gravityPending;
    std::vector<std::vector<int>> grid;

};