#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
using namespace std;
#include <vector>
#include <string>
#include <fstream>
#include <memory>
#define WINDOW_HEIGHT 800
#define WINDOW_WIDTH 1400
#define BIRD_HEIGHT 60
#define BIRD_WIDTH 60
#define MIN_SPACE_BETWEEN_PIPES 250
#define PIPE_MAX_HEIGHT 420
#define PIPE_MIN_HEIGHT 120
#define PIPE_MAX_WIDTH 100
#define PIPE_MIN_WIDTH 70
#define SOFT_PIPE_CHANCE 50
#define VELOCITY_Y 0
#define VELOCITY_X -1
#define JUMP -0.5

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


class Heart : public Entity
{
	static const int definePositionY()
	{
		int y = rand() % (WINDOW_HEIGHT - 160) + 80; 
		return y;
	}

	bool caught = false;
public:

	Heart() :Entity(WINDOW_WIDTH, definePositionY(), BIRD_WIDTH, BIRD_HEIGHT) {}

	
	bool isCaught() const 
	{ 
		return caught; 
	}

	void markAsCaught() 
	{ 
		caught = true; 
	}

	void updatePostion(const double value)
	{
		x += value;
	}





};

class Bird : public Entity
{
	double velocityY = VELOCITY_Y;
	vector<Heart> hearts;
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
	void addHeart(const Heart& heart)
	{
		if (hearts.size() < 3)
		{
			hearts.push_back(heart);
		}
	}
	void deleteHeart()
	{
		hearts.pop_back();
	}
	const int getCounter()const
	{
		return hearts.size();
	}
	const vector<Heart>& getLives()const
	{
		return hearts;
	}
};

class GameEngine
{
	const double gravity = 0.005;
	vector<Heart> hearts;
	static GameEngine* instance;
	sf::Clock clock;
	vector<Pipe> pipes;
	Bird bird;
	Score score;
	double velocityX = VELOCITY_X;
	sf::Time lastPipeSGenerated;
	double timePipes = 3;
	double timeHearts = 4;
	sf::Time lastHeartGenerated;

	void generatePipes()
	{
		int topPipeHeight = rand() % (PIPE_MAX_HEIGHT - PIPE_MIN_HEIGHT + 1) + PIPE_MIN_HEIGHT;
		int topY = 0;
		int bottomPipeHeight = rand() % (PIPE_MAX_HEIGHT - topPipeHeight - PIPE_MIN_HEIGHT + 1) + PIPE_MIN_HEIGHT;
		int bottomY = WINDOW_HEIGHT - bottomPipeHeight - 1;
		int width = rand() % (PIPE_MAX_WIDTH - PIPE_MIN_WIDTH + 1) + PIPE_MIN_WIDTH;
		int space = WINDOW_HEIGHT - topPipeHeight - bottomPipeHeight;
		Pipe topPipe(topY, width, topPipeHeight, Top);
		Pipe bottomPipe(bottomY, width, bottomPipeHeight, Bottom);
		pipes.push_back(topPipe);
		pipes.push_back(bottomPipe);
	}

