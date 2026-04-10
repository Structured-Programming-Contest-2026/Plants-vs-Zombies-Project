#include <iostream>
#include <vector>
#define SFML_STATIC
#include <SFML/Graphics.hpp>
#include <functional>

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
	ZOMBIE_RA,

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
	ZOMBIE_RA,

	PLANT_NORMAL,
	PLANT_SHROOM,
	PLANT_SUNFLOWER,

	NormalBullet,
};
struct Animation {
	int frames = 1;
	float frameTimer = 1.0f;
	int currentFrame = 0;
	int cycles = 0;
	Clock clock;
};
struct Entity {
	ENTITY_CATEGORY category;
	ENTITY_TYPE type;
	int health = 100;
	int attack = 15;
	int price = 25;
	int n_animations=1, currentAnimationIndex=0;
	Animation animations[10];
	Animation currentAnimation;
	
	int animationForCycles = -1;

	float x_vel = 0.05f;
	Clock attackClock;
	int attackTimer = 1;
	Vector2f position;
	float scale = 1.0f;
	int gridRow = -1;
	int gridCol = -1;

	bool isDead = false;
	function<void(Entity& entity)> afterAnimation = nullptr;	// called after animation finish (if animationForCycles != -1)

	void setCurrentAnimation(int idx) {
		// You MUST change both
		currentAnimationIndex = idx;
		currentAnimation = animations[currentAnimationIndex];
	}
	void setAllAnimations(int n, Animation anim) {
		// example : all animations are the same size, time, etc .. 
		// so zombie.setAllAnimations(2, {frames, frameTimer})
		n_animations = n;
		for (int i = 0; i < n; i++) {
			animations[i] = anim;
		}
	}
	void nextFrame() {
		// next frame + check animation finish
		if (currentAnimation.clock.getElapsedTime().asSeconds() >= currentAnimation.frameTimer) {
			currentAnimation.clock.restart();
			int & f = currentAnimation.currentFrame, & cycles = currentAnimation.cycles;
			f++;
			if (f >= currentAnimation.frames) {
				cycles++;
				if (cycles >= animationForCycles && animationForCycles!=-1) {
					setCurrentAnimation(0); // reset to default
					animationForCycles = -1;
					afterAnimation(*this);
				}
			}
			f %= currentAnimation.frames;
		}
	}
	void doAnimation(int forCycles=1, int idx=0) {
		// forCycles = (-1)  ---> Infinite / Manual change
		animationForCycles = forCycles;
		setCurrentAnimation(idx);
		cout << "Doing animation " << idx << " with currentAnimationIndex=" << currentAnimationIndex << endl;
	}
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
map<TEXTURE_TYPE, Texture[10]> textures;
#define Default_Texture_Index 0

Texture& getTexture(TEXTURE_TYPE type) {
	return textures[type][Default_Texture_Index];
}
TEXTURE_TYPE getTextureType(ENTITY_TYPE type) {
	// To use textures[getTextureType(entity.type)][entity.currentAnimationIndex] on setting texture of sprites
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
	case ENTITY_TYPE:: ZOMBIE_RA:
		textureType = TEXTURE_TYPE::ZOMBIE_RA;
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
	return textureType;
}
Texture& getDefaultEntityTexture(ENTITY_TYPE type) {
	// Returns default ([0]) texture of the entity type
	TEXTURE_TYPE textureType = getTextureType(type);
	return textures[textureType][Default_Texture_Index];
}
void resizeSpriteToTex(Sprite& sprite, Texture& tex) {
	sprite.setTextureRect(IntRect{ 0, 0, (int)tex.getSize().x, (int)tex.getSize().y });
}
IntRect getFrameRect(Entity &entity) {
	// Smart Crop
	// auto frame dimensions
	Texture& tex = textures[getTextureType(entity.type)][entity.currentAnimationIndex];
	int frameWidth = tex.getSize().x / entity.currentAnimation.frames;
	int frameHeight = tex.getSize().y;

	// next frame + check animation finish
	entity.nextFrame();

	// crop current frame
	IntRect textureRect = {
		entity.currentAnimation.currentFrame * frameWidth,
		0,
		frameWidth,
		frameHeight
	};
	return textureRect;
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

		if (entity.category != ENTITY_CATEGORY::PLANT || entity.isDead) continue;

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
	
	Texture tex;
	tex.loadFromFile("assets/Repeater.png");
	textures[TEXTURE_TYPE::PLANT_NORMAL][Default_Texture_Index] = tex;
	/*tex.loadFromFile("assets/Repeater.png");
	textures[TEXTURE_TYPE::PLANT_NORMAL][1] = tex;*/
	Entity planet;
	planet.category = ENTITY_CATEGORY::PLANT;
	planet.type = ENTITY_TYPE::PLANT_NORMAL;
	planet.health = 100;
	planet.attack = 1000;
	planet.price = 25;

	// Idle animation
	planet.n_animations = 1;
	planet.animations[0].frameTimer = 0.1f;
	planet.animations[0].frames = 14;
	planet.setCurrentAnimation(0);
	return planet;
	
}
Entity defineShroomPlant() {
	Texture tex;
	tex.loadFromFile("assets/shroom.png");
	textures[TEXTURE_TYPE::PLANT_SHROOM][Default_Texture_Index] = tex;

	Entity shroom;
	shroom.category = ENTITY_CATEGORY::PLANT;
	shroom.type = ENTITY_TYPE::PLANT_SHROOM;
	shroom.health = 500;
	shroom.attack = 0;
	shroom.price = 15;
	shroom.scale = 0.9;
	// Idle animation
	shroom.n_animations = 1;
	shroom.animations[0].frameTimer = 0.1f;
	shroom.animations[0].frames = 30;
	shroom.setCurrentAnimation(0);

	return shroom;
}
Entity defineSunflower() {
	Texture tex;
	tex.loadFromFile("assets/sunflower.png");
	textures[TEXTURE_TYPE::PLANT_SUNFLOWER][Default_Texture_Index] = tex;

	Entity plant;
	plant.category = ENTITY_CATEGORY::PLANT;
	plant.type = ENTITY_TYPE::PLANT_SUNFLOWER;
	plant.health = 80;
	plant.attack = 0;
	plant.price = 50;
	plant.scale=1.2;

	// Idle animation
	plant.n_animations = 1;
	plant.animations[0].frameTimer = 0.15f;
	plant.animations[0].frames = 8;
	plant.setCurrentAnimation(0);

	return plant;
}
// zombies
Entity defineNormalZombie() {
	Texture tex;
	tex.loadFromFile("assets/zombie4.png");
	textures[TEXTURE_TYPE::ZOMBIE_NORMAL][Default_Texture_Index] = tex;
	Entity zombie = { ENTITY_CATEGORY::ZOMBIE, ENTITY_TYPE::ZOMBIE_NORMAL };
	zombie.x_vel = .05;
	zombie.scale = 1.0;
	zombie.attack = 10;
	zombie.health = 100;
	// Walk animation
	zombie.n_animations = 1;
	zombie.animations[0].frames = 20;
	zombie.animations[0].frameTimer = .1f;
	zombie.setCurrentAnimation(0);
	return zombie;
}
Entity defineJapaneseZombie() {
	Texture tex;
	tex.loadFromFile("assets/japanese.png");
	textures[TEXTURE_TYPE::ZOMBIE_JAPANESE][Default_Texture_Index] = tex;
	Entity zombie = { ENTITY_CATEGORY::ZOMBIE, ENTITY_TYPE::ZOMBIE_JAPANESE};
	zombie.health = 70;
	zombie.attack = 200;
	zombie.x_vel = .05;
	// Walk animation
	zombie.n_animations = 1;
	zombie.animations[0].frames = 20;
	zombie.animations[0].frameTimer = .01f;
	zombie.setCurrentAnimation(0);

	return zombie;
}
Entity defineBigZombie() {
	// Walk + Dying Textures
	textures[TEXTURE_TYPE::ZOMBIE_BIG][0].loadFromFile("assets/big-zombie-walking.png");;
	textures[TEXTURE_TYPE::ZOMBIE_BIG][1].loadFromFile("assets/big-zombie-dying.png");

	Entity zombie{};
	zombie.category = ENTITY_CATEGORY::ZOMBIE;
	zombie.type = ENTITY_TYPE::ZOMBIE_BIG;
	zombie.health = 100;
	zombie.attack = 30;
	zombie.x_vel = 0.05f;
	zombie.scale = 1.1f;

	// Walk + Dying animations
	zombie.n_animations = 2;
	Animation anim;
	anim.frames = 38;
	anim.frameTimer = 0.1f;
	zombie.animations[0] = anim; // walking animation
	anim.frames = 59;
	anim.frameTimer = 0.05f;
	zombie.animations[1] = anim; // dying animation
	
	zombie.setCurrentAnimation(0);

	return zombie;
}
Entity defineTorchlightZombie() {
	Texture tex;
	tex.loadFromFile("assets/torchlightzombie.png");
	textures[TEXTURE_TYPE::ZOMBIE_TORCHLIGHT][Default_Texture_Index] = tex;

	Entity zombie{};
	zombie.category = ENTITY_CATEGORY::ZOMBIE;
	zombie.type = ENTITY_TYPE::ZOMBIE_TORCHLIGHT;

	zombie.health = 200;
	zombie.attack = 20;
	zombie.x_vel = 0.05f;
	zombie.scale = 0.8f;
	// Walk animation
	zombie.n_animations = 1;
	zombie.animations[0].frameTimer = 0.1f;
	zombie.animations[0].frames = 8;
	zombie.setCurrentAnimation(0);
	return zombie;
}
Entity defineRaZombie() {
	Texture tex;
	tex.loadFromFile("assets/RaZombie.png");
	textures[TEXTURE_TYPE::ZOMBIE_RA][Default_Texture_Index] = tex;

	Entity zombie{};
	zombie.category = ENTITY_CATEGORY::ZOMBIE;
	zombie.type = ENTITY_TYPE::ZOMBIE_RA;
	zombie.health = 200;
	zombie.attack = 20;
	zombie.x_vel = 0.05;
	// Walk animation
	zombie.n_animations = 1;
	zombie.animations[0].frameTimer = 0.11f;
	zombie.animations[0].frames = 20;
	zombie.setCurrentAnimation(0);
	return zombie;
}
//bullets
Entity defineNormalBullet() {
	Texture tex;
	tex.loadFromFile("assets/bullet.png");
	textures[TEXTURE_TYPE::NormalBullet][Default_Texture_Index] = tex;

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

		// Animation
		plant.nextFrame(); // next frame
		plantSprite.setTextureRect(getFrameRect(plant)); // crop texture

		// plant attacking system
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
			// Sun Flower
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
		plantSprite.setTexture(textures[getTextureType(plant.type)][plant.currentAnimationIndex]);
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

		// dead		// NOTE: Currently not showing where [isDead==true] not [health <= 0] because of the dying animation, but we might change it later
		if (zombie.health <= 0) {
			if (!zombie.isDead && zombie.currentAnimationIndex == 0) { // health done but not started dying animation yet
				
				zombie.doAnimation(1, 1); // play dying animation once
				zombie.afterAnimation = [&zombie](Entity& entity) {
					zombie.isDead = true;	// TODO: maybe make isDead and health<=0 in another way?
					entity.category = ENTITY_CATEGORY::NONE; // remove after animation
					};
				zombie.x_vel = 0;
			}
			//else if(!zombie.isDead){
			//	// health done and dying animation started but not finished yet, so we do nothing and wait for the animation to finish
			//}
			else if(zombie.isDead){
				// health done and animation finished, so we remove the zombie
				zombie.category = ENTITY_CATEGORY::NONE;
				continue;
			}
		}
		
		// Animation
		zombie.nextFrame(); // next frame
		IntRect frameRect = getFrameRect(zombie);
		zombieSprite.setTextureRect(frameRect); // crop texture
		
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
						plant.isDead = true;
						cout << "Plant Killed\n";
					}
				}
			}
			else if(zombie.health > 0) {
				zombie.x_vel = .05; // no plant - has health - move normally edit_bug
			}
		}
		
		int frameHeight = frameRect.getSize().y;
		float actualHeight = frameHeight * zombie.scale;
		Vector2f adjustedPosition = zombie.position;
		adjustedPosition.y -= (actualHeight - cellHeight);

		zombieSprite.setPosition(adjustedPosition);
		if(zombie.currentAnimationIndex !=0)
			printf("text type %i -- idx %i\n", getTextureType(zombie.type), zombie.currentAnimationIndex);
		zombieSprite.setTexture(textures[getTextureType(zombie.type)][zombie.currentAnimationIndex]);

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

			if (zombie.category != ENTITY_CATEGORY::ZOMBIE || zombie.isDead) continue;// skip non-zombie or dead ones

			if (abs(bullet.position.y - zombie.position.y) < 100.0f && // same row
				bullet.position.x >= zombie.position.x &&
				bullet.position.x <= zombie.position.x + 100.0f) // almost touched
			{
				zombie.health -= bullet.attack;
				bullet.category = ENTITY_CATEGORY::NONE;

				cout << "Zombie hit! Health left: " << zombie.health << endl; //console just testing


				if (zombie.isDead) {
					zombie.category = ENTITY_CATEGORY::NONE;
					cout << "Zombie Killed!" << endl;
				}

				break;
			}
		}

		if (bullet.category == ENTITY_CATEGORY::Bullet) {
			Texture& bulletTex = getDefaultEntityTexture(bullet.type);
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

			//Smart Crop [ Each Seed Packets ? ]
			Texture& tex = getDefaultEntityTexture(plant.type);
			int frameWidth = tex.getSize().x / plant.currentAnimation.frames;
			int frameHeight = tex.getSize().y / plant.n_animations;

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
	Entity zombieRa = defineRaZombie();

	zombie_types = { zombie, zombieJapanese, zombieBig ,zombieTorch, zombieRa };

	// Other Textures
	textures[TEXTURE_TYPE::SUN][Default_Texture_Index].loadFromFile("assets/Sun3.png");
	textures[TEXTURE_TYPE::SEEDBANK][Default_Texture_Index].loadFromFile("assets/seedbank.png");
	textures[TEXTURE_TYPE::SEEDPACKET][Default_Texture_Index].loadFromFile("assets/seedpacket.png");

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

							Texture& tex = getDefaultEntityTexture(plant.type);
							int plantWidth = (tex.getSize().x / plant.currentAnimation.frames) * plant.scale;
							int plantHeight = (tex.getSize().y / plant.n_animations) * plant.scale;

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
