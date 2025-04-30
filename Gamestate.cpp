#include "Gamestate.h"
#include <cstring>

GameState::GameState() {
	for (int i = 0; i < 2; i++) paddlePositions[i] = { 0, 0 };

	for (int i = 0; i < MAX_BALLS; i++)
	{
		ballPositions[i] = { 0, 0 };
		ballVelocities[i] = { 0, 0 };
		ballActive[i] = false;
	}
	for (int i = 0; i < MAX_POWERUPS; i++)
	{
		powerups[i].position = { 0, 0 };
		powerups[i].type = 0;
		powerups[i].active = false;
	}
	scores[0] = 0;
	scores[1] = 0;
}

void GameState::Serialize(std::ostream& os) const {
	for (int i = 0; i < 2; i++)
	{
		os.write((char*)&paddlePositions[i], sizeof(Vector2));
		os.write((char*)&paddleHeights[i], sizeof(float));
	}
	for (int i = 0; i < MAX_BALLS; i++) {
		os.write((char*)&ballPositions[i], sizeof(Vector2));
		os.write((char*)&ballVelocities[i], sizeof(Vector2));
		os.write((char*)&ballActive[i], sizeof(bool));
	}

	os.write((char*)&scores, sizeof(scores));

	for (int i = 0; i < MAX_POWERUPS; i++) {
		os.write((char*)&powerups[i].position, sizeof(Vector2));
		os.write((char*)&powerups[i].type, sizeof(int));
		os.write((char*)&powerups[i].active, sizeof(bool));
	}

	os.write((char*)&gameOver, sizeof(bool));
	os.write((char*)&winner, sizeof(int));
}

void GameState::Deserialize(std::istream& is) {
	for (int i = 0; i < 2; i++)
	{
		is.read((char*)&paddlePositions[i], sizeof(Vector2));
		is.read((char*)&paddleHeights[i], sizeof(float));
	}
	for (int i = 0; i < MAX_BALLS; i++) {
		is.read((char*)&ballPositions[i], sizeof(Vector2));
		is.read((char*)&ballVelocities[i], sizeof(Vector2));
		is.read((char*)&ballActive[i], sizeof(bool));
	}

	is.read((char*)&scores, sizeof(scores));

	for (int i = 0; i < MAX_POWERUPS; i++) {
		is.read((char*)&powerups[i].position, sizeof(Vector2));
		is.read((char*)&powerups[i].type, sizeof(int));
		is.read((char*)&powerups[i].active, sizeof(bool));
	}

	is.read((char*)&gameOver, sizeof(bool));
	is.read((char*)&winner, sizeof(int));
}
