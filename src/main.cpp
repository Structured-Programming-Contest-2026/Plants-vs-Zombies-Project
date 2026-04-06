#include <iostream>
#include <vector>
#define SFML_STATIC
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

float dt = 1.0f;

// Global Variables
const int
gridRows = 5,
gridCols = 5,
cellWidth = 200,
cellHeight = 200,
windowWidth = gridCols * cellWidth,
windowHeight = gridRows * cellHeight;
const int edgeSpace = -100;		// when zombie reaches this space, It disappears (end of screen) - should end game later

enum class ENTITY_CATEGORY : uint8_t {
	NONE = 0,
	ZOMBIE,
	PLANT
};

enum class ENTITY_TYPE : uint8_t {
	NONE = 0,
	ZOMBIE_NORMAL,
	ZOMBIE_JAPANESE,
	ZOMBIE_BIG,
	PLANT_NORMAL,
	PLANT_SHROOM
};

enum class TEXTURE_TYPE : uint8_t {
	NONE = 0,
	SUN,
	ZOMBIE_NORMAL,
	ZOMBIE_JAPANESE,
	ZOMBIE_BIG,
	PLANT_NORMAL,
	PLANT_SHROOM
};

struct Entity {
	ENTITY_CATEGORY category;
	ENTITY_TYPE type;
	float frameTimer = 1;
	int	nframes = 6;
	int health = 100;
	int attack = 20;
	float xvel = 0.5;
	int frame = 0;
	Clock frameClock;
	Clock attackClock;
	int attackTimer = 1;
	Vector2f position;
};

Sprite sprite;

// TODO: remove vectors and maps and replace them with arrays

// Plants + Plant Grid
vector<Entity> entities;
vector<Entity> plant_types;

struct Sun {
	Vector2f position;
	Clock lifeClock;
	float distance = 500.0f;
	bool clicked = true;
};

#define MAX_SUNS 16

Clock sunSpawnClock;
int money = 100;
Sun suns[MAX_SUNS];

