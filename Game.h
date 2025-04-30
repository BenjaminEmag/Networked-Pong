#pragma once
#include "raylib.h"
#include <vector>
#include "Gamestate.h"

class GameObject {
public:
	Vector2 position;
	Vector2 velocity;
	bool active = true;

	virtual void Update(float deltaTime) = 0;
	virtual void Draw() = 0;
	virtual ~GameObject() = default;

};

class Paddle : public GameObject {
public:
	float width = 20.0f;
	float height = 100.0f;
	float speed = 320.0f;
	int upKey;
	int downKey;
	float screenHeight;
	float sizeBoostTimer = 0.0f;
	float speedBoostTimer = 0.0f;
	bool sizeBoostActive = false;
	bool speedBoostActive = false;

	float baseHeight;
	float baseSpeed;

	Paddle(Vector2 pos, int up, int down, float screenHeight);

	void Update(float deltaTime) override;
	void Draw() override;
};

class Ball : public GameObject {
public:
	float radius = 10.0f;
	Vector2 screenSize;
	Vector2 maxVelocity;
	class Paddle* paddles[2];
	int* scores;

	Ball(Vector2 pos, Vector2 vel, Vector2 screenSize, Paddle* p1, Paddle* p2, int* scores, Vector2 maxVelocity);

	void Update(float deltaTime) override;
	void Draw() override;
};

enum PowerupType
{
	POWERUP_MULTIBALL,
	POWERUP_BIGGER_PADDLE,
	POWERUP_SPEED_BOOST
};

class Powerup : public GameObject {
public:
	float radius = 12.0f;
	PowerupType type;
	Color color;

	Powerup(Vector2 pos, PowerupType type);

	void Update(float deltaTime) override;
	void Draw() override;
};


class Game {
public:
	Game(float screenWidth, float screenHeight);
	~Game();

	void Reset();
	void ResetPrevState();
	void Update(float deltaTime);
	void Draw();
	void ApplyInput(int player, InputCommand input);

	GameState GetState() const;
	void SetState(const GameState& state);

private:
	float screenWidth, screenHeight;
	Paddle* paddles[2];
	std::vector<GameObject*> objects;
	int scores[2];
	Vector2 MAX_VELOCITY = { 700.0f, 400.0f };

	bool gameOver = false;
	int winner = -1;
	const int WIN_SCORE = 15;
	float powerupSpawnTimer = 0.0f;
	float nextPowerupSpawnTime = 5.0f;

	void SpawnBall();
	void RemoveInactiveBalls(const std::vector<size_t>& ballIndices, std::vector<size_t>& toDelete);
	void HandlePowerupSpawning(float deltaTime);
	void HandlePowerupCollisions(const std::vector<size_t>& powerupIndices, const std::vector<size_t>& ballIndices, std::vector<size_t>& toDelete);
	void ApplyPowerup(class Powerup* powerup, class Ball* ball);
};