	GameEngine() = default;

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
			bird.getX() + bird.getWidth() / 2 >= pipe.getX() )
		{
			if (pipe.getType() == Top && bird.getY() - bird.getHeight() / 2 < pipeBottomY)
			{
				return true;
			}
			if (pipe.getType() == Bottom && bird.getY() + bird.getHeight() / 2 > pipe.getY())
			{
				return true;
			}
		}
		return false;

	}

	bool checkBirdCatch( Heart& heart) const 
	{
		if (!heart.isCaught() &&
			bird.getX() + bird.getWidth() / 2 >= heart.getX() &&
			bird.getX() - bird.getWidth() / 2 <= heart.getX() + heart.getWidth() &&
			bird.getY() + bird.getHeight() / 2 >= heart.getY() &&
			bird.getY() - bird.getHeight() / 2 <= heart.getY() + heart.getHeight())
		{
			heart.markAsCaught();
			return true;
		}
		return false;
	}

	void generateHearts()
	{
		Heart heart;
		hearts.push_back(heart);
	}

	void add_hearts(sf::Time& currentTime)
	{
		if (currentTime.asSeconds() - lastPipeSGenerated.asSeconds() >= timePipes)
		{
			generatePipes();
			lastPipeSGenerated = currentTime;
		}
	}
	void add_pipes(sf::Time& currentTime)
	{
		if (bird.getCounter() < 3 && currentTime.asSeconds() - lastHeartGenerated.asSeconds() >= timeHearts) {
			generateHearts();
			lastHeartGenerated = currentTime;
		}
	}

	void manage_hearts()
	{
		for (auto heart = hearts.begin(); heart != hearts.end(); )
		{
			heart->updatePostion(velocityX);
			if (checkBirdCatch(*heart)) {
				bird.addHeart(*heart);
				heart = hearts.erase(heart);
			}
			else if (heart->getX() + heart->getWidth() < 0) 
			{
				heart = hearts.erase(heart);
			}
			else {
				heart++;
			}
		}
	}

	void manage_pipes()
	{
		for (auto pipe = pipes.begin(); pipe != pipes.end();)
		{
			pipe->updatePosition(velocityX);
			if (!pipe->checkIfSoft() && collisionPipe(*pipe) && !pipe->checkIfPassed())
			{
				if (bird.getCounter() > 0)
				{
					bird.deleteHeart();
					pipe->makePassed();
					pipe = pipes.erase(pipe);
					score += 0.5;
				}
				else
				{
					Over();
					return;
				}
			}
			else if (!pipe->checkIfPassed() && bird.getX() > pipe->getX() + pipe->getWidth())
			{
				pipe->makePassed();
				score += 0.5;
				pipe++;
			}
			else
			{
				if (pipe->getX() + pipe->getWidth() < 0)
				{
					pipe = pipes.erase(pipe);
				}
				else
				{
					pipe++;
				}
			}
		}
	}

