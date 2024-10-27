#include <iostream>
#include <SFML/Graphics.hpp>
using namespace std;
#include <vector>
#include <string>
#include <fstream>
#include <memory>
#define WINDOW_HEIGHT 800
#define WINDOW_WIDTH 1400
#define GRAVITY 0.005
#define BIRD_HEIGHT 40 
#define BIRD_WIDTH 40
#define MIN_SPACE_BETWEEN_PIPES 150
#define PIPE_MAX_HEIGHT 550
#define PIPE_MIN_HEIGHT 100
#define PIPE_MAX_WIDTH 150
#define PIPE_MIN_WIDTH 70
#define SOFT_PIPE_CHANCE  50
#define VELOCITY_Y 0
#define VELOCITY_X -1
#define JUMP -0.75
class Entity
{
protected:
	int height;
	int width;
	int x;
	int y;

public:

	Entity(const int the_x, const int the_y, const int the_width, const int the_height) : x(the_x), y(the_y), width(the_width), height(the_height) {}

	const int getY()const
	{
		return y;
	}

	const int getX()const
	{
		return x;
	}

	const int getWidth()const
	{
		return width;
	}

	const int getHeight()const
	{
		return height;
	}

};

enum PipeType {
	Top,
	Bottom
};

class Pipe : public Entity {
	PipeType type;
	bool passed = false;
	bool makeSoft()const
	{
		const int chance = rand() % (SOFT_PIPE_CHANCE + 1);
		if (chance % 17 == 0)
		{
			return true;
		}

		return false;
	};

	bool soft = makeSoft();

public:
	Pipe(int new_y, int new_width, int new_height, PipeType the_type) : Entity(WINDOW_WIDTH, new_y, new_width, new_height), type(the_type) {}

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
	void updatePosition(const int value)
	{
		x += value;
	}

	bool checkIfPassed()const
	{
		return passed;
	}
};

class Bird : public Entity
{
	double velocityY = VELOCITY_Y;

public:
	Bird() : Entity(WINDOW_WIDTH / 5, WINDOW_HEIGHT / 2, BIRD_WIDTH, BIRD_HEIGHT) {}

	void flapWings()
	{
		velocityY = JUMP;
	}

	void updatePosition()
	{
		y += velocityY;
	}

	void updateVelocity(const double value)
	{
		velocityY += value;
	}
	void setVelocity(const double value)
	{
		velocityY = value;
	}
};

class Score
{
	double currentScore = 0;
	const double getPreviousBestScore()const
	{
		ifstream file("score.txt");
		if (!file.is_open())
		{
			cout << "Problems with score.txt!" << endl;
			return 0;
		}
		string score;
		getline(file, score);
		file.close();
		return score.empty() ? 0 : stoi(score);
	};
public:
	Score() {};

	const Score& operator+=(const double value)
	{
		currentScore += value;
		return *this;
	}

	const Score& operator=(const double value)
	{
		currentScore = value;
		return *this;
	}

	const double getScore() const
	{
		return currentScore;
	}

	void saveBestScore() const
	{
		const double previous_best_score = getPreviousBestScore();
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
		file.close();

	}

	const double getBestScore()const
	{
		return getPreviousBestScore() > currentScore ? getPreviousBestScore() : currentScore;

	}
};

class GameEngine
{
	static GameEngine* instance;
	sf::Clock clock;
	vector<Pipe> pipes;
	Bird bird;
	Score score;
	double velocityX = VELOCITY_X;
	sf::Time lastPipeSGenerated;

	void increaseSpeed()
	{
		static sf::Time lastUpdate = sf::seconds(0);
		sf::Time elapsedTime = clock.getElapsedTime();

		if (elapsedTime.asSeconds() - lastUpdate.asSeconds() >= 50)
		{
			velocityX -= 0.25;
			bird.updateVelocity(0.25);
			lastUpdate = elapsedTime;
		}
	};


	void generatePipes()
	{
		int topPipeHeight = rand() % (PIPE_MAX_HEIGHT - PIPE_MIN_HEIGHT + 1) + PIPE_MIN_HEIGHT;
		int topY = 0;
		int bottomPipeHeight = rand() % (PIPE_MAX_HEIGHT - topPipeHeight - PIPE_MIN_HEIGHT + 1) + PIPE_MIN_HEIGHT;
		int bottomY = WINDOW_HEIGHT - bottomPipeHeight - 1;
		int width = rand() % (PIPE_MAX_WIDTH - PIPE_MIN_WIDTH + 1) + PIPE_MIN_WIDTH;

		Pipe topPipe(topY, width, topPipeHeight, Top);
		Pipe bottomPipe(bottomY, width, bottomPipeHeight, Bottom);
		pipes.push_back(topPipe);
		pipes.push_back(bottomPipe);
	}

