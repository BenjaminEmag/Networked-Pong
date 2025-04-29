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
	if (IsKeyDown(upKey)) position.y -= speed * deltaTime;
	if (IsKeyDown(downKey)) position.y += speed * deltaTime;

	if (position.y < 0) position.y = 0;
	if (position.y + height > screenHeight) position.y = screenHeight - height;

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

	SpawnBall();
}

void Game::SpawnBall()
{
	Vector2 vel = { (GetRandomValue(0, 1) == 0 ? -1 : 1) * 350.0f, (float)GetRandomValue(-100, 100) };
	objects.push_back(new Ball({ screenWidth / 2, screenHeight / 2 }, vel, { screenWidth, screenHeight }, paddles[0], paddles[1], scores, MAX_VELOCITY));
}

void Game::Update()
{
	float deltaTime = GetFrameTime();

	std::vector<size_t> ballIndices;
	std::vector<size_t> powerupIndices;

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
	HandlePowerupCollisions(powerupIndices, ballIndices);
	RemoveInactiveBalls(ballIndices);
}

void Game::RemoveInactiveBalls(const std::vector<size_t>& ballIndices)
{
	int activeBallCount = 0;

	for (size_t i = 0; i < ballIndices.size(); ++i)
	{
		size_t idx = ballIndices[i];
		if (idx >= objects.size()) continue;

		Ball* ball = dynamic_cast<Ball*>(objects[idx]);
		if (!ball) continue;

		if (!ball->active)
		{
			delete ball;
			objects.erase(objects.begin() + idx);
			continue;
		}

		activeBallCount++;
	}

	if (activeBallCount == 0)
		SpawnBall();
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

void Game::HandlePowerupCollisions(const std::vector<size_t>& powerupIndices, const std::vector<size_t>& ballIndices)
{
	for (size_t pIdx : powerupIndices)
	{
		if (pIdx >= objects.size()) continue;

		Powerup* powerup = dynamic_cast<Powerup*>(objects[pIdx]);
		if (!powerup || !powerup->active) continue;

		for (size_t bIdx : ballIndices)
		{
			if (bIdx >= objects.size()) continue;

			Ball* ball = dynamic_cast<Ball*>(objects[bIdx]);

			if (CheckCollisionCircles(ball->position, ball->radius, powerup->position, powerup->radius))
			{
				ApplyPowerup(powerup, ball);
				powerup->active = false;
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