#include "game.h"
#include <algorithm>
#include <cmath>

Paddle::Paddle(Vector2 pos, int up, int down, float screenHeight) : upKey(up), downKey(down), screenHeight(screenHeight)
{
	position = pos;
	velocity = { 0, 0 };
	baseHeight = height;
	baseSpeed = speed;
}

void Paddle::Update(float deltaTime)
{
	/*
	if (IsKeyDown(upKey)) position.y -= speed * deltaTime;
	if (IsKeyDown(downKey)) position.y += speed * deltaTime;

	if (position.y < 0) position.y = 0;
	if (position.y + height > screenHeight) position.y = screenHeight - height;
	*/

	// Handle size boost expiration
	if (sizeBoostActive)
	{
		sizeBoostTimer -= deltaTime;
		if (sizeBoostTimer <= 0.0f)
		{
			height = baseHeight;
			sizeBoostActive = false;
		}
	}

	// Handle speed boost expiration
	if (speedBoostActive)
	{
		speedBoostTimer -= deltaTime;
		if (speedBoostTimer <= 0.0f)
		{
			speed = baseSpeed;
			speedBoostActive = false;
		}
	}
}

void Paddle::Draw()
{
	DrawRectangleV(position, { width, height }, WHITE);
}

Ball::Ball(Vector2 pos, Vector2 vel, Vector2 screenSize, Paddle* p1, Paddle* p2, int* scores, Vector2 maxVelocity) : screenSize(screenSize), maxVelocity(maxVelocity), scores(scores)
{
	position = pos;
	velocity = vel;
	paddles[0] = p1;
	paddles[1] = p2;
}

void Ball::Update(float deltaTime)
{
	if (!active) return;

	position.x += velocity.x * deltaTime;
	position.y += velocity.y * deltaTime;

	// Wall hit
	if (position.y - radius <= 0 || position.y + radius >= screenSize.y)
		velocity.y *= -1;

	// Paddle collision
	for (int i = 0; i < 2; i++)
	{
		Rectangle paddleRect = { paddles[i]->position.x, paddles[i]->position.y, paddles[i]->width, paddles[i]->height };
		if (CheckCollisionCircleRec(position, radius, paddleRect))
		{

			float paddleCenterY = paddles[i]->position.y + paddles[i]->height / 2.0f;
			float distanceFromCenter = (position.y - paddleCenterY);
			float normalizedDistance = distanceFromCenter / (paddles[i]->height / 2.0f);
			float bounceAngle = normalizedDistance * 45.0f * (PI / 180.0f);

			float speed = sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
			if (speed < 200.0f) speed = 200.0f;

			int direction = (i == 0) ? 1 : -1;
			velocity.x = speed * cos(bounceAngle) * direction;
			velocity.y = speed * sin(bounceAngle);

			velocity.x *= 1.05f;
			velocity.y *= 1.05f;

			velocity.x = std::clamp(velocity.x, -maxVelocity.x, maxVelocity.x);
			velocity.y = std::clamp(velocity.y, -maxVelocity.y, maxVelocity.y);
		}
	}

	// out bounds
	if (position.x < 0)
	{
		scores[1]++;
		active = false;
	}

	// out bounds
	if (position.x > screenSize.x)
	{
		scores[0]++;
		active = false;
	}
}

void Ball::Draw()
{
	if (active) DrawCircleV(position, radius, RED);
}

Game::Game(float screenWidth, float screenHeight) : screenWidth(screenWidth), screenHeight(screenHeight)
{
	Reset();
}

Game::~Game()
{
	for (auto obj : objects)
		delete obj;
}

void Game::Reset()
{
	for (auto obj : objects)
		delete obj;

	objects.clear();

	scores[0] = 0;
	scores[1] = 0;

	paddles[0] = new Paddle({ 50, screenHeight / 2 - 50 }, KEY_W, KEY_S, screenHeight);
	paddles[1] = new Paddle({ screenWidth - 70, screenHeight / 2 - 50 }, KEY_UP, KEY_DOWN, screenHeight);
	objects.push_back(paddles[0]);
	objects.push_back(paddles[1]);

	gameOver = false;

	SpawnBall();
}

// I don't love this but it is what it is
void Game::ResetPrevState()
{
	for (auto obj : objects)
		delete obj;

	objects.clear();

	scores[0] = 0;
	scores[1] = 0;

	paddles[0] = new Paddle({ 50, screenHeight / 2 - 50 }, KEY_W, KEY_S, screenHeight);
	paddles[1] = new Paddle({ screenWidth - 70, screenHeight / 2 - 50 }, KEY_UP, KEY_DOWN, screenHeight);
	objects.push_back(paddles[0]);
	objects.push_back(paddles[1]);
}

