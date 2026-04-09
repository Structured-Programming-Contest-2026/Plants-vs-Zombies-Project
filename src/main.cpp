#include <iostream>
#include <vector>
#define SFML_STATIC
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

#define width 1920
#define height 1080
#define gridraws 5
#define gridcols 9
#define cellwidth 140
#define cellheight 165
#define offsetX 500
#define offsetY 100
#define edgespace -100

float delta_time = 1.0f;
Vector2i mousePos;

// left - right

bool mouseJustClicked[2];
bool uiInteracted = false;
// Global Variables
const int
gridRows = gridraws,
gridCols = gridcols,
cellWidth = cellwidth,
cellHeight = cellheight,
windowWidth = width,
windowHeight = height;
const int edgeSpace = edgespace; // when zombie reaches this space, It disappears (end of screen) - should end game later

enum class ENTITY_CATEGORY : uint8_t {
	NONE = 0,
	ZOMBIE,
	PLANT,
	Bullet,
};

////zombie states
//enum class ZOMBIE_STATE : uint8_t {
//	WALKING = 0,
//	EATING,
//	DYING,
//};

enum class ENTITY_TYPE : uint8_t {
	NONE = 0,
	ZOMBIE_NORMAL,
	ZOMBIE_JAPANESE,
	ZOMBIE_BIG,
	ZOMBIE_TORCHLIGHT,

	PLANT_NORMAL,
	PLANT_SHROOM,
	PLANT_SUNFLOWER,

	NormalBullet,
};

enum class TEXTURE_TYPE : uint8_t {
	NONE = 0,
	SUN,
	SEEDBANK,
	SEEDPACKET,

	ZOMBIE_NORMAL,
	ZOMBIE_JAPANESE,
	ZOMBIE_BIG,
	ZOMBIE_TORCHLIGHT,

	PLANT_NORMAL,
	PLANT_SHROOM,
	PLANT_SUNFLOWER,

	NormalBullet,
};

struct Entity {
	ENTITY_CATEGORY category;
	ENTITY_TYPE type;
	float frameTimer = 1;
	int	animation_frames = 6;
	int health = 100;
	int attack = 15;
	float x_vel = 0.5;
	int current_frame = 0;
	Clock frameClock;
	Clock attackClock;
	int attackTimer = 1;
	Vector2f position;
	int price = 25;
	float scale = 1.0f;
	int gridRow = -1;
	int gridCol = -1;
};

//Independet Sprites

Sprite plantSprite;
Sprite zombieSprite;
Sprite bulletSprite;
Sprite uiSprite;

// TODO: remove vectors and maps and replace them with arrays :)

// Plants + Plant Grid
vector<Entity> entities;
vector<Entity> plant_types;

int selectedPlant = 0;

// Suns
#define MAX_SUNS 16
Clock sunSpawnClock;
int money = 3000;
struct Sun {
	Vector2f position;
	Clock lifeClock;
	float distance = 500.0f;
	bool clicked = true;
};
Sun suns[MAX_SUNS];
bool isSunAlive(const Sun& sun) {
	return !sun.clicked && sun.lifeClock.getElapsedTime().asSeconds() <= 10; //cool trick   
}
bool addSun(Sun sun) {
	for (int i = 0; i < MAX_SUNS; i++) {
		if (!isSunAlive(suns[i])) {
			suns[i] = sun;
			return true;
		}
	}
	return false;
}

// Zombies + Zombie List + Zombie Types	- we can have multiple zombies at a time, so we will keep them in a list
vector<Entity> zombie_types;
// Zombie Clock - to add new zombie after some time interval
Clock zombieClock;
//Sending Zombies
float nextZombieTimer = 3;
float ZombieMinGapTime = 3;