	GameEngine() = default;

public:

	const vector<Pipe>& getPipes() const
	{
		return pipes;
	}

	bool collisionCanvas()const
	{
		return bird.getY() + bird.getHeight() / 2 >= WINDOW_HEIGHT || bird.getY() - bird.getHeight() / 2 <= 0;
	}

	bool collisionPipe(const Pipe& pipe) const
	{
		const int pipeX = pipe.getX();
		const int pipeY = pipe.getY();

		const int pipeRightX = pipe.getX() + pipe.getWidth();
		const int pipeBottomY = pipeY + pipe.getHeight();

		if (bird.getX() - bird.getWidth() / 2 <= pipeRightX &&
			bird.getX() + bird.getWidth() / 2 >= pipe.getX())
		{
			if (pipe.getType() == Top && bird.getY() - bird.getHeight() / 2 < pipeBottomY && !pipe.checkIfSoft())
			{
				cout << "Collision with a pipe, bird right x -  " << -bird.getWidth() / 2 <<", bird left x "<< bird.getX() + bird.getWidth()<< " pipe left x - " << pipeX << ", pipe right x" << pipeRightX << endl;
				cout << "Collision with a top pipe, bird y -  " << bird.getY() << ", pipe bottom y - " << pipeBottomY << endl;
				return true;
			}
			if (pipe.getType() == Bottom && bird.getY() + bird.getHeight() / 2 > pipe.getY() && !pipe.checkIfSoft())
			{
				cout << "Collision with a pipe, bird x -  " << bird.getX() << ", pipe left x - " << pipeX << ", pipe right x" << pipeRightX << endl;
				cout << "Collision with a bottom pipe, bird y -  " << bird.getY() << ", pipe top y - " << pipe.getY() << endl;
				return true;
			}
		}
		return false;
	}

	void go()
	{
		sf::Time currentTime = clock.getElapsedTime();
		if (currentTime.asSeconds() - lastPipeSGenerated.asSeconds() >= 1)
		{
			generatePipes();
			lastPipeSGenerated = currentTime;
		}

		bird.updateVelocity(GRAVITY);
		bird.updatePosition();

		if (collisionCanvas())
		{
			Over();
			return;
		}

		for (Pipe& pipe : pipes)
		{

			pipe.updatePosition(velocityX);
			if (collisionPipe(pipe))
			{
				cout << pipe.getX() << endl;

				if (!pipe.checkIfSoft())
				{
					Over();
					return;
				}
			}
			else if (!pipe.checkIfPassed() && bird.getX() > pipe.getX() + pipe.getWidth())
			{
				cout << pipe.getX() << endl;

				pipe.makePassed();
				score += 0.5;
			}
		}

	};
	void start()
	{
		started = true;
		clock.restart();
		lastPipeSGenerated = clock.getElapsedTime();
		generatePipes();
		go();

	}

	bool over = false;
	bool started = false;

	void reset()
	{
		score.saveBestScore();
		pipes.clear();
		score = 0;
		start();
	}

	void flap()
	{
		bird.flapWings();

		if (collisionCanvas())
		{
			over = true;
			cout << "coliision with canvas" << endl;
		}
	}

	void Over()
	{
		over = true;
		score.saveBestScore();
	}

	const Bird& getBird()const
	{
		return bird;
	}

	const Score& getScore()const
	{
		return score;
	}

	static GameEngine* getInstance()
	{
		if (instance == nullptr)
		{
			srand(static_cast<unsigned>(time(0)));
			instance = new GameEngine();
		}
		return instance;
	}
};
GameEngine* GameEngine::instance = nullptr;


class Render {
private:

	static Render* instance;

	sf::RenderWindow window;
	sf::Texture pipeTexture;
	sf::Texture birdTexture;
	sf::Font font;

	Render() : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Flappy Bird")
	{
		if (!pipeTexture.loadFromFile("C:\\Margo\\Uni\\five\\FlappyBird\\pipe2.png")) {
			cout << "The pipe image could not be loaded!" << endl;
		}
		if (!birdTexture.loadFromFile("C:\\Margo\\Uni\\five\\FlappyBird\\im.png")) {
			cout << "The bird image could not be loaded!" << endl;
		}
		if (!font.loadFromFile("C:\\Margo\\Uni\\five\\FlappyBird\\Home Creative.otf"))
		{
			cout << "The font could not be loaded!" << endl;

		}
	}
	const pair<float, float> getScaleFactors(const sf::Texture& texture, const Entity& entity)const
	{
		float desiredWidth = static_cast<float>(entity.getWidth());
		float desiredHeight = static_cast<float>(entity.getHeight());

		float originalWidth = texture.getSize().x;
		float originalHeight = texture.getSize().y;

		float scaleX = desiredWidth / originalWidth;
		float scaleY = desiredHeight / originalHeight;
		return { scaleX, scaleY };
	}

