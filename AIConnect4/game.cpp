#include "game.h"
#include "minimax.h"
#include "mcts.h"
#include "raylib.h"

namespace
{
	const float CELL_SIZE = 90.0f;
	const float BOARD_CENTRE_Y = 520.0f;
	const float ROTATE_SECONDS = 0.8f;
	const float FALL_SECONDS = 0.4f;
	const float MENU_TWO_PLAYER_Y = 370.0f;
	const float MENU_MINIMAX_Y = 480.0f;
	const float MENU_MONTE_CARLO_Y = 590.0f;
	const float LEVEL_EASY_Y = 400.0f;
	const float LEVEL_MEDIUM_Y = 510.0f;
	const float LEVEL_HARD_Y = 620.0f;
	const float LEVEL_BACK_Y = 770.0f;

	Vector2 BoardCentre()
	{
		Vector2 centre;
		centre.x = GetScreenWidth() * 0.5f;
		centre.y = BOARD_CENTRE_Y;
		return centre;
	}
	Vector2 BoardOrigin(int columnCount, int rowCount)
	{
		Vector2 centre = BoardCentre();
		Vector2 origin;
		origin.x = centre.x - columnCount * CELL_SIZE * 0.5f;
		origin.y = centre.y - rowCount * CELL_SIZE * 0.5f;
		return origin;
	}
	Vector2 RotateAround(Vector2 point, Vector2 centre, float degrees)
	{
		float radians = degrees * DEG2RAD;
		float cosine = cosf(radians);
		float sine = sinf(radians);
		float offsetX = point.x - centre.x;
		float offsetY = point.y - centre.y;
		Vector2 rotated;
		rotated.x = centre.x + offsetX * cosine - offsetY * sine;
		rotated.y = centre.y + offsetX * sine + offsetY * cosine;
		return rotated;
	}
	Color ColourForPlayer(int player)
	{
		if (player == Board::PLAYER_ONE)
			return RED;
		if (player == Board::PLAYER_TWO)
			return GOLD;
		return Color{ 235, 235, 235, 255 };
	}

	Rectangle CentredButton(float top)
	{
		const float width = 640.0f;
		const float height = 84.0f;
		Rectangle button;
		button.width = width;
		button.height = height;
		button.x = (GetScreenWidth() - width) * 0.5f;
		button.y = top;
		return button;
	}

	bool ButtonPressed(Rectangle button)
	{
		Vector2 mouse = GetMousePosition();
		return IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mouse, button);
	}

	void DrawCentredText(const char* text, int top, int fontSize, Color colour)
	{
		int width = MeasureText(text, fontSize);
		int x = (GetScreenWidth() - width) / 2;
		DrawText(text, x, top, fontSize, colour);
	}

	void DrawButton(Rectangle button, const char* label)
	{
		Vector2 mouse = GetMousePosition();
		bool hover = CheckCollisionPointRec(mouse, button);
		Color fill = hover ? Color{ 36, 110, 200, 255 } : Color{ 25, 80, 170, 255 };
		DrawRectangleRec(button, fill);
		DrawRectangleLinesEx(button, 3.0f, Color{ 210, 220, 235, 255 });

		int fontSize = 32;
		int textWidth = MeasureText(label, fontSize);
		int textX = (int)(button.x + (button.width - textWidth) * 0.5f);
		int textY = (int)(button.y + (button.height - fontSize) * 0.5f);
		DrawText(label, textX, textY, fontSize, RAYWHITE);
	}

	bool PressedDigit(int digit)
	{
		if (digit == 1)
			return IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_KP_1);
		if (digit == 2)
			return IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_KP_2);
		if (digit == 3)
			return IsKeyPressed(KEY_THREE) || IsKeyPressed(KEY_KP_3);
		return false;
	}

	void DrawLevelButton(float top, AiAlgorithm algorithm, Difficulty difficulty)
	{
		const char* label;
		if (algorithm == AiAlgorithm::Minimax)
			label = TextFormat("%s    -    depth %d", DifficultyName(difficulty), SearchDepthFor(difficulty));
		else
			label = TextFormat("%s    -    %d games", DifficultyName(difficulty), SimulationCountFor(difficulty));
		DrawButton(CentredButton(top), label);
	}
}

Game::Game()
{
	hoveredColumn = -1;
	phase = MENU;
	shiftTime = 0.0f;
	aiThinking = false;
	playAgainstAi = true;
	selectedAlgorithm = AiAlgorithm::Minimax;
	selectedDifficulty = Difficulty::Medium;
}

