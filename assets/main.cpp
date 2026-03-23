#include <iostream>
#include <vector>
#define SFML_STATIC
#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

// Global Variables
const int
gridRows = 5,
gridCols = 5,
cellWidth = 200,
cellHeight = 200,
windowWidth = gridCols * cellWidth,
windowHeight = gridRows * cellHeight;
const int edgeSpace = -100;		// when zombie reaches this space, It disappears (end of screen) - should end game later

// Plants + Plant Grid
struct Plant {
	Sprite sprite;
	string type = "";
	float frameTimer = 1;
	int	nframes = 6;
	int health = 100;
	int attack = 0;
	int frame = 0;
	Clock frameClock;
};
Plant grid[gridRows][gridCols] = {};
vector<Plant> plant_types;

// Zombies + Zombie List + Zombie Types	- we can have multiple zombies at a time, so we will keep them in a list
struct Zombie {
	Sprite sprite;
	string type = "normal";
	float frameTimer = 1;
	int	nframes = 6;
	int health = 100;
	int attack = 20;
	float xvel = 0.5;
	int frame = 0;
	Clock frameClock;
	Clock attackClock;
	int attackTimer = 1;
};
vector<Zombie> zombies, zombie_types;
// Zombie Clock - to add new zombie after some time interval
Clock zombieClock;
float nextZombieTimer = 1;
float ZombieMinGapTime = 1;

// Helping - Functions

Vector2i gridCell_imp(Vector2f pos) {
	auto [x, y] = pos;
	int r = y / cellHeight, c = x / cellWidth;
	//cout << "x = " << x << " y = " << y << "i = " << r << "j = " << c << endl;
	return { r, c };
}
Vector2i gridCell(Vector2i pos) { return gridCell_imp({ float(pos.x), float(pos.y) }); }
Vector2i gridCell(Vector2f pos) { return gridCell_imp({ float(pos.x), float(pos.y) }); }

// Define Characters - Functions

// ground
Sprite defineGroundSprite(Texture &groundTexture) {
	//cout<<(groundTexture.loadFromFile("src/ground.png")? "loaded texture\n":"not loaded\n");
	groundTexture.loadFromFile("src/ground.png");
	groundTexture.setRepeated(true);
	Sprite groundSp;
	groundSp.setTexture(groundTexture);
	IntRect groundTexRect(0, 0, 1200, 1200);
	groundSp.setTextureRect(groundTexRect);
	return groundSp;
}
// plants
Plant defineNormalPlant(Texture &plantTexture) {
	plantTexture.loadFromFile("src/plant.png");
	Plant plant = { Sprite(plantTexture), "plant", .05, 28};
	return plant;
}
Plant defineShroomPlant(Texture &shroomTexture) {
	shroomTexture.loadFromFile("src/shroom.png");
	Plant shroom = { Sprite(shroomTexture), "shroom", .1, 30, 500, 0 };
	return shroom;
}
// zombies
Zombie defineNormalZombie(Texture &zombieTex) {
	zombieTex.loadFromFile("src/zombie4.png");
	Zombie zombie = { Sprite(zombieTex), "normal", .1, 20, 100, 10};
	zombie.xvel = .05;
	return zombie;
}
Zombie defineJapaneseZombie(Texture &zombieTex) {
	zombieTex.loadFromFile("src/japanese.png");
	Zombie zombie = { Sprite(zombieTex), "japanese", .01, 20, 200, 70, .05};
	return zombie;
}
Zombie defineBigZombie(Texture& zombieTex) {
	zombieTex.loadFromFile("src/big-zombie.png");
	Zombie zombie = { Sprite(zombieTex), "big", .1, 38, 500, 30, .05 };
	return zombie;
}

// Drawing - Functions 