// Textures
map<TEXTURE_TYPE, Texture> textures;
Texture& getTexture(TEXTURE_TYPE type) {
	return textures[type];
}
Texture& getEntityTexture(ENTITY_TYPE type) {
	TEXTURE_TYPE textureType = TEXTURE_TYPE::NONE;
	switch (type) {
		//zmobies 
	case ENTITY_TYPE::ZOMBIE_NORMAL:
		textureType = TEXTURE_TYPE::ZOMBIE_NORMAL;
		break;
	case ENTITY_TYPE::ZOMBIE_JAPANESE:
		textureType = TEXTURE_TYPE::ZOMBIE_JAPANESE;
		break;
	case ENTITY_TYPE::ZOMBIE_BIG:
		textureType = TEXTURE_TYPE::ZOMBIE_BIG;
		break;
	case ENTITY_TYPE::ZOMBIE_TORCHLIGHT:
		textureType = TEXTURE_TYPE::ZOMBIE_TORCHLIGHT;
		break;
		//plantes
	case ENTITY_TYPE::PLANT_NORMAL:
		textureType = TEXTURE_TYPE::PLANT_NORMAL;
		break;
	case ENTITY_TYPE::PLANT_SHROOM:
		textureType = TEXTURE_TYPE::PLANT_SHROOM;
		break;
	case ENTITY_TYPE::PLANT_SUNFLOWER:
		textureType = TEXTURE_TYPE::PLANT_SUNFLOWER;
		break;
		//other
	case ENTITY_TYPE::NormalBullet:
		textureType = TEXTURE_TYPE::NormalBullet;
		break;
	default:
		textureType = TEXTURE_TYPE::NONE;
		break;
	}

	return textures[textureType];
}
void resizeSpriteToTex(Sprite& sprite, Texture& tex) {
	sprite.setTextureRect(IntRect{ 0, 0, (int)tex.getSize().x, (int)tex.getSize().y });
}

// Helping - Functions

Vector2i gridCell_imp(Vector2f pos)
{
	int column = (pos.x - offsetX) / cellWidth;
	int raw = (pos.y - offsetY) / cellHeight;
	return { raw, column };
}

Vector2i gridCell(Vector2i pos) { return gridCell_imp({ float(pos.x), float(pos.y) }); }
Vector2i gridCell(Vector2f pos) { return gridCell_imp({ float(pos.x), float(pos.y) }); }

Entity& getEntity(Vector2i gridPos)
{

	static Entity emptyEntity{};
	emptyEntity.category = ENTITY_CATEGORY::NONE;

	for (Entity& entity : entities) 
	{

		if (entity.category != ENTITY_CATEGORY::PLANT || entity.health <= 0) continue;

		if (entity.gridRow == gridPos.x && entity.gridCol == gridPos.y) 
		{
			return entity;
		}
	}
	return emptyEntity;
}

// Define Characters - Functions

// ground
Sprite defineGroundSprite(Texture& tex) {
	tex.loadFromFile("assets/background.png");
	if (!tex.loadFromFile("assets/background.png")) {
		cout << "Error: Could not load assets/background.png" << endl;
	}

	/*tex.setRepeated(true);*/

	Sprite groundSp;
	groundSp.setTexture(tex);

	/*IntRect groundTexRect(0, 0, width, height);
	groundSp.setTextureRect(groundTexRect);*/
	groundSp.setTextureRect(IntRect(0, 0, (int)tex.getSize().x, (int)tex.getSize().y));
	
	return groundSp;
}
// plants
Entity defineNormalPlant()
{
	{
		Texture tex;
		tex.loadFromFile("assets/Repeater.png");
		textures[TEXTURE_TYPE::PLANT_NORMAL] = tex;
		Entity planet;
		planet.category = ENTITY_CATEGORY::PLANT;
		planet.type = ENTITY_TYPE::PLANT_NORMAL;
		planet.frameTimer = .1;
		planet.animation_frames = 14;
		planet.health = 100;
		planet.attack = 1000;
		planet.price = 25;

		return planet;
	}
}
Entity defineShroomPlant() {
	Texture tex;
	tex.loadFromFile("assets/shroom.png");
	textures[TEXTURE_TYPE::PLANT_SHROOM] = tex;

	Entity shroom;
	shroom.category = ENTITY_CATEGORY::PLANT;
	shroom.type = ENTITY_TYPE::PLANT_SHROOM;
	shroom.frameTimer = .1;
	shroom.animation_frames = 30;
	shroom.health = 500;
	shroom.attack = 0;
	shroom.price = 15;
	shroom.scale = 0.9;
	return shroom;
}

