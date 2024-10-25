#include <iostream>
#include <SFML/Graphics.hpp>
using namespace std;
#include <vector>
#include <string>
#include <fstream>
#define WINDOW_HEIGHT 600
#define WINDOW_WIDTH 800
#define GRAVITY 0.25
#define BIRD_HEIGHT 20 
#define BIRD_WIDTH 20
#define MIN_SPACE_BETWEEN_PIPES 40
#define PIPE_MAX_HEIGHT 550
#define PIPE_MIN_HEIGHT 10
#define PIPE_MAX_WIDTH 30
#define PIPE_MIN_WIDTH 10
#define  SOFT_PIPE_CHANCE  50

class Entity
{
protected:
	int height;
	int width;
	int x;
	int y;

public:

	Entity(const int the_x, const int the_y, const int the_width, const int the_height): x(the_x), y(the_y), width(the_width), height(the_height) {}

	int getY()const
	{
		return y;
	}

	int getX()const
	{
		return x;
	}

	int getWidth()const
	{
		return width;
	}

	int getHeight()const
	{
		return height;
	}
	virtual void updatePosition(const int value) = 0;
	
};
enum PipeType {
	Top,
	Bottom
};

class Pipe: public Entity {
	PipeType type;	
	bool passed = false;
	bool makeSoft()const
	{
		srand(time(0));
		int chance = rand() % (SOFT_PIPE_CHANCE + 1);
		if (chance % 17 == 0)
		{
			return true;
		}

		return false;
	};
	bool soft = makeSoft();

public:
	Pipe(int new_y, int new_width, int new_height, PipeType the_type): Entity(WINDOW_WIDTH, new_y, new_width, new_height), type(the_type){}
	
	void makePassed()
	{	
		passed = true;
	};
	
	bool checkIfSoft()const 
	{
		return soft;
	};

	PipeType getType()const 
	{
		return type;
	}
	void updatePosition(const int value)override
	{
		x += value;
	}
};

struct Bird: public Entity
{
	Bird(): Entity(WINDOW_WIDTH / 5, WINDOW_HEIGHT / 2, BIRD_WIDTH, BIRD_HEIGHT) {}
	void flapWings()
	{
		y--;
	};
	void updatePosition(const int value)override
	{
		y += value;
	}
};

class Score
{
	double currentScore = 0;
	double getBestScore()const
	{
		ifstream file("score.txt");
		if (!file.is_open())
		{
			cout << "Problems with score.txt!" << endl;
			return;
		}
		string score;
		getline(file, score);	
		file.close();
		return score.empty() ? 0: stoi(score);
	}; 
public:
	Score() {};

	Score& operator+=(const double value)
	{
		currentScore+= value;
		return *this;
	}
	
	double getScore() const
	{
		return currentScore;
	};
	void saveBestScore() const
	{
		double previous_best_score = getBestScore();
		if (previous_best_score > currentScore)
		{
			return;
		}
		ofstream file("score.txt", ofstream::out | ofstream::trunc);
		if (!file.is_open())
		{
			cout << "Problems with score.txt!" << endl;
			return;
		}
		
		file << currentScore;
		cout << "The score was loaded successfully!" << endl;
		file.close();

	};
};

class GameEngine {
	static GameEngine* instance;
	sf::Clock clock;
	vector<Pipe> pipes;
	Bird bird;
	Score score;
	double velocityX = -2;
	double velocityY = -1;
	sf::Time lastPipeSGenerated;

	void increaseSpeed()
	{
		static sf::Time lastUpdate = sf::seconds(0);
		sf::Time elapsedTime = clock.getElapsedTime();

		if (elapsedTime.asSeconds() - lastUpdate.asSeconds() >= 50)
		{
			velocityX += 0.25;
			velocityY += 0.25;
			lastUpdate = elapsedTime;
		}
	};


	void generatePipes()
	{
		srand(time(0));
		int topPipeHeight = rand() % (PIPE_MAX_HEIGHT - PIPE_MIN_HEIGHT + 1) + PIPE_MIN_HEIGHT;
		int topY = topPipeHeight-1;
		int bottomPipeHeight = rand() % (PIPE_MAX_HEIGHT - topPipeHeight - PIPE_MIN_HEIGHT + 1) + PIPE_MIN_HEIGHT;
		int bottomY = WINDOW_HEIGHT - bottomPipeHeight - 1;
		int width = rand() % (PIPE_MAX_WIDTH - PIPE_MIN_WIDTH + 1)+ PIPE_MAX_WIDTH;

		Pipe topPipe(topY, width, topPipeHeight, Top);
		Pipe bottomPipe(bottomY, width, bottomPipeHeight, Bottom);
		pipes.push_back(topPipe);
		pipes.push_back(bottomPipe);
	};


	GameEngine();
public:

	const vector<Pipe> getPipes() const 
	{
		return pipes;
	};
	bool collisionCanvas()const
	{
		return bird.getY() + bird.getHeight() / 2 >= WINDOW_HEIGHT || bird.getY() - bird.getHeight() / 2 <= 0;
	}
	bool collisionPipe(const Pipe& pipe) const
	{
		const int pipeTopY = (pipe.getType () == Bottom) ? pipe.getY() - pipe.getHeight() : pipe.getY();
		const int pipeBottomY = (pipe.getType() == Bottom) ? pipe.getY() : pipe.getY() + pipe.getHeight();

		if (bird.getX() - bird.getWidth() / 2 < pipe.getX() + pipe.getWidth() &&
			bird.getX() + bird.getWidth() / 2 > pipe.getX() &&
			bird.getY() - bird.getHeight() / 2 < pipeBottomY &&
			bird.getY() + bird.getHeight() / 2 > pipeTopY)
		{
			return true; 
		}
		return false;
		
	};

	void go()
	{
		sf::Time currentTime = clock.getElapsedTime();
		if (currentTime.asSeconds() - lastPipeSGenerated.asSeconds() >= 1.5)
		{
			generatePipes();
			lastPipeSGenerated = currentTime;
		}

		velocityY += GRAVITY;
		bird.updatePosition(velocityY);
		if (collisionCanvas())
		{
			gameOver = true;
			return;
		}
		

		for (Pipe& pipe : pipes)
		{
			pipe.updatePosition(velocityX);

			if (collisionPipe(pipe))
			{
				if (!pipe.checkIfSoft())
				{
					gameOver = true;
					return;
				}
			}
			else
			{
				pipe.makePassed();
				score+=0.5;
			}
		}
		
	};
	void startGame()
	{
		gameStarted = true;
		clock.restart();
		lastPipeSGenerated = clock.getElapsedTime();
		generatePipes();
		go();
		
	};
	bool gameOver = false;
	bool gameStarted = false;
	void resetGame()
	{
		score.saveBestScore();
		startGame();
	};
	void flap() 
	{
		bird.flapWings();
		if (collisionCanvas())
		{
			gameOver = true;
		};
		
	}
	static GameEngine* getInstance()
	{
		if (instance == nullptr)
		{
			instance = new GameEngine();
		}
		return instance;
	};
};
GameEngine* GameEngine::instance = nullptr;

enum Command
{
	StartGame,
	FlapWings,
	Reset,
};

class UserAction {
	Command type; 
public:
	UserAction(Command type) : type(type) {}
	void execute(GameEngine& game)
	{
		if (type == StartGame)
		{
			game.go();
		}
		else if (type == FlapWings)
		{
			
			game.flap();
		}
		else
		{
			game.resetGame();

		}
	
	};
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

