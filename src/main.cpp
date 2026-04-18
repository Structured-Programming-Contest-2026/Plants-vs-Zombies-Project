#include <iostream>
#include <vector>
#define SFML_STATIC
#include <SFML/Graphics.hpp>
#include <functional>
#include <cmath>

using namespace std;
using namespace sf;

#define gridraws 5
#define gridcols 9
#define cellwidth 150
#define cellheight 170
#define offsetX 470
#define offsetY 100
#define edgespace -100
#define z_eat_p_distance 100

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
windowWidth = 1920,
windowHeight = 1080;
const int edgeSpace = edgespace; // when zombie reaches this space, It disappears (end of screen) - should end game later
const int eatDistance = z_eat_p_distance;
int nextEntityId = 0; // for ra
bool isGameOver, isGameWon;
RectangleShape gameOverScreen, gameWonScreen;
Sprite ground;
int zombieCount = 0, createdZombiesCount = 0;

enum class ENTITY_CATEGORY : uint8_t {
	NONE = 0,
	ZOMBIE,
	PLANT,
	Bullet,
};


enum class ENTITY_TYPE : uint8_t {
	NONE = 0,
	ZOMBIE_NORMAL,
	ZOMBIE_JAPANESE,
	ZOMBIE_BIG,
	ZOMBIE_TORCHLIGHT,
	ZOMBIE_RA,
	ZOMBIE_MUMMY,
	ZOMBIE_CONEHEAD,

	PLANT_NORMAL,
	PLANT_SHROOM,
	PLANT_SUNFLOWER,
	PLANT_WALLNUT,
	PLANT_CHOMPER,

	NormalBullet,
};

enum class TEXTURE_TYPE : uint8_t {
	NONE = 0,
	LOGO,
	LEVEL_SELECTOR_BACKGROUND,
	LEVEL_OPTION_BACKGROUND,
	LEVEL_BACKGROUND,
	LEVEL_PLAY_BUTTON,
	SEED_CHOOSER,
	SEED_CHOOSER_BUTTON,
	SUN,
	SEEDBANK,
	SEEDPACKET,

	ZOMBIE_NORMAL,
	ZOMBIE_JAPANESE,
	ZOMBIE_BIG,
	ZOMBIE_TORCHLIGHT,
	ZOMBIE_RA,
	ZOMBIE_MUMMY,
	ZOMBIE_CONEHEAD,

	PLANT_NORMAL,
	PLANT_SHROOM,
	PLANT_SUNFLOWER,
	PLANT_WALLNUT,
	PLANT_CHOMPER,

	NormalBullet,
};

// Textures 1
map<TEXTURE_TYPE, Texture[10]> textures;
#define Default_Texture_Index 0
Texture& getTexture(TEXTURE_TYPE type, int animation_index = Default_Texture_Index) {
	return textures[type][animation_index];
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
	case ENTITY_TYPE::ZOMBIE_RA:
		textureType = TEXTURE_TYPE::ZOMBIE_RA;
		break;
	case ENTITY_TYPE::ZOMBIE_MUMMY:
		textureType = TEXTURE_TYPE::ZOMBIE_MUMMY;
		break;
	case ENTITY_TYPE::ZOMBIE_CONEHEAD:
		textureType = TEXTURE_TYPE::ZOMBIE_CONEHEAD;
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
	case ENTITY_TYPE::PLANT_WALLNUT:
		textureType = TEXTURE_TYPE::PLANT_WALLNUT;
		break;
	case ENTITY_TYPE::PLANT_CHOMPER:
		textureType = TEXTURE_TYPE::PLANT_CHOMPER;
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

	int num_animations = 1,
		currentAnimationIndex = 0;
	Animation animations[10];
	Animation currentAnimation;

	int AnimationForCycles = -1;

	float x_vel = 0.05f;
	Clock attackClock;
	int attackTimer = 1;
	Vector2f position;
	int width;
	int height;
	float scale = 1.0f;
	int gridRow = -1;
	int gridCol = -1;

	bool isUsingAbility = false; // for ra
	Clock abilityClock; // for ra
	float abilityDuration = 5.0f; // for ra

	int storedSunValue = 0; // for ra
	int maxSunValue = 350; // for ra
	int id = -1; // ra test

	bool isDead = false;
	function<void(Entity& entity)> afterAnimation = [](Entity&) {};	// called after animation finish (if animationForCycles != -1)

	void updateDimensions() {
		Texture tex = getTexture(getTextureType(type), currentAnimationIndex);
		int w = (tex.getSize().x / currentAnimation.frames) * scale;
		int h = tex.getSize().y * scale;

		width = w, height=h;
	}
	void setCurrentAnimation(int idx) {
		// You MUST change both
		currentAnimationIndex = idx;
		currentAnimation = animations[currentAnimationIndex];
		updateDimensions();
	}
	void setAllAnimations(int num, Animation anim) {
		// example : all animations are the same size, time, etc .. 
		// so zombie.setAllAnimations(2, {frames, frameTimer})
		num_animations = num;
		for (int i = 0; i < num; i++) {
			animations[i] = anim;
		}
		updateDimensions();
	}
	void nextFrame() {
		// next frame + check animation finish
		if (currentAnimation.clock.getElapsedTime().asSeconds() >= currentAnimation.frameTimer) {
			currentAnimation.clock.restart();
			int& frame = currentAnimation.currentFrame, & cycles = currentAnimation.cycles;
			frame++;
			if (frame >= currentAnimation.frames) {
				cycles++;
				if (cycles >= AnimationForCycles && AnimationForCycles != -1) {
					setCurrentAnimation(0); // reset to default
					AnimationForCycles = -1;
					afterAnimation(*this); //Not Understood
				}
			}
			frame %= currentAnimation.frames;
		}
	}
	void DoAnimation(int forCycles = 1, int idx = 0) {
		// forCycles = (-1)  ---> Infinite / Manual change
		AnimationForCycles = forCycles;
		setCurrentAnimation(idx);
		updateDimensions();
		cout << "Doing animation " << idx << " with currentAnimationIndex=" << currentAnimationIndex << endl;
	}
};