Entity defineSunflower() {
	Texture tex;
	tex.loadFromFile("assets/sunflower.png");
	textures[TEXTURE_TYPE::PLANT_SUNFLOWER] = tex;

	Entity plant;
	plant.category = ENTITY_CATEGORY::PLANT;
	plant.type = ENTITY_TYPE::PLANT_SUNFLOWER;
	plant.frameTimer = 0.15f;
	plant.animation_frames = 8;
	plant.health = 80;
	plant.attack = 0;
	plant.price = 50;
	plant.scale=1.2;

	return plant;
}
// zombies
Entity defineNormalZombie() {
	Texture tex;
	tex.loadFromFile("assets/zombie4.png");
	textures[TEXTURE_TYPE::ZOMBIE_NORMAL] = tex;
	Entity zombie = { ENTITY_CATEGORY::ZOMBIE, ENTITY_TYPE::ZOMBIE_NORMAL, .1, 20, 100, 10 };
	zombie.x_vel = .05;
	zombie.scale = 1.0;
	return zombie;
}
Entity defineJapaneseZombie() {
	Texture tex;
	tex.loadFromFile("assets/japanese.png");
	textures[TEXTURE_TYPE::ZOMBIE_JAPANESE] = tex;
	Entity zombie = { ENTITY_CATEGORY::ZOMBIE, ENTITY_TYPE::ZOMBIE_JAPANESE, .01, 20, 200, 70, .05 };
	return zombie;
}
Entity defineBigZombie() {
	Texture tex;
	tex.loadFromFile("assets/big-zombie.png");
	textures[TEXTURE_TYPE::ZOMBIE_BIG] = tex;
	Entity zombie{};
	zombie.category = ENTITY_CATEGORY::ZOMBIE;
	zombie.type = ENTITY_TYPE::ZOMBIE_BIG;
	zombie.frameTimer = 0.1f;
	zombie.animation_frames = 38;
	zombie.health = 500;
	zombie.attack = 30;
	zombie.x_vel = 0.05f;
	zombie.scale = 1.1f;
	return zombie;
}

Entity defineTorchlightZombie() {
	Texture tex;
	tex.loadFromFile("assets/torchlightzombie.png");
	textures[TEXTURE_TYPE::ZOMBIE_TORCHLIGHT] = tex;

	Entity zombie{};
	zombie.category = ENTITY_CATEGORY::ZOMBIE;
	zombie.type = ENTITY_TYPE::ZOMBIE_TORCHLIGHT;

	zombie.health = 200;
	zombie.attack = 20;
	zombie.x_vel = 0.05f;
	zombie.frameTimer = 0.1f;
	zombie.animation_frames = 8;
	zombie.scale = 0.8f;

	return zombie;
}
//bullets

Entity defineNormalBullet() {
	Texture tex;
	tex.loadFromFile("assets/bullet.png");
	textures[TEXTURE_TYPE::NormalBullet] = tex;

	Entity bullet{};
	bullet.category = ENTITY_CATEGORY::Bullet;
	bullet.type = ENTITY_TYPE::NormalBullet;
	bullet.attack = 20;
	bullet.x_vel = -0.4f;


	return bullet;
}
// Drawing - Functions 

