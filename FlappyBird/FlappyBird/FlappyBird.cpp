#include <iostream>
#include <SFML/Graphics.hpp>
using namespace std;
#include <vector>
#include <string>
#define WINDOW_HEIGHT 600
#define WINDOW_WIDTH 800
#define GRAVITY 0.25

enum PipeType {
	Top,
	Bottom
};

class Pipe {
	PipeType type;
	int height;
	const int width;
	int x;
	int y;
	int velocity;
	bool passed;
	bool soft;
public:
	Pipe(int new_height, int new_width, PipeType the_type): height(new_height), width(new_width), type(the_type){}
	void setPosition(int new_x, int new_y);
	void makePassed();
	void makeSoft();
	bool checkIfSoft();
	void changeVelocity();
	void move();
};

class Bird {
	int size;
	int height;
	int width;
	int x;
	int y;
	int velocity;
public:
	Bird();
	void flapWings(int times);
	void updatePosition();
	void changeVelocity();

};

class Score
{
	int currentScore = 0;
	int getPreviousBestScore();
public:
	void updateScore();
	int getScore();
	void saveBestScore();
	int getBestScore();
	Score();

};

class GameEngine {
	static GameEngine* instance;
	sf::Clock clock;
	vector<Pipe> pipes;
	Bird bird;
	Score score;
	void increaseSpeed();
	void generatePipes();
	GameEngine();
public:
	vector<Pipe> getPipes();
	bool checkCollisions();
	void go();
	void startGame();
	bool gameOver;
	bool gameStarted;
	void resetGame();
	GameEngine getInstance(); 
};
GameEngine* GameEngine::instance = nullptr;

enum Command
{
	StartGame,
	FlapWings,
	Reset,
};

class UserAction {
public:
	Command type;  
	UserAction(Command type) : type(type) {}
	void execute(GameEngine& game);
};

class Render {
private:
	static Render* instance; 

	sf::RenderWindow window;
	Render() : window(sf::VideoMode(WINDOW_HEIGHT, WINDOW_WIDTH), "Game") {}

	void drawPipe(const Pipe& pipe);
	void drawBird(const Bird& bird);
	void manageWindow();
	void displayGameOver();
	void render(GameEngine& game);
	void processEvents();

public:

	static Render* getInstance() {
		if (!instance)
			instance = new Render();
		return instance;
	}

	void launch();
};

Render* Render::instance = nullptr;



int main()
{
   cout << "Hello World!\n";
}

