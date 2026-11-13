#include "game.h"
#include "raylib.h"

namespace
{
	const float CELL_SIZE = 90.0f;
	const float BOARD_CENTRE_Y = 520.0f;
	const float ROTATE_SECONDS = 0.8f;
	const float FALL_SECONDS = 0.4f;
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
}

Game::Game()
{
	hoveredColumn = -1;
	phase = PLAYING;
	shiftTime = 0.0f;
}

void Game::Update()
{
	if (IsKeyPressed(KEY_R))
	{
		board.Reset();
		phase = PLAYING;
		shiftTime = 0.0f;
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


void Game::Draw()
{
	ClearBackground(RAYWHITE);
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

void Game::DrawStatus() const
{
	const char* message = "Player 1";
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
		message = "Player 1 wins";
		messageColour = RED;
	}
	else if (board.GetWinner() == Board::PLAYER_TWO)
	{
		message = "Player 2 wins";
		messageColour = GOLD;
	}
	else if (board.GetCurrentPlayer() == Board::PLAYER_TWO)
	{
		message = "Player 2";
		messageColour = GOLD;
	}

	DrawText(message, 40, 30, 40, messageColour);
	DrawText(TextFormat("Gravity shift in %d", board.GetMovesUntilShift()), 40, 80, 28, DARKGRAY);
	DrawText("R restarts", 40, 112, 24, GRAY);
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