void UpdateSuns(RenderWindow& window) {
	for (int i = 0; i < MAX_SUNS; i++) {
		Sun& sun = suns[i];
		if (!isSunAlive(sun)) continue;

		if (sun.position.y <= sun.distance) {
			sun.position.y += 50.0f * delta_time;
		}

		Texture& tex = getTexture(TEXTURE_TYPE::SUN);
		uiSprite.setPosition(sun.position);
		uiSprite.setTexture(tex);
		uiSprite.setTextureRect(IntRect{ 0, 0, (int)tex.getSize().x, (int)tex.getSize().y });

		window.draw(uiSprite);
	}
}

//we will remove it later
void DrawDebugGrid(RenderWindow& window) {
	RectangleShape rect(Vector2f(cellWidth, cellHeight));
	rect.setFillColor(Color::Transparent);
	rect.setOutlineThickness(1.0f);
	rect.setOutlineColor(Color(255, 255, 255, 100));

	for (int r = 0; r < gridRows; r++) {
		for (int c = 0; c < gridCols; c++) {
			rect.setPosition(Vector2f(offsetX + (c * cellWidth), offsetY + (r * cellHeight)));
			window.draw(rect);
		}
	}
}

void DrawPlantsGrid(RenderWindow& window) {
	vector<Entity> newBullets; //temporary vector
	for (Entity& plant : entities) {
		if (plant.category != ENTITY_CATEGORY::PLANT) continue;

		//smart crop
		Texture& plantTex = getEntityTexture(plant.type);
		int frameWidth = plantTex.getSize().x / plant.animation_frames;
		int frameHeight = plantTex.getSize().y;

		if (plant.frameClock.getElapsedTime().asSeconds() >= plant.frameTimer) {
			plant.frameClock.restart();
			plant.current_frame++, plant.current_frame %= plant.animation_frames;
		}
		plantSprite.setTextureRect(IntRect{
			plant.current_frame * frameWidth,
			0,
			frameWidth,
			frameHeight
			});

		if (plant.type != ENTITY_TYPE::PLANT_SUNFLOWER) {
		
			if (plant.attackClock.getElapsedTime().asSeconds() >= 2.0f) {

				bool zombieInLane = false;
				for (Entity& zombie : entities) {
					if (zombie.category == ENTITY_CATEGORY::ZOMBIE && zombie.health > 0) {

						if (abs(zombie.position.y - plant.position.y) < 100.0f &&
							zombie.position.x > plant.position.x &&
							zombie.position.x < windowWidth - 50.0f)
						{
							zombieInLane = true;
							break;
						}
					}
				}

				if (zombieInLane) {
					plant.attackClock.restart();
					Entity newBullet = defineNormalBullet();
					newBullet.position = Vector2f(plant.position.x + 100, plant.position.y + 50);
					newBullets.push_back(newBullet);
				}
			}
		}
		else
		{
			if (plant.attackClock.getElapsedTime().asSeconds() >= 10.0f) {
				plant.attackClock.restart();

				Sun newSun{};
				newSun.position = plant.position;
				newSun.distance = plant.position.y + 50;
				newSun.clicked = false;
				newSun.lifeClock.restart();

				addSun(newSun);
				cout << "Sunflower produced a sun!" << endl;
			}
		}

		plantSprite.setPosition(plant.position);
		plantSprite.setTexture(getEntityTexture(plant.type));

		plantSprite.setScale(Vector2f(plant.scale, plant.scale));

		window.draw(plantSprite);

		plantSprite.setScale(Vector2f(1.0f, 1.0f));//Normal Scale for the other plants
	}

	for (Entity& bullet : newBullets) {
		entities.push_back(bullet);
	}
}
void DrawZombies(RenderWindow& window) {
	for (Entity& zombie : entities) {
		if (zombie.category != ENTITY_CATEGORY::ZOMBIE) continue;

		// dead
		if (zombie.health <= 0) continue;
		
		//crop its size
		Texture& zombieTex = getEntityTexture(zombie.type);
		int frameWidth = zombieTex.getSize().x / zombie.animation_frames;
		int frameHeight = zombieTex.getSize().y;

		// animation
		if (zombie.frameClock.getElapsedTime().asSeconds() >= zombie.frameTimer) {
			zombie.frameClock.restart();
			zombie.current_frame++, zombie.current_frame %= zombie.animation_frames;
		}

		zombieSprite.setTextureRect(IntRect
			{
			zombie.current_frame * frameWidth,
			0,
			frameWidth,
			frameHeight
			});

		// move
		zombie.position.x -= zombie.x_vel;
		auto [r, c] = gridCell(zombie.position);
		if (zombie.position.x <= 0 + edgeSpace) {
			zombie.category = ENTITY_CATEGORY::NONE;
		}
		else {		// plant
			// Eat
			Entity& plant = getEntity({ r, c });
			if (plant.category == ENTITY_CATEGORY::PLANT) {
				zombie.x_vel = 0;
				if (zombie.attackClock.getElapsedTime().asSeconds() >= zombie.attackTimer) {
					zombie.attackClock.restart();
					plant.health -= zombie.attack;
					if (plant.health <= 0) {
						zombie.x_vel = .05;
						plant.category = ENTITY_CATEGORY::NONE;
						cout << "Plant Killed\n";
					}
				}
			}
			else {
				zombie.x_vel = .05; // no plant - move normally edit_bug
			}
		}
		
		float actualHeight = frameHeight * zombie.scale;
		Vector2f adjustedPosition = zombie.position;
		adjustedPosition.y -= (actualHeight - cellHeight);

		zombieSprite.setPosition(adjustedPosition);
		zombieSprite.setTexture(getEntityTexture(zombie.type));

		zombieSprite.setScale(Vector2f(zombie.scale, zombie.scale));

		window.draw(zombieSprite);
	}
}
void UpdateAndDrawBullets(RenderWindow& window) {
	for (Entity& bullet : entities) {
		if (bullet.category != ENTITY_CATEGORY::Bullet) continue;

		bullet.position.x -= bullet.x_vel; // move right

		if (bullet.position.x >= windowWidth) {
			bullet.category = ENTITY_CATEGORY::NONE;
			continue;
		}

		for (Entity& zombie : entities) {

			if (zombie.category != ENTITY_CATEGORY::ZOMBIE || zombie.health <= 0) continue;// skip non-zombie or dead ones

			if (abs(bullet.position.y - zombie.position.y) < 100.0f && // same row
				bullet.position.x >= zombie.position.x &&
				bullet.position.x <= zombie.position.x + 100.0f) // almost touched
			{
				zombie.health -= bullet.attack;
				bullet.category = ENTITY_CATEGORY::NONE;

				cout << "Zombie hit! Health left: " << zombie.health << endl; //console just testing


				if (zombie.health <= 0) {
					zombie.category = ENTITY_CATEGORY::NONE;
					cout << "Zombie Killed!" << endl;
				}

				break;
			}
		}

		if (bullet.category == ENTITY_CATEGORY::Bullet) {
			Texture& bulletTex = getEntityTexture(bullet.type);
			bulletSprite.setTexture(bulletTex);
			bulletSprite.setTextureRect(IntRect{ 0, 0, (int)bulletTex.getSize().x, (int)bulletTex.getSize().y });
			bulletSprite.setPosition(bullet.position);
			window.draw(bulletSprite);
		}
	}
}
void CheckTimer_AddZombie(RenderWindow& window) {
	if (zombieClock.getElapsedTime().asSeconds() >= nextZombieTimer) {
		cout << "New Zombie\n";
		zombieClock.restart();
		nextZombieTimer = ZombieMinGapTime + rand() % 10;//I will edit it

		// get random zombie type
		int zombie_type_index = rand() % zombie_types.size();
		Entity newZombie = zombie_types[zombie_type_index];

		newZombie.position = Vector2f(width,offsetY + (rand() % gridraws) * cellheight);
		auto [raw, column] = gridCell(newZombie.position);
		entities.push_back(newZombie);
	}
}
void CheckTimer_AddSun(RenderWindow& window) {
	if (sunSpawnClock.getElapsedTime().asSeconds() >= 7) {
		cout << "New Sun\n";
		sunSpawnClock.restart();

		Sun sun{};
		sun.position = Vector2f(rand() % (windowWidth - 200), -215);
		sun.distance = rand() % 750;
		sun.clicked = false;

		addSun(sun);
	}
}

