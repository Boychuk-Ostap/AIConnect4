#pragma once

// Shared by both AIs and by the difficulty menu.
// The algorithms themselves do not change between levels.
// Only the search budget changes, and both budgets are defined here
// so the menu text and the AI cannot drift apart.

enum class AiAlgorithm
{
	Minimax,
	MonteCarlo
};

enum class Difficulty
{
	Easy,
	Medium,
	Hard
};

// Minimax depth is in plies. One ply is one drop, and that drop includes
// the gravity shift when it is the 4th move of the cycle.
// From a fresh board the shift is on ply 4:
//   Easy   2  stops before the shift
//   Medium 4  evaluates the board after the shift
//   Hard   5  looks one move past that shift
inline int SearchDepthFor(Difficulty difficulty)
{
	if (difficulty == Difficulty::Easy)
		return 2;
	if (difficulty == Difficulty::Medium)
		return 4;
	return 5;
}

// Monte Carlo plays this many random games from the current position
// before it chooses. More games make the average more reliable.
inline int SimulationCountFor(Difficulty difficulty)
{
	if (difficulty == Difficulty::Easy)
		return 100;
	if (difficulty == Difficulty::Medium)
		return 500;
	return 2000;
}

inline const char* AlgorithmName(AiAlgorithm algorithm)
{
	if (algorithm == AiAlgorithm::Minimax)
		return "Minimax";
	return "Monte Carlo";
}

inline const char* DifficultyName(Difficulty difficulty)
{
	if (difficulty == Difficulty::Easy)
		return "Easy";
	if (difficulty == Difficulty::Medium)
		return "Medium";
	return "Hard";
}
