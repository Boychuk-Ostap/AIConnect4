#pragma once
#include "board.h"

class Game {
public:
	Game();
	void Update();
	void Draw();
private:
	enum Phase
	{
		PLAYING,
		ROTATING,
		FALLING
	};
	int ColumnUnderMouse() const;
	void DrawStatus() const;
	void DrawBoard() const;
	void DrawRotatingBoard() const;
	void DrawFallingBoard() const;

	Board board;
	int hoveredColumn;
	Phase phase;
	float shiftTime;
};