// There is a bug in SFML that causes the program to crash when exiting
// Making this a pointer and deleting it early prevents that crash
Font* font;

void DrawUI(RenderWindow& window) {
	// seeds
	{
		const Vector2f pos = Vector2f(10.0f, 0);

		Texture& seedBank = getTexture(TEXTURE_TYPE::SEEDBANK);
		
		uiSprite.setPosition(pos);
		uiSprite.setTexture(seedBank);
		resizeSpriteToTex(uiSprite, seedBank);
		uiSprite.setScale(Vector2f(1.4f, 1.4f));

		window.draw(uiSprite);
		uiSprite.setScale(Vector2f(1.0f, 1.0f));

		Text text;
		text.setFont(*font);
		text.setString(std::to_string(money));
		text.setFillColor(Color());
		text.setPosition(pos + Vector2f(32, 82));

		window.draw(text);

		Texture& seedPacket = getTexture(TEXTURE_TYPE::SEEDPACKET);
		uiSprite.setTexture(seedPacket);
		resizeSpriteToTex(uiSprite, seedPacket);
		uiSprite.setScale(Vector2f(0.75f, 0.75f));


		int i = -1;
		for (auto& plant : plant_types) {
			i++;
			Vector2f spritePos = pos + Vector2f(115.0f + i * (seedPacket.getSize().x - 10.0f), 10.0f);
			uiSprite.setPosition(spritePos);

			Vector2u size = Vector2u(Vector2f(seedPacket.getSize()) * 0.75f);

			uiSprite.setColor(Color(255, 255, 255, 255));
			plantSprite.setColor(Color(255, 255, 255, 255));

			if (mousePos.x >= spritePos.x && mousePos.x <= spritePos.x + size.x && mousePos.y >= spritePos.y && mousePos.y <= spritePos.y + size.y) {
				if (mouseJustClicked[0]) {
					uiInteracted = true;
					selectedPlant = i;
				}
			}

			//Smart Crop
			Texture& tex = getEntityTexture(plant.type);
			int frameWidth = tex.getSize().x / plant.animation_frames;
			int frameHeight = tex.getSize().y;

			plantSprite.setTexture(tex);
			plantSprite.setTextureRect(IntRect{ 0, 0, frameWidth, frameHeight });

			plantSprite.setPosition(spritePos + Vector2f(10, 20));

			float scaleFactor = 55.0f / frameHeight;
			plantSprite.setScale(Vector2f(scaleFactor, scaleFactor));

			window.draw(uiSprite);
			window.draw(plantSprite);

			text.setString(std::to_string(plant.price));
			text.setPosition(spritePos + Vector2f(10, 77));
			text.setCharacterSize(24);
			window.draw(text);
		}
	}
}