bool isSunAlive(const Sun& sun) {
	return !sun.clicked && sun.lifeClock.getElapsedTime().asSeconds() <= 10;
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
float nextZombieTimer = 1;
float ZombieMinGapTime = 1;

map<TEXTURE_TYPE, Texture> textures;

Texture& getTexture(TEXTURE_TYPE type) {
	return textures[type];
}

Texture& getEntityTexture(ENTITY_TYPE type) {
	TEXTURE_TYPE textureType = TEXTURE_TYPE::NONE;
	switch (type) {
	case ENTITY_TYPE::ZOMBIE_NORMAL:
		textureType = TEXTURE_TYPE::ZOMBIE_NORMAL;
		break;
	case ENTITY_TYPE::ZOMBIE_JAPANESE:
		textureType = TEXTURE_TYPE::ZOMBIE_JAPANESE;
		break;
	case ENTITY_TYPE::ZOMBIE_BIG:
		textureType = TEXTURE_TYPE::ZOMBIE_BIG;
		break;
	case ENTITY_TYPE::PLANT_NORMAL:
		textureType = TEXTURE_TYPE::PLANT_NORMAL;
		break;
	case ENTITY_TYPE::PLANT_SHROOM:
		textureType = TEXTURE_TYPE::PLANT_SHROOM;
		break;
	default:
		textureType = TEXTURE_TYPE::NONE;
		break;
	}

	return textures[textureType];
}

// Helping - Functions

Vector2i gridCell_imp(Vector2f pos) {
	auto [x, y] = pos;
	int r = y / cellHeight, c = x / cellWidth;
	//cout << "x = " << x << " y = " << y << "i = " << r << "j = " << c << endl;
	return { r, c };
}
Vector2i gridCell(Vector2i pos) { return gridCell_imp({ float(pos.x), float(pos.y) }); }
Vector2i gridCell(Vector2f pos) { return gridCell_imp({ float(pos.x), float(pos.y) }); }

Entity& getEntity(Vector2i gridPos) {
	// Returns only Plant or None - maybe later we can get all entites in a cell 
	Entity entity{};
	entity.category = ENTITY_CATEGORY::NONE;
	for (Entity& entity : entities) {
		//if (category != ENTITY_CATEGORY::PLANT && entity.category != category) continue; // category specified and doesn't match
		if (entity.category != ENTITY_CATEGORY::PLANT) continue;	// killed - empty cell
		if (gridCell(entity.position) != gridPos) continue;

		return entity;
	}
	return entity;
}

// Define Characters - Functions

// ground
Sprite defineGroundSprite(Texture& tex) {
	tex.loadFromFile("assets/ground.png");
	tex.setRepeated(true);
	Sprite groundSp;
	groundSp.setTexture(tex);
	IntRect groundTexRect(0, 0, 1200, 1200);
	groundSp.setTextureRect(groundTexRect);
	return groundSp;
}
// plants
Entity defineNormalPlant() {
	Texture tex;
	tex.loadFromFile("assets/Repeater.png");
	textures[TEXTURE_TYPE::PLANT_NORMAL] = tex;
	Entity plant = { ENTITY_CATEGORY::PLANT, ENTITY_TYPE::PLANT_NORMAL, .05 };
	plant.nframes = 14;
	//plant.nframes = 28;
	return plant;
}
Entity defineShroomPlant() {
	Texture tex;
	tex.loadFromFile("assets/shroom.png");
	textures[TEXTURE_TYPE::PLANT_SHROOM] = tex;
	Entity shroom = { ENTITY_CATEGORY::PLANT, ENTITY_TYPE::PLANT_SHROOM, .1, 30, 500, 0 };
	return shroom;
}
// zombies
Entity defineNormalZombie() {
	Texture tex;
	tex.loadFromFile("assets/zombie4.png");
	textures[TEXTURE_TYPE::ZOMBIE_NORMAL] = tex;
	Entity zombie = { ENTITY_CATEGORY::ZOMBIE, ENTITY_TYPE::ZOMBIE_NORMAL, .1, 20, 100, 10 };
	zombie.xvel = .05;
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
	Entity zombie = { ENTITY_CATEGORY::ZOMBIE, ENTITY_TYPE::ZOMBIE_BIG, .1, 38, 500, 30, .05 };
	return zombie;
}

// Drawing - Functions 

void UpdateSuns(RenderWindow& window) {
	for (int i = 0; i < MAX_SUNS; i++) {
		Sun& sun = suns[i];
		if (!isSunAlive(sun)) continue;

		if (sun.position.y <= sun.distance) {
			sun.position.y += 50.0f * dt;
		}

		Texture& tex = getTexture(TEXTURE_TYPE::SUN);
		sprite.setPosition(sun.position);
		sprite.setTexture(tex);
		sprite.setTextureRect(IntRect{ 0, 0, (int)tex.getSize().x, (int)tex.getSize().y });

		window.draw(sprite);
	}
}

void DrawPlantsGrid(RenderWindow& window) {
	for (Entity& plant : entities) {
		if (plant.category != ENTITY_CATEGORY::PLANT) continue;

		// Change Frame
		if (plant.frameClock.getElapsedTime().asSeconds() >= plant.frameTimer) {
			plant.frameClock.restart();
			plant.frame++, plant.frame %= plant.nframes;
		}
		sprite.setTextureRect(IntRect{
			plant.frame * cellWidth,
			0,
			cellWidth,
			cellHeight
			});
		// Draw cell character

		sprite.setPosition(plant.position);
		sprite.setTexture(getEntityTexture(plant.type));

		window.draw(sprite);
	}
}
void DrawZombies(RenderWindow& window) {
	for (Entity& zombie : entities) {
		if (zombie.category != ENTITY_CATEGORY::ZOMBIE) continue;

		// dead
		// Note: We don't remove the zombie from the list .. kept as dead for now, to avoid complications with erasing from vector while iterating
		if (zombie.health <= 0) continue;

		// animation
		if (zombie.frameClock.getElapsedTime().asSeconds() >= zombie.frameTimer) {
			zombie.frameClock.restart();
			zombie.frame++, zombie.frame %= zombie.nframes;
		}
		sprite.setTextureRect(IntRect{ zombie.frame * cellWidth, 0, cellWidth, cellHeight });

		// move
		zombie.position.x -= zombie.xvel;
		auto [r, c] = gridCell(zombie.position);
		if (zombie.position.x <= 0 + edgeSpace) {	// edge
			zombie.category = ENTITY_CATEGORY::NONE;
		} else {		// plant
			// Eat
			Entity& plant = getEntity({ r, c });
			if (plant.category == ENTITY_CATEGORY::PLANT) {
				zombie.xvel = 0;
				if (zombie.attackClock.getElapsedTime().asSeconds() >= zombie.attackTimer) {
					zombie.attackClock.restart();
					plant.health -= zombie.attack;
					if (plant.health <= 0) {
						zombie.xvel = .05;
						plant.category = ENTITY_CATEGORY::NONE;
						cout << "Plant Killed\n";
					}
				}
			}
		}

		sprite.setPosition(zombie.position);
		sprite.setTexture(getEntityTexture(zombie.type));

		window.draw(sprite);
	}
}
void CheckTimer_AddZombie(RenderWindow& window) {
	if (zombieClock.getElapsedTime().asSeconds() >= nextZombieTimer) {
		cout << "New Zombie\n";
		zombieClock.restart();
		nextZombieTimer = ZombieMinGapTime + rand() % 10;

		// get random zombie type
		int zombie_type_index = rand() % zombie_types.size();
		Entity newZombie = zombie_types[zombie_type_index];

		newZombie.position = Vector2f(windowWidth, (rand() % gridRows) * cellHeight);
		auto [r, c] = gridCell(newZombie.position);
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



int main() {
	RenderWindow window(VideoMode(windowWidth, windowHeight), "Plants vs Zombies");
	Event event;
	// Characters
	Texture groundTex;
	Sprite ground = defineGroundSprite(groundTex);

	// Plants
	Entity plant = defineNormalPlant();
	Entity shroom = defineShroomPlant();

	plant_types = { plant, shroom };

	// Zombies
	Entity zombie = defineNormalZombie();
	Entity zombieJapanese = defineJapaneseZombie();
	Entity zombieBig = defineBigZombie();

	zombie_types = { zombie, zombieJapanese, zombieBig };


	entities.push_back(plant);

	textures[TEXTURE_TYPE::SUN].loadFromFile("assets/sun.png");

	// Game Loop

	Clock dtClock;

	while (window.isOpen()) {
		sf::Time dtTime = dtClock.restart();
		dt = dtTime.asSeconds();

		while (window.pollEvent(event)) {
			if (event.type == Event::Closed || Keyboard::isKeyPressed(Keyboard::Escape)) {
				window.close();
			}
			// Events
			else if (Mouse::isButtonPressed(Mouse::Left)) {
				Vector2i pos = Mouse::getPosition(window);

				bool clickedOnSun = false;

				for (int i = 0; i < MAX_SUNS; i++) {
					Sun& sun = suns[i];
					if (!isSunAlive(sun)) continue;

					if (pos.x >= sun.position.x && pos.x <= sun.position.x + 200 && pos.y >= sun.position.y && pos.y <= sun.position.y + 200) {
						money += 25;

						cout << "You now have " << money << endl;

						suns[i].clicked = true;
						clickedOnSun = true;
						break;
					}
				}

				if (!clickedOnSun) {
					auto [i, j] = gridCell(pos);
					if (getEntity({ i , j }).category == ENTITY_CATEGORY::NONE) {
						cout << "Placing Plant at " << i << " " << j << endl;
						Entity plant;
						plant = plant_types[rand() % 2];
						int x = j * cellWidth, y = i * cellHeight;
						plant.position = Vector2f(x, y);

						entities.push_back(plant);
					}
				}
			}
		}

		CheckTimer_AddZombie(window);
		CheckTimer_AddSun(window);

		// Drawing

		window.clear(Color::Black);
		window.draw(ground);

		DrawPlantsGrid(window);
		DrawZombies(window);
		UpdateSuns(window);

		window.display();
	}

	textures.clear();

	return 0;
}