// TODO: this will later include stuff like difficulty, grid size, ...
struct Level {
	vector<Entity> zombie_types;
	vector<Entity> plant_types;
	int startingSuns = 300;
	string bgPath = "assets/background.png";
	bool arePlantsSelected = false;
	vector<Entity> selectedPlants;
	int lvlNum = 0;
	int targetZombie = 20,	// total zombies in a level 
		waveLimit = 10,		// a wave has up to (waveLimit) zombies
		waveTimer = 15,		// between waves
		waveLimitChange = 5;	// each wave has more zombies than previous by (waveLimitChange)
};
int number_levels = 6;
vector<Level> levels(number_levels + 1);
Level* currentLevel = nullptr;


// Zombie Clock - to add new zombie after some time interval
Clock zombieClock, waveClockX;
//Sending Zombies
float nextZombieTimer = 5; // First zombie timer - it's assigned, current value is only used for FIRST ZOMBIE
float ZombieMinGapTime = 7,
ZombieMaxGapTime = ZombieMinGapTime + 3,
ZombieGap_dt = 0.1;	// Level Pace

//Independet Sprites

Sprite plantSprite;
Sprite zombieSprite;
Sprite bulletSprite;
Sprite uiSprite;

// TODO: remove vectors and maps and replace them with arrays :)

vector<Entity> entities;
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

	bool targeted = false; // for ra
	int ownerIndex = -1; // for ra
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