int main() {
	RenderWindow window(VideoMode(windowWidth, windowHeight), "Plants vs Zombies");
	Event event;
	// Characters
	Texture groundTex;
	Sprite ground = defineGroundSprite(groundTex);
	//New
	ground.setScale
	(
		(float)windowWidth / (float)groundTex.getSize().x,
		(float)windowHeight / (float)groundTex.getSize().y
	);

	// Loading Textures + Defining entities
	// Plants
	Entity plant = defineNormalPlant();
	Entity shroom = defineShroomPlant();
	Entity sunflower = defineSunflower();
	plant_types = { plant, shroom ,sunflower };

	// Zombies
	Entity zombie = defineNormalZombie();
	Entity zombieJapanese = defineJapaneseZombie();
	Entity zombieBig = defineBigZombie();
	Entity zombieTorch = defineTorchlightZombie();
	zombie_types = { zombie, zombieJapanese, zombieBig ,zombieTorch };

	// Other Textures
	textures[TEXTURE_TYPE::SUN].loadFromFile("assets/Sun3.png");
	textures[TEXTURE_TYPE::SEEDBANK].loadFromFile("assets/seedbank.png");
	textures[TEXTURE_TYPE::SEEDPACKET].loadFromFile("assets/seedpacket.png");

	// Font
	font = new Font();
	font->loadFromFile("assets/impact.ttf");

	// Game Loop

	Clock dtClock;

	while (window.isOpen()) {
		sf::Time dtTime = dtClock.restart();
		delta_time = dtTime.asSeconds();
		mousePos = Mouse::getPosition(window);

		uiInteracted = false;
		mouseJustClicked[0] = false;
		mouseJustClicked[1] = false;

		while (window.pollEvent(event)) {
			if (event.type == Event::Closed || Keyboard::isKeyPressed(Keyboard::Escape)) {
				window.close();
			}
			else if (event.type == sf::Event::MouseButtonPressed) {
				if (event.mouseButton.button == sf::Mouse::Left) mouseJustClicked[0] = true;
				else if (event.mouseButton.button == sf::Mouse::Right) mouseJustClicked[1] = true; //edit_bug
			}
			else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F) {
				// FOR TESTING ONLY
				money += 1000;
			}
		}

		CheckTimer_AddZombie(window);
		CheckTimer_AddSun(window);

		// Drawing

		window.clear(Color::Black);
		window.draw(ground);

		DrawPlantsGrid(window);
		DrawZombies(window);
		UpdateAndDrawBullets(window);
		UpdateSuns(window);

		DrawUI(window);
		/*DrawDebugGrid(window);*/

		if (!uiInteracted && mouseJustClicked[0]) {
			for (int i = 0; i < MAX_SUNS; i++) {
				// Check all suns + Add clicked suns to money
				Sun& sun = suns[i];
				if (!isSunAlive(sun)) continue;

				if (mousePos.x >= sun.position.x && mousePos.x <= sun.position.x + 200 && mousePos.y >= sun.position.y && mousePos.y <= sun.position.y + 200) {
					money += 25;

					cout << "You now have " << money << endl;

					suns[i].clicked = true;
					uiInteracted = true;
					break;
				}
			}

			if (!uiInteracted)
			{
				auto [row, col] = gridCell(mousePos);

				if (row >= 0 && row < gridRows && col >= 0 && col < gridCols)
				{

					if (getEntity({ row , col }).category == ENTITY_CATEGORY::NONE)
					{
						Entity plant;

						if (money >= plant_types[selectedPlant].price) {
							cout << "Placing Plant at " << row << " " << col << endl;
							plant = plant_types[selectedPlant];

							plant.gridRow = row;
							plant.gridCol = col;

							Texture& tex = getEntityTexture(plant.type);
							int plantWidth = (tex.getSize().x / plant.animation_frames) * plant.scale;
							int plantHeight = tex.getSize().y * plant.scale;

							//centrize
							int x = offsetX + (col * cellWidth) + ((cellWidth - plantWidth) / 2); //general formula -> start + (containerSize - objectSize) / 2

							//alighn to the bottom
							int y = offsetY + (row * cellHeight) + (cellHeight - plantHeight);

							plant.position = Vector2f(x, y);

							entities.push_back(plant);
							money -= plant.price;
						}
					}
				}
			}
		}

		window.display();
	}

	delete font;
	textures.clear();

	return 0;
}
