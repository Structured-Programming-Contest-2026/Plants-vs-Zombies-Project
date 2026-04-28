#include <iostream>
#include <vector>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <functional>
#include <cmath>
#include <fstream>

using namespace std;
using namespace sf;

#define gridraws 5
#define gridcols 9
//#define cellWidth 150
//#define cellHeight 170
#define edgespace 250
#define z_eat_p_distance 100
#define MAX_SUNS 16
#define Default_Texture_Index 0

// Global Variables
// consts
const int
gridRows = gridraws,
gridCols = gridcols,
windowWidth = 1920,
windowHeight = 1080;
const int edgeSpace = edgespace; // when zombie reaches this space, It disappears (end of screen) - should end game later
const int eatDistance = z_eat_p_distance;

// Drawables
RectangleShape gameOverScreen, gameWonScreen;
Sprite ground, lawnmowerSprite;

float delta_time = 1.0f;
Vector2i mousePos;
// left - right
bool mouseJustClicked[2];
bool uiInteracted = false;
int nextEntityId = 0; // for ra
bool isGameOver = false, isGameWon = false;
int leftEdgeForZombies = 225;	// TODO: Adjust with lawnmower and stuff
int deadZombieCount = 0, createdZombiesCount = 0;
int lawnmower_backoff = 150,	// from beginning edge
lawnmower_endline = windowWidth - 150; // which x pos to stop at
//Sending Zombies
float nextZombieTimer = 5; // First zombie timer - it's assigned, current value is only used for FIRST ZOMBIE
float ZombieMinGapTime = 7,
//float nextZombieTimer = 1; // First zombie timer - it's assigned, current value is only used for FIRST ZOMBIE
//float ZombieMinGapTime = 1,

ZombieMaxGapTime = ZombieMinGapTime + 3,
ZombieGap_dt = 0.1;	// Level Pace
int selectedPlant = 0;
int sunsMoney = 3000;
int coinsMoney = 0;

// Clocks
Clock zombieClock, waveClockX;
Clock sunSpawnClock;

//shovel
bool isShovelActive = false;

//relocate
bool isRelocateActive = false;
bool isRelocating = false;
int selectedPlantIndex = -1;

// lawnmower - cars
struct Lawnmower {
	bool isUsed = false, isMoving = false;
	int row = -1;
	Vector2f position = { -1000, -700 };

	void setRow(int r, int currentOffsetX, int currentOffsetY, int currentCellHeight) {
		row = r;
		position = { float(currentOffsetX - lawnmower_backoff),  float(currentOffsetY + row * currentCellHeight) };
	}

	void move(float v) {
		position.x += v;
	}
};

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
	ZOMBIE_NEWSPAPER,
	ZOMBIE_ALLSTAR,

	PLANT_NORMAL,
	PLANT_REPEATER, // <-- ADDED: Repeater Plant
	PLANT_SHROOM,
	PLANT_MINISHROOM,
	PLANT_SUNFLOWER,
	PLANT_WALLNUT,
	PLANT_CHOMPER,
	PLANT_SUNSHROOM,
	PLANT_ICE_PEASHOOTER,
	PLANT_POTATO_MINE,
	PLANT_DOOM_MINE,
	PLANT_CHEERY_BOMB,

	NormalBullet,
	IceBullet,
	ShroomBullet,



	ENTITY_TYPE_COUNT	// keep it last
};
enum class TEXTURE_TYPE : uint8_t {
	NONE = 0,
	LOGO,
	LEVEL_SELECTOR_BACKGROUND,
	LEVEL_OPTION_BACKGROUND,
	LEVEL_BACKGROUND,
	LEVEL_BACKGROUND_HOUSE,
	LEVEL_BACKGROUND_OPENDOOR,
	LEVEL_PLAY_BUTTON,
	LUSTER,
	SEED_CHOOSER,
	SEED_CHOOSER_BUTTON,
	SUN,
	SEEDBANK,
	SEEDPACKET,
	BOOSTER,


	ZOMBIE_NORMAL,
	ZOMBIE_JAPANESE,
	ZOMBIE_BIG,
	ZOMBIE_TORCHLIGHT,
	ZOMBIE_RA,
	ZOMBIE_MUMMY,
	ZOMBIE_CONEHEAD,
	ZOMBIE_NEWSPAPER,
	ZOMBIE_ALLSTAR,

	PLANT_NORMAL,
	PLANT_REPEATER, // <-- ADDED: Repeater Plant Texture
	PLANT_SHROOM,
	PLANT_MINISHROOM,
	PLANT_SUNFLOWER,
	PLANT_WALLNUT,
	PLANT_CHOMPER,
	PLANT_SUNSHROOM,
	PLANT_ICE_PEASHOOTER,
	PLANT_POTATO_MINE,
	PLANT_DOOM_MINE,
	PLANT_CHEERY_BOMB,
	NormalBullet,
	IceBullet,
	ShroomBullet,
	SHOVEL,
	MAGIC_SHOVEL,
};


map<ENTITY_TYPE, bool> isEntityUnlocked;

// Textures 1
map<TEXTURE_TYPE, Texture[10]> textures;
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
	case ENTITY_TYPE::ZOMBIE_NEWSPAPER:
		textureType = TEXTURE_TYPE::ZOMBIE_NEWSPAPER;
		break;
	case ENTITY_TYPE::ZOMBIE_ALLSTAR:
		textureType = TEXTURE_TYPE::ZOMBIE_ALLSTAR;
		break;
		//plantes
	case ENTITY_TYPE::PLANT_NORMAL:
		textureType = TEXTURE_TYPE::PLANT_NORMAL;
		break;
	case ENTITY_TYPE::PLANT_REPEATER: // <-- ADDED: Repeater Plant case
		textureType = TEXTURE_TYPE::PLANT_REPEATER;
		break;
	case ENTITY_TYPE::PLANT_SHROOM:
		textureType = TEXTURE_TYPE::PLANT_SHROOM;
		break;
	case ENTITY_TYPE::PLANT_MINISHROOM:
		textureType = TEXTURE_TYPE::PLANT_MINISHROOM;
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
	case ENTITY_TYPE::PLANT_SUNSHROOM:
		textureType = TEXTURE_TYPE::PLANT_SUNSHROOM;
		break;
	case ENTITY_TYPE::PLANT_ICE_PEASHOOTER:
		textureType = TEXTURE_TYPE::PLANT_ICE_PEASHOOTER;
		break;
		//other
	case ENTITY_TYPE::NormalBullet:
		textureType = TEXTURE_TYPE::NormalBullet;
		break;
	case ENTITY_TYPE::IceBullet:
		textureType = TEXTURE_TYPE::IceBullet;
		break;

	case ENTITY_TYPE::PLANT_POTATO_MINE:
		textureType = TEXTURE_TYPE::PLANT_POTATO_MINE;
		break;
	case ENTITY_TYPE::PLANT_DOOM_MINE:
		textureType = TEXTURE_TYPE::PLANT_DOOM_MINE;
		break;
	case ENTITY_TYPE::PLANT_CHEERY_BOMB:
		textureType = TEXTURE_TYPE::PLANT_CHEERY_BOMB;
		break;

	case ENTITY_TYPE::ShroomBullet:
		textureType = TEXTURE_TYPE::ShroomBullet;

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
	int frames = 0;
	float frameTimer = 1.0f;
	int currentFrame = 0;
	int cycles = 0;
	Clock clock;
};

struct Bullet {
	Vector2f position;
	Vector2f velocity;
	int damage = 0;
	// type
	// Animation

	/*ENTITY_TYPE type;
	&Entity sender;
	bool isActive = false;
	void updatePosition(float deltaTime) {
		position += velocity * deltaTime;
	}*/
};


struct Entity {
	ENTITY_CATEGORY category;
	ENTITY_TYPE type;
	int health = 100;
	int attack = 15;
	int price = 25;

	float seedCooldown = 1.0f;       // <--- ADDED: default cooldown for all plants
	float currentSeedCooldown = 0.0f; // <--- ADDED: active cooldown timer

	int num_animations = 1,
		currentAnimationIndex = 0;
	Animation animations[10];
	Animation currentAnimation;

	int AnimationForCycles = -1;

	float freezeTimer = 0.0f;

	Bullet bullet;

	float x_vel = 0.05f;
	Clock attackClock;
	int attackTimer = 1;
	Vector2f position;
	int width;
	int height;
	float scale = 1.0f;
	int gridRow = -1;
	int gridCol = -1;

	bool enteredHome = false;

	float growthTimer = 0.0f;       // For SunShroom
	bool isFullyGrown = false;      // For SunShroom
	int sunValueSmall = 15;         // For SunShroom
	int sunValueLarge = 25;         // For SunShroom
	float growthDuration = 20.0f;	// For SunShroom

	bool isUsingAbility = false; // for ra
	Clock abilityClock; // for ra
	float abilityDuration = 5.0f; // for ra
	int storedSunValue = 0; // for ra
	int maxSunValue = 350; // for ra
	int id = -1; // for ra

	bool hasTackled = false; // for allstar
	bool isTackling = false; // for allstar

	bool isBoosted = false;
	float boostAttackTimer = 2.7;

	bool isDead = false;
	function<void(Entity& entity)> afterAnimation = [](Entity&) {};	// called after animation finish (if animationForCycles != -1)

	// Updates width and height whenever Animation is changed - ex: To calculate zombie position properly
	void updateDimensions() {
		if (currentAnimation.frames == 0) {
			cout << "ERROR: Switching To Animation with 0 frames .. which is probably unexisting animation or you forgot to set frames .. Check whether you put animation.frames or check the animation index";
			return;
		}
		Texture tex = getTexture(getTextureType(type), currentAnimationIndex);
		int w = (tex.getSize().x / currentAnimation.frames) * scale;
		int h = tex.getSize().y * scale;

		width = w, height = h;
	}
	void setCurrentAnimation(int idx) {
		// You MUST change both
		cout << "set current animation idx to " << idx << " which has " << animations[idx].frames << " frames , by entity of type " << int(type) << endl;
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
					afterAnimation(*this);
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

	void (*updateEntity)(Entity& entity) = nullptr; // This function should not have problems with Global variables .. but local variables (on its definition) are probably not captured
};

map<ENTITY_TYPE, Entity> plants;

vector<Entity> entities;
vector<Entity> newBullets;	// test
vector<Bullet> bullets;

struct Reward {
	enum {
		None = 0, Plant, Coins
	} type = None;
	int coins = 0;
	ENTITY_TYPE plantType = ENTITY_TYPE::NONE;
	bool isClaimed = false;
	Reward() = default;
	Reward(int coins) : type(Coins), coins(coins) {}
	Reward(ENTITY_TYPE plantType) : type(Plant), plantType(plantType) {}
	Reward(Entity entity) {
		if (entity.category == ENTITY_CATEGORY::PLANT) {
			type = Plant;
			plantType = entity.type;
		} else {
			type = None;
		}
	}
};

struct Level {
	vector<Entity> zombie_types;
	vector<Entity> plant_types;
	int startingSuns = 3000;
	bool arePlantsSelected = false;
	bool isUnlocked = false;
	vector<Entity> selectedPlants;
	int lvlNum = 0;
	int targetZombie = 20,	// total zombies in a level 
		waveLimit = 10,		// a wave has up to (waveLimit) zombies
		waveTimer = 15,		// between waves
		waveLimitChange = 5;	// each wave has more zombies than previous by (waveLimitChange)
	float offsetX = 470;
	float offsetY = 100;
	float cellWidth = 150;
	float cellHeight = 170;
	float bgScaleX = 1920.0f / 1024.0f;
	float bgScaleY = 1080.0f / 600.0f;
	int bgRectWidth = 1024;
	int bgRectHeight = 600;
	//Reward rewards[10];
	Reward coinReward, plantReward;

	bool lawnmowersEnabled = false;
	Lawnmower lawnmowers[gridRows]{};

	Clock clock;
	Time plantsSelectedTime;
	Time gameOverTime;
	Time winTime;