void Game::Update()
{
	if (phase == MENU)
	{
		UpdateMenu();
		return;
	}
	if (phase == LEVELSELECTION)
	{
		UpdateLevelSelection();
		return;
	}

	if (IsKeyPressed(KEY_M))
	{
		aiThinking = false;
		phase = MENU;
		return;
	}

	if (IsKeyPressed(KEY_R))
	{
		board.Reset();
		phase = PLAYING;
		shiftTime = 0.0f;
		aiThinking = false;
		return;
	}

	if (phase == ROTATING)
	{
		shiftTime += GetFrameTime();
		if (shiftTime >= ROTATE_SECONDS)
		{
			board.CommitRotation();
			phase = FALLING;
			shiftTime = 0.0f;
		}
		return;
	}

	if (phase == FALLING)
	{
		shiftTime += GetFrameTime();
		if (shiftTime >= FALL_SECONDS)
		{
			board.CommitGravity();
			phase = PLAYING;
			shiftTime = 0.0f;
		}
		return;
	}

	if (playAgainstAi && !board.IsGameOver() && board.GetCurrentPlayer() == Board::PLAYER_TWO)
	{
		hoveredColumn = -1;
		// Show "AI is thinking" for one frame, then search on the next.
		// The window stays on that message while the search runs.
		if (!aiThinking)
		{
			aiThinking = true;
			return;
		}
		aiThinking = false;
		PlayAiTurn();
		return;
	}

	aiThinking = false;
	hoveredColumn = ColumnUnderMouse();
	if (board.IsGameOver())
		return;
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
	{
		if (board.DropPiece(hoveredColumn) && board.IsShiftPending())
		{
			phase = ROTATING;
			shiftTime = 0.0f;
		}
	}
}

void Game::UpdateMenu()
{
	bool chooseTwoPlayers = PressedDigit(1) || ButtonPressed(CentredButton(MENU_TWO_PLAYER_Y));
	bool chooseMinimax = PressedDigit(2) || ButtonPressed(CentredButton(MENU_MINIMAX_Y));
	bool chooseMonteCarlo = PressedDigit(3) || ButtonPressed(CentredButton(MENU_MONTE_CARLO_Y));

	if (chooseTwoPlayers)
	{
		playAgainstAi = false;
		StartMatch();
		return;
	}

	if (chooseMinimax)
		selectedAlgorithm = AiAlgorithm::Minimax;
	else if (chooseMonteCarlo)
		selectedAlgorithm = AiAlgorithm::MonteCarlo;
	else
		return;

	playAgainstAi = true;
	phase = LEVELSELECTION;
}

void Game::UpdateLevelSelection()
{
	if (IsKeyPressed(KEY_B) || IsKeyPressed(KEY_BACKSPACE) || ButtonPressed(CentredButton(LEVEL_BACK_Y)))
	{
		phase = MENU;
		return;
	}

	if (PressedDigit(1) || ButtonPressed(CentredButton(LEVEL_EASY_Y)))
		selectedDifficulty = Difficulty::Easy;
	else if (PressedDigit(2) || ButtonPressed(CentredButton(LEVEL_MEDIUM_Y)))
		selectedDifficulty = Difficulty::Medium;
	else if (PressedDigit(3) || ButtonPressed(CentredButton(LEVEL_HARD_Y)))
		selectedDifficulty = Difficulty::Hard;
	else
		return;

	StartMatch();
}

void Game::StartMatch()
{
	board.Reset();
	hoveredColumn = -1;
	shiftTime = 0.0f;
	aiThinking = false;
	phase = PLAYING;
}

void Game::PlayAiTurn()
{
	int column = -1;
	if (selectedAlgorithm == AiAlgorithm::Minimax)
	{
		Minimax minimax(selectedDifficulty);
		column = minimax.ChooseColumn(board);
	}
	else
	{
		MonteCarloTreeSearch monteCarlo(selectedDifficulty);
		column = monteCarlo.ChooseColumn(board);
	}

	if (!board.IsColumnOpen(column))
		return;

	if (board.DropPiece(column) && board.IsShiftPending())
	{
		phase = ROTATING;
		shiftTime = 0.0f;
	}
}


void Game::Draw()
{
	ClearBackground(RAYWHITE);
	if (phase == MENU)
	{
		DrawMenu();
		return;
	}
	if (phase == LEVELSELECTION)
	{
		DrawLevelSelection();
		return;
	}

	DrawStatus();
	if (phase == ROTATING)
		DrawRotatingBoard();
	else if (phase == FALLING)
		DrawFallingBoard();
	else
		DrawBoard();
}

int Game::ColumnUnderMouse() const
{
	float boardWidth = board.GetColumnCount() * CELL_SIZE;
	float originX = (GetScreenWidth() - boardWidth) * 0.5f;
	int column = (int)((GetMouseX() - originX) / CELL_SIZE);
	if (column < 0 || column >= board.GetColumnCount())
		return -1;
	return column;
}