// Textures 2
void resizeSpriteToTex(Sprite& sprite, Texture& tex) {
	sprite.setTextureRect(IntRect{ 0, 0, (int)tex.getSize().x, (int)tex.getSize().y });
}
IntRect getFrameRect(Entity& entity) {
	// Smart Crop
	// auto frame dimensions
	Texture& tex = textures[getTextureType(entity.type)][entity.currentAnimationIndex]; //must be num%10=0 if not resize it
	float frameWidthF = float(tex.getSize().x) / float(entity.currentAnimation.frames);
	int frameWidth = round(frameWidthF);
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

bool anySunAlive() { // for ra
	for (int i = 0; i < MAX_SUNS; i++) {
		if (isSunAlive(suns[i]))
			return true;
	}
	return false;
}

Vector2i gridCell_imp(Vector2f pos) {
	int column = (pos.x - offsetX) / cellWidth;
	int raw = (pos.y - offsetY) / cellHeight;
	return { raw, column };
}

Vector2i gridCell(Vector2i pos) { return gridCell_imp({ float(pos.x), float(pos.y) }); }
Vector2i gridCell(Vector2f pos) { return gridCell_imp({ float(pos.x), float(pos.y) }); }

Entity& getEntity(Vector2i gridPos) {

	static Entity emptyEntity{};
	emptyEntity.category = ENTITY_CATEGORY::NONE;

	for (Entity& entity : entities) {

		if (entity.category != ENTITY_CATEGORY::PLANT || entity.isDead) continue;

		if (entity.gridRow == gridPos.x && entity.gridCol == gridPos.y) {
			return entity;
		}
	}
	return emptyEntity;
}

// Define Characters - Functions

// ground
Sprite defineGroundSprite(Texture& tex) {
	if (currentLevel != nullptr) {
		// this doesnt run .. cuz its run only once, beginning of main, before even textures is loaded
		tex.loadFromFile(currentLevel->bgPath);
		cout << "trying to load " << currentLevel->bgPath << endl;
		if (!tex.loadFromFile(currentLevel->bgPath)) {
			cout << "Error: Could not load assets/background.png" << endl;
		}
	}
	else { 
		tex.loadFromFile("assets/background.png");
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
Entity defineNormalPlant() {

	Texture tex;
	tex.loadFromFile("assets/Peashooter.png");
	textures[TEXTURE_TYPE::PLANT_NORMAL][Default_Texture_Index] = tex;
	/*tex.loadFromFile("assets/Repeater.png");
	textures[TEXTURE_TYPE::PLANT_NORMAL][1] = tex;*/
	Entity planet;
	planet.category = ENTITY_CATEGORY::PLANT;
	planet.type = ENTITY_TYPE::PLANT_NORMAL;
	planet.health = 100;
	planet.attack = 1000;
	planet.price = 25;
	planet.scale = 0.75;

	// Idle animation
	planet.num_animations = 1;
	planet.animations[0].frameTimer = 0.1f;
	planet.animations[0].frames = 13;
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
	shroom.num_animations = 1;
	shroom.animations[0].frameTimer = 0.1f;
	shroom.animations[0].frames = 30;
	shroom.setCurrentAnimation(0);

	return shroom;
}
Entity defineSunflower() {
	Texture tex;
	tex.loadFromFile("assets/sunflower.png");
	textures[TEXTURE_TYPE::PLANT_SUNFLOWER][Default_Texture_Index] = tex;
	textures[TEXTURE_TYPE::PLANT_SUNFLOWER][2].loadFromFile("assets/sunflower-produce.png");

	Entity plant;
	plant.category = ENTITY_CATEGORY::PLANT;
	plant.type = ENTITY_TYPE::PLANT_SUNFLOWER;
	plant.health = 80;
	plant.attack = 0;
	plant.price = 50;
	plant.scale = 1.2;

	plant.num_animations = 3;
	// Idle animation
	plant.animations[0].frameTimer = 0.15f;
	plant.animations[0].frames = 8;
	// Produce sun animation
	plant.animations[2].frameTimer = 0.018f;
	plant.animations[2].frames = 60;
	plant.setCurrentAnimation(0);

	return plant;
}
// zombies
Entity defineNormalZombie() {
	textures[TEXTURE_TYPE::ZOMBIE_NORMAL][0].loadFromFile("assets/NormalZombie.png");
	textures[TEXTURE_TYPE::ZOMBIE_NORMAL][1].loadFromFile("assets/NormalZombieDeath.png");
	Entity zombie = { ENTITY_CATEGORY::ZOMBIE, ENTITY_TYPE::ZOMBIE_NORMAL };
	zombie.x_vel = .05;
	zombie.scale = 1.0;
	zombie.attack = 10;
	zombie.health = 100;
	zombie.num_animations = 2;
	Animation anim;
	// Walk animation
	anim.frames = 22;
	anim.frameTimer = 0.07f;
	zombie.animations[0] = anim;
	// Death animation
	anim.frames = 10;
	anim.frameTimer = 0.1f;
	zombie.animations[1] = anim;
	zombie.setCurrentAnimation(0);
	return zombie;
}
Entity defineConeheadZombie() {
	textures[TEXTURE_TYPE::ZOMBIE_CONEHEAD][0].loadFromFile("assets/ConeheadZombie.png");
	textures[TEXTURE_TYPE::ZOMBIE_CONEHEAD][1].loadFromFile("assets/NormalZombieDeath.png");
	textures[TEXTURE_TYPE::ZOMBIE_CONEHEAD][2].loadFromFile("assets/NormalZombie.png");
	Entity zombie = { ENTITY_CATEGORY::ZOMBIE, ENTITY_TYPE::ZOMBIE_CONEHEAD };
	zombie.x_vel = .05;
	zombie.scale = 1.0;
	zombie.attack = 10;
	zombie.health = 300;
	zombie.num_animations = 2;
	Animation anim;
	// Walk animation
	anim.frames = 21;
	anim.frameTimer = 0.07f;
	zombie.animations[0] = anim;
	// Death animation
	anim.frames = 10;
	anim.frameTimer = 0.1f;
	zombie.animations[1] = anim;
	// No cone walk animation
	anim.frames = 22;
	anim.frameTimer = 0.07f;
	zombie.animations[2] = anim;
	zombie.setCurrentAnimation(0);
	return zombie;
}
Entity defineJapaneseZombie() {
	Texture tex;
	tex.loadFromFile("assets/japanese.png");
	textures[TEXTURE_TYPE::ZOMBIE_JAPANESE][Default_Texture_Index] = tex;
	Entity zombie = { ENTITY_CATEGORY::ZOMBIE, ENTITY_TYPE::ZOMBIE_JAPANESE };
	zombie.health = 70;
	zombie.attack = 50;
	zombie.x_vel = .05;
	// Walk animation
	zombie.num_animations = 1;
	zombie.animations[0].frames = 20;
	zombie.animations[0].frameTimer = .01f;
	zombie.setCurrentAnimation(0);

	return zombie;
}
Entity defineBigZombie() {
	// Walk + Dying Textures
	textures[TEXTURE_TYPE::ZOMBIE_BIG][0].loadFromFile("assets/big-zombie-walking.png");
	textures[TEXTURE_TYPE::ZOMBIE_BIG][1].loadFromFile("assets/big-zombie-dying.png");

	Entity zombie{};
	zombie.category = ENTITY_CATEGORY::ZOMBIE;
	zombie.type = ENTITY_TYPE::ZOMBIE_BIG;
	zombie.health = 100;
	zombie.attack = 30;
	zombie.x_vel = 0.05f;
	zombie.scale = 1.1f;

	// Walk + Dying animations
	zombie.num_animations = 2;
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

// ÏÇáÉ áÊÚÑíÝ ÇáÜ Wall-nut
Entity defineWallnut() {
	Entity wallnut{};
	wallnut.category = ENTITY_CATEGORY::PLANT;
	wallnut.type = ENTITY_TYPE::PLANT_WALLNUT;
	//wallnut.health = 40000;
	wallnut.health = 400;
	wallnut.price = 50;
	wallnut.scale = 2.0f;


	textures[TEXTURE_TYPE::PLANT_WALLNUT][0].loadFromFile("assets/wallnut.png");
	textures[TEXTURE_TYPE::PLANT_WALLNUT][0].setSmooth(false);
	textures[TEXTURE_TYPE::PLANT_WALLNUT][1].loadFromFile("assets/crackedwallnut.png");
	textures[TEXTURE_TYPE::PLANT_WALLNUT][1].setSmooth(false);
	textures[TEXTURE_TYPE::PLANT_WALLNUT][2].loadFromFile("assets/DamagedWallnut.png");
	textures[TEXTURE_TYPE::PLANT_WALLNUT][2].setSmooth(false);

	wallnut.num_animations = 3;
	Animation anim;
	anim.frames = 8;
	anim.frameTimer = 0.1f;

	wallnut.animations[0] = anim; //Full
	wallnut.animations[1] = anim; //half
	wallnut.animations[2] = anim; //Dying

	wallnut.setCurrentAnimation(0);

	return wallnut;
}

// Chomper Plant
Entity defineChomperPlant() {
	Entity Chomper;
	Chomper.category = ENTITY_CATEGORY::PLANT;
	Chomper.type = ENTITY_TYPE::PLANT_CHOMPER;
	Chomper.health = 200;
	Chomper.price = 150;
	Chomper.scale = 0.85;

	textures[TEXTURE_TYPE::PLANT_CHOMPER][0].loadFromFile("assets/Chomper.png"); // IDLE
	textures[TEXTURE_TYPE::PLANT_CHOMPER][0].setSmooth(false);
	textures[TEXTURE_TYPE::PLANT_CHOMPER][1].loadFromFile("assets/ChomperAttack.png"); // ATTACK
	textures[TEXTURE_TYPE::PLANT_CHOMPER][1].setSmooth(false);
	textures[TEXTURE_TYPE::PLANT_CHOMPER][2].loadFromFile("assets/ChomperDigest.png");
	textures[TEXTURE_TYPE::PLANT_CHOMPER][2].setSmooth(false);

	Chomper.num_animations = 3;
	Animation a;
	a.frames = 13;
	a.frameTimer = 0.1f;
	Chomper.animations[0] = a; // IDLE Animation
	a.frames = 9;
	a.frameTimer = 0.1f;
	Chomper.animations[1] = a; // ATTACK Animation
	a.frames = 6;
	a.frameTimer = 0.1f;
	Chomper.animations[2] = a; // DIGEST Animation

	Chomper.setCurrentAnimation(0);

	return Chomper;
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
	zombie.num_animations = 1;
	zombie.animations[0].frameTimer = 0.1f;
	zombie.animations[0].frames = 8;
	zombie.setCurrentAnimation(0);
	return zombie;
}
Entity defineRaZombie() {

	textures[TEXTURE_TYPE::ZOMBIE_RA][0].loadFromFile("assets/RaZombie.png");
	textures[TEXTURE_TYPE::ZOMBIE_RA][1].loadFromFile("assets/RaZombieDeath.png");
	textures[TEXTURE_TYPE::ZOMBIE_RA][2].loadFromFile("assets/RaZombieAbility.png");

	Entity zombie{};
	zombie.category = ENTITY_CATEGORY::ZOMBIE;
	zombie.type = ENTITY_TYPE::ZOMBIE_RA;
	zombie.health = 200;
	zombie.attack = 20;
	zombie.x_vel = 0.05;
	// Animations
	zombie.num_animations = 3;
	Animation anim;
	// walk
	anim.frames = 20;
	anim.frameTimer = 0.11f;
	zombie.animations[0] = anim;
	// death
	anim.frames = 65;
	anim.frameTimer = 0.05f;
	zombie.animations[1] = anim;
	// ability
	anim.frames = 30;
	anim.frameTimer = 0.08f;
	zombie.animations[2] = anim;

	zombie.setCurrentAnimation(0);

	return zombie;
}

Entity defineMummyZombie() {
	textures[TEXTURE_TYPE::ZOMBIE_MUMMY][0].loadFromFile("assets/MummyZombie.png");
	textures[TEXTURE_TYPE::ZOMBIE_MUMMY][1].loadFromFile("assets/MummyZombieDeath.png");
	textures[TEXTURE_TYPE::ZOMBIE_MUMMY][2].loadFromFile("assets/MummyZombieEating.png");

	Entity zombie{};
	zombie.category = ENTITY_CATEGORY::ZOMBIE;
	zombie.type = ENTITY_TYPE::ZOMBIE_MUMMY;
	zombie.health = 100;
	zombie.attack = 10;
	zombie.x_vel = 0.05;
	// Animations
	zombie.num_animations = 2;
	Animation anim;
	// walk
	anim.frames = 30;
	anim.frameTimer = 0.07f;
	zombie.animations[0] = anim;
	// death
	anim.frames = 55;
	anim.frameTimer = 0.05f;
	zombie.animations[1] = anim;
	// eat
	anim.frames = 54;
	anim.frameTimer = 0.05f;
	zombie.animations[2] = anim;

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

		if (sun.position.y <= sun.distance && !sun.targeted) {
			sun.position.y += 50.0f * delta_time;
		}

		Texture& tex = getTexture(TEXTURE_TYPE::SUN);
		uiSprite.setPosition(sun.position);
		uiSprite.setTexture(tex);
		uiSprite.setTextureRect(IntRect{ 0, 0, (int)tex.getSize().x, (int)tex.getSize().y });

		if (sun.targeted) { // for ra
			uiSprite.setColor(Color(255, 80, 80));
		} else {
			uiSprite.setColor(Color(255, 255, 255));
		}

		window.draw(uiSprite);
	}
	uiSprite.setColor(Color(255, 255, 255)); // for ra
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

		if (plant.type == ENTITY_TYPE::PLANT_WALLNUT) {
			int CurrentState = 0;
			if (plant.health <= 250) {
				CurrentState = 1;
			}
			if (plant.health <= 125) {
				CurrentState = 2;
			}

			//change state
			if (plant.currentAnimationIndex != CurrentState) {
				plant.setCurrentAnimation(CurrentState);
				//cout << "Wallnut state changed to: " << CurrentState << " (Health: " << plant.health << ")\n";
			}
		} else if (plant.type == ENTITY_TYPE::PLANT_CHOMPER) {
			if (plant.currentAnimationIndex == 0) { // IDLE
				for (Entity& zombie : entities) {
					if (zombie.category != ENTITY_CATEGORY::ZOMBIE || zombie.health <= 0) continue;

					bool sameRow = abs(zombie.position.y - plant.position.y) < 100.0f;
					bool inFront = zombie.position.x > plant.position.x &&
						zombie.position.x < plant.position.x + plant.width * 0.5;	
					// NOTE: Distance from zombie depends on (Texture) width * scale

					if ((sameRow && inFront) && zombie.type != ENTITY_TYPE::ZOMBIE_BIG) {
						zombie.health = 0;
						zombie.category = ENTITY_CATEGORY::NONE;
						plant.attackClock.restart().asSeconds();

						plant.DoAnimation(1, 1);
						plant.afterAnimation = [](Entity& entity) {
							entity.DoAnimation(20, 2);
							entity.afterAnimation = [](Entity& entity) {
								entity.DoAnimation(-1, 0);
								};
							};
						break;
					}
				}
			}
		}

		// Animation
		plant.nextFrame(); // next frame
		plantSprite.setTextureRect(getFrameRect(plant)); // crop texture

		// plant attacking system
		if ((plant.type != ENTITY_TYPE::PLANT_SUNFLOWER)
			&& (plant.type != ENTITY_TYPE::PLANT_WALLNUT)
			&& (plant.type != ENTITY_TYPE::PLANT_CHOMPER)) {

			if (plant.attackClock.getElapsedTime().asSeconds() >= 3.0f) {

				bool zombieInLane = false;
				for (Entity& zombie : entities) {
					if (zombie.category == ENTITY_CATEGORY::ZOMBIE && zombie.health > 0) {

						if (abs(zombie.position.y - plant.position.y) < 100.0f &&
							zombie.position.x > plant.position.x &&
							zombie.position.x < windowWidth - 50.0f) {
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
		} else if (plant.type == ENTITY_TYPE::PLANT_SUNFLOWER) {
			// Sun Flower
			if (plant.attackClock.getElapsedTime().asSeconds() >= 10.0f) {
				plant.attackClock.restart();
				plant.DoAnimation(1, 2); // produce sun animation

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
			if (!zombie.isDead && (zombie.currentAnimationIndex == 0 || (zombie.type == ENTITY_TYPE::ZOMBIE_CONEHEAD && zombie.currentAnimationIndex == 2) || (zombie.type == ENTITY_TYPE::ZOMBIE_RA && zombie.isUsingAbility))) { // health done but not started dying animation yet
				
				if (zombie.type == ENTITY_TYPE::ZOMBIE_RA && zombie.isUsingAbility) { // for ra
					for (int i = 0; i < MAX_SUNS; i++) {
						if (suns[i].ownerIndex == zombie.id) {
							suns[i].targeted = false;
							suns[i].ownerIndex = -1;
						}
					}
					zombie.isUsingAbility = false;
				}

				zombie.DoAnimation(1, 1); // play dying animation once
				if (zombie.type == ENTITY_TYPE::ZOMBIE_RA) // for ra
					zombie.scale = 1.3f;
				zombie.afterAnimation = [&zombie](Entity& entity) {

					if (zombie.type == ENTITY_TYPE::ZOMBIE_RA && zombie.storedSunValue > 0) { // for ra
						int sunsToDrop = zombie.storedSunValue / 25;

						for (int i = 0; i < sunsToDrop; i++) {
							Sun newSun{};
							newSun.position = zombie.position;
							newSun.clicked = false;
							newSun.lifeClock.restart();

							addSun(newSun);
						}

						zombie.storedSunValue = 0;
					}
					// Dead
					zombie.isDead = true, zombieCount++, ZombieGap_dt += 0.05;	// TODO: maybe make isDead and health<=0 in another way?
					entity.category = ENTITY_CATEGORY::NONE; // remove after animation
					if (zombieCount >= currentLevel->targetZombie) {
						isGameWon = true;
					}
					};
				zombie.x_vel = 0;
			}
			//else if(!zombie.isDead){
			//	// health done and dying animation started but not finished yet, so we do nothing and wait for the animation to finish
			//}
			else if (zombie.isDead) {
				// health done and animation finished, so we remove the zombie
				zombie.category = ENTITY_CATEGORY::NONE;
				continue;
			}
		}

		// Animation
		zombie.nextFrame(); // next frame
		IntRect frameRect = getFrameRect(zombie);
		zombieSprite.setTextureRect(frameRect); // crop texture

		if (zombie.type == ENTITY_TYPE::ZOMBIE_RA && zombie.health > 0) { // for ra

			// start ability
			if (!zombie.isUsingAbility && zombie.abilityClock.getElapsedTime().asSeconds() >= 3.0f
				&& anySunAlive() && zombie.storedSunValue < zombie.maxSunValue && zombie.position.x < windowWidth - 300) {

				bool hasUnclaimedSun = false;
				for (int i = 0; i < MAX_SUNS; i++) {
					if (isSunAlive(suns[i]) && !suns[i].targeted && suns[i].position.y >= 0) {
						hasUnclaimedSun = true;
						break;
					}
				}

				if (hasUnclaimedSun) {
					zombie.isUsingAbility = true;
					zombie.abilityClock.restart();
					zombie.x_vel = 0;
					zombie.DoAnimation(-1, 2);
					zombie.scale = 1.5f;

					for (int i = 0; i < MAX_SUNS; i++) {
						if (isSunAlive(suns[i]) && !suns[i].targeted && suns[i].position.y >= 0) {
							suns[i].targeted = true;
							suns[i].ownerIndex = zombie.id;
							suns[i].lifeClock.restart();
						}
					}
				}
			}

			// pull suns
			if (zombie.isUsingAbility) {
				for (int i = 0; i < MAX_SUNS; i++) {
					Sun& sun = suns[i];
					if (!sun.targeted || !isSunAlive(sun) || sun.ownerIndex != zombie.id) continue;

					float dx = zombie.position.x - sun.position.x;
					float dy = zombie.position.y - sun.position.y;
					float dist = sqrt(dx * dx + dy * dy);

					if (dist > 1.0f) {
						sun.position.x += (dx / dist) * 300.0f * delta_time;
						sun.position.y += (dy / dist) * 300.0f * delta_time;
					}

					if (dist < 60.0f) {
						if (zombie.storedSunValue + 25 <= zombie.maxSunValue) {
							sun.clicked = true;
							sun.targeted = false;
							sun.ownerIndex = -1;
							zombie.storedSunValue += 25;
						}
					}
				}
				bool hasLiveSuns = false;
				for (int i = 0; i < MAX_SUNS; i++) {
					if (suns[i].ownerIndex == zombie.id && isSunAlive(suns[i]))
						hasLiveSuns = true;
				}
				// end ability
				if (zombie.isUsingAbility && (zombie.abilityClock.getElapsedTime().asSeconds() >= zombie.abilityDuration || !hasLiveSuns)) {

					for (int i = 0; i < MAX_SUNS; i++) {
						if (suns[i].ownerIndex == zombie.id) {
							suns[i].targeted = false;
							suns[i].ownerIndex = -1;
						}
					}
					zombie.isUsingAbility = false;
					zombie.x_vel = 0.05f;
					zombie.setCurrentAnimation(0);
					zombie.scale = 1.0f;
					zombie.abilityClock.restart();
				}
			}
		}

		if (zombie.type == ENTITY_TYPE::ZOMBIE_CONEHEAD && zombie.health > 0) { // for conehead
			if (zombie.health <= 100 && zombie.currentAnimationIndex == 0) {
				zombie.setCurrentAnimation(2);
			}
		}

		// move
		zombie.position.x -= zombie.x_vel;
		auto [r, c] = gridCell(zombie.position);
		if (zombie.position.x <= 0 + edgeSpace) {
			zombie.category = ENTITY_CATEGORY::NONE;
		} else {		// plant
			// Eat
			Entity& plant = getEntity({ r, c });
			if (plant.category == ENTITY_CATEGORY::PLANT) {
				if (zombie.position.x - (plant.position.x + plant.width - eatDistance) <= 0) {
					// close enough - not just touch
					zombie.x_vel = 0;
					if (zombie.attackClock.getElapsedTime().asSeconds() >= zombie.attackTimer) {
						zombie.attackClock.restart();
						plant.health -= zombie.attack;
						if (plant.health <= 0) {
							if (zombie.health > 0) zombie.x_vel = .05;
							plant.category = ENTITY_CATEGORY::NONE;
							plant.isDead = true;
							cout << "Plant Killed\n";
						}
					}
				}
			} else if (zombie.health > 0) {
				if (!(zombie.type == ENTITY_TYPE::ZOMBIE_RA && zombie.isUsingAbility)) { // for ra
					zombie.x_vel = .05; // no plant - has health - move normally edit_bug
				}
			}
		}

		int frameHeight = frameRect.getSize().y;
		float actualHeight = frameHeight * zombie.scale;
		Vector2f adjustedPosition = zombie.position;
		adjustedPosition.y -= (actualHeight - cellHeight);

		zombieSprite.setPosition(adjustedPosition);
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

				//cout << "Zombie hit! Health left: " << zombie.health << endl; //console just testing


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

int t=1, dz=0, zmn, zmx;
void CheckTimer_AddZombie(RenderWindow& window) {
	if (currentLevel == nullptr) { cout << "Still lvl; unchosen, didnt add zombie\n"; return; }

	if (createdZombiesCount >= currentLevel->targetZombie) return;

	// waves
	if (dz>=currentLevel->waveLimit && t==0) {
		// in to bet
		t = 1, waveClockX.restart();
		ZombieMinGapTime = zmn, ZombieMaxGapTime = zmx, currentLevel->waveLimit += currentLevel->waveLimitChange;
		cout << "switch to bet----------------------------\n";
	}
	else if (t==1 && waveClockX.getElapsedTime().asSeconds() >= currentLevel->waveTimer) {
		// bet to in
		t = 0, waveClockX.restart();
		dz = 0, zmn = ZombieMinGapTime, zmx = ZombieMaxGapTime;
		ZombieMinGapTime /= 2, ZombieMaxGapTime /= 2;
		cout << "switch to in a wave---------\n";
	}

	// single zombie
	if (zombieClock.getElapsedTime().asSeconds() >= nextZombieTimer) {
		cout << "New Zombie\n";
		zombieClock.restart();
		nextZombieTimer = ZombieMinGapTime + rand() % max(int(ZombieMaxGapTime - ZombieMinGapTime), 1);//I will edit it
		if (ZombieMinGapTime > 0 && ZombieMaxGapTime > 0) ZombieMinGapTime -= ZombieGap_dt, ZombieMaxGapTime -= ZombieGap_dt;

		// get random zombie type
		cout << "Current Level Zombies: " << currentLevel->zombie_types.size() << endl;
		int zombie_type_index = rand() % currentLevel->zombie_types.size();
		Entity newZombie = currentLevel->zombie_types[zombie_type_index];

		newZombie.position = Vector2f(windowWidth, offsetY + (rand() % gridraws) * cellheight);
		auto [raw, column] = gridCell(newZombie.position);
		newZombie.id = nextEntityId++; // for ra
		entities.push_back(newZombie);
		createdZombiesCount++;
		dz++;
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

void DrawSeeds(RenderWindow& window) {
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

	int removePlant = -1;

	int i = -1;
	for (auto& plant : currentLevel->selectedPlants) {
		i++;
		Vector2f spritePos = pos + Vector2f(115.0f + i * (seedPacket.getSize().x - 10.0f), 10.0f);
		uiSprite.setPosition(spritePos);

		Vector2u size = Vector2u(Vector2f(seedPacket.getSize()) * 0.75f);

		uiSprite.setColor(Color(255, 255, 255, 255));
		plantSprite.setColor(Color(255, 255, 255, 255));

		if (uiSprite.getGlobalBounds().contains(Vector2f(mousePos))) {
			uiSprite.setColor(Color(128, 128, 128, 255));
			plantSprite.setColor(Color(128, 128, 128, 255));

			if (mouseJustClicked[0]) {
				uiInteracted = true;
				if (currentLevel->arePlantsSelected) {
					selectedPlant = i;
				} else {
					removePlant = i;
				}
			}
		}

		//Smart Crop [ Each Seed Packets ? ]
		Texture& tex = getDefaultEntityTexture(plant.type);
		int frameWidth = tex.getSize().x / plant.currentAnimation.frames;
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
	uiSprite.setColor(Color(255, 255, 255, 255));
	plantSprite.setColor(Color(255, 255, 255, 255));

	if (removePlant != -1) {
		currentLevel->selectedPlants.erase(currentLevel->selectedPlants.begin() + removePlant);
	}
}

void UpdateLevel(RenderWindow& window) {

	CheckTimer_AddZombie(window);
	CheckTimer_AddSun(window);

	// Drawing

	window.clear(Color::Black);
	window.draw(ground);

	if (isGameOver == true) {
		window.draw(gameOverScreen);
		return;
	}
	if (isGameWon == true) {
		window.draw(gameWonScreen);
		return;
	}

	if (!currentLevel->arePlantsSelected) {
		DrawSeeds(window);

		Sprite bgSprite;
		bgSprite.setTexture(getTexture(TEXTURE_TYPE::SEED_CHOOSER));
		bgSprite.setPosition(Vector2f(10, 123.0f));
		bgSprite.setScale(Vector2f(1.5f, 1.5f));

		window.draw(bgSprite);

		Text text;
		text.setFont(*font);
		text.setString("Choose your plants");
		text.setFillColor(Color(255, 255, 255, 128));
		text.setPosition(Vector2f(240, 130.0f));

		window.draw(text);

		text.setFillColor(Color());

		Texture& seedPacket = getTexture(TEXTURE_TYPE::SEEDPACKET);

		Sprite seedBg;
		seedBg.setTexture(seedPacket);
		resizeSpriteToTex(seedBg, seedPacket);
		seedBg.setScale(Vector2f(0.75f, 0.75f));

		int i = -1;
		for (auto& plant : currentLevel->plant_types) {
			i++;
			Vector2f spritePos = Vector2f(50.0f, 190.0f) + Vector2f((i % 7) * (seedPacket.getSize().x - 10.0f), 10.0f + (i / 7) * 130.0f);
			seedBg.setPosition(spritePos);

			Vector2u size = Vector2u(Vector2f(seedPacket.getSize()) * 0.75f);

			seedBg.setColor(Color(255, 255, 255, 255));
			plantSprite.setColor(Color(255, 255, 255, 255));

			bool disabled = false;

			for (auto& selectedPlant : currentLevel->selectedPlants) {
				if (selectedPlant.type == plant.type) {
					disabled = true;
					break;
				}
			}

			if (disabled) {
				seedBg.setColor(Color(64, 64, 64, 255));
				plantSprite.setColor(Color(64, 64, 64, 255));
			} else if (seedBg.getGlobalBounds().contains(Vector2f(mousePos))) {
				seedBg.setColor(Color(128, 128, 128, 255));
				plantSprite.setColor(Color(128, 128, 128, 255));
				if (mouseJustClicked[0]) {
					uiInteracted = true;
					currentLevel->selectedPlants.push_back(plant);
				}
			}

			window.draw(seedBg);

			//Smart Crop [ Each Seed Packets ? ]
			Texture& tex = getDefaultEntityTexture(plant.type);
			int frameWidth = tex.getSize().x / plant.currentAnimation.frames;
			int frameHeight = tex.getSize().y;

			plantSprite.setTexture(tex);
			plantSprite.setTextureRect(IntRect{ 0, 0, frameWidth, frameHeight });

			plantSprite.setPosition(spritePos + Vector2f(10, 20));

			float scaleFactor = 55.0f / frameHeight;
			plantSprite.setScale(Vector2f(scaleFactor, scaleFactor));

			window.draw(plantSprite);

			text.setString(std::to_string(plant.price));
			text.setPosition(spritePos + Vector2f(10, 77));
			text.setCharacterSize(24);
			window.draw(text);
		}

		Sprite startButton;

		startButton.setTexture(getTexture(TEXTURE_TYPE::SEED_CHOOSER_BUTTON));
		startButton.setScale(Vector2f(1.5f, 1.5f));
		startButton.setPosition(Vector2f(250.0f, 780.0f));

		if (currentLevel->selectedPlants.size() == 0) {
			startButton.setColor(Color(64, 64, 64, 255));
		} else if (startButton.getGlobalBounds().contains(Vector2f(mousePos))) {
			startButton.setColor(Color(128, 128, 128, 255));
			if (mouseJustClicked[0]) {
				currentLevel->arePlantsSelected = true;
			}
		}

		window.draw(startButton);

		text.setString("Start Game");
		text.setPosition(startButton.getPosition() + Vector2f(40, 8.0f));
		text.setScale(Vector2f(1.5f, 1.5f));
		text.setFillColor(Color(255, 255, 255, 128));

		window.draw(text);

		return;
	}

	DrawPlantsGrid(window);
	DrawZombies(window);
	UpdateAndDrawBullets(window);
	UpdateSuns(window);
	DrawSeeds(window);
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
				suns[i].targeted = false; // for ra
				uiInteracted = true;
				break;
			}
		}

		if (!uiInteracted) {
			auto [row, col] = gridCell(mousePos);

			if (row >= 0 && row < gridRows && col >= 0 && col < gridCols) {

				if (getEntity({ row , col }).category == ENTITY_CATEGORY::NONE) {
					Entity plant;

					if (money >= currentLevel->selectedPlants[selectedPlant].price) {
						cout << "Placing Plant at " << row << " " << col << endl;
						plant = currentLevel->selectedPlants[selectedPlant];

						plant.gridRow = row;
						plant.gridCol = col;

						//centrize
						int x = offsetX + (col * cellWidth) + ((cellWidth - plant.width) / 2); //general formula -> start + (containerSize - objectSize) / 2

						//alighn to the bottom
						int y = offsetY + (row * cellHeight) + (cellHeight - plant.height);

						plant.position = Vector2f(x, y);

						entities.push_back(plant);
						money -= plant.price;
					}
				}
			}
		}
	}
	for (Entity& zombie : entities) {
		if (zombie.category == ENTITY_CATEGORY::ZOMBIE && !zombie.isDead)//zombie and not dead
		{
			if (zombie.position.x <= 0)//end of the screen
			{
				isGameOver = true;
				cout << "Zombies ate your brains!\n";
				break;
			}
		}
	}
}

void startLevel(int level) {
	// Set currentLevel and other global variables based on the selected level
	if (level < levels.size()) {
		cout << "selecting lvl " << level << " who whas " << levels[level].zombie_types.size() << endl;
		currentLevel = new Level(levels[level]);	// Note: level is 1 indexed
		money = currentLevel->startingSuns;
		isGameOver = false, isGameWon = false;
		Texture &groundTex = textures[TEXTURE_TYPE::LEVEL_BACKGROUND][level];
		cout << "txt size " << groundTex.getSize().x << endl;
		ground.setTexture(groundTex);
		ground.setTextureRect({ 0, 0, windowWidth, windowHeight });
		ground.setScale
		(
			(float)windowWidth / (float)groundTex.getSize().x,
			(float)windowHeight / (float)groundTex.getSize().y
		);
	}
}
void DrawMainMenu(RenderWindow& window) {
	Vector2f windowSize = { windowWidth, windowHeight };

	Sprite sprite;

	Texture bgTex = getTexture(TEXTURE_TYPE::LEVEL_SELECTOR_BACKGROUND);
	sprite.setTexture(bgTex);
	sprite.setPosition(Vector2f(0, 0));
	sprite.setScale(Vector2f(
		windowWidth / (float)bgTex.getSize().x,
		windowHeight / (float)bgTex.getSize().y
	));

	window.draw(sprite);

	{
		Sprite logoSprite;

		Texture logoTex = getTexture(TEXTURE_TYPE::LOGO);
		logoSprite.setTexture(logoTex);
		logoSprite.setOrigin(Vector2f(logoTex.getSize()) / 2.0f);
		logoSprite.setPosition(windowSize.x / 2.0f, 100.0f);

		window.draw(logoSprite);
	}

	Text text;

	static int selectedLevel = 0;
	static int previousSelectedLevel = -1;
	static Clock selectionChanged;

	Texture levelTex = getTexture(TEXTURE_TYPE::LEVEL_OPTION_BACKGROUND);
	Vector2f levelTexSize = Vector2f(levelTex.getSize());
	sprite.setTexture(levelTex);
	sprite.setOrigin(levelTexSize / 2.0f);

	bool levelClicked = false;

	const float animationDuration = 0.5f;

	float animationProgress = std::min(selectionChanged.getElapsedTime().asSeconds(), animationDuration) / animationDuration;
	// ease in out
	animationProgress = -(cos(animationProgress * 3.14159265358979323846f) - 1) / 2;

	float offset = selectedLevel * animationProgress + std::max(previousSelectedLevel, 0) * (1 - animationProgress);

	Vector2f initialPos = Vector2f(windowSize / 2.0f) - Vector2f(0, 100.0f);

	for (int i = 0; i < number_levels; i++) {
		bool selected = i == selectedLevel;
		bool hovered = false;

		Vector2f pos = initialPos + Vector2f((i - offset) * int(levelTex.getSize().x), 0);

		Vector2f scale = { 1, 1 };
		if (mousePos.x >= pos.x - levelTexSize.x / 2.0f && mousePos.x <= pos.x + levelTexSize.x / 2.0f &&
			mousePos.y >= pos.y - levelTexSize.x / 2.0f && mousePos.y <= pos.y + levelTexSize.y / 2.0f) {

			hovered = true;

			if (!levelClicked && mouseJustClicked[0] && selectedLevel != i) {
				previousSelectedLevel = selectedLevel;
				selectedLevel = i;
				selectionChanged.restart();
				levelClicked = true;
			}
		}

		if (previousSelectedLevel == i) {
			scale = Vector2f(1.0f, 1.0f) * (1 - animationProgress) + Vector2f(0.75f, 0.75f) * animationProgress;
		} else if (!selected) {
			scale = Vector2f(0.75f, 0.75f);
		}

		if (!selected && !hovered) {
			sprite.setColor(Color(128, 128, 128, 255));
		} else {
			sprite.setColor(Color::White);
		}

		sprite.setScale(scale);
		sprite.setPosition(pos);

		window.draw(sprite);

		text.setFont(*font);
		text.setString(std::string("Level ") + std::to_string(i + 1));
		text.setFillColor(Color());
		FloatRect textrc = text.getLocalBounds();
		text.setOrigin(textrc.width / 2.0f, textrc.width / 2.0f);

		text.setPosition(pos + Vector2f(0, (1 + scale.y - 0.75) * 120));

		window.draw(text);
	}

	{
		Vector2f pos = initialPos + Vector2f(0, levelTexSize.y / 2.0f);

		Sprite playButton;
		Texture playTex = getTexture(TEXTURE_TYPE::LEVEL_PLAY_BUTTON);
		Vector2f playTexSize = Vector2f(playTex.getSize());

		playButton.setTexture(playTex);

		playButton.setOrigin(playTexSize / 2.0f);
		playButton.setPosition(pos);
		playButton.setScale(Vector2f(0.5f, 0.5f));
		playButton.setColor(Color::White);

		if (playButton.getGlobalBounds().contains(Vector2f(mousePos))) {
			playButton.setColor(Color(128, 128, 128, 255));

			if (mouseJustClicked[0]) {
				startLevel(selectedLevel + 1);
			}
		}

		window.draw(playButton);
	}
}

int main() {
	RenderWindow window(VideoMode(windowWidth, windowHeight), "Plants vs Zombies");
	Event event;
	// Characters
	Texture groundTex;
	ground = defineGroundSprite(groundTex);

	//New

	// Loading Textures + Defining entities
	// Plants
	Entity repeaterPlant = defineNormalPlant();
	Entity shroom = defineShroomPlant();
	Entity sunflower = defineSunflower();
	Entity wallnut = defineWallnut();
	Entity chomper = defineChomperPlant();

	// Zombies
	Entity zombie = defineNormalZombie();
	Entity zombieJapanese = defineJapaneseZombie();
	Entity zombieBig = defineBigZombie();
	Entity zombieTorch = defineTorchlightZombie();
	Entity zombieRa = defineRaZombie();
	Entity zombieMummy = defineMummyZombie();
	Entity zombieConehead = defineConeheadZombie();

	// Other Textures
	textures[TEXTURE_TYPE::LOGO][Default_Texture_Index].loadFromFile("assets/logo.png");
	textures[TEXTURE_TYPE::LEVEL_SELECTOR_BACKGROUND][Default_Texture_Index].loadFromFile("assets/level_selector_background.png");
	textures[TEXTURE_TYPE::LEVEL_OPTION_BACKGROUND][Default_Texture_Index].loadFromFile("assets/level_option_background.png");
	textures[TEXTURE_TYPE::LEVEL_PLAY_BUTTON][Default_Texture_Index].loadFromFile("assets/play_button.png");
	textures[TEXTURE_TYPE::SEED_CHOOSER][Default_Texture_Index].loadFromFile("assets/seedchooser.png");
	textures[TEXTURE_TYPE::SEED_CHOOSER_BUTTON][Default_Texture_Index].loadFromFile("assets/seedchooser_button.png");
	textures[TEXTURE_TYPE::SUN][Default_Texture_Index].loadFromFile("assets/Sun3.png");
	textures[TEXTURE_TYPE::SEEDBANK][Default_Texture_Index].loadFromFile("assets/seedbank.png");
	textures[TEXTURE_TYPE::SEEDPACKET][Default_Texture_Index].loadFromFile("assets/seedpacket.png");

	// Font
	font = new Font();
	font->loadFromFile("assets/impact.ttf");


	// Levels
	levels[1].plant_types = { sunflower, repeaterPlant, wallnut, };
	levels[1].zombie_types = { zombieConehead };
	//levels[1].zombie_types = { zombie, zombieJapanese, zombieBig, zombieConehead };
	levels[1].startingSuns = 300;
	levels[1].bgPath = "assets/background.png";			// TODO: SIMON change it
	levels[1].targetZombie = 20;
	levels[1].waveLimit = 10, levels[1].waveTimer = 15;

	levels[2].plant_types = { sunflower, shroom, chomper };
	levels[2].zombie_types = { zombieTorch, zombieRa, zombieMummy };
	levels[2].startingSuns = 350;
	levels[2].bgPath = "assets/egypt.png";

	levels[3].plant_types = { sunflower, repeaterPlant };
	levels[3].zombie_types = { zombie, zombieConehead };
	levels[3].startingSuns = 100;

	textures[TEXTURE_TYPE::LEVEL_BACKGROUND][0].loadFromFile("assets/background.png");
	for (int lvl = 1; lvl <= number_levels; lvl++) {
		cout << "loading file txt " << levels[lvl].bgPath << endl;
		levels[lvl].lvlNum = lvl;
		textures[TEXTURE_TYPE::LEVEL_BACKGROUND][lvl].loadFromFile(levels[lvl].bgPath);
		cout << "done loaded " << levels[lvl].bgPath << endl;
	}


	//losing state

	Texture gameOverTex;
	gameOverTex.loadFromFile("assets/gameover.png");
	gameOverScreen.setSize(Vector2f(800.0f, 664.0f));
	gameOverScreen.setTexture(&gameOverTex);
	gameOverScreen.setOrigin(250.0f, 350.0f);
	gameOverScreen.setPosition(Vector2f(windowWidth / 2.0, windowHeight / 2.0));

	Texture gameWonTex;
	gameWonTex.loadFromFile("assets/gamewon.png");
	gameWonScreen.setSize(Vector2f(800.0f, 664.0f));
	gameWonScreen.setTexture(&gameWonTex);
	gameWonScreen.setOrigin(250.0f, 350.0f);
	gameWonScreen.setPosition(Vector2f(windowWidth / 2.0, windowHeight / 2.0));

	// Game Loop

	Clock DeltaTimeClock;

	while (window.isOpen()) {
		sf::Time dtTime = DeltaTimeClock.restart();
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

		// Separate UI -> Main Menu and Level
		if (currentLevel != nullptr) {
			UpdateLevel(window);
		}
		else {
			DrawMainMenu(window);
		}

		window.display();
	}

	delete font;
	textures.clear();

	return 0;
}