	void addReward(Reward r) {
		/*for(auto &reward : rewards) {
			if(reward.type == Reward::None) {
				reward = r;
				return;
			}
		}*/
		if (r.type == Reward::Coins) {
			coinReward = r;
		} else if (r.type == Reward::Plant) {
			plantReward = r;
		}
	}
	void claimRewards() {
		if (!coinReward.isClaimed) {
			coinsMoney += coinReward.coins, coinReward.isClaimed = true;
			cout << "coins " << int(coinReward.coins) << " added! now its " << coinsMoney << endl;
		} else if (coinReward.isClaimed) {
			cout << "coins reward already claimed!" << endl;
		}

		if (!plantReward.isClaimed) {
			isEntityUnlocked[plantReward.plantType] = true, plantReward.isClaimed = true;
			cout << "plant " << int(plantReward.plantType) << " unlocked!" << endl;
		} else if (plantReward.isClaimed) {
			cout << "plant reward already claimed!" << endl;
		}
	}
};
const int number_levels = 6;
vector<Level> levels(number_levels + 1);
Level* currentLevel = nullptr;


// Zombie Clock - to add new zombie after some time interval

//Independet Sprites

Sprite plantSprite;
Sprite zombieSprite;
Sprite bulletSprite;
Sprite uiSprite;
Sprite ariseSprite;	// for booster for now

// TODO: remove vectors and maps and replace them with arrays :)


// Suns
struct Sun {
	Vector2f position;
	Clock lifeClock;
	float distance = 500.0f;
	bool clicked = true;
	int sunValue = 25;

	bool targeted = false; // for ra
	int ownerIndex = -1; // for ra
};
Sun suns[MAX_SUNS];

// Booster
int curBoosterCnt = 0;
int maxBoosterCnt = 2;
float boosterSpawnDuration = 1.0f;
float boosterLifeDuration = 3.0f;
bool isBoostActive = false;
//float boosterSpawnDuration = 10.0f;
//float boosterLifeDuration = 5.0f;
struct Booster {
	Clock spawnClock;
	Clock lifeClock;
	float minSpawnDuration = boosterSpawnDuration;
	float maxLifeDuration = boosterLifeDuration;
	//float minDuration = 10.0f;
	bool isAlive = false;
	Vector2f position;
	bool collect() {
		if (curBoosterCnt >= maxBoosterCnt) {
			cout << "----------Booster Not Collected ! Boosters already at max capacity!" << endl;
			return false;
		}
		curBoosterCnt++;
		isAlive = false;
		spawnClock.restart();
		cout << "----------Booster collected! total boosters: " << curBoosterCnt << endl;
		return true;
	}
	void activate(Entity& plant) {
		cout << "activate" << endl;
		if (!plant.animations[3].frames) {
			cout << "No Boost for thsi plant\n";
			return;
		}
		curBoosterCnt--;
		int orgAttackTimer = plant.attackTimer;
		plant.attackTimer = plant.boostAttackTimer;
		plant.isBoosted = true;
		plant.DoAnimation(50, 3); // TODO: ntimes/eachcycletakes + boolmod
		plant.afterAnimation = [orgAttackTimer](Entity& p) {
			p.attackTimer = orgAttackTimer;
			};
	}
};
Booster booster;
void CheckTimer_AddBooster(bool zombieJustDead = false, Vector2f position = { 0,0 }) {
	//if(10 sec && zombie just dead) ariseBooster
	if (zombieJustDead && !booster.isAlive && booster.spawnClock.getElapsedTime().asSeconds() >= booster.minSpawnDuration) {
		//boosterCnt++;
		booster.isAlive = true;
		booster.position = position;
		booster.lifeClock.restart();
		booster.spawnClock.restart();
		//cout << "----------Booster added! total boosters: " << boosterCnt << endl;
	}
	if (booster.isAlive && booster.lifeClock.getElapsedTime().asSeconds() >= booster.maxLifeDuration) {
		booster.isAlive = false;
		booster.spawnClock.restart();
		//cout << "----------Booster disappeared!" << endl;
	}
}

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
	if ((!isGameOver && !isGameWon) || entity.enteredHome) entity.nextFrame();

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
	if (currentLevel == nullptr) return { -1, -1 };
	if (pos.x < currentLevel->offsetX || pos.y < currentLevel->offsetY) return { -1, -1 };