	void drawPipe(const Pipe& pipe) 
	{
		sf::Sprite pipe_shape;
		pipe_shape.setTexture(pipeTexture);
		if (pipe.getType() == Top)
		{
			pipe_shape.setOrigin(pipeTexture.getSize().x / 2, pipeTexture.getSize().y / 2);
			pipe_shape.setRotation(180);
			pipe_shape.setOrigin(pipeTexture.getSize().x, pipeTexture.getSize().y );

		}
		pipe_shape.setPosition(pipe.getX(), pipe.getY());
		pair<float, float> scale = getScaleFactors(pipeTexture, pipe);
		if (pipe.checkIfSoft())
		{
			pipe_shape.setColor(sf::Color::Magenta);
		}
		pipe_shape.setScale(scale.first, scale.second);
		window.draw(pipe_shape);
		cout << "drawing pipe" << endl;

	}

	void drawBird(const Bird& bird) 
	{
		sf::Sprite bird_shape;
		bird_shape.setTexture(birdTexture);
		pair<float, float> scale = getScaleFactors(birdTexture, bird);
		bird_shape.setScale(scale.first, scale.second);
		bird_shape.setPosition(bird.getX() - (bird.getWidth() / 2), bird.getY() - (bird.getHeight() / 2));
		window.draw(bird_shape);
	}
	void manageWindow()
	{

	}
	void applyStylesToText(sf::Text& text)
	{
		text.setFont(font);
		text.setCharacterSize(90);
		text.setFillColor(sf::Color::Magenta);
		text.setStyle(sf::Text::Bold);
		sf::FloatRect textBounds = text.getLocalBounds();
		text.setOrigin(textBounds.width / 2, 0);
	}

	void displayGameOver(GameEngine* game)
	{
		sf::Text gameOverMessage;
		sf::Text scoreMessage;

		gameOverMessage.setString("Game over!\n");
		scoreMessage.setString("Your score: " + to_string(static_cast<int>(game->getScore().getScore())) + "\nBest score: " + to_string(static_cast<int>(game->getScore().getBestScore())));
		applyStylesToText(gameOverMessage);
		applyStylesToText(scoreMessage);
		
		gameOverMessage.setPosition(sf::Vector2f(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 6));
		scoreMessage.setPosition(sf::Vector2f(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 6 + 90));
	    window.draw(gameOverMessage);
		window.draw(scoreMessage);

	}

	void render(GameEngine* game)
	{
		window.clear();
		drawBird(game->getBird());
		for (const Pipe& pipe : game->getPipes())
		{
			drawPipe(pipe);
		}
		if (game->over)
		{
			displayGameOver(game);
		}
		window.display();
	}
	
	void processEvents(GameEngine* game)
	{
		sf::Event event;
		while (window.pollEvent(event) && !game->over)
		{
			if (event.type == sf::Event::Closed)
			{
				game->Over();
				window.close();
			}
			else if ((event.type == sf::Event::MouseButtonPressed || event.type == sf::Event::KeyPressed) && !game->started)
			{
				if (event.mouseButton.button == sf::Mouse::Left || event.key.code == sf::Keyboard::Space)
				{
					game->start();

				}

			}
			else if ((event.type == sf::Event::MouseButtonPressed || event.type == sf::Event::KeyReleased) && game->started)
			{
				if (event.mouseButton.button == sf::Mouse::Left || event.key.code == sf::Keyboard::Space)
				{
					game->flap();
				}
			}
			else if (event.type == sf::Event::KeyReleased)
			{
				if (event.key.code == sf::Keyboard::R)
				{
					game->reset();
				}

			}
		}

	};
public:

	static Render* getInstance() {
		if (instance == nullptr)
		{
			instance = new Render();
		}
		return instance;
	}

	void launch(GameEngine* game)
	{
		while (window.isOpen()&&!game->over)
		{
			processEvents(game);
			if (!game->over && game->started)
			{
				game->go();

			}
			render(game);
		}

	}
};

Render* Render::instance = nullptr;



int main()
{
	GameEngine* gameEngine = GameEngine::getInstance();
	Render* render = Render::getInstance();

	render->launch(gameEngine);
	return 0;
}