void DrawPlantsGrid(RenderWindow& window) {
	for(int i=gridRows-1; i>=0; i--) {
		for(int j=gridCols-1; j>=0; j--) {
			Plant &plant = grid[i][j];
			
			// dead
			if (plant.health <= 0) plant = Plant{};
			// empty
			if (plant.type.empty()) continue;	
			// Change Frame
			if (plant.frameClock.getElapsedTime().asSeconds() >= plant.frameTimer) {
				plant.frameClock.restart();
				plant.frame++, plant.frame %= plant.nframes;
				plant.sprite.setTextureRect(IntRect{
					plant.frame * cellWidth,
					0,
					cellWidth,
					cellHeight 
				});
			}
			// Draw cell character
			window.draw(plant.sprite);
		}
	}
}
void DrawZombies(RenderWindow& window) {
	for (auto& zombie : zombies) {

		// dead
		// Note: We don't remove the zombie from the list .. kept as dead for now, to avoid complications with erasing from vector while iterating
		if (zombie.health <= 0) continue;

		// animation
		if (zombie.frameClock.getElapsedTime().asSeconds() >= zombie.frameTimer) {
			zombie.frameClock.restart();
			zombie.frame++, zombie.frame %= zombie.nframes;
			zombie.sprite.setTextureRect(IntRect{ zombie.frame * cellWidth, 0, cellWidth, cellHeight });
		}

		// move
		zombie.sprite.move(-zombie.xvel, 0);
		auto [r, c] = gridCell(zombie.sprite.getPosition());
		if (zombie.sprite.getPosition().x <= 0 + edgeSpace) {	// edge
			zombie.health = 0;
		}
		else if (!grid[r][c].type.empty()) {		// plant
			// Eat
			zombie.xvel = 0;
			Plant& plant = grid[r][c];
			if (zombie.attackClock.getElapsedTime().asSeconds() >= zombie.attackTimer) {
				zombie.attackClock.restart();
				plant.health -= zombie.attack;
			}
		}

		// Move After Killing Plant
		if (grid[r][c].type.empty() && zombie.xvel == 0) {
			zombie.xvel = .05;
			cout << "Plant Killed\n";
		}

		window.draw(zombie.sprite);
	}
}
void CheckTimer_AddZombie(RenderWindow& window) {
	if (zombieClock.getElapsedTime().asSeconds() >= nextZombieTimer) {
		cout << "New Zombie\n";
		zombieClock.restart();
		nextZombieTimer = ZombieMinGapTime + rand() % 10;
		
		// get random zombie type
		int zombie_type_index = rand() % zombie_types.size();
		Zombie newZombie = zombie_types[zombie_type_index];	

		newZombie.sprite.setPosition(windowWidth, (rand() % gridRows) * cellHeight);
		newZombie.sprite.setTextureRect(IntRect{ 0, 0, cellWidth, cellHeight });
		zombies.push_back(newZombie);
	}
}



int main() {
	RenderWindow window(VideoMode(1000, 1000), "SFML works!");
	Event event;
	// Characters
	Texture groundTexture;
	Sprite ground = defineGroundSprite(groundTexture);

	// Plants
	Texture  plantTexture, shroomTexture;
	Plant plant = defineNormalPlant(plantTexture);
	Plant shroom = defineShroomPlant(shroomTexture);

	plant_types = { plant, shroom };

	// Zombies
	Texture zombieTexture, japaneseTexture, bigTexture;

	Zombie zombie = defineNormalZombie(zombieTexture);
	Zombie zombieJapanese = defineJapaneseZombie(japaneseTexture);
	Zombie zombieBig = defineBigZombie(bigTexture);
	
	zombie_types = { zombie, zombieJapanese, zombieBig };


	grid[0][0] = plant;

	// Game Loop

	while(window.isOpen()) {
		while (window.pollEvent(event)) {
			if (event.type == Event::Closed || Keyboard::isKeyPressed(Keyboard::Escape)) {
				window.close();
			}
			// Events
			else if (Mouse::isButtonPressed(Mouse::Left)) {
				auto [i, j] = gridCell(Mouse::getPosition(window));
				if (grid[i][j].type.empty()) {
					grid[i][j] = plant_types[rand()%2];
					int x = j * cellWidth, y = i * cellHeight;
					grid[i][j].sprite.setPosition(x, y);
				}
			}
		}
		
		// New Zombie after Timer 
		CheckTimer_AddZombie(window);

		// Drawing

		window.clear(Color::Black);
		window.draw(ground);
		
		DrawPlantsGrid(window);
		DrawZombies(window);

		window.display();

		// Remove dead zombies from list
		zombies.erase(remove_if(zombies.begin(), zombies.end(), [](Zombie& z) { return z.health <= 0; }), zombies.end());
	}
	return 0;
}