	int column = (pos.x - currentLevel->offsetX) / currentLevel->cellWidth;
	int raw = (pos.y - currentLevel->offsetY) / currentLevel->cellHeight;

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

// Adjust health, isDead and deadZombieCount
bool attackZombie(Entity& zombie, int attack = 1000000, bool withDyingAnimation = true) {
	if (zombie.type == ENTITY_TYPE::NONE || zombie.isDead) return false; // already dead

	zombie.health -= attack;
	//cout << "from `attackzombie` Zombie attacked! health now: " << zombie.health << endl;
	if (zombie.health <= 0 && !withDyingAnimation) {
		zombie.category = ENTITY_CATEGORY::NONE;
		zombie.isDead = true, deadZombieCount++, ZombieGap_dt += 0.05;
		if (deadZombieCount >= currentLevel->targetZombie) {
			isGameWon = true;
		}
		cout << "Zombie died! total dead: " << deadZombieCount << " isDead = " << zombie.isDead << endl;
		return true;
	}
	// Arise a Booster if time is good
	if (zombie.health <= 0) CheckTimer_AddBooster(true, zombie.position);

	return true;
}

// Reset Level
void resetLevel() {

	delta_time = 1.0f;
	uiInteracted = false;
	nextEntityId = 0; // for ra
	isGameOver = false, isGameWon = false;
	deadZombieCount = 0, createdZombiesCount = 0;
	//Sending Zombies
	nextZombieTimer = 5; // First zombie timer - it's assigned, current value is only used for FIRST ZOMBIE
	ZombieMinGapTime = 7,
		ZombieMaxGapTime = ZombieMinGapTime + 3,
		ZombieGap_dt = 0.1;	// Level Pace
	selectedPlant = 0;

	sunsMoney = currentLevel->startingSuns;

	for (int i = 0; i < gridRows; i++) {
		currentLevel->lawnmowers[i].setRow(i, currentLevel->offsetX, currentLevel->offsetY, currentLevel->cellHeight);
	}
	entities.clear();
	zombieClock.restart();
	waveClockX.restart();
	sunSpawnClock.restart();

	currentLevel->plantsSelectedTime = Time::Zero;
	currentLevel->arePlantsSelected = false;
	currentLevel->selectedPlants.clear();
}

void startLevel(int level) {
	// Set currentLevel and other global variables based on the selected level
	if (level < levels.size()) {
		cout << "selecting lvl " << level << " who whas " << levels[level].zombie_types.size() << endl;
		currentLevel = new Level(levels[level]);	// Note: level is 1 indexed
		sunsMoney = currentLevel->startingSuns;
		isGameOver = false, isGameWon = false;
		Texture& groundTex = textures[TEXTURE_TYPE::LEVEL_BACKGROUND][level];
		//cout << "txt size " << groundTex.getSize().x << endl;
		ground.setTexture(textures[TEXTURE_TYPE::LEVEL_BACKGROUND][(currentLevel ? currentLevel->lvlNum : 0)]);
		//ground.setTextureRect({ 0, 0, windowWidth, windowHeight });	// PROBLEM - Doesnt take the whole texture size, but only the part of it that corresponds to the window size, so if texture is smaller than window, it will be repeated, if it's bigger, it will be cropped
		ground.setTextureRect({ 0, 0, (int)groundTex.getSize().x, (int)groundTex.getSize().y });
		ground.setScale
		(
			(float)windowWidth / (float)groundTex.getSize().x,
			(float)windowHeight / (float)groundTex.getSize().y
		);

		currentLevel->clock.restart();
	}
}

void nextLevel() {
	if (currentLevel == nullptr) {
		currentLevel = &levels[1];
	} else if (currentLevel->lvlNum < number_levels) {
		currentLevel = &levels[currentLevel->lvlNum + 1];
	} else {
		cout << "No more levels!" << endl;
		return;
	}
	resetLevel();
	startLevel(currentLevel->lvlNum);
}


// Define Characters - Functions

// ground
Sprite defineGroundSprite(Texture& tex) {
	/*tex.setRepeated(true);*/

	Sprite groundSp;
	groundSp.setTexture(tex);

	/*IntRect groundTexRect(0, 0, width, height);
	groundSp.setTextureRect(groundTexRect);*/
	groundSp.setTextureRect(IntRect(0, 0, (int)tex.getSize().x, (int)tex.getSize().y));

	return groundSp;
}
//lawnmower
Sprite defineLawnmowerSprite(Texture& tex) {
	tex.loadFromFile("assets/lawnmower.png");
	Sprite lawnmower;
	lawnmower.setTexture(tex);
	lawnmower.setTextureRect(IntRect(0, 0, (int)tex.getSize().x, (int)tex.getSize().y));
	/*lawnmower.setScale(1.0f, 1.0);*/
	return lawnmower;
}

// Bullets
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
Entity defineIceBullet() {
	Texture tex;
	tex.loadFromFile("assets/SnowBullet.png");
	textures[TEXTURE_TYPE::IceBullet][Default_Texture_Index] = tex;

	Entity bullet{};
	bullet.scale = 0.5;
	bullet.category = ENTITY_CATEGORY::Bullet;
	bullet.type = ENTITY_TYPE::IceBullet;
	bullet.attack = 10;
	bullet.x_vel = -0.4f;

	return bullet;
}
Entity defineShroomBullet() {
	Texture tex;
	tex.loadFromFile("assets/ShroomBullet.png");
	textures[TEXTURE_TYPE::ShroomBullet][Default_Texture_Index] = tex;

	Entity bullet{};
	bullet.scale = 0.3;
	bullet.category = ENTITY_CATEGORY::Bullet;
	bullet.type = ENTITY_TYPE::ShroomBullet;
	bullet.attack = 5;
	bullet.x_vel = -0.4f;

	return bullet;
}

// plants
Entity defineNormalPlant() {

	Texture tex;
	tex.loadFromFile("assets/Peashooter.png");
	textures[TEXTURE_TYPE::PLANT_NORMAL][Default_Texture_Index] = tex;
	textures[TEXTURE_TYPE::PLANT_NORMAL][2].loadFromFile("assets/peashooter-attack.png");
	textures[TEXTURE_TYPE::PLANT_NORMAL][3].loadFromFile("assets/peashooter-boost.png");
	/*tex.loadFromFile("assets/Repeater.png");
	textures[TEXTURE_TYPE::PLANT_NORMAL][1] = tex;*/
	Entity plant;
	plant.category = ENTITY_CATEGORY::PLANT;
	plant.type = ENTITY_TYPE::PLANT_NORMAL;
	plant.health = 100;
	plant.attack = 30;
	plant.attackTimer = 3.0f;
	//planet.attack = 5000;
	plant.price = 25;
	plant.scale = 0.75;

	plant.num_animations = 4;
	// Idle animation
	plant.animations[0].frameTimer = 0.05f;
	plant.animations[0].frames = 31;
	// Attack animation
	plant.animations[2].frameTimer = 0.05f;
	plant.animations[2].frames = 31;
	// Boost animation
	plant.animations[3].frameTimer = 0.05f;
	plant.animations[3].frames = 13;
	plant.setCurrentAnimation(0);

	plants[ENTITY_TYPE::PLANT_NORMAL] = plant;

	// update
	plant.updateEntity = [](Entity& plant) {
		if (plant.attackClock.getElapsedTime().asSeconds() >= plant.attackTimer) {

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

			// shoot bullet
			if (zombieInLane) {
				plant.attackClock.restart();
				Entity newBullet = defineNormalBullet();
				newBullet.attack = plant.attack;
				newBullet.position = Vector2f(plant.position.x + 100, plant.position.y + 50);
				newBullets.push_back(newBullet);
				//entities.push_back(newBullet);

				// plant attack animation
				plant.DoAnimation(1, 2);
			}
		}
		};
	return plant;

}
Entity defineRepeaterPlant() {
	Texture tex;
	tex.loadFromFile("assets/Repeater.png");
	textures[TEXTURE_TYPE::PLANT_REPEATER][Default_Texture_Index] = tex;

	Entity plant;
	plant.category = ENTITY_CATEGORY::PLANT;
	plant.type = ENTITY_TYPE::PLANT_REPEATER;
	plant.health = 100;
	plant.attack = 30;
	plant.price = 100;
	plant.scale = 0.75;

	plant.num_animations = 1;
	// Idle animation
	plant.animations[0].frameTimer = 0.1f;
	plant.animations[0].frames = 14;

	plant.setCurrentAnimation(0);

	plants[ENTITY_TYPE::PLANT_REPEATER] = plant;

	// update
	plant.updateEntity = [](Entity& plant) {

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

				Vector2f pos = plant.position; // copy position before push
				int atk = plant.attack;        // copy attack before push


				Entity newBullet1 = defineNormalBullet();
				newBullet1.attack = atk;
				newBullet1.position = Vector2f(pos.x + 100, pos.y + 50);
				newBullets.push_back(newBullet1);
				//entities.push_back(newBullet1);

				Entity newBullet2 = defineNormalBullet();
				newBullet2.attack = atk;
				newBullet2.position = Vector2f(pos.x + 50, pos.y + 50);
				newBullets.push_back(newBullet2);
				//entities.push_back(newBullet2);

				// Commented By Omar Hadidi - Missing Attack Animation
				// plant attack animation
				//plant.DoAnimation(1, 2);
			}

		}
		};
	return plant;
}
Entity defineShroomPlant() {
	Texture tex;
	tex.loadFromFile("assets/shroom.png");
	textures[TEXTURE_TYPE::PLANT_SHROOM][Default_Texture_Index] = tex;

	Entity shroom;
	shroom.category = ENTITY_CATEGORY::PLANT;
	shroom.type = ENTITY_TYPE::PLANT_SHROOM;
	shroom.health = 500;
	//shroom.attack = 0;
	shroom.attack = 20;
	shroom.price = 15;
	shroom.scale = 0.9;
	// Idle animation
	shroom.num_animations = 1;
	shroom.animations[0].frameTimer = 0.1f;
	shroom.animations[0].frames = 30;
	shroom.setCurrentAnimation(0);

	plants[ENTITY_TYPE::PLANT_SHROOM] = shroom;

	// update
	shroom.updateEntity = [](Entity& plant) {

		};

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

	plants[ENTITY_TYPE::PLANT_SUNFLOWER] = plant;

	// update
	plant.updateEntity = [](Entity& plant) {
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
		};

	return plant;
}
Entity defineSunShroom() {
	Entity SunShroom;
	SunShroom.category = ENTITY_CATEGORY::PLANT;
	SunShroom.type = ENTITY_TYPE::PLANT_SUNSHROOM;
	SunShroom.health = 100;
	SunShroom.attack = 0;
	SunShroom.price = 25;

	SunShroom.sunValueSmall = 15;
	SunShroom.sunValueLarge = 25;
	SunShroom.growthDuration = 20.0f;


	textures[TEXTURE_TYPE::PLANT_SUNSHROOM][0].loadFromFile("assets/SunShroomIdleS.png"); // IDLE SMALL
	textures[TEXTURE_TYPE::PLANT_SUNSHROOM][1].loadFromFile("assets/SunShroomIdleB.png"); // IDLE BIG
	textures[TEXTURE_TYPE::PLANT_SUNSHROOM][2].loadFromFile("assets/SunShroomProduceS.png"); // SUN PRODUCE SMALL
	textures[TEXTURE_TYPE::PLANT_SUNSHROOM][3].loadFromFile("assets/SunShroomProduceB.png");

	SunShroom.num_animations = 4;
	Animation a;
	a.frames = 30;
	a.frameTimer = 0.08f;
	SunShroom.animations[0] = a; // IDLE SMALL
	SunShroom.animations[1] = a; // IDLE BIG
	a.frames = 60;
	a.frameTimer = 0.04;
	SunShroom.animations[2] = a; // PRODUCE SMALL
	SunShroom.animations[3] = a; // PRODUCE BIG

	SunShroom.setCurrentAnimation(0);

	plants[ENTITY_TYPE::PLANT_SUNSHROOM] = SunShroom;

	SunShroom.updateEntity = [](Entity& plant) {
		plant.growthTimer += delta_time;

		if (!plant.isFullyGrown && plant.growthTimer >= plant.growthDuration) {
			plant.isFullyGrown = true;
			plant.setCurrentAnimation(1);
			cout << "SunShroom has grown!" << endl;
		}

		if (plant.attackClock.getElapsedTime().asSeconds() >= 10.0f) {

			plant.attackClock.restart();

			/*if (plant.isFullyGrown) {
				plant.DoAnimation(1, 3);
				plant.afterAnimation = [&plant](Entity& entity) {
					entity.setCurrentAnimation(1);
					};
			}
			else {
				plant.DoAnimation(1, 2);
				plant.afterAnimation = [&plant](Entity& entity) {
					entity.setCurrentAnimation(0);
					};
			}*/ // STILL WOKRING ON ANIMATION

			int sunValue = (plant.isFullyGrown) ? plant.sunValueLarge : plant.sunValueSmall;

			Sun newSun{};
			newSun.position = plant.position;
			newSun.distance = plant.position.y + 50;
			newSun.clicked = false;
			newSun.sunValue = sunValue;
			newSun.lifeClock.restart();

			addSun(newSun);
			cout << "Sunflower produced" << sunValue << " sun!" << endl;
		}
		};

	return SunShroom;
}
Entity defineIcePeaShooter() {
	Texture tex;
	tex.loadFromFile("assets/IcePeashooter.png");
	textures[TEXTURE_TYPE::PLANT_ICE_PEASHOOTER][Default_Texture_Index] = tex;

	Entity icePea;
	icePea.category = ENTITY_CATEGORY::PLANT;
	icePea.type = ENTITY_TYPE::PLANT_ICE_PEASHOOTER;
	icePea.health = 100;
	icePea.attack = 15;
	icePea.price = 75;
	icePea.scale = 0.75;

	icePea.num_animations = 1;

	Animation a;
	a.frames = 24;
	a.frameTimer = 0.08f;
	icePea.animations[0] = a;

	icePea.setCurrentAnimation(0);

	plants[ENTITY_TYPE::PLANT_ICE_PEASHOOTER] = icePea;

	icePea.updateEntity = [](Entity& plant) {
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
				Entity newBullet = defineIceBullet();
				newBullet.attack = plant.attack;
				newBullet.position = Vector2f(plant.position.x + 100, plant.position.y);
				newBullets.push_back(newBullet);
				//entities.push_back(newBullet);
			}
		}
		};

	return icePea;
}
Entity defineCherryBomb() {
	textures[TEXTURE_TYPE::PLANT_CHEERY_BOMB][0].loadFromFile("assets/CherryBomb.png");
	textures[TEXTURE_TYPE::PLANT_CHEERY_BOMB][1].loadFromFile("assets/cherryexp.png");

	Entity cherry{};
	cherry.category = ENTITY_CATEGORY::PLANT;
	cherry.type = ENTITY_TYPE::PLANT_CHEERY_BOMB;
	cherry.health = 300;
	cherry.attack = 9999;
	cherry.price = 150;
	cherry.scale = 1.0f;
	cherry.width = 120;
	cherry.height = 120;

	cherry.num_animations = 2;

	// animation 0 = النبتة العادية
	cherry.animations[0].frames = 7;   // ← غير لعدد الفريمات
	cherry.animations[0].frameTimer = 0.1f;

	// animation 1 = الانفجار
	cherry.animations[1].frames = 8;  // ← غير لعدد الفريمات
	cherry.animations[1].frameTimer = 0.05f;

	cherry.setCurrentAnimation(0);

	plants[ENTITY_TYPE::PLANT_CHEERY_BOMB] = cherry;

	// update
	cherry.updateEntity = [](Entity& plant) {
		// START EXPLOSION ANIMATION

		if (plant.attackClock.getElapsedTime().asSeconds() >= 3.0f && plant.currentAnimationIndex != 1) {
			plant.setCurrentAnimation(1); // تتحول للحالة الجاهزة
		}

		if (plant.currentAnimationIndex != 1) {
			plant.DoAnimation(1, 1);


			plant.afterAnimation = [&](Entity& p) {
				// RANGE OF EXPLOSION 
				float explosionRange = 150.0f;

				for (Entity& z : entities) {
					if (z.category == ENTITY_CATEGORY::ZOMBIE && !z.isDead) {
						float diffY = abs(z.position.y - p.position.y);
						float diffX = abs(z.position.x - p.position.x);

						// 3*3
						if (diffY < 120.0f && diffX < explosionRange) {
							z.health -= p.attack; // عادة الـ attack بتاعها بيموت أي زومبي عادي فوراً
						}
					}
				}
				//DISAPPER AFTER EXPLOSION
				p.category = ENTITY_CATEGORY::NONE;
				};

		}
		};
	return cherry;
}
Entity defineMiniShroom() {
	Texture tex;
	tex.loadFromFile("assets/Minishroom.png");
	textures[TEXTURE_TYPE::PLANT_MINISHROOM][Default_Texture_Index] = tex;

	Entity miniShroom;
	miniShroom.category = ENTITY_CATEGORY::PLANT;
	miniShroom.type = ENTITY_TYPE::PLANT_MINISHROOM;
	miniShroom.health = 50;
	miniShroom.attack = 30;
	miniShroom.price = 0;
	miniShroom.seedCooldown = 5.0f;
	miniShroom.scale = 0.4f;

	miniShroom.num_animations = 1;

	Animation a;
	a.frames = 30;
	a.frameTimer = 0.05f;
	miniShroom.animations[0] = a;

	miniShroom.setCurrentAnimation(0);

	plants[ENTITY_TYPE::PLANT_MINISHROOM] = miniShroom;
	miniShroom.updateEntity = [](Entity& plant) {
		if (plant.attackClock.getElapsedTime().asSeconds() >= 2.0f) {
			bool zombieInFront = false;

			for (Entity& zombie : entities) {
				// Check if it is actually a living zombie!
				if (zombie.category == ENTITY_CATEGORY::ZOMBIE && zombie.health > 0) {
					bool sameRow = abs(zombie.position.y - plant.position.y) < 100.0f;
					bool inFront = zombie.position.x > plant.position.x &&
						zombie.position.x < plant.position.x + plant.width * 5;

					if (sameRow && inFront) {
						zombieInFront = true;
						break; // Found a target, stop looking
					}
				}
			}

			if (zombieInFront) {
				plant.attackClock.restart();
				Entity newBullet = defineShroomBullet();
				newBullet.attack = plant.attack;
				newBullet.position = Vector2f(plant.position.x + 70, plant.position.y);
				newBullets.push_back(newBullet);
				//entities.push_back(newBullet);
			}
		}
		};

	return miniShroom;
}
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


	plants[ENTITY_TYPE::PLANT_WALLNUT] = wallnut;

	// update
	wallnut.updateEntity = [](Entity& plant) {
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
		};
	return wallnut;
}
Entity defineChomperPlant() {
	Entity Chomper;
	Chomper.category = ENTITY_CATEGORY::PLANT;
	Chomper.type = ENTITY_TYPE::PLANT_CHOMPER;
	Chomper.health = 200;
	Chomper.price = 150;
	Chomper.scale = 0.85;
	Chomper.attack = 10000; // one attack kill

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


	plants[ENTITY_TYPE::PLANT_CHOMPER] = Chomper;

	// update
	Chomper.updateEntity = [](Entity& plant) {
		if (plant.currentAnimationIndex == 0) { // IDLE
			for (Entity& zombie : entities) {
				if (zombie.category != ENTITY_CATEGORY::ZOMBIE || zombie.isDead) continue;

				bool sameRow = abs(zombie.position.y - plant.position.y) < 100.0f;
				bool inFront = zombie.position.x > plant.position.x &&
					zombie.position.x < plant.position.x + plant.width * 0.5;
				// NOTE: Distance from zombie depends on (Texture) width * scale

				if ((sameRow && inFront) && zombie.type != ENTITY_TYPE::ZOMBIE_BIG) {
					attackZombie(zombie, plant.attack, false); // adjust health, isDead and deadZombieCount
					plant.attackClock.restart().asSeconds();

					plant.DoAnimation(1, 1);
					plant.afterAnimation = [](Entity& entity) {
						entity.DoAnimation(20, 2);
						entity.afterAnimation = [](Entity& entity) {
							entity.DoAnimation(-1, 0);
							};
						};
					return;	// attacked 1
				}
			}
		}
		};
	return Chomper;
}
Entity definePOTATO_MINE() {
	textures[TEXTURE_TYPE::PLANT_POTATO_MINE][0].loadFromFile("assets/PotatoMine2.png");
	textures[TEXTURE_TYPE::PLANT_POTATO_MINE][1].loadFromFile("assets/PotatoMine.png");
	Entity mine{};
	mine.category = ENTITY_CATEGORY::PLANT;
	mine.type = ENTITY_TYPE::PLANT_POTATO_MINE;
	mine.health = 200;
	mine.attack = 1800;
	mine.price = 25;
	mine.scale = 0.8f;
	mine.num_animations = 2;
	// Sleeping animation
	mine.animations[0].frames = 1;
	mine.animations[0].frameTimer = 0.1f;
	// Ready animation
	mine.animations[1].frames = 8;
	mine.animations[1].frameTimer = 0.08f;
	mine.setCurrentAnimation(0);

	plants[ENTITY_TYPE::PLANT_POTATO_MINE] = mine;

	mine.updateEntity = [](Entity& plant) {
		if (plant.type == ENTITY_TYPE::PLANT_POTATO_MINE && plant.currentAnimationIndex == 0) {
			// ready to exp
			if (plant.attackClock.getElapsedTime().asSeconds() >= 1.0f) {
				plant.setCurrentAnimation(1); // تتحول للحالة الجاهزة
			}
			return; //cant exp under ground
		}

		// ثانياً: تشيك لو في زومبي قريب عشان يحصل انفجار
		for (Entity& zombie : entities) {
			if (zombie.category != ENTITY_CATEGORY::ZOMBIE || zombie.isDead) continue;

			float diffY = abs(zombie.position.y - plant.position.y);
			float diffX = abs(zombie.position.x - plant.position.x);


			float triggerRange = (plant.type == ENTITY_TYPE::PLANT_DOOM_MINE) ? 200.0f : 50.0f;

			if (diffY < 60.0f && diffX < triggerRange) {
				// 1. شغل أنميشن الانفجار (افترضنا إنه رقم 1 في الدوم ورقم 2 في البطاطس)
				int blastAnimIdx = (plant.type == ENTITY_TYPE::PLANT_DOOM_MINE) ? 1 : 2;
				plant.DoAnimation(1, blastAnimIdx);

				// 2. حدد اللي هيحصل لما الأنميشن يخلص (الضرر)
				plant.afterAnimation = [&](Entity& p) {
					// Replace the lambda capture [this] with a capture of the entities vector by reference.
					// Find this code (inside DrawPlantsGrid or similar):
					// plant.afterAnimation = [this](Entity& p) { ... }

					// Replace with:
					plant.afterAnimation = [&](Entity& p) {
						float damageRange = (p.type == ENTITY_TYPE::PLANT_DOOM_MINE) ? 300.0f : 80.0f;

						for (Entity& z : entities) {
							if (z.category == ENTITY_CATEGORY::ZOMBIE && !z.isDead) {
								if (abs(z.position.y - p.position.y) < 100.0f && abs(z.position.x - p.position.x) < damageRange) {
									z.health -= p.attack;
								}
							}
						}
						p.category = ENTITY_CATEGORY::NONE; // plant disappears
						};
					float damageRange = (p.type == ENTITY_TYPE::PLANT_DOOM_MINE) ? 300.0f : 80.0f;

					for (Entity& z : entities) {
						if (z.category == ENTITY_CATEGORY::ZOMBIE && !z.isDead) {
							if (abs(z.position.y - p.position.y) < 100.0f && abs(z.position.x - p.position.x) < damageRange) {
								z.health -= p.attack;
							}
						}
					}
					p.category = ENTITY_CATEGORY::NONE; // النبات يختفي
					};
				break;
			}
		}
		};

	return mine;
}
Entity defineDOOM_SHROOM() {
	textures[TEXTURE_TYPE::PLANT_DOOM_MINE][0].loadFromFile("assets/doom_shroom.png");
	textures[TEXTURE_TYPE::PLANT_DOOM_MINE][1].loadFromFile("assets/doomexp.png");

	Entity shroom{};
	shroom.category = ENTITY_CATEGORY::PLANT;
	shroom.type = ENTITY_TYPE::PLANT_DOOM_MINE;
	shroom.health = 300;
	shroom.attack = 9999;
	shroom.price = 125;
	shroom.scale = 1.2f;
	shroom.width = 120;
	shroom.height = 120;

	shroom.num_animations = 2;

	// animation 0 = النبتة العادية
	shroom.animations[0].frames = 41;   // ← غير الرقم ده لعدد الفريمات
	shroom.animations[0].frameTimer = 0.1f;

	// animation 1 = الانفجار
	shroom.animations[1].frames = 14;  // ← غير الرقم ده لعدد الفريمات
	shroom.animations[1].frameTimer = 0.05f;

	shroom.setCurrentAnimation(0);

	plants[ENTITY_TYPE::PLANT_DOOM_MINE] = shroom;
	// update
	shroom.updateEntity = [](Entity& plant) {
		for (Entity& zombie : entities) {
			if (zombie.category != ENTITY_CATEGORY::ZOMBIE || zombie.isDead) continue;

			float diffY = abs(zombie.position.y - plant.position.y);
			float diffX = abs(zombie.position.x - plant.position.x);


			float triggerRange = (plant.type == ENTITY_TYPE::PLANT_DOOM_MINE) ? 200.0f : 50.0f;

			if (diffY < 60.0f && diffX < triggerRange) {
				// 1. شغل أنميشن الانفجار (افترضنا إنه رقم 1 في الدوم ورقم 2 في البطاطس)
				int blastAnimIdx = (plant.type == ENTITY_TYPE::PLANT_DOOM_MINE) ? 1 : 2;
				plant.DoAnimation(1, blastAnimIdx);

				// 2. حدد اللي هيحصل لما الأنميشن يخلص (الضرر)
				plant.afterAnimation = [&](Entity& p) {
					// Replace the lambda capture [this] with a capture of the entities vector by reference.
					// Find this code (inside DrawPlantsGrid or similar):
					// plant.afterAnimation = [this](Entity& p) { ... }

					// Replace with:
					plant.afterAnimation = [&](Entity& p) {
						float damageRange = (p.type == ENTITY_TYPE::PLANT_DOOM_MINE) ? 300.0f : 80.0f;

						for (Entity& z : entities) {
							if (z.category == ENTITY_CATEGORY::ZOMBIE && !z.isDead) {
								if (abs(z.position.y - p.position.y) < 100.0f && abs(z.position.x - p.position.x) < damageRange) {
									z.health -= p.attack;
								}
							}
						}
						p.category = ENTITY_CATEGORY::NONE; // plant disappears
						};
					float damageRange = (p.type == ENTITY_TYPE::PLANT_DOOM_MINE) ? 300.0f : 80.0f;

					for (Entity& z : entities) {
						if (z.category == ENTITY_CATEGORY::ZOMBIE && !z.isDead) {
							if (abs(z.position.y - p.position.y) < 100.0f && abs(z.position.x - p.position.x) < damageRange) {
								z.health -= p.attack;
							}
						}
					}
					p.category = ENTITY_CATEGORY::NONE; // النبات يختفي
					};
				break;
			}
		}
		};
	return shroom;
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
	zombie.health = 200;
	zombie.num_animations = 3;
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
Entity defineNewspaperZombie() {
	textures[TEXTURE_TYPE::ZOMBIE_NEWSPAPER][0].loadFromFile("assets/NewspaperZombie.png");
	textures[TEXTURE_TYPE::ZOMBIE_NEWSPAPER][1].loadFromFile("assets/NewspaperZombieDeath.png");
	textures[TEXTURE_TYPE::ZOMBIE_NEWSPAPER][2].loadFromFile("assets/NewspaperZombieAngry.png");

	Entity zombie{};
	zombie.category = ENTITY_CATEGORY::ZOMBIE;
	zombie.type = ENTITY_TYPE::ZOMBIE_NEWSPAPER;
	zombie.health = 250;
	zombie.attack = 10;
	zombie.x_vel = 0.05;
	zombie.scale = 1.2f;
	// Animations
	zombie.num_animations = 3;
	Animation anim;
	// walk
	anim.frames = 19;
	anim.frameTimer = 0.07f;
	zombie.animations[0] = anim;
	// death
	anim.frames = 11;
	anim.frameTimer = 0.1f;
	zombie.animations[1] = anim;
	// angry
	anim.frames = 14;
	anim.frameTimer = 0.05f;
	zombie.animations[2] = anim;

	zombie.setCurrentAnimation(0);

	return zombie;
}
Entity defineAllStarZombie() {
	textures[TEXTURE_TYPE::ZOMBIE_ALLSTAR][0].loadFromFile("assets/AllStarZombie.png");
	textures[TEXTURE_TYPE::ZOMBIE_ALLSTAR][1].loadFromFile("assets/AllStarZombieDeath.png");
	textures[TEXTURE_TYPE::ZOMBIE_ALLSTAR][2].loadFromFile("assets/AllStarZombieRunning.png");
	textures[TEXTURE_TYPE::ZOMBIE_ALLSTAR][3].loadFromFile("assets/AllStarZombieTackle.png");

	Entity zombie{};
	zombie.category = ENTITY_CATEGORY::ZOMBIE;
	zombie.type = ENTITY_TYPE::ZOMBIE_ALLSTAR;
	zombie.health = 400;
	zombie.attack = 10;
	zombie.x_vel = 0.18f;
	// Animations
	zombie.num_animations = 4;
	Animation anim;
	// walk
	anim.frames = 37;
	anim.frameTimer = 0.07f;
	zombie.animations[0] = anim;
	// death
	anim.frames = 55;
	anim.frameTimer = 0.05f;
	zombie.animations[1] = anim;
	// running
	anim.frames = 20;
	anim.frameTimer = 0.05f;
	zombie.animations[2] = anim;
	// tackle
	anim.frames = 39;
	anim.frameTimer = 0.07f;
	zombie.animations[3] = anim;

	zombie.setCurrentAnimation(2);

	return zombie;
}


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
		uiSprite.setScale(Vector2f(1.0f, 1.0f));

		if (sun.targeted) { // for ra
			uiSprite.setColor(Color(255, 80, 80));
		} else {
			uiSprite.setColor(Color(255, 255, 255));
		}

		window.draw(uiSprite);
	}
	uiSprite.setColor(Color(255, 255, 255)); // for ra
}
void UpdateBooster(RenderWindow& window) {
	if (!booster.isAlive) return;
	ariseSprite.setPosition(booster.position);
	ariseSprite.setTexture(textures[TEXTURE_TYPE::BOOSTER][Default_Texture_Index]);
	ariseSprite.setScale(1.0f, 1.0f);
	window.draw(ariseSprite);
}
//we will remove it later
void DrawDebugGrid(RenderWindow& window) {
	RectangleShape rect(Vector2f(currentLevel->cellWidth, currentLevel->cellHeight));
	rect.setFillColor(Color::Transparent);
	rect.setOutlineThickness(1.0f);
	rect.setOutlineColor(Color(255, 255, 255, 100));

	for (int r = 0; r < gridRows; r++) {
		for (int c = 0; c < gridCols; c++) {
			rect.setPosition(Vector2f(currentLevel->offsetX + (c * currentLevel->cellWidth), currentLevel->offsetY + (r * currentLevel->cellHeight)));
			window.draw(rect);
		}
	}
}
void DrawPlantsGrid(RenderWindow& window) {
	//vector<Entity> newBullets; //temporary vector
	for (Entity& plant : entities) {
		if (plant.category != ENTITY_CATEGORY::PLANT) continue;

		// Animation
		if (!isGameOver && !isGameWon) plant.nextFrame(); // next frame
		plantSprite.setTextureRect(getFrameRect(plant)); // crop textures

		// Update plant - plant attacking system
		if (!plant.updateEntity) continue;
		plant.updateEntity(plant);

		// Update Sprite
		plantSprite.setPosition(plant.position);
		plantSprite.setTexture(textures[getTextureType(plant.type)][plant.currentAnimationIndex]);
		plantSprite.setScale(Vector2f(plant.scale, plant.scale));
		//hover for the plants
		if (isRelocating && selectedPlantIndex != -1 &&
			plant.gridRow == entities[selectedPlantIndex].gridRow &&
			plant.gridCol == entities[selectedPlantIndex].gridCol && !plant.isDead) {
			plantSprite.setColor(Color(200, 200, 200, 180));
		} else {
			plantSprite.setColor(Color::White);
		}
		// ----------------------------------------
		window.draw(plantSprite);
		plantSprite.setColor(Color::White);//normal highlight
		plantSprite.setScale(Vector2f(1.0f, 1.0f));//Normal Scale for the other plants
	}

	for (Entity& bullet : newBullets) {
		entities.push_back(bullet);
	}
	newBullets.clear();
}
// Returns true if zombie disappeared (finished death animation) , false otherwise (living or still dying)
bool healthDone_DyingZombie(Entity& zombie) {
	if (zombie.health <= 0) {
		//cout << "zombie health is " << zombie.health << endl;

		if (zombie.animations[1].frames == 0) zombie.isDead = true;	// if dying animation not set, just disappear

		// [1] health done but not started dying animation yet
		if (!zombie.isDead && (zombie.currentAnimationIndex == 0
			|| (zombie.type == ENTITY_TYPE::ZOMBIE_CONEHEAD && zombie.currentAnimationIndex == 2)
			|| (zombie.type == ENTITY_TYPE::ZOMBIE_NEWSPAPER && zombie.currentAnimationIndex == 2)
			|| (zombie.type == ENTITY_TYPE::ZOMBIE_ALLSTAR && zombie.currentAnimationIndex == 2)
			|| (zombie.type == ENTITY_TYPE::ZOMBIE_ALLSTAR && zombie.currentAnimationIndex == 3)
			|| (zombie.type == ENTITY_TYPE::ZOMBIE_RA && zombie.isUsingAbility))) {

			// for ra
			if (zombie.type == ENTITY_TYPE::ZOMBIE_RA && zombie.isUsingAbility) {
				for (int i = 0; i < MAX_SUNS; i++) {
					if (suns[i].ownerIndex == zombie.id) {
						suns[i].targeted = false;
						suns[i].ownerIndex = -1;
					}
				}
				zombie.isUsingAbility = false;
			}

			// [1.1] play dying animation once
			zombie.DoAnimation(1, 1);
			if (zombie.type == ENTITY_TYPE::ZOMBIE_RA) // for ra
				zombie.scale = 1.3f;


			// [1.2] After Death Animation
			zombie.afterAnimation = [&zombie](Entity& entity) {
				// (For ra) Ra drops stored suns after death
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
				// Zombie Dead 
				attackZombie(zombie, 10000, false); // kill without animation (already played)

				};
			zombie.x_vel = 0;
		}

		// [2] health done and animation finished, so we remove the zombie
		else if (zombie.isDead) {
			zombie.category = ENTITY_CATEGORY::NONE;
			return true;
		}
	}
	return false;
	// NOTE: Currently zombie disappears (dont draw) where[isDead==true] not where [health <= 0] because of the dying animation, but we might change it later
}