void Game::DrawMenu()
{
	DrawCentredText("Gravity Shift Connect 4", 120, 64, Color{ 20, 55, 120, 255 });
	DrawCentredText("Choose who to play", 220, 40, DARKGRAY);
	DrawCentredText("Red starts. Yellow plays second.", 280, 32, GRAY);
	DrawButton(CentredButton(MENU_TWO_PLAYER_Y), "Two players");
	DrawButton(CentredButton(MENU_MINIMAX_Y), "Minimax");
	DrawButton(CentredButton(MENU_MONTE_CARLO_Y), "Monte Carlo Tree Search");
	DrawCentredText("Click a button, or press 1, 2 or 3", 710, 28, GRAY);
}

void Game::DrawLevelSelection()
{
	DrawCentredText("Choose a difficulty", 120, 56, Color{ 20, 55, 120, 255 });
	DrawCentredText(AlgorithmName(selectedAlgorithm), 200, 36, DARKGRAY);

	if (selectedAlgorithm == AiAlgorithm::Minimax)
	{
		DrawCentredText(
			TextFormat("Easy looks %d moves ahead. Medium looks %d. Hard looks %d.",
				SearchDepthFor(Difficulty::Easy),
				SearchDepthFor(Difficulty::Medium),
				SearchDepthFor(Difficulty::Hard)),
			270, 28, GRAY);
		DrawCentredText("From a new board, Medium sees the gravity shift. Hard looks one move past it.", 310, 28, GRAY);
		DrawLevelButton(LEVEL_EASY_Y, selectedAlgorithm, Difficulty::Easy);
		DrawLevelButton(LEVEL_MEDIUM_Y, selectedAlgorithm, Difficulty::Medium);
		DrawLevelButton(LEVEL_HARD_Y, selectedAlgorithm, Difficulty::Hard);
	}
	else
	{
		DrawCentredText(
			TextFormat("Easy plays %d games. Medium plays %d. Hard plays %d.",
				SimulationCountFor(Difficulty::Easy),
				SimulationCountFor(Difficulty::Medium),
				SimulationCountFor(Difficulty::Hard)),
			270, 28, GRAY);
		DrawCentredText("The search rules stay the same. More games make the choice steadier.", 310, 28, GRAY);
		DrawLevelButton(LEVEL_EASY_Y, selectedAlgorithm, Difficulty::Easy);
		DrawLevelButton(LEVEL_MEDIUM_Y, selectedAlgorithm, Difficulty::Medium);
		DrawLevelButton(LEVEL_HARD_Y, selectedAlgorithm, Difficulty::Hard);
	}

	DrawButton(CentredButton(LEVEL_BACK_Y), "Back");
	DrawCentredText("1, 2 or 3 to choose. B to go back.", 880, 28, GRAY);
}

void Game::DrawStatus() const
{
	const char* message = playAgainstAi ? "Your turn" : "Red's turn";
	Color messageColour = RED;

	if (phase != PLAYING)
	{
		message = "Gravity shift";
		messageColour = DARKBLUE;
	}
	else if (board.IsGameOver() && board.GetWinner() == Board::EMPTY)
	{
		message = "Draw";
		messageColour = DARKGRAY;
	}
	else if (board.GetWinner() == Board::PLAYER_ONE)
	{
		message = playAgainstAi ? "You win" : "Red wins";
		messageColour = RED;
	}
	else if (board.GetWinner() == Board::PLAYER_TWO)
	{
		message = playAgainstAi ? "AI wins" : "Yellow wins";
		messageColour = GOLD;
	}
	else if (aiThinking)
	{
		message = "AI is thinking";
		messageColour = GOLD;
	}
	else if (board.GetCurrentPlayer() == Board::PLAYER_TWO)
	{
		message = playAgainstAi ? "AI turn" : "Yellow's turn";
		messageColour = GOLD;
	}

	DrawText(message, 40, 30, 40, messageColour);
	DrawText(TextFormat("Gravity shift in %d", board.GetMovesUntilShift()), 40, 80, 28, DARKGRAY);
	DrawText("R restarts     M menu", 40, 112, 24, GRAY);

	if (!playAgainstAi)
	{
		DrawText("Two players", 40, 146, 24, GRAY);
	}
	else if (selectedAlgorithm == AiAlgorithm::Minimax)
	{
		DrawText(
			TextFormat("Minimax  |  %s  |  depth %d",
				DifficultyName(selectedDifficulty),
				SearchDepthFor(selectedDifficulty)),
			40, 146, 24, GRAY);
	}
	else
	{
		DrawText(
			TextFormat("Monte Carlo  |  %s  |  %d games",
				DifficultyName(selectedDifficulty),
				SimulationCountFor(selectedDifficulty)),
			40, 146, 24, GRAY);
	}
}