public:

	const vector<Pipe>& getPipes() const
	{
		return pipes;
	}

	const vector<Heart>& getHearts() const
	{
		return hearts;
	}

	void decreaseTimePipesHearts(const double time)
	{
		if (time == 0.1)
		{
			return;
		}
		timeHearts += time;
		timePipes -= time;
	}

	void go()
	{
	    sf::Time currentTime = clock.getElapsedTime();
		add_pipes(currentTime);
		add_hearts(currentTime);
		bird.updateVelocity(gravity);
		bird.updatePosition();
		if (collisionCanvas())
		{		
			Over();
			return;
		}
		manage_hearts();
		manage_pipes();
	}

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
	bool paused = false;

	void reset()
	{
		timePipes = 3;
		score.saveBestScore();
		pipes.clear();
		hearts.clear();
		score = 0;
		bird = Bird();
		start();
	}

	void flap()
	{
		bird.flapWings();

		if (collisionCanvas())
		{
			over = true;
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
	sf::Texture restartTexture;
	sf::Texture playTexture;
	sf::Texture heartTexture;
	sf::Font font;
	sf::Texture backgroundTexture;
	sf::SoundBuffer bufferForSquick;
	sf::SoundBuffer heartBuffer;
	sf::Sound squick; 
	sf::Music music;
	sf::CircleShape triangle;
	sf::Sprite restart_shape;
	sf::Sprite play_shape;
	sf::Clock clock;
	float FPS = 200.f;

	Render() : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Flappy Bird")
	{
		loadFiles();
		music.setLoop(true);
		squick.setBuffer(bufferForSquick);
		squick.setVolume(25);
	}

	void loadFiles()
	{
		if (!pipeTexture.loadFromFile("C:\\Margo\\Uni\\five\\FlappyBird\\pipe2.png")) {
			cout << "The pipe image could not be loaded!" << endl;
		}
		if (!birdTexture.loadFromFile("C:\\Margo\\Uni\\five\\FlappyBird\\im.png")) {
			cout << "The bird image could not be loaded!" << endl;
		}
		if (!playTexture.loadFromFile("C:\\Margo\\Uni\\five\\FlappyBird\\play.png")) {
			cout << "The play image could not be loaded!" << endl;
		}
		if (!restartTexture.loadFromFile("C:\\Margo\\Uni\\five\\FlappyBird\\restart.png")) {
			cout << "The restart image could not be loaded!" << endl;
		}
		if (!font.loadFromFile("C:\\Margo\\Uni\\five\\FlappyBird\\Home Creative.otf"))
		{
			cout << "The font could not be loaded!" << endl;
		}
		if (!backgroundTexture.loadFromFile("C:\\Margo\\Uni\\five\\FlappyBird\\R.jpg"))
		{
			cout << "The background could not be loaded!" << endl;
		}
		if (!bufferForSquick.loadFromFile("C:\\Margo\\Uni\\five\\FlappyBird\\squick.ogg")) {
			cout << "Squick sound could not be loaded!" << endl;
		}
		if (!music.openFromFile("C:\\Margo\\Uni\\five\\FlappyBird\\song.ogg"))
		{
			cout << "Music could not be loaded!" << endl;
		}
		if (!heartTexture.loadFromFile("C:\\Margo\\Uni\\five\\FlappyBird\\heart.png")) {
			cout << "The heart image could not be loaded!" << endl;
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

	sf::Sprite drawHeart(const Heart& heart) {
		sf::Sprite heart_shape;
		heart_shape.setTexture(heartTexture);
		pair<float, float> scale = getScaleFactors(heartTexture, heart);
		heart_shape.setScale(scale.first, scale.second);
		heart_shape.setPosition(heart.getX(), heart.getY());
		return heart_shape;
	}

	void drawHeartAtTop(const vector<Heart>& hearts)
	{
		int value = 20;
		for (const Heart& heart : hearts)
		{
			sf::Sprite heart_shape = drawHeart(heart);
			pair<float, float> scale = getScaleFactors(heartTexture, heart);
			heart_shape.setScale(scale.first * 0.5f, scale.second * 0.5f);
			heart_shape.setPosition(WINDOW_WIDTH - value - heart.getWidth(), 20);
			value += 20 + heart.getWidth();
			window.draw(heart_shape);
		}
	}

	void processSignPicture(sf::Sprite& image, sf::Texture&  texture)
	{
		image.setTexture(texture);
		sf::FloatRect bounds = image.getLocalBounds();
		image.setOrigin(bounds.width / 2, bounds.height / 2);
		pair<float, float> scale = getScaleFactors(texture, Entity(0, 0, 160, 160));
		image.setScale(scale.first, scale.second);
	}

	void displayPaused()
	{
		play_shape.setPosition(WINDOW_WIDTH / 2 - 100, WINDOW_HEIGHT / 2);
		processSignPicture(play_shape, playTexture);
		restart_shape.setPosition(WINDOW_WIDTH / 2 + 100, WINDOW_HEIGHT / 2);
		processSignPicture(restart_shape, restartTexture);
		triangle.setPosition(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
		window.draw(play_shape);
		window.draw(restart_shape);
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
	}

	void displayAtStart()
	{
		processSignPicture(play_shape, playTexture);
		play_shape.setPosition(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
		window.draw(play_shape);
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
		Entity background_entity(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
		sf::Sprite background;
		background.setTexture(backgroundTexture);
		pair<float, float> scale = getScaleFactors(backgroundTexture, background_entity);
		background.setScale(scale.first, scale.second);
		background.setPosition(0, 0);
		window.draw(background);
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

	void drawRestart()
	{	
		processSignPicture(restart_shape, restartTexture);
		restart_shape.setPosition(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 6 + 380);
		window.draw(restart_shape);
	}

	void displayPoints(GameEngine* game)
	{
		if (!game->over)
		{
			sf::Text points;
			points.setString(to_string(static_cast<int>(game->getScore().getScore())));
			applyStylesToText(points);
			points.setPosition(sf::Vector2f(50.f, 10.f));
			window.draw(points);
		}	
	}

	void render(GameEngine* game)
	{
		window.clear();
		manageWindow();
		if (game->started)
		{
			drawBird(game->getBird());
			for (const Pipe& pipe : game->getPipes())
			{
				drawPipe(pipe);
			}
			if (!game->getHearts().empty()) {
				for (const Heart& heart : game->getHearts()) 
				{
					window.draw(drawHeart(heart));
					
					
				}
			}
			drawHeartAtTop(game->getBird().getLives());
			displayPoints(game);
		}
		if (game->over)
		{
			displayGameOver(game);
			drawRestart();
		}
		else if  (!game->started)
		{
			displayAtStart();
		}
		else if (game->paused)
		{
			displayPaused();
		}
		window.display();
	}

	bool checkIfClicked(sf::Sprite& shape)
	{
		sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window)); 
		if (shape.getGlobalBounds().contains(mousePos))
		{
			shape.setColor(sf::Color::Blue);
			if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
			{
				return true;
			}
		}
		else
		{
			shape.setColor(sf::Color::White);
		}
		return false;
	}

	void processEvents(GameEngine* game)
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				window.close();
				return;
			}
			if (event.type == sf::Event::MouseButtonPressed || event.type == sf::Event::KeyPressed)
			{
				if (!game->started)
				{
					if (checkIfClicked(play_shape) || event.key.code == sf::Keyboard::Space)
					{
						game->start();
						music.setLoop(true);
						music.play();
					}
				}
				else if (game->paused)
				{
					if (checkIfClicked(restart_shape) || event.key.code == sf::Keyboard::R)
					{
						game->paused = false;
						music.play();
						game->reset();
					}
					else if (checkIfClicked(play_shape) || event.key.code == sf::Keyboard::Space)
					{
						game->paused = false;
						music.play();
					}
				}
				else if (game->over)
				{
					if (checkIfClicked(restart_shape) || event.key.code == sf::Keyboard::Space)
					{
						game->over = false;
						music.play();
						window.setFramerateLimit(200);
						clock.restart();
						game->reset();
					}
				}
				else 
				{
					if (event.mouseButton.button == sf::Mouse::Left || event.key.code == sf::Keyboard::Space)
					{
						game->flap();
						if (!game->over)
						{
							squick.play();
						}
					}
					else if (event.mouseButton.button == sf::Mouse::Right || event.key.code == sf::Keyboard::P)
					{
						game->paused = true;
						music.stop();
					}
				}
			}
		}
		if (game->over)
		{
			music.stop();
		}		
	}

	void managegameSpeed(GameEngine* game)
	{
		static sf::Time lastUpdate = sf::seconds(0);
		sf::Time elapsedTime = clock.getElapsedTime();

		if (elapsedTime.asSeconds() - lastUpdate.asSeconds() >= 15)
		{
			FPS += 70.0f; 
			window.setFramerateLimit(FPS);
			game->decreaseTimePipesHearts(0.4);
			lastUpdate = elapsedTime; 
		}
	}

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
		window.setFramerateLimit(FPS);
		while (window.isOpen())
		{
			managegameSpeed(game);
			checkIfClicked(restart_shape);
			checkIfClicked(play_shape);
			processEvents(game);
			if (!game->over && game->started && !game->paused) {
				game->go();
			}			
			render(game);			
		}
		clear();
	}	

	void clear() {
		squick.stop();
		music.stop();
	}

};

Render* Render::instance = nullptr;


int main()
{
	srand(static_cast<unsigned>(time(0)));  
	GameEngine* gameEngine = GameEngine::getInstance();
	Render* render = Render::getInstance();
	render->launch(gameEngine);
	return 0;
}