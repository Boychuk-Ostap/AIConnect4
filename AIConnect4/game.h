#pragma once
#include "board.h"
#include "ai.h"

class Game {
public:
	Game();
	void Update();
	void Draw();
private:
	enum Phase
	{
		MENU,
		LEVELSELECTION,
		PLAYING,
		ROTATING,
		FALLING
	};
	int ColumnUnderMouse() const;
	void UpdateMenu();
	void UpdateLevelSelection();
	void StartMatch();
	void PlayAiTurn();
	void DrawMenu();
	void DrawLevelSelection();
	void DrawStatus() const;
	void DrawBoard() const;
	void DrawRotatingBoard() const;
	void DrawFallingBoard() const;

	Board board;
	int hoveredColumn;
	Phase phase;
	float shiftTime;
	bool aiThinking;
	bool playAgainstAi;
	AiAlgorithm selectedAlgorithm;
	Difficulty selectedDifficulty;
};