void Game::SpawnBall()
{
	Vector2 vel = { (GetRandomValue(0, 1) == 0 ? -1 : 1) * 350.0f, (float)GetRandomValue(-100, 100) };
	objects.push_back(new Ball({ screenWidth / 2, screenHeight / 2 }, vel, { screenWidth, screenHeight }, paddles[0], paddles[1], scores, MAX_VELOCITY));
}

void Game::ApplyInput(int player, InputCommand input) 
{
	if (input == InputCommand::Restart)
	{
		Reset();
		gameOver = false;
		winner = -1;
		return;
	}
	
	Paddle* paddle = paddles[player];
	if (input == InputCommand::Up)
		paddle->position.y -= paddle->speed * GetFrameTime();
	
	if (input == InputCommand::Down) 
		paddle->position.y += paddle->speed * GetFrameTime();

	if (paddle->position.y < 0) paddle->position.y = 0;
	if (paddle->position.y + paddle->height > screenHeight)
		paddle->position.y = screenHeight - paddle->height;
}

GameState Game::GetState() const {
	GameState state;

	// Copy paddle positions
	for (int i = 0; i < 2; i++) {
		state.paddlePositions[i] = paddles[i]->position;
		state.paddleHeights[i] = paddles[i]->height;
	}

	// Copy scores
	state.scores[0] = scores[0];
	state.scores[1] = scores[1];

	// Copy balls
	int ballIndex = 0;
	for (GameObject* obj : objects) 
	{
		Ball* ball = dynamic_cast<Ball*>(obj);
		if (ball && ballIndex < GameState::MAX_BALLS) 
		{
			state.ballPositions[ballIndex] = ball->position;
			state.ballVelocities[ballIndex] = ball->velocity;
			state.ballActive[ballIndex] = ball->active;
			ballIndex++;
		}
	}

	for (int i = ballIndex; i < GameState::MAX_BALLS; i++) {
		state.ballActive[i] = false;
	}

	// Copy powerups
	int powerupIndex = 0;
	for (GameObject* obj : objects) 
	{
		Powerup* powerup = dynamic_cast<Powerup*>(obj);
		if (powerup && powerupIndex < GameState::MAX_POWERUPS)
		{
			state.powerups[powerupIndex].position = powerup->position;
			state.powerups[powerupIndex].type = (int)powerup->type;
			state.powerups[powerupIndex].active = powerup->active;
			powerupIndex++;
		}
	}
	for (int i = powerupIndex; i < GameState::MAX_POWERUPS; i++) 
	{
		state.powerups[i].active = false;
	}

	// Update win state
	state.gameOver = gameOver;
	state.winner = winner;

	return state;
}

void Game::SetState(const GameState& state) {
	ResetPrevState();

	// Update paddles
	for (int i = 0; i < 2; i++) {
		paddles[i]->position = state.paddlePositions[i];
		paddles[i]->height = state.paddleHeights[i];
	}

	// Update scores
	scores[0] = state.scores[0];
	scores[1] = state.scores[1];

	// Update balls
	for (int i = 0; i < GameState::MAX_BALLS; i++) 
	{
		if (state.ballActive[i])
		{
			Ball* ball = new Ball(state.ballPositions[i], state.ballVelocities[i], { screenWidth, screenHeight }, paddles[0], paddles[1], scores, MAX_VELOCITY);
			objects.push_back(ball);
		}
	}

	// Update powerups
	for (int i = 0; i < GameState::MAX_POWERUPS; i++) 
	{
		if (state.powerups[i].active) 
		{
			Powerup* powerup = new Powerup(state.powerups[i].position, (PowerupType)state.powerups[i].type);
			objects.push_back(powerup);
		}
	}

	// Update win state
	gameOver = state.gameOver;
	winner = state.winner;
}

void Game::Update(float deltaTime)
{
	if (gameOver)
		return;


	std::vector<size_t> ballIndices;
	std::vector<size_t> powerupIndices;
	std::vector<size_t> toDelete;

	for (size_t i = 0; i < objects.size(); ++i)
	{
		objects[i]->Update(deltaTime);

		if (dynamic_cast<Ball*>(objects[i]))
		{
			ballIndices.push_back(i);
			continue;
		}
		if (dynamic_cast<Powerup*>(objects[i]))
		{
			powerupIndices.push_back(i);
		}
	}

	HandlePowerupSpawning(deltaTime);
	HandlePowerupCollisions(powerupIndices, ballIndices, toDelete);
	RemoveInactiveBalls(ballIndices, toDelete);

	std::sort(toDelete.rbegin(), toDelete.rend());
	for (size_t idx : toDelete)
	{
		if (idx >= objects.size()) continue;
		delete objects[idx];
		objects.erase(objects.begin() + idx);
	}

	int activeBallCount = 0;
	for (GameObject* obj : objects)
	{
		Ball* ball = dynamic_cast<Ball*>(obj);
		if (ball && ball->active) activeBallCount++;
	}
	if (activeBallCount == 0)
		SpawnBall();

	if (scores[0] >= WIN_SCORE || scores[1] >= WIN_SCORE)
	{
		gameOver = true;
		winner = (scores[0] >= WIN_SCORE) ? 0 : 1;
	}
}

