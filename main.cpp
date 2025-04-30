#include "socklib.h"
#include "raylib.h"
#include <iostream>
#include <sstream>
#include "Game.h"
#include <algorithm>

void RunServer();
void RunClient();

int main(int argc, char* argv[]) {
	SockLibInit();

	if (argc > 1)
	{
		std::string mode = argv[1];
		if (mode == "server")
			RunServer();

		if (mode == "client")
			RunClient();


		SockLibShutdown();
		return 0;
	}

	std::cout << "[s]erver or [c]lient? ";
	std::string choice;
	std::cin >> choice;

	if (choice == "s" || choice == "S" || choice == "Server" || choice == "server")
		RunServer();
	
	if (choice == "c" || choice == "C" || choice == "Client" || choice == "client")
		RunClient();
	

	SockLibShutdown();
	return 0;
}

void RunServer() {
	Socket server(Socket::INET, Socket::STREAM);
	Address serverAddress("0.0.0.0", 12345);

	server.Bind(serverAddress);
	server.Listen();
	std::cout << "Server listening\n";

	Socket client = server.Accept();
	client.SetNonBlockingMode(true);

	std::cout << "Client connected\n";

	Game game(800, 600);

	InitWindow(800, 600, "Server - Player 1 (W/S)");
	SetTargetFPS(60);

	while (!WindowShouldClose()) 
	{
		InputCommand serverInput = InputCommand::None;
		if (IsKeyDown(KEY_W)) serverInput = InputCommand::Up;
		if (IsKeyDown(KEY_S)) serverInput = InputCommand::Down;
		if (IsKeyDown(KEY_R)) serverInput = InputCommand::Restart;

		InputCommand clientInput = InputCommand::None;
		char buffer[1];
		if (client.Recv(buffer, 1) > 0) 
			clientInput = static_cast<InputCommand>(buffer[0]);
		
		game.ApplyInput(0, serverInput);
		game.ApplyInput(1, clientInput);

		game.Update(GetFrameTime());

		std::ostringstream oss;
		GameState gameState = game.GetState();
		gameState.Serialize(oss);
		std::string data = oss.str();
		client.SendAll(data.data(), data.size());

		BeginDrawing();
		ClearBackground(BLACK);
		game.Draw();
		DrawFPS(20, 20);
		EndDrawing();
	}

	CloseWindow();
}

void RunClient() {
	Socket socket(Socket::INET, Socket::STREAM);
	Address serverAddress("127.0.0.1", 12345);

	if (socket.Connect(serverAddress) < 0) 
	{
		std::cout << "Failed to connect to server\n";
		return;
	}

	InitWindow(800, 600, "Client - Player 2 (Arrow keys)");
	SetTargetFPS(60);

	Game game(800, 600);

	while (!WindowShouldClose()) 
	{
		InputCommand input = InputCommand::None;
		if (IsKeyDown(KEY_UP)) input = InputCommand::Up;
		if (IsKeyDown(KEY_DOWN)) input = InputCommand::Down;
		if (IsKeyDown(KEY_R)) input = InputCommand::Restart;

		socket.Send((char*)&input, 1);

		// Receive GameState
		char buffer[4096];
		int received = socket.Recv(buffer, sizeof(buffer));

		if (received > 0) {
			std::istringstream iss(std::string(buffer, received), std::ios::binary);
			GameState state;
			state.Deserialize(iss);

			game.SetState(state);
		}

		BeginDrawing();
		ClearBackground(BLACK);

		game.Draw();
		DrawFPS(20, 20);
		EndDrawing();
	}

	CloseWindow();
}