void checkZombieReachedHome(Entity& zombie) {}
void checkZombieHitByLawnmower(Entity& zombie) {
	// hit by lawnmower (car)
	auto& mwr = currentLevel->lawnmowers[zombie.gridRow];
	if (!mwr.isUsed && mwr.position.x >= zombie.position.x) {
		attackZombie(zombie, 100000, true);
		mwr.isMoving = true;
	}
}
void checkZombieEnteredHome(Entity& zombie) {
	if (zombie.enteredHome) {
		if (zombie.position.y < 525) {
			zombie.position.y += 0.075;
			if (zombie.position.y > 525) zombie.position.y = 525;
		} else if (zombie.position.y > 525) {
			zombie.position.y -= 0.075;
			if (zombie.position.y < 525) zombie.position.y = 525;
		} else {
			zombie.position.x -= zombie.x_vel;
		}
	}
}
// Update Zombies
void updateZombieRa(Entity& zombie) {
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
void updateZombieAllStars(Entity& zombie) {
	if (!zombie.hasTackled && !zombie.isTackling) {
		bool plantAhead = false;
		for (int i = 0; i < entities.size(); i++) {
			if (entities[i].category != ENTITY_CATEGORY::PLANT)
				continue;
			if (entities[i].isDead)
				continue;
			if (abs(entities[i].position.y - zombie.position.y) < 100.0f &&
				zombie.position.x - (entities[i].position.x + entities[i].width) <= eatDistance - 90) {
				plantAhead = true;
				break;
			}
		}

		if (plantAhead) {
			zombie.isTackling = true;
			zombie.x_vel = 0;
			zombie.DoAnimation(1, 3);
			zombie.afterAnimation = [](Entity& entity) {
				entity.isTackling = false;
				entity.hasTackled = true;
				entity.x_vel = 0.05f;
				entity.setCurrentAnimation(0);
				};
		}
	}

	if (zombie.isTackling) {
		if (zombie.currentAnimation.currentFrame >= 8) {
			for (int i = 0; i < entities.size(); i++) {
				if (entities[i].category != ENTITY_CATEGORY::PLANT)
					continue;
				if (entities[i].isDead)
					continue;
				if (abs(entities[i].position.y - zombie.position.y) < 100.0f &&
					zombie.position.x - (entities[i].position.x + entities[i].width) <= eatDistance - 90) {
					entities[i].health = 0;
					entities[i].category = ENTITY_CATEGORY::NONE;
					entities[i].isDead = true;
					break;
				}
			}
		}
	}
}
void updateNewspaperZombie(Entity& zombie) {
	if (zombie.health <= 100 && zombie.currentAnimationIndex == 0) {
		zombie.setCurrentAnimation(2);
		zombie.x_vel = 0.12f;
		zombie.attackTimer = 0.4f;
	}
}
void updateConeheadZombie(Entity& zombie) {
	if (zombie.health <= 100 && zombie.currentAnimationIndex == 0) {
		zombie.setCurrentAnimation(2);
	}
}

void zombieEatPlant(Entity& zombie, Entity& plant) {
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
}

void DrawZombies(RenderWindow& window) {
	for (Entity& zombie : entities) {
		if (zombie.category != ENTITY_CATEGORY::ZOMBIE)
			continue;

		if (zombie.freezeTimer > 0) {
			zombie.freezeTimer -= delta_time;
		}
		// lawnmowers
		checkZombieHitByLawnmower(zombie);

		// zombie dead - zombie dying
		bool disappearedDead = false;
		if (zombie.health <= 0) disappearedDead = healthDone_DyingZombie(zombie); // no need for condition but for clarification
		if (disappearedDead) continue;

		// zombie Animation
		if ((!isGameOver && !isGameWon) || zombie.enteredHome) zombie.nextFrame(); // next frame
		IntRect frameRect = getFrameRect(zombie);
		zombieSprite.setTextureRect(frameRect); // crop texture

		// call update function if exists
		if (zombie.updateEntity != nullptr) {
			zombie.updateEntity(zombie);
			continue;
		}

		// no internal update function
		if (zombie.type == ENTITY_TYPE::ZOMBIE_RA && zombie.health > 0) { // for ra
			updateZombieRa(zombie);
		}
		if (zombie.type == ENTITY_TYPE::ZOMBIE_CONEHEAD && zombie.health > 0) { // for conehead
			updateConeheadZombie(zombie);
		}
		if (zombie.type == ENTITY_TYPE::ZOMBIE_NEWSPAPER && zombie.health > 0) { // for newspaper
			updateNewspaperZombie(zombie);
		}
		if (zombie.type == ENTITY_TYPE::ZOMBIE_ALLSTAR && zombie.health > 0) { // for allstar
			updateZombieAllStars(zombie);
		}

		// zombie move
		if (!isGameOver && !isGameWon) zombie.position.x -= zombie.x_vel;

		// check enters home
		checkZombieEnteredHome(zombie);

		// touch plant
		auto [r, c] = gridCell(zombie.position);
		if (zombie.position.x > 0 + edgeSpace) {
			// Eat
			Entity& plant = getEntity({ r, c });
			if (plant.category == ENTITY_CATEGORY::PLANT) {	// touching a plant
				zombieEatPlant(zombie, plant);
			} else if (zombie.health > 0) {
				if (!(zombie.type == ENTITY_TYPE::ZOMBIE_RA && zombie.isUsingAbility)) { // for ra
					if (zombie.freezeTimer > 0) {
						zombie.x_vel = 0.02f;
					} else if (zombie.type == ENTITY_TYPE::ZOMBIE_NEWSPAPER && zombie.currentAnimationIndex == 2) { // for newspaper
						zombie.x_vel = 0.12f;
					} else if (zombie.type == ENTITY_TYPE::ZOMBIE_ALLSTAR && !zombie.hasTackled && !zombie.isTackling) {
						zombie.x_vel = 0.18f;
					} else {
						zombie.x_vel = .05; // no plant - has health - move normally
					}
				}
			}
		}

		int frameHeight = frameRect.getSize().y;
		float actualHeight = frameHeight * zombie.scale;
		Vector2f adjustedPosition = zombie.position;
		adjustedPosition.y -= (actualHeight - currentLevel->cellHeight);

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
				attackZombie(zombie, bullet.attack, true);

				if (bullet.type == ENTITY_TYPE::IceBullet) {
					zombie.freezeTimer = 3.0f;
					cout << "Zombie Slowed!" << endl;
				}

				bullet.category = ENTITY_CATEGORY::NONE;

				//cout << "Zombie hit! Health left: " << zombie.health << endl; //console just testing


				if (zombie.isDead) {
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
			bulletSprite.setScale(Vector2f(bullet.scale, bullet.scale));
			window.draw(bulletSprite);
		}
	}
}

void UpdateLawnmowers(RenderWindow& window) {
	if (currentLevel == nullptr || !currentLevel->lawnmowersEnabled) return;
	// draw lawnmowers - cars
	for (int r = 0; r < gridRows; r++) {
		if (currentLevel->lawnmowers[r].isUsed) continue;
		lawnmowerSprite.setPosition(currentLevel->lawnmowers[r].position);
		lawnmowerSprite.setScale(2.0f, 2.0f);

		window.draw(lawnmowerSprite);
	}

	for (auto& mwr : currentLevel->lawnmowers) {
		if (mwr.position.x >= lawnmower_endline) {
			mwr.isUsed = true;
			mwr.isMoving = false;
			mwr.position.x = -1000.0f;
		}
		if (mwr.isMoving) {
			mwr.move(0.2f);
		}
	}
}

void setupLevelLawnmowers(Level& level) {
	level.lawnmowersEnabled = true;
	for (int r = 0; r < gridRows; r++) {
		level.lawnmowers[r].setRow(r, 475, 170, 170);
	}
}

int t = 1, dz = 0, zmn, zmx;
void CheckTimer_AddZombie(RenderWindow& window) {
	if (currentLevel == nullptr) { cout << "Still lvl; unchosen, didnt add zombie\n"; return; }

	if (createdZombiesCount >= currentLevel->targetZombie) return;

	// waves
	if (dz >= currentLevel->waveLimit && t == 0) {
		// in to bet
		t = 1, waveClockX.restart();
		ZombieMinGapTime = zmn, ZombieMaxGapTime = zmx, currentLevel->waveLimit += currentLevel->waveLimitChange;
		cout << "switch to bet----------------------------\n";
	} else if (t == 1 && waveClockX.getElapsedTime().asSeconds() >= currentLevel->waveTimer) {
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

		newZombie.position = Vector2f(windowWidth, currentLevel->offsetY + (rand() % gridraws) * currentLevel->cellHeight);
		auto [raw, column] = gridCell(newZombie.position);
		newZombie.id = nextEntityId++; // for ra
		newZombie.gridRow = raw, newZombie.gridCol = column;
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

Sprite DrawSeedPacket(RenderWindow& window, Entity& plant, Vector2f pos, float scale = 1.0f, Color color = Color::White) {
	Sprite sprite;
	sprite.setPosition(pos);

	Texture& seedPacket = getTexture(TEXTURE_TYPE::SEEDPACKET);

	Vector2u size = Vector2u(Vector2f(seedPacket.getSize()) * scale);
	sprite.setTexture(seedPacket);
	resizeSpriteToTex(sprite, seedPacket);
	sprite.setScale({ scale, scale });

	Text text;
	text.setFont(*font);
	text.setString(std::to_string(sunsMoney));
	text.setFillColor(Color());
	text.setPosition(pos + Vector2f(43, 110) * scale);

	sprite.setColor(color);
	plantSprite.setColor(color);

	//Smart Crop [ Each Seed Packets ? ]
	Texture& tex = getDefaultEntityTexture(plant.type);
	int frameWidth = tex.getSize().x / plant.currentAnimation.frames;
	int frameHeight = tex.getSize().y;

	plantSprite.setTexture(tex);
	plantSprite.setTextureRect(IntRect{ 0, 0, frameWidth, frameHeight });

	plantSprite.setPosition(pos + Vector2f(13, 27) * scale);

	float scaleFactor = 74.0f / frameHeight;
	plantSprite.setScale(Vector2f(scaleFactor, scaleFactor) * scale);

	window.draw(sprite);
	window.draw(plantSprite);

	sprite.setScale(1, 1);

	text.setString(std::to_string(plant.price));
	text.setPosition(pos + Vector2f(13, 103) * scale);
	text.setCharacterSize(32 * scale);
	window.draw(text);

	return sprite;
}
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
	text.setString(std::to_string(sunsMoney));
	text.setFillColor(Color());
	text.setPosition(pos + Vector2f(32, 82));

	window.draw(text);

	Vector2u seedPacketSize = getTexture(TEXTURE_TYPE::SEEDPACKET).getSize();

	int removePlant = -1;

	int i = -1;
	for (auto& plant : currentLevel->selectedPlants) {
		i++;
		Vector2f spritePos = pos + Vector2f(115.0f + i * (seedPacketSize.x - 10.0f), 10.0f);

		Color packetColor = Color::White;
		if (sunsMoney < plant.price || plant.currentSeedCooldown > 0) {
			packetColor = Color(100, 100, 100, 255);
		}

		Sprite sprite = DrawSeedPacket(window, plant, spritePos, 0.75f, packetColor);

		if (plant.currentSeedCooldown > 0.0f && plant.seedCooldown > 0.0f) {
			float progress = plant.currentSeedCooldown / plant.seedCooldown;
			float h = sprite.getGlobalBounds().height * .75f;
			float w = sprite.getGlobalBounds().width * .75f;

			RectangleShape overlay(Vector2f(w, h * progress));
			overlay.setPosition(spritePos.x, spritePos.y + h * (1.0f - progress));
			overlay.setFillColor(Color(0, 0, 0, 150));
			window.draw(overlay);
		}

		if (sprite.getGlobalBounds().contains(Vector2f(mousePos))) {
			uiSprite.setColor(Color(128, 128, 128, 255));
			plantSprite.setColor(Color(128, 128, 128, 255));

			if (mouseJustClicked[0]) {
				uiInteracted = true;
				if (currentLevel->arePlantsSelected) {
					if (plant.currentSeedCooldown <= 0 && sunsMoney >= plant.price) {
						selectedPlant = i;
					}
				} else {
					removePlant = i;
				}
			}
		}
	}
	uiSprite.setColor(Color(255, 255, 255, 255));
	plantSprite.setColor(Color(255, 255, 255, 255));

	if (removePlant != -1) {
		currentLevel->selectedPlants.erase(currentLevel->selectedPlants.begin() + removePlant);
	}
}
void DrawSelectPlants(RenderWindow& window) {
	if (currentLevel->clock.getElapsedTime().asSeconds() < 3.75f) {
		return;
	}

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

	Vector2u seedPacketSize = getTexture(TEXTURE_TYPE::SEEDPACKET).getSize();

	int i = -1;
	for (auto& plant : currentLevel->plant_types) {
		if (!isEntityUnlocked[plant.type]) continue;

		i++;
		Vector2f spritePos = Vector2f(50.0f, 190.0f) + Vector2f((i % 7) * (seedPacketSize.x - 10.0f), 10.0f + (i / 7) * 130.0f);

		Vector2u size = Vector2u(Vector2f(seedPacketSize) * 0.75f);

		bool disabled = false;

		for (auto& selectedPlant : currentLevel->selectedPlants) {
			if (selectedPlant.type == plant.type) {
				disabled = true;
				break;
			}
		}

		Color color = Color::White;
		if (disabled) {
			color = Color(64, 64, 64, 255);
		}

		Sprite sprite = DrawSeedPacket(window, plant, spritePos, 0.75f, color);

		if (!disabled && mouseJustClicked[0] && currentLevel->selectedPlants.size() < 5 && sprite.getGlobalBounds().contains(Vector2f(mousePos))) {
			uiInteracted = true;
			currentLevel->selectedPlants.push_back(plant);
		}
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
			currentLevel->plantsSelectedTime = currentLevel->clock.getElapsedTime();
		}
	}

	window.draw(startButton);

	text.setString("Start Game");
	text.setPosition(startButton.getPosition() + Vector2f(40, 8.0f));
	text.setScale(Vector2f(1.5f, 1.5f));
	text.setFillColor(Color(255, 255, 255, 128));
	text.setCharacterSize(24);

	window.draw(text);
}
void DrawRewardsScreen(RenderWindow& window) {
	Text text;
	text.setFont(*font);
	text.setString("You unlocked a new plant!");
	text.setFillColor(Color(255, 255, 255, 255));
	text.setPosition(Vector2f(960, 130.0f));
	text.setCharacterSize(64);
	text.setOrigin(text.getLocalBounds().getSize() / 2.0f);

	window.draw(text);

	Vector2u seedPacketSize = getTexture(TEXTURE_TYPE::SEEDPACKET).getSize();

	float diff = currentLevel->clock.getElapsedTime().asSeconds() - currentLevel->winTime.asSeconds();
	float progress = (diff - 1) / 0.5f;
	if (progress > 1) progress = 1;
	float scale = 2.0f * progress;

	Sprite packetBg;
	Texture& packetBgTex = getTexture(TEXTURE_TYPE::LUSTER);
	packetBg.setTexture(packetBgTex);
	packetBg.setPosition(Vector2f(960, 540));
	packetBg.setScale(Vector2f(scale, scale));
	packetBg.setOrigin(Vector2f(packetBgTex.getSize()) / 2.0f);
	Vector2f texSize = Vector2f(packetBgTex.getSize());
	Vector2f size = Vector2f(512, 512) * progress + Vector2f(32, 32) * sin(diff * 1.57f);
	packetBg.setScale(Vector2f(size.x / texSize.x, size.y / texSize.y));

	packetBg.setRotation(diff * 3.14f);

	window.draw(packetBg);

	DrawSeedPacket(window, plants[currentLevel->plantReward.plantType], Vector2f(960, 540) - Vector2f(seedPacketSize) * scale / 2.0f, scale);

	Sprite startButton;

	startButton.setTexture(getTexture(TEXTURE_TYPE::SEED_CHOOSER_BUTTON));
	startButton.setScale(Vector2f(1.5f, 1.5f));
	startButton.setPosition(Vector2f(843, 900));

	if (currentLevel->selectedPlants.size() == 0) {
		startButton.setColor(Color(64, 64, 64, 255));
	} else if (startButton.getGlobalBounds().contains(Vector2f(mousePos))) {
		startButton.setColor(Color(128, 128, 128, 255));
		if (mouseJustClicked[0]) {
			currentLevel->claimRewards();
			if (currentLevel->lvlNum < number_levels - 1)
				levels[currentLevel->lvlNum + 1].isUnlocked = true; // unlock next level

			//for (auto& r : currentLevel->rewards) {
			//	if (r.type == Reward::None) continue;
			//	r.isClaimed = true;	// maybe when click the claim button
			//	if (r.type == Reward::Plant) cout << "- Won Plant " << int(r.plantType) << '\n';
			//	else if (r.type == Reward::Coins) cout << "- Won " << r.coins << " Coins\n";
			//}

			nextLevel();
		}
	}

	window.draw(startButton);

	text.setString("Next Level");
	text.setPosition(startButton.getPosition() + Vector2f(40, 8.0f));
	text.setScale(Vector2f(1.5f, 1.5f));
	text.setFillColor(Color(255, 255, 255, 128));
	text.setCharacterSize(24);
	text.setOrigin({ 0, 0 });

	window.draw(text);
}
void DrawWinScreen(RenderWindow& window) {
	if (currentLevel->winTime == Time::Zero) {
		currentLevel->winTime = currentLevel->clock.getElapsedTime();
	}

	float diff = currentLevel->clock.getElapsedTime().asSeconds() - currentLevel->winTime.asSeconds();
	if (diff < 1) {
		float progress = diff / 1.0f;
		if (progress < 0) progress = 0;
		if (progress > 1) progress = 1;

		int c = 255 - (int)(128 * progress);
		ground.setColor(Color(c, c, c, 255));
		zombieSprite.setColor(Color(c, c, c, 255));
		plantSprite.setColor(Color(c, c, c, 255));
	} else {
		DrawRewardsScreen(window);
	}

}

// Screen Animations
void screenAnimation(RenderWindow& window) {
	int actualTexWidth = ground.getTexture()->getSize().x;
	int actualTexHeight = ground.getTexture()->getSize().y;
	//zoom_scale
	float scaleY = (float)windowHeight / actualTexHeight;
	float scaleX = scaleY;
	ground.setScale({ scaleX, scaleY });
	int visibleWidth = windowWidth / scaleX;
	int maxPan = std::max(0, actualTexWidth - visibleWidth);
	if (currentLevel->clock.getElapsedTime().asSeconds() < 1) {
		ground.setTextureRect(IntRect(0, 0, visibleWidth, actualTexHeight));
		ground.setColor(Color(255, 255, 255, (int)(255 * currentLevel->clock.getElapsedTime().asSeconds())));
	} else if (currentLevel->clock.getElapsedTime().asSeconds() < 3.75f) {
		float progress = (currentLevel->clock.getElapsedTime().asSeconds() - 1.5f) / 2.0f;
		if (progress < 0) progress = 0;
		if (progress > 1) progress = 1;

		ground.setTextureRect(IntRect(maxPan * progress, 0, visibleWidth, actualTexHeight));
	} else if (!currentLevel->arePlantsSelected) {
		ground.setTextureRect(IntRect(maxPan, 0, visibleWidth, actualTexHeight));
	} else if (currentLevel->clock.getElapsedTime().asSeconds() - currentLevel->plantsSelectedTime.asSeconds() < 2) {
		float progress = (currentLevel->clock.getElapsedTime().asSeconds() - currentLevel->plantsSelectedTime.asSeconds()) / 2.0f;
		if (progress < 0) progress = 0;
		if (progress > 1) progress = 1;

		ground.setTextureRect(IntRect(maxPan - (maxPan * progress), 0, visibleWidth, actualTexHeight));
	}
}
void gameOverAnimation(RenderWindow& window) {
	gameOverScreen.setFillColor(Color(255, 255, 255, 0));
	gameOverScreen.setScale({ 0, 0 });

	if (currentLevel->clock.getElapsedTime().asSeconds() - currentLevel->gameOverTime.asSeconds() > 1) {
		float progress = (currentLevel->clock.getElapsedTime().asSeconds() - currentLevel->gameOverTime.asSeconds() - 1) / 2.0f;
		if (progress < 0) progress = 0;
		if (progress > 1) progress = 1;

		int c = 255 - (int)(128 * progress);
		ground.setColor(Color(c, c, c, 255));
		zombieSprite.setColor(Color(c, c, c, 255));
		plantSprite.setColor(Color(c, c, c, 255));
	}
	if (currentLevel->clock.getElapsedTime().asSeconds() - currentLevel->gameOverTime.asSeconds() > 2.5) {
		float progress = (currentLevel->clock.getElapsedTime().asSeconds() - currentLevel->gameOverTime.asSeconds() - 2.5) / 0.75f;
		if (progress < 0) progress = 0;
		if (progress > 1) progress = 1;

		gameOverScreen.setFillColor(Color(255, 255, 255, (int)(255 * progress)));
		gameOverScreen.setScale({ progress, progress });
	}

	if (currentLevel->clock.getElapsedTime().asSeconds() - currentLevel->gameOverTime.asSeconds() > 7.5f) {
		float progress = (currentLevel->clock.getElapsedTime().asSeconds() - currentLevel->gameOverTime.asSeconds() - 7.5) / 1.0f;
		if (progress < 0) progress = 0;
		if (progress > 1) progress = 1;

		int c = 255 - (int)(255 * progress);
		gameOverScreen.setFillColor(Color(c, c, c, 255));

		c = 128 - (int)(128 * progress);
		ground.setColor(Color(c, c, c, 255));
		zombieSprite.setColor(Color(c, c, c, 255));
		plantSprite.setColor(Color(c, c, c, 255));
	}
	if (currentLevel->clock.getElapsedTime().asSeconds() - currentLevel->gameOverTime.asSeconds() > 8.5f) {
		resetLevel();
		currentLevel = nullptr;
		return;
	}
}

// UI interaction parts
void uiNormalShovel(RenderWindow& window, Entity& clickedEntity) {
	if (clickedEntity.category == ENTITY_CATEGORY::PLANT) {
		clickedEntity.category = ENTITY_CATEGORY::NONE;
		clickedEntity.isDead = true;
		isShovelActive = false;
	}
}
void uiMagicShovel(RenderWindow& window, Entity& clickedEntity, int row, int col) {
	if (!isRelocating && clickedEntity.category == ENTITY_CATEGORY::PLANT) {
		for (int i = 0; i < entities.size(); i++) {
			if (entities[i].category == ENTITY_CATEGORY::PLANT &&
				entities[i].gridRow == row && entities[i].gridCol == col && !entities[i].isDead) {
				selectedPlantIndex = i;
				isRelocating = true;
				break;
			}
		}
	}

	else if (isRelocating) {
		if (clickedEntity.category == ENTITY_CATEGORY::NONE) {
			// if it is possible
			entities[selectedPlantIndex].gridRow = row;
			entities[selectedPlantIndex].gridCol = col;

			int x = currentLevel->offsetX + (col * currentLevel->cellWidth) + ((currentLevel->cellWidth - entities[selectedPlantIndex].width) / 2);
			int y = currentLevel->offsetY + (row * currentLevel->cellHeight) + (currentLevel->cellHeight - entities[selectedPlantIndex].height);
			entities[selectedPlantIndex].position = Vector2f(x, y);


			isRelocating = false;
			selectedPlantIndex = -1;
			isRelocateActive = false;
		} else {
			isRelocating = false;
			selectedPlantIndex = -1;
		}
	}
}
void uiBoostPlant(RenderWindow& window, Entity& clickedEntity) {
	//curBoosterCnt--;
	isBoostActive = false;
	booster.activate(clickedEntity);
	cout << "Booster Activated! Remaining Boosters: " << curBoosterCnt << endl;
	/*clickedEntity.boosted = true;
	clickedEntity.boostStartTime = currentLevel->clock.getElapsedTime();
					isBoostActive = false;*/
}
void uiNormalMode(RenderWindow& window, Entity& clickedEntity, int row, int col) {
	Entity plant;
	if (sunsMoney >= currentLevel->selectedPlants[selectedPlant].price && currentLevel->selectedPlants[selectedPlant].currentSeedCooldown <= 0) {
		plant = currentLevel->selectedPlants[selectedPlant];

		plant.gridRow = row;
		plant.gridCol = col;

		int x = currentLevel->offsetX + (col * currentLevel->cellWidth) + ((currentLevel->cellWidth - plant.width) / 2);
		int y = currentLevel->offsetY + (row * currentLevel->cellHeight) + (currentLevel->cellHeight - plant.height);

		plant.position = Vector2f(x, y);
		entities.push_back(plant);
		sunsMoney -= plant.price;
		currentLevel->selectedPlants[selectedPlant].currentSeedCooldown = plant.seedCooldown;
	}
}
void uiCollectSuns(RenderWindow& window) {
	for (int i = 0; i < MAX_SUNS; i++) {
		// Check all suns + Add clicked suns to money
		Sun& sun = suns[i];
		if (!isSunAlive(sun)) continue;

		// check mouse pos in sun rect (200x200)
		if (mousePos.x >= sun.position.x && mousePos.x <= sun.position.x + 200 && mousePos.y >= sun.position.y && mousePos.y <= sun.position.y + 200) {
			sunsMoney += suns[i].sunValue;

			cout << "You now have " << sunsMoney << endl;

			suns[i].clicked = true;
			suns[i].targeted = false; // for ra
			uiInteracted = true;
			break;
		}
	}
}

//	UI interaction - Buttons click, Collect Items, Click plants ... etc
void UI_DrawClick(RenderWindow& window) {
	//Normal_Shovel
	Texture& shovelTex = getTexture(TEXTURE_TYPE::SHOVEL);
	uiSprite.setTexture(shovelTex);
	resizeSpriteToTex(uiSprite, shovelTex);
	uiSprite.setScale(Vector2f(0.8f, 0.8f));
	uiSprite.setPosition(700.0f, 20.0f);

	if (isShovelActive) uiSprite.setColor(Color(150, 150, 150));
	else uiSprite.setColor(Color::White);

	if (uiSprite.getGlobalBounds().contains(Vector2f(mousePos))) {
		if (mouseJustClicked[0]) {
			isShovelActive = !isShovelActive;
			uiInteracted = true;
		}
	}
	window.draw(uiSprite);
	uiSprite.setColor(Color::White);
	uiSprite.setScale(Vector2f(1.0f, 1.0f)); // return to normal scale for next use

	// Magic_Shovel
	Texture& relocateTex = getTexture(TEXTURE_TYPE::MAGIC_SHOVEL);
	uiSprite.setTexture(relocateTex);
	resizeSpriteToTex(uiSprite, relocateTex);
	uiSprite.setScale(Vector2f(0.09f, 0.09f));
	uiSprite.setPosition(780.0f, 20.0f);

	if (isRelocateActive) uiSprite.setColor(Color(150, 150, 150));
	else uiSprite.setColor(Color::White);

	if (uiSprite.getGlobalBounds().contains(Vector2f(mousePos))) {
		if (mouseJustClicked[0]) {
			isRelocateActive = !isRelocateActive;
			isShovelActive = false; //NORMAL SHOVEL AND MAGIC SHOVEL DON'T MIX

			if (!isRelocateActive) {
				isRelocating = false;
				selectedPlantIndex = -1;
			}
			uiInteracted = true;
		}
	}
	window.draw(uiSprite);
	uiSprite.setColor(Color::White);
	uiSprite.setScale(Vector2f(1.0f, 1.0f)); // return to normal scale for next use
}
void UI_Interaction(RenderWindow& window) {
	if (!uiInteracted && mouseJustClicked[0]) {
		// Collect Suns
		uiCollectSuns(window);
		// Collect booster
		if (booster.isAlive && !uiInteracted) {
			if (mousePos.x >= booster.position.x && mousePos.x <= booster.position.x + 200 && mousePos.y >= booster.position.y && mousePos.y <= booster.position.y + 200) {
				booster.collect();
				uiInteracted = true;
			}
		}
		if (!uiInteracted) {
			// clicked item on grid
			auto [row, col] = gridCell(mousePos);
			if (row >= 0 && row < gridRows && col >= 0 && col < gridCols) {
				Entity& clickedEntity = getEntity({ row , col });
				//nomral_shovel
				if (isShovelActive && mouseJustClicked[0])
					uiNormalShovel(window, clickedEntity);
				// Magic Shovel
				else if (isRelocateActive && mouseJustClicked[0])
					uiMagicShovel(window, clickedEntity, row, col);
				// Boost Plant
				else if (isBoostActive && mouseJustClicked[0] && clickedEntity.category == ENTITY_CATEGORY::PLANT)
					uiBoostPlant(window, clickedEntity);
				//normal mode
				else if (!isShovelActive && !isRelocateActive && mouseJustClicked[0] && clickedEntity.category == ENTITY_CATEGORY::NONE)
					uiNormalMode(window, clickedEntity, row, col);
			}
		}
	}

}

void UpdateLevel(RenderWindow& window) {
	if (currentLevel == nullptr) return;
	// Drawing
	window.clear(Color::Black);
	// 
	screenAnimation(window);

	zombieSprite.setColor(Color::White);
	plantSprite.setColor(Color::White);

	if (isGameOver) {
		gameOverAnimation(window);
	}

	if (currentLevel == nullptr) return;
	int level = currentLevel->lvlNum;
	ground.setTexture(textures[TEXTURE_TYPE::LEVEL_BACKGROUND][level]);
	window.draw(ground);

	if (isGameOver) {
		ground.setTexture(textures[TEXTURE_TYPE::LEVEL_BACKGROUND_OPENDOOR][level]);
		window.draw(ground);
	}

	DrawZombies(window);
	DrawPlantsGrid(window);

	if (isGameOver) {
		ground.setTexture(textures[TEXTURE_TYPE::LEVEL_BACKGROUND_HOUSE][level]);
		window.draw(ground);
	}

	if (isGameOver == true) {
		window.draw(gameOverScreen);
		return;
	}
	if (isGameWon == true) {
		// rewards (debug)
		//cout << "You win!\n";
		DrawWinScreen(window);
		return;
	}
	if (currentLevel == nullptr) return;

	// Plant Selection Screen
	if (!currentLevel->arePlantsSelected) {
		DrawSelectPlants(window);

		return;
	} else if (currentLevel->clock.getElapsedTime().asSeconds() - currentLevel->plantsSelectedTime.asSeconds() < 2) {
		return;
	}

	// Level Update
	CheckTimer_AddZombie(window);
	CheckTimer_AddSun(window);
	CheckTimer_AddBooster(false);

	for (auto& plant : currentLevel->selectedPlants) {
		if (plant.currentSeedCooldown > 0.0f) {
			plant.currentSeedCooldown -= delta_time;
			if (plant.currentSeedCooldown < 0.0f) {
				plant.currentSeedCooldown = 0.0f;
			}
		}
	}

	UpdateLawnmowers(window);
	UpdateAndDrawBullets(window);
	UpdateSuns(window);
	UpdateBooster(window);
	DrawSeeds(window);
	DrawDebugGrid(window);


	UI_DrawClick(window);
	UI_Interaction(window);

	for (Entity& zombie : entities) {
		if (zombie.category == ENTITY_CATEGORY::ZOMBIE)//zombie and not dead
		{
			if (!zombie.isDead && zombie.position.x <= leftEdgeForZombies && (!currentLevel->lawnmowersEnabled || currentLevel->lawnmowers[zombie.gridRow].isUsed))//end of the screen
			{
				currentLevel->gameOverTime = currentLevel->clock.getElapsedTime();
				isGameOver = true;
				zombie.enteredHome = true;
				cout << "Zombies ate your brains!\n";
				break;
			}
		}
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
		if (!levels[i + 1].isUnlocked) continue;	// skip locked levels

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

		sprite.setTexture(textures[TEXTURE_TYPE::LEVEL_BACKGROUND][i + 1]);
		sprite.setTextureRect({ 0, 200, 370, 320 });
		sprite.setScale(scale / 1.5f);
		sprite.setPosition(pos + Vector2f(0, -32));
		if (i + 1 == 2) {
			sprite.setTextureRect({ 200, 500, 400, 375 });
			sprite.setScale(scale / 1.75f);
			sprite.setPosition(pos + Vector2f(-8, -46));
		}
		window.draw(sprite);

		sprite.setTextureRect({ 0, 0, (int)levelTexSize.x, (int)levelTexSize.y });
		sprite.setTexture(levelTex);
		sprite.setPosition(pos);
		sprite.setScale(scale);
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

// File
// unlocked plants, coins
void loadStorage() {
	// load from file
	ifstream file("storage.txt");
	if (!file.is_open()) {
		cout << "No storage file found, starting fresh.\n";
		return;
	}
	string line;
	while (getline(file, line)) {
		string msg = "Unlocked:";
		if (line.find(msg) == 0) {
			int typeInt = stoi(line.substr(msg.size()));
			isEntityUnlocked[ENTITY_TYPE(typeInt)] = true;
		}
		msg = "Level:";
		if (line.find(msg) == 0) {
			int lvlNum = stoi(line.substr(msg.size()));
			getline(file, line);
			bool isUnlocked = line.substr(line.find(":") + 1) == "1";
			getline(file, line);
			bool isPlantRewardClaimed = line.substr(line.find(":") + 1) == "1";
			getline(file, line);
			bool isCoinRewardClaimed = line.substr(line.find(":") + 1) == "1";

			levels[lvlNum].isUnlocked = isUnlocked;
			levels[lvlNum].plantReward.isClaimed = isPlantRewardClaimed;
			levels[lvlNum].coinReward.isClaimed = isCoinRewardClaimed;
		}
		msg = "Current Coins:";
		if (line.find("Current Coins:") == 0) {
			coinsMoney = stoi(line.substr(msg.size()));
		}
	}
	file.close();
}
void saveStorage() {
	// save to file
	ofstream file("storage.txt");
	if (!file.is_open()) {
		cout << "Failed to open storage file for writing.\n";
		return;
	}
	file << "Unlocked Plants:\n";
	for (int i = 0; i < int(ENTITY_TYPE::ENTITY_TYPE_COUNT); i++) {
		ENTITY_TYPE type = ENTITY_TYPE(i);
		if (isEntityUnlocked[type]) {
			file << "Unlocked:" << i << "\n";
		}
	}
	file << "\n";
	file << "Current Coins:" << coinsMoney << "\n";
	file << "\n";
	for (int lvl = 1; lvl <= number_levels; lvl++) {
		file << "Level:" << lvl << "\n";
		file << "Unlocked:" << (levels[lvl].isUnlocked ? "1" : "0") << "\n";
		file << "PlantRewardClaimed:" << (levels[lvl].plantReward.isClaimed ? "1" : "0") << "\n";
		file << "CoinRewardClaimed:" << (levels[lvl].coinReward.isClaimed ? "1" : "0") << "\n";
		file << "\n";
	}
	file.close();
}

int select_levels_background_music(Music& music) {
	if (!music.openFromFile("assets/audio/select-levels-music.ogg")) {
		std::cout << "Error loading file!" << std::endl;
		return -1;
	}
	music.setLoop(true);
	music.play();
	return 0;
}

int main() {
	Music music;	// can't be global - OpenAL must be loaded first
	RenderWindow window(VideoMode(windowWidth, windowHeight), "Plants vs Zombies");
	Event event;
	// Characters
	Texture groundTex, lawnTex;
	ground = defineGroundSprite(groundTex);
	lawnmowerSprite = defineLawnmowerSprite(lawnTex);
	//New

	// Loading Textures + Defining entities

	// Plants
	Entity normalPlant = defineNormalPlant();
	Entity repeaterPlant = defineRepeaterPlant();
	Entity shroom = defineShroomPlant();
	Entity sunflower = defineSunflower();
	Entity wallnut = defineWallnut();
	Entity chomper = defineChomperPlant();
	Entity sunshroom = defineSunShroom();
	Entity icePeashooter = defineIcePeaShooter();
	Entity miniShroom = defineMiniShroom();
	Entity POTATOMINE = definePOTATO_MINE();
	Entity doomShroom = defineDOOM_SHROOM();
	Entity CHEERY_BOMB = defineCherryBomb();

	// Zombies
	Entity zombie = defineNormalZombie();
	Entity zombieJapanese = defineJapaneseZombie();
	Entity zombieBig = defineBigZombie();
	Entity zombieTorch = defineTorchlightZombie();
	Entity zombieRa = defineRaZombie();
	Entity zombieMummy = defineMummyZombie();
	Entity zombieConehead = defineConeheadZombie();
	Entity zombieNewspaper = defineNewspaperZombie();
	Entity zombieAllStar = defineAllStarZombie();

	// Other Textures
	textures[TEXTURE_TYPE::LOGO][Default_Texture_Index].loadFromFile("assets/logo.png");
	textures[TEXTURE_TYPE::LEVEL_SELECTOR_BACKGROUND][Default_Texture_Index].loadFromFile("assets/level_selector_background.png");
	textures[TEXTURE_TYPE::LEVEL_OPTION_BACKGROUND][Default_Texture_Index].loadFromFile("assets/level_option_background.png");
	textures[TEXTURE_TYPE::LEVEL_PLAY_BUTTON][Default_Texture_Index].loadFromFile("assets/play_button.png");
	textures[TEXTURE_TYPE::LUSTER][Default_Texture_Index].loadFromFile("assets/luster.png");
	textures[TEXTURE_TYPE::SEED_CHOOSER][Default_Texture_Index].loadFromFile("assets/seedchooser.png");
	textures[TEXTURE_TYPE::SEED_CHOOSER_BUTTON][Default_Texture_Index].loadFromFile("assets/seedchooser_button.png");
	textures[TEXTURE_TYPE::SUN][Default_Texture_Index].loadFromFile("assets/Sun3.png");
	textures[TEXTURE_TYPE::SEEDBANK][Default_Texture_Index].loadFromFile("assets/seedbank.png");
	textures[TEXTURE_TYPE::SEEDPACKET][Default_Texture_Index].loadFromFile("assets/seedpacket.png");
	textures[TEXTURE_TYPE::SHOVEL][Default_Texture_Index].loadFromFile("assets/shovel.png");
	textures[TEXTURE_TYPE::MAGIC_SHOVEL][Default_Texture_Index].loadFromFile("assets/relocate.png");
	textures[TEXTURE_TYPE::BOOSTER][Default_Texture_Index].loadFromFile("assets/booster.png");

	// Font
	font = new Font();
	font->loadFromFile("assets/impact.ttf");

	// General Plants to any level

	vector<Entity> mainPlantTypes = { sunflower, repeaterPlant, normalPlant, wallnut, chomper, icePeashooter, miniShroom, POTATOMINE, doomShroom, CHEERY_BOMB };//, shroom, sunshroom };

	// Unlock starting plants
	isEntityUnlocked[repeaterPlant.type] =
		isEntityUnlocked[normalPlant.type] =
		isEntityUnlocked[sunflower.type] =
		isEntityUnlocked[icePeashooter.type] =
		isEntityUnlocked[chomper.type] =
		isEntityUnlocked[miniShroom.type] =
		isEntityUnlocked[POTATOMINE.type] =
		isEntityUnlocked[doomShroom.type] =
		isEntityUnlocked[CHEERY_BOMB.type] =
		true;

	// Levels
//---------------------------------------
// Level 1: Day
	levels[1].lvlNum = 1;
	levels[1].plant_types = mainPlantTypes;
	levels[1].zombie_types = { zombie, zombieConehead };
	levels[1].targetZombie = 20;
	//levels[1].targetZombie = 2;
	levels[1].waveLimit = 10; levels[1].waveTimer = 15;
	levels[1].plantReward = chomper;
	levels[1].coinReward = 100;
	levels[1].isUnlocked = true;

	levels[1].offsetX = 450;
	levels[1].offsetY = 135;
	levels[1].cellWidth = 150;
	levels[1].cellHeight = 170;
	setupLevelLawnmowers(levels[1]);
	//---------------------------------------
	// Level 2: Egypt
	levels[2].lvlNum = 2;
	levels[2].plant_types = mainPlantTypes;
	levels[2].plant_types.push_back(shroom);
	levels[2].zombie_types = { zombieTorch, zombieRa, zombieMummy };
	levels[2].targetZombie = 25;
	//levels[2].targetZombie = 2;
	levels[2].addReward(500);
	levels[2].addReward(wallnut);

	levels[2].offsetX = 450;
	levels[2].offsetY = 135;
	levels[2].cellWidth = 150;
	levels[2].cellHeight = 170;
	setupLevelLawnmowers(levels[2]);



	//------------------------------------------
	// Level 3: Night
	levels[3].lvlNum = 3;
	levels[3].plant_types = mainPlantTypes;
	levels[3].zombie_types = { zombie, zombieConehead, zombieNewspaper, zombieAllStar };
	levels[3].startingSuns = 100;
	levels[3].offsetX = 450;
	levels[3].offsetY = 135;
	levels[3].cellWidth = 150;
	levels[3].cellHeight = 170;
	setupLevelLawnmowers(levels[3]);



	// setting levels background textures
	textures[TEXTURE_TYPE::LEVEL_BACKGROUND][0].loadFromFile("assets/levels/level1.png");
	textures[TEXTURE_TYPE::LEVEL_BACKGROUND_HOUSE][0].loadFromFile("assets/levels/level1_house.png");
	for (int lvl = 1; lvl <= number_levels; lvl++) {
		string path = "assets/levels/level" + to_string(lvl) + ".png";
		string housePath = "assets/levels/level" + to_string(lvl) + "_house.png";
		string opendoorPath = "assets/levels/level" + to_string(lvl) + "_opendoor.png";
		cout << "Loading level " << lvl << " files" << endl;
		levels[lvl].lvlNum = lvl;
		textures[TEXTURE_TYPE::LEVEL_BACKGROUND][lvl].loadFromFile(path);
		textures[TEXTURE_TYPE::LEVEL_BACKGROUND_HOUSE][lvl].loadFromFile(housePath);
		textures[TEXTURE_TYPE::LEVEL_BACKGROUND_OPENDOOR][lvl].loadFromFile(opendoorPath);
		cout << "Loaded level " << lvl << " files" << endl;
	}


	//losing state

	Texture gameOverTex;
	gameOverTex.loadFromFile("assets/gameover.png");
	gameOverScreen.setSize(Vector2f(800.0f, 664.0f));
	gameOverScreen.setTexture(&gameOverTex);
	gameOverScreen.setOrigin(400.0f, 332.0f);
	gameOverScreen.setPosition(Vector2f(windowWidth / 2.0, windowHeight / 2.0));

	Texture gameWonTex;
	gameWonTex.loadFromFile("assets/gamewon.png");
	gameWonScreen.setSize(Vector2f(800.0f, 664.0f));
	gameWonScreen.setTexture(&gameWonTex);
	gameWonScreen.setOrigin(250.0f, 350.0f);
	gameWonScreen.setPosition(Vector2f(windowWidth / 2.0, windowHeight / 2.0));

	// Load storage
	loadStorage();


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
			} else if (event.type == sf::Event::MouseButtonPressed) {
				if (event.mouseButton.button == sf::Mouse::Left) mouseJustClicked[0] = true;
				else if (event.mouseButton.button == sf::Mouse::Right) mouseJustClicked[1] = true; //edit_bug
			} else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F) {
				// FOR TESTING ONLY
				sunsMoney += 1000;
			} else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::B) {
				// FOR TESTING ONLY
				if (curBoosterCnt) isBoostActive = true;
				cout << "Booster Pressed! Waiting to click plant" << endl;
			}
		}

		// Separate UI -> Main Menu and Level
		if (currentLevel != nullptr) {
			if (music.getStatus() == Music::Playing) music.stop();
			UpdateLevel(window);
		} else {
			if (music.getStatus() != Music::Playing) select_levels_background_music(music);
			DrawMainMenu(window);
		}

		window.display();
	}

	delete font;
	textures.clear();
	saveStorage();

	return 0;
}