void Game::RemoveInactiveBalls(const std::vector<size_t>& ballIndices, std::vector<size_t>& toDelete)
{
	for (size_t idx : ballIndices)
	{
		if (idx >= objects.size()) continue;
		Ball* ball = dynamic_cast<Ball*>(objects[idx]);
		if (!ball) continue;

		if (!ball->active)
		{
			toDelete.push_back(idx);
		}
	}
}


void Game::HandlePowerupSpawning(float deltaTime)
{
	powerupSpawnTimer += deltaTime;
	if (powerupSpawnTimer >= nextPowerupSpawnTime)
	{
		Vector2 pos = { static_cast<float>(GetRandomValue(screenWidth / 3, 2 * screenWidth / 3)), static_cast<float>(GetRandomValue(screenHeight / 4, 3 * screenHeight / 4)) };
		PowerupType type = static_cast<PowerupType>(GetRandomValue(0, 2));
		objects.push_back(new Powerup(pos, type));

		powerupSpawnTimer = 0.0f;
		nextPowerupSpawnTime = static_cast<float>(GetRandomValue(5, 10));
	}
}

void Game::HandlePowerupCollisions(const std::vector<size_t>& powerupIndices, const std::vector<size_t>& ballIndices, std::vector<size_t>& toDelete)
{
	for (size_t pIdx = 0; pIdx < powerupIndices.size(); pIdx++)
	{
		size_t idx = powerupIndices[pIdx];
		if (idx >= objects.size()) continue;

		Powerup* powerup = dynamic_cast<Powerup*>(objects[idx]);
		if (!powerup || !powerup->active) continue;

		for (size_t bIdx : ballIndices)
		{
			if (bIdx >= objects.size()) continue;

			Ball* ball = dynamic_cast<Ball*>(objects[bIdx]);
			if (!ball || !ball->active) continue;

			if (CheckCollisionCircles(ball->position, ball->radius, powerup->position, powerup->radius))
			{
				// Apply the powerup effect
				ApplyPowerup(powerup, ball);
				toDelete.push_back(std::distance(objects.begin(), std::find(objects.begin(), objects.end(), powerup)));
				break;
			}
		}
	}
}

void Game::ApplyPowerup(Powerup* powerup, Ball* ball)
{
	Paddle* targetPaddle = (ball->velocity.x < 0) ? paddles[1] : paddles[0];

	switch (powerup->type)
	{
	case POWERUP_MULTIBALL:
		SpawnBall();
		break;

	case POWERUP_BIGGER_PADDLE:
		if (!targetPaddle->sizeBoostActive)
			targetPaddle->height *= 1.5f;

		targetPaddle->sizeBoostTimer = 5.0f;
		targetPaddle->sizeBoostActive = true;
		break;

	case POWERUP_SPEED_BOOST:
		if (!targetPaddle->speedBoostActive)
			targetPaddle->speed *= 1.5f;

		targetPaddle->speedBoostTimer = 5.0f;
		targetPaddle->speedBoostActive = true;
		break;
	}
}

void Game::Draw()
{
	if (gameOver)
	{
		const char* msg = TextFormat("Player %d Wins! Press R to Restart", winner + 1);
		DrawText(msg, screenWidth / 2 - MeasureText(msg, 20) / 2, screenHeight / 2 - 10, 20, YELLOW);
		return;
	}

	for (auto obj : objects)
		obj->Draw();

	DrawText(TextFormat("%d", scores[0]), screenWidth / 4, 20, 40, WHITE);
	DrawText(TextFormat("%d", scores[1]), 3 * screenWidth / 4, 20, 40, WHITE);
}

Powerup::Powerup(Vector2 pos, PowerupType type) : type(type)
{
	position = pos;
	velocity = { 0, 0 };
	active = true;

	// Error color lol
	color = MAGENTA;
	if (type == POWERUP_MULTIBALL) color = YELLOW;
	if (type == POWERUP_BIGGER_PADDLE) color = GREEN;
	if (type == POWERUP_SPEED_BOOST) color = BLUE;
}

void Powerup::Update(float deltaTime)
{
}

void Powerup::Draw()
{
	if (active) DrawCircleV(position, radius, color);
}