void Game::DrawBoard() const
{
    int rowCount = board.GetRowCount();
    int columnCount = board.GetColumnCount();
    float boardWidth = columnCount * CELL_SIZE;
    float boardHeight = rowCount * CELL_SIZE;
	Vector2 origin = BoardOrigin(columnCount, rowCount);
	float originX = origin.x;
	float originY = origin.y;
    DrawRectangle(originX, originY, boardWidth, boardHeight, Color{ 25, 80, 170, 255 });
    if (!board.IsGameOver() && hoveredColumn >= 0 && board.IsColumnOpen(hoveredColumn))
    {
        Color ghost = (board.GetCurrentPlayer() == Board::PLAYER_ONE) ? RED : GOLD;
        ghost.a = 120;
        float centreX = originX + hoveredColumn * CELL_SIZE + CELL_SIZE * 0.5f;
        DrawCircle(centreX, originY - 50.0f, CELL_SIZE * 0.35f, ghost);
    }
    for (int row = 0; row < rowCount; row++)
    {
        for (int column = 0; column < columnCount; column++)
        {
            float centreX = originX + column * CELL_SIZE + CELL_SIZE * 0.5f;
            float centreY = originY + row * CELL_SIZE + CELL_SIZE * 0.5f;
            int cell = board.GetCell(row, column);
            Color pieceColour = Color{ 235, 235, 235, 255 };
            if (cell == Board::PLAYER_ONE)
                pieceColour = RED;
            else if (cell == Board::PLAYER_TWO)
                pieceColour = GOLD;
            DrawCircle(centreX, centreY, CELL_SIZE * 0.38f, pieceColour);
        }
    }
}

void Game::DrawRotatingBoard() const
{
	float progress = shiftTime / ROTATE_SECONDS;
	if (progress > 1.0f)
		progress = 1.0f;
	float angle = progress * 90.0f;
	int rowCount = board.GetRowCount();
	int columnCount = board.GetColumnCount();
	float boardWidth = columnCount * CELL_SIZE;
	float boardHeight = rowCount * CELL_SIZE;
	Vector2 centre = BoardCentre();
	Vector2 origin = BoardOrigin(columnCount, rowCount);
	Rectangle boardRectangle;
	boardRectangle.x = centre.x;
	boardRectangle.y = centre.y;
	boardRectangle.width = boardWidth;
	boardRectangle.height = boardHeight;
	Vector2 pivot = { boardWidth * 0.5f, boardHeight * 0.5f };
	DrawRectanglePro(boardRectangle, pivot, angle, Color{ 25, 80, 170, 255 });
	for (int row = 0; row < rowCount; row++)
	{
		for (int column = 0; column < columnCount; column++)
		{
			Vector2 cellCentre;
			cellCentre.x = origin.x + column * CELL_SIZE + CELL_SIZE * 0.5f;
			cellCentre.y = origin.y + row * CELL_SIZE + CELL_SIZE * 0.5f;
			Vector2 drawnAt = RotateAround(cellCentre, centre, angle);
			DrawCircleV(drawnAt, CELL_SIZE * 0.38f, ColourForPlayer(board.GetCell(row, column)));
		}
	}
}

void Game::DrawFallingBoard() const
{
	int rowCount = board.GetRowCount();
	int columnCount = board.GetColumnCount();
	Vector2 origin = BoardOrigin(columnCount, rowCount);
	DrawRectangle(origin.x, origin.y, columnCount * CELL_SIZE, rowCount * CELL_SIZE, Color{ 25, 80, 170, 255 });
	for (int row = 0; row < rowCount; row++)
	{
		for (int column = 0; column < columnCount; column++)
		{
			float centreX = origin.x + column * CELL_SIZE + CELL_SIZE * 0.5f;
			float centreY = origin.y + row * CELL_SIZE + CELL_SIZE * 0.5f;
			DrawCircle(centreX, centreY, CELL_SIZE * 0.38f, ColourForPlayer(Board::EMPTY));
		}
	}
	float progress = shiftTime / FALL_SECONDS;
	if (progress > 1.0f)
		progress = 1.0f;
	std::vector<FallingPiece> pieces = board.PreviewGravity();
	for (int i = 0; i < (int)pieces.size(); i++)
	{
		float row = pieces[i].fromRow + (pieces[i].toRow - pieces[i].fromRow) * progress;
		float centreX = origin.x + pieces[i].column * CELL_SIZE + CELL_SIZE * 0.5f;
		float centreY = origin.y + row * CELL_SIZE + CELL_SIZE * 0.5f;
		DrawCircle(centreX, centreY, CELL_SIZE * 0.38f, ColourForPlayer(pieces[i].player));
	}
}
