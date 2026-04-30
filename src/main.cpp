#include <iostream>
#include <vector>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <functional>
#include <cmath>
#include <fstream>
#include <queue>

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

float screenFlashAlpha = 0.0f;
RectangleShape screenFlashOverlay;

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
lawnmower_endline = windowWidth; // which x pos to stop at
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
Clock gameClock;
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
	ZOMBIE_PHARAOH,
	ZOMBIE_SHIELDGUARD,

	PLANT_NORMAL,
	PLANT_REPEATER,
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
	PLANT_ICESHROOM,

	NormalBullet,
	IceBullet,
	ShroomBullet,



	ENTITY_TYPE_COUNT	// keep it last
};
enum class TEXTURE_TYPE : uint8_t {
	NONE = 0,
	LOGO,
	GREEN_BUTTON,
	WOOD_BUTTON,
	MAINMENU_BACKGROUND,
	STORE_WINDOW,
	LEVEL_SELECTOR_BACKGROUND,
	LEVEL_SELECTOR_BACKGROUND_BACK,
	LEVEL_OPTION_BACKGROUND,
	LEVEL_BACKGROUND,
	LEVEL_BACKGROUND_DIRT,
	LEVEL_BACKGROUND_HOUSE,
	LEVEL_BACKGROUND_OPENDOOR,
	LEVEL_PLAY_BUTTON,
	LEVEL_ICON,
	GRID_ROLL,
	LUSTER,
	SEED_CHOOSER,
	SEED_CHOOSER_BUTTON,
	SUN,
	COIN,
	SEEDBANK,
	SEEDPACKET,
	BOOSTER,

	PROGRESS_EMPTY,
	PROGRESS_FULL,
	PROGRESS_HEAD,

	ITEM_CONE,
	ITEM_NEWSPAPER,
	ITEM_PHAROAH_CASKET,

	ZOMBIE_NORMAL,
	ZOMBIE_JAPANESE,
	ZOMBIE_BIG,
	ZOMBIE_TORCHLIGHT,
	ZOMBIE_RA,
	ZOMBIE_MUMMY,
	ZOMBIE_CONEHEAD,
	ZOMBIE_NEWSPAPER,
	ZOMBIE_ALLSTAR,
	ZOMBIE_PHARAOH,
	ZOMBIE_SHIELDGUARD,

	PLANT_NORMAL,
	PLANT_REPEATER,
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
	PLANT_ICESHROOM,

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
	case ENTITY_TYPE::ZOMBIE_PHARAOH:
		textureType = TEXTURE_TYPE::ZOMBIE_PHARAOH;
		break;
	case ENTITY_TYPE::ZOMBIE_SHIELDGUARD:
		textureType = TEXTURE_TYPE::ZOMBIE_SHIELDGUARD;
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
	case ENTITY_TYPE::PLANT_ICESHROOM:
		textureType = TEXTURE_TYPE::PLANT_ICESHROOM;
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
	ENTITY_CATEGORY category = ENTITY_CATEGORY::Bullet;
	Vector2f position;
	Vector2f velocity;
	int attack = 0;
	int idx = -1;
	int x_vel = 0.05f;
	// type
	// Animation

	/*ENTITY_TYPE type;
	&Entity sender;
	bool isActive = false;
	void updatePosition(float deltaTime) {
		position += velocity * deltaTime;
	}*/
};


//queue<int> emptyBulletIdx;
//void addBullet(Bullet bullet) {
//	if (emptyBulletIdx.empty()) {
//		bullet.idx = bullets.size();
//		bullets.push_back(bullet);
//	}
//	else {
//		bullets[emptyBulletIdx.front()] = bullet;
//		emptyBulletIdx.pop();
//	}
//}
//void removeBullet(Entity bullet) {
//	emptyBulletIdx.push(bullet.idx);
//	bullets[bullet.idx].category = ENTITY_CATEGORY::NONE;	// reset the bullet at that index
//	bullets[bullet.idx].idx = -1;	// reset the bullet at that index
//}


struct Entity {
	ENTITY_CATEGORY category;
	ENTITY_TYPE type;
	int health = 100;
	int attack = 15;
	int price = 25;

	Time lastDamaged = Time::Zero;
	int targetId = -1; //for the chomper

	float seedCooldown = 1.0f;       // <--- ADDED: default cooldown for all plants
	float currentSeedCooldown = 0.0f; // <--- ADDED: active cooldown timer

	int num_animations = 1,
		currentAnimationIndex = 0;
	Animation animations[10];
	Animation currentAnimation;

	int AnimationForCycles = -1;

	float freezeTimer = 0.0f;
	bool isSuperFrozen = false;

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

	bool shellDestroyed = false;   // for pharaoh
	bool playingShellAnim = false; // for pharaoh
	Animation shellAnim;           // for pharaoh

	bool isBoosted = false;
	float boostAttackTimer = 0.1;

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
		if (currentAnimationIndex == idx) return;

		// forCycles = (-1)  ---> Infinite / Manual change
		AnimationForCycles = forCycles;
		setCurrentAnimation(idx);
		updateDimensions();
		cout << "Doing animation " << idx << " with currentAnimationIndex=" << currentAnimationIndex << endl;
	}

	void (*updateEntity)(Entity& entity) = nullptr; // This function should not have problems with Global variables .. but local variables (on its definition) are probably not captured
};

struct FallenItem {
	TEXTURE_TYPE texture;
	Clock clock;
	Vector2f fallPosition;
};

map<ENTITY_TYPE, Entity> plants;

vector<Entity> entities;
vector<Entity> newBullets;	// test
vector<Entity> bullets;
vector<FallenItem> fallenItems;

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

	int lastRow = 0;

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
int curBoosterCnt = 10;
bool isBoostActive = false;
int maxBoosterCnt = 2;
float boosterSpawnDuration = 1.0f;
float boosterLifeDuration = 3.0f;
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
		plant.DoAnimation(10, 3); // TODO: ntimes/eachcycletakes + boolmod
		plant.afterAnimation = [orgAttackTimer](Entity& p) {
			p.attackTimer = orgAttackTimer;
			p.isBoosted = false;
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
	zombie.lastDamaged = currentLevel->clock.getElapsedTime();
	//cout << "from `attackzombie` Zombie attacked! health now: " << zombie.health << endl;
	if (zombie.health <= 0 && !withDyingAnimation) {
		zombie.category = ENTITY_CATEGORY::NONE;
		zombie.isDead = true, deadZombieCount++, ZombieGap_dt += 0.05;
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

	curBoosterCnt = 0;
	isBoostActive = false;

	for (int i = 0; i < gridRows; i++) {
		currentLevel->lawnmowers[i].setRow(i, currentLevel->offsetX, currentLevel->offsetY, currentLevel->cellHeight);
	}
	entities.clear();
	zombieClock.restart();
	waveClockX.restart();
	sunSpawnClock.restart();
	fallenItems.clear();

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




float easeInOut(float t) {
	float k = 1.5f;

	float shaped = (t < 0.5f)
		? 0.5f * pow(2.0f * t, k)
		: 1.0f - 0.5f * pow(2.0f * (1.0f - t), k);

	float eased = -(cos(shaped * 3.14159265358979323846f) - 1.0f) / 2.0f;

	return eased;
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

			// Boost
			if (plant.isBoosted) {
				// shoot bullets
				if (plant.currentAnimation.currentFrame == plant.currentAnimation.frames / 2) {
					plant.attackClock.restart();
					Entity newBullet = defineNormalBullet();
					//Bullet newBullet = defineNormalBullet();
					newBullet.attack = plant.attack;
					newBullet.x_vel *= 2;
					newBullet.position = Vector2f(plant.position.x + 100, plant.position.y + 50);
					bullets.push_back(newBullet);
					newBullet.position = Vector2f(plant.position.x + 200, plant.position.y + 50);
					bullets.push_back(newBullet);
					newBullet.position = Vector2f(plant.position.x + 300, plant.position.y + 50);
					bullets.push_back(newBullet);
				}
				return;	 // to not loop zombies
			}

			// Normal
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
				plant.DoAnimation(1, 2);

				if (plant.currentAnimation.currentFrame == plant.currentAnimation.frames / 2) {
					plant.attackClock.restart();
					Entity newBullet = defineNormalBullet();
					//Bullet newBullet = defineNormalBullet();
					newBullet.attack = plant.attack;
					newBullet.position = Vector2f(plant.position.x + 100, plant.position.y + 50);
					//addBullet(newBullet);
					bullets.push_back(newBullet);
				}
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
				bullets.push_back(newBullet1);
				//newBullets.push_back(newBullet1);
				//entities.push_back(newBullet1);

				Entity newBullet2 = defineNormalBullet();
				newBullet2.attack = atk;
				newBullet2.position = Vector2f(pos.x + 50, pos.y + 50);
				bullets.push_back(newBullet2);
				//newBullets.push_back(newBullet2);
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
	shroom.num_animations = 2;
	shroom.animations[0].frameTimer = 0.1f;
	shroom.animations[0].frames = 30;
	shroom.setCurrentAnimation(0);

	plants[ENTITY_TYPE::PLANT_SHROOM] = shroom;

	// update
	shroom.updateEntity = [](Entity& plant) {
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
				Entity newBullet = defineShroomBullet();
				newBullet.attack = plant.attack;
				newBullet.position = Vector2f(plant.position.x + 100, plant.position.y + 50);
				bullets.push_back(newBullet);
				//newBullets.push_back(newBullet);
				//entities.push_back(newBullet)
			}
		}

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
				bullets.push_back(newBullet);
				//newBullets.push_back(newBullet);
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

	cherry.seedCooldown = 20.0f;

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
	textures[TEXTURE_TYPE::PLANT_MINISHROOM][1].loadFromFile("assets/Minishroom_attack.png");

	Entity miniShroom;
	miniShroom.category = ENTITY_CATEGORY::PLANT;
	miniShroom.type = ENTITY_TYPE::PLANT_MINISHROOM;
	miniShroom.health = 50;
	miniShroom.attack = 30;
	miniShroom.price = 0;
	miniShroom.seedCooldown = 5.0f;
	miniShroom.scale = 0.75f;

	miniShroom.num_animations = 2;

	miniShroom.animations[0].frames = 12;
	miniShroom.animations[0].frameTimer = 0.25f;

	miniShroom.animations[1].frames = 10;
	miniShroom.animations[1].frameTimer = 0.15f;

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
				plant.DoAnimation(1, 1);

				if (plant.currentAnimation.currentFrame == 4) {
					plant.attackClock.restart();
					Entity newBullet = defineShroomBullet();
					newBullet.attack = plant.attack;
					newBullet.position = Vector2f(plant.position.x + 70, plant.position.y + 50);
					bullets.push_back(newBullet);
					//newBullets.push_back(newBullet);
				}
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
	Chomper.health = 300;
	Chomper.price = 150;
	Chomper.scale = 0.85;
	Chomper.attack = 10000;

	textures[TEXTURE_TYPE::PLANT_CHOMPER][0].loadFromFile("assets/Chomper.png"); // IDLE
	textures[TEXTURE_TYPE::PLANT_CHOMPER][0].setSmooth(false);
	textures[TEXTURE_TYPE::PLANT_CHOMPER][1].loadFromFile("assets/ChomperAttack.png"); // ATTACK
	textures[TEXTURE_TYPE::PLANT_CHOMPER][1].setSmooth(false);
	textures[TEXTURE_TYPE::PLANT_CHOMPER][2].loadFromFile("assets/ChomperDigest.png"); // DIGEST
	textures[TEXTURE_TYPE::PLANT_CHOMPER][2].setSmooth(false);

	Chomper.num_animations = 3;
	Animation anim;

	anim.frames = 13;
	anim.frameTimer = 0.1f;
	Chomper.animations[0] = anim;

	anim.frames = 9;
	anim.frameTimer = 0.08f;
	Chomper.animations[1] = anim;

	anim.frames = 6;
	anim.frameTimer = 0.15f;
	Chomper.animations[2] = anim;

	Chomper.setCurrentAnimation(0);

	plants[ENTITY_TYPE::PLANT_CHOMPER] = Chomper;

	Chomper.updateEntity = [](Entity& plant) {

		if (plant.currentAnimationIndex == 0) {
			for (Entity& zombie : entities) {
				if (zombie.category != ENTITY_CATEGORY::ZOMBIE || zombie.isDead) continue;

				bool sameRow = abs(zombie.position.y - plant.position.y) < 100.0f;


				bool inFront = zombie.position.x > plant.position.x &&
					zombie.position.x < plant.position.x + plant.width * 0.55;

				if (sameRow && inFront && zombie.type != ENTITY_TYPE::ZOMBIE_BIG) {
					plant.targetId = zombie.id;
					zombie.x_vel = 0;

					plant.DoAnimation(1, 1);
					plant.afterAnimation = [](Entity& p) {
						p.DoAnimation(25, 2);
						p.afterAnimation = [](Entity& e) {
							e.DoAnimation(-1, 0);
							e.afterAnimation = [](Entity&) {};
							};
						};
					break;
				}
			}
		}

		if (plant.currentAnimationIndex == 1) {
			int currentF = plant.currentAnimation.currentFrame;

			if (plant.targetId != -1) {
				for (Entity& z : entities) {
					if (z.id == plant.targetId && !z.isDead) {

						if (currentF >= 3 && currentF <= 4) {

							z.scale = plant.scale * 0.60f;

							z.position.x = plant.position.x + 35;
							z.position.y = plant.position.y - 10;

						}

						if (currentF == 5) {
							attackZombie(z, 10000, false);
							plant.targetId = -1;
						}

						break;
					}
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
	mine.scale = 0.6f;
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
				plant.setCurrentAnimation(1);
			}
			return; //cant exp under ground
		}

		for (Entity& zombie : entities) {
			if (zombie.category != ENTITY_CATEGORY::ZOMBIE || zombie.isDead) continue;

			float diffY = abs(zombie.position.y - plant.position.y);
			float diffX = abs(zombie.position.x - plant.position.x);


			float triggerRange = (plant.type == ENTITY_TYPE::PLANT_DOOM_MINE) ? 200.0f : 50.0f;

			if (diffY < 60.0f && diffX < triggerRange) {
				int blastAnimIdx = (plant.type == ENTITY_TYPE::PLANT_DOOM_MINE) ? 1 : 2;
				plant.DoAnimation(1, blastAnimIdx);

				plant.afterAnimation = [&](Entity& p) {
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
	shroom.animations[0].frames = 41;
	shroom.animations[0].frameTimer = 0.1f;

	// animation 1 = الانفجار
	shroom.animations[1].frames = 14;
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
				int blastAnimIdx = (plant.type == ENTITY_TYPE::PLANT_DOOM_MINE) ? 1 : 2;
				plant.DoAnimation(1, blastAnimIdx);

				plant.afterAnimation = [&](Entity& p) {
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
Entity defineIceShroom() {
	// Load the texture into index 0
	textures[TEXTURE_TYPE::PLANT_ICESHROOM][0].loadFromFile("assets/IceShroom.png");

	Entity plant;
	plant.category = ENTITY_CATEGORY::PLANT;
	plant.type = ENTITY_TYPE::PLANT_ICESHROOM;
	plant.health = 100;
	plant.price = 75;
	plant.seedCooldown = 20.0f;
	plant.scale = 1.0f;

	plant.num_animations = 1;
	plant.animations[0].frames = 11;
	plant.animations[0].frameTimer = 0.1f;

	plant.setCurrentAnimation(0);

	plant.updateEntity = [](Entity& p) {
		if (p.attackClock.getElapsedTime().asSeconds() >= 1.5f) {

			screenFlashAlpha = 160.0f;

			for (Entity& z : entities) {
				if (z.category == ENTITY_CATEGORY::ZOMBIE && !z.isDead) {
					z.freezeTimer = 6.0f;
					z.isSuperFrozen = true;
					z.health -= 20;
				}
			}

			p.category = ENTITY_CATEGORY::NONE;
			p.isDead = true;
		}
		};

	plants[ENTITY_TYPE::PLANT_ICESHROOM] = plant;
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
	textures[TEXTURE_TYPE::ZOMBIE_TORCHLIGHT][0].loadFromFile("assets/torchlightzombie.png");
	textures[TEXTURE_TYPE::ZOMBIE_TORCHLIGHT][1].loadFromFile("assets/TorchlightZombieDeath.png");

	Entity zombie{};
	zombie.category = ENTITY_CATEGORY::ZOMBIE;
	zombie.type = ENTITY_TYPE::ZOMBIE_TORCHLIGHT;

	zombie.health = 200;
	zombie.attack = 20;
	zombie.x_vel = 0.05f;
	zombie.scale = 0.8f;

	zombie.num_animations = 2;
	Animation anim;
	anim.frameTimer = 0.1f;
	anim.frames = 8;
	zombie.animations[0] = anim;

	anim.frameTimer = 0.12f;
	anim.frames = 8;
	zombie.animations[1] = anim;

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

Entity definePharaohZombie() {
	textures[TEXTURE_TYPE::ZOMBIE_PHARAOH][0].loadFromFile("assets/PharaohZombie.png");
	textures[TEXTURE_TYPE::ZOMBIE_PHARAOH][1].loadFromFile("assets/PharaohZombieDeath.png");
	textures[TEXTURE_TYPE::ZOMBIE_PHARAOH][2].loadFromFile("assets/PharaohZombie2.png");
	textures[TEXTURE_TYPE::ZOMBIE_PHARAOH][3].loadFromFile("assets/ShellBreak.png");

	Entity zombie{};
	zombie.category = ENTITY_CATEGORY::ZOMBIE;
	zombie.type = ENTITY_TYPE::ZOMBIE_PHARAOH;
	zombie.health = 600;
	zombie.attack = 10;
	zombie.x_vel = 0.05f;
	// Animations
	zombie.num_animations = 4;
	Animation anim;
	// walk
	anim.frames = 66;
	anim.frameTimer = 0.05f;
	zombie.animations[0] = anim;
	// death
	anim.frames = 55;
	anim.frameTimer = 0.05f;
	zombie.animations[1] = anim;
	// no shell walk
	anim.frames = 65;
	anim.frameTimer = 0.05f;
	zombie.animations[2] = anim;
	// shell break
	anim.frames = 41;
	anim.frameTimer = 0.05f;
	zombie.animations[3] = anim;

	zombie.setCurrentAnimation(0);

	return zombie;
}

Entity defineShieldGuardZombie() {
	textures[TEXTURE_TYPE::ZOMBIE_SHIELDGUARD][0].loadFromFile("assets/ShieldGuardZombie.png");
	textures[TEXTURE_TYPE::ZOMBIE_SHIELDGUARD][1].loadFromFile("assets/ShieldGuardZombieDeath.png");

	Entity zombie{};
	zombie.category = ENTITY_CATEGORY::ZOMBIE;
	zombie.type = ENTITY_TYPE::ZOMBIE_SHIELDGUARD;
	zombie.health = 150;
	zombie.attack = 10;
	zombie.x_vel = 0.05f;
	// Animations
	zombie.num_animations = 2;
	Animation anim;
	// walk
	anim.frames = 36;
	anim.frameTimer = 0.05f;
	zombie.animations[0] = anim;
	// death
	anim.frames = 28;
	anim.frameTimer = 0.08f;
	zombie.animations[1] = anim;

	zombie.setCurrentAnimation(0);

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
		if (!isGameOver) {
			int damage = 255 - (std::max(1 - ((currentLevel->clock.getElapsedTime() - plant.lastDamaged).asSeconds() / 0.25f), 0.0f) * 155);
			plantSprite.setColor(Color(255, damage, damage));
		}
		window.draw(plantSprite);
		plantSprite.setColor(Color::White);//normal highlight
		plantSprite.setScale(Vector2f(1.0f, 1.0f));//Normal Scale for the other plants
	}

	// Bullets Cleaning
	/*int b = 0, nb = 0;
	for (b = 0, nb = 0; b < bullets.size() && nb < newBullets.size(); ) {
		if(bullets[b].type == ENTITY_CATEGORY::NONE) bullets[b] = newBullets[nb++];
		else b++;
	}*/
	/*while(nb < newBullets.size()) {
		bullets.push_back(newBullets[nb++]);
	}*/
	//newBullets.clear();
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
			|| (zombie.type == ENTITY_TYPE::ZOMBIE_PHARAOH && zombie.currentAnimationIndex == 2)
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
			if (zombie.type == ENTITY_TYPE::ZOMBIE_PHARAOH) // for pharaoh
				zombie.scale = 0.8f;
			if (zombie.type == ENTITY_TYPE::ZOMBIE_SHIELDGUARD) // for shield guard
				zombie.scale = 1.2f;

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
		if (zombie.currentAnimationIndex != 2) {
			zombie.setCurrentAnimation(2);
			FallenItem item;
			item.texture = TEXTURE_TYPE::ITEM_NEWSPAPER;
			item.clock.restart();
			item.fallPosition = zombie.position + Vector2f(100, 0);
			fallenItems.push_back(item);
		}

		zombie.x_vel = 0.12f;
		zombie.attackTimer = 0.4f;
	}
}
void updateConeheadZombie(Entity& zombie) {
	if (zombie.health <= 100 && zombie.currentAnimationIndex == 0) {
		if (zombie.currentAnimationIndex != 2) {
			zombie.setCurrentAnimation(2);
			FallenItem item;
			item.texture = TEXTURE_TYPE::ITEM_CONE;
			item.clock.restart();
			item.fallPosition = zombie.position + Vector2f(100, 0);
			fallenItems.push_back(item);
		}
	}
}

void updatePharaohZombie(Entity& zombie) {
	if (zombie.health <= 300 && !zombie.shellDestroyed) {
		zombie.shellDestroyed = true;
		zombie.playingShellAnim = true;
		zombie.shellAnim = zombie.animations[3];
		zombie.shellAnim.currentFrame = 0;
		zombie.shellAnim.clock.restart();
		if (zombie.currentAnimationIndex != 2) {
			zombie.setCurrentAnimation(2);
			FallenItem item;
			item.texture = TEXTURE_TYPE::ITEM_PHAROAH_CASKET;
			item.clock.restart();
			item.fallPosition = zombie.position + Vector2f(100, 0);
			fallenItems.push_back(item);
		}
		zombie.x_vel = 0.08f;
	}
}

void zombieEatPlant(Entity& zombie, Entity& plant) {
	if (zombie.position.x - (plant.position.x + plant.width - eatDistance) <= 0) {
		// close enough - not just touch
		zombie.x_vel = 0;
		if (zombie.attackClock.getElapsedTime().asSeconds() >= zombie.attackTimer) {
			zombie.attackClock.restart();
			plant.health -= zombie.attack;
			plant.lastDamaged = currentLevel->clock.getElapsedTime();
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
			zombieSprite.setColor(Color(100, 150, 255));
			zombie.freezeTimer -= delta_time;
		} else {
			zombie.isSuperFrozen = false;
			zombieSprite.setColor(Color::White);
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
		if (zombie.type == ENTITY_TYPE::ZOMBIE_PHARAOH && zombie.health > 0) { // for pharaoh
			updatePharaohZombie(zombie);
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
					if (zombie.isSuperFrozen) {
						zombie.x_vel = 0.002f;
					} else if (zombie.freezeTimer > 0) {
						zombie.x_vel = 0.02f;
					} else if (zombie.type == ENTITY_TYPE::ZOMBIE_NEWSPAPER && zombie.currentAnimationIndex == 2) { // for newspaper
						zombie.x_vel = 0.12f;
					} else if (zombie.type == ENTITY_TYPE::ZOMBIE_ALLSTAR && !zombie.hasTackled && !zombie.isTackling) {
						zombie.x_vel = 0.18f;
					} else if (zombie.type == ENTITY_TYPE::ZOMBIE_PHARAOH && zombie.shellDestroyed) {
						zombie.x_vel = 0.08f;
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

		if (!isGameOver) {
			int damage = 255 - (std::max(1 - ((currentLevel->clock.getElapsedTime() - zombie.lastDamaged).asSeconds() / 0.25f), 0.0f) * 155);
			zombieSprite.setColor(Color(255, damage, damage));
		}

		window.draw(zombieSprite);

		// to play pharaoh zombie's shell breaking animation
		if (zombie.type == ENTITY_TYPE::ZOMBIE_PHARAOH && zombie.playingShellAnim) {
			if (zombie.shellAnim.clock.getElapsedTime().asSeconds() >= zombie.shellAnim.frameTimer) {
				zombie.shellAnim.clock.restart();
				zombie.shellAnim.currentFrame++;
				if (zombie.shellAnim.currentFrame >= zombie.shellAnim.frames) {
					zombie.playingShellAnim = false;     // all frames done and only 1 cycle so stop animation
				}
			}

			Texture& shellTex = textures[TEXTURE_TYPE::ZOMBIE_PHARAOH][3];
			float frameWidth = shellTex.getSize().x / zombie.shellAnim.frames;
			float frameHeight = shellTex.getSize().y;

			uiSprite.setTexture(shellTex);
			uiSprite.setTextureRect(IntRect(zombie.shellAnim.currentFrame * frameWidth, 0, frameWidth, shellTex.getSize().y));
			// (x initial position for each frame, 0 is top of frame)

			uiSprite.setOrigin(frameWidth / 2.0, frameHeight / 2.0); // to scale from center
			uiSprite.setScale(3.0f, 3.0f);
			uiSprite.setPosition(zombie.position.x + 100, zombie.position.y + 130);
			window.draw(uiSprite);
			uiSprite.setOrigin(0.0f, 0.0f); // reset
			uiSprite.setScale(1.0f, 1.0f);  // reset
		}
	}
}

void UpdateAndDrawBullets(RenderWindow& window) {
	for (Entity& bullet : bullets) {
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
				// shield guard zombie only dies to shroom bullet
				if (zombie.type == ENTITY_TYPE::ZOMBIE_SHIELDGUARD && bullet.type != ENTITY_TYPE::ShroomBullet) {
					bullet.category = ENTITY_CATEGORY::NONE; // bullet disappears and doesnt do damage
					break;   // bullet is gone so stop loop from checking other zombies
				}

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

		int row = 0;
		do {
			row = rand() % gridraws;
		} while (row == currentLevel->lastRow);
		currentLevel->lastRow = row;

		newZombie.position = Vector2f(windowWidth, currentLevel->offsetY + (row)*currentLevel->cellHeight);
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

void screenFade(RenderWindow& window, float v) {
	sf::RectangleShape fade(Vector2f(windowWidth, windowHeight));
	fade.setFillColor(Color(0, 0, 0, 255 * v));
	window.draw(fade);
}

// There is a bug in SFML that causes the program to crash when exiting
// Making this a pointer and deleting it early prevents that crash
Font* font;

Sprite DrawSeedPacket(RenderWindow& window, Entity& plant, Vector2f pos, bool interactable = false, float scale = 1.0f, Color color = Color::White) {
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

	if (interactable && sprite.getGlobalBounds().contains(Vector2f(mousePos))) {
		sprite.setColor(color * Color(128, 128, 128, 255));
		plantSprite.setColor(color * Color(128, 128, 128, 255));
	}

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
	float animationProgress = (currentLevel->clock.getElapsedTime().asSeconds() - currentLevel->plantsSelectedTime.asSeconds() - 4.0f) / 0.25f;
	if (animationProgress > 1) animationProgress = 1;
	Vector2f animationOffset = Vector2f(0, -100) * (1 - easeInOut(animationProgress));
	if (!currentLevel->arePlantsSelected) {
		animationProgress = (currentLevel->clock.getElapsedTime().asSeconds() - 2.25f) / 1.0f;
		if (animationProgress > 1) animationProgress = 1;
		animationOffset = Vector2f(-750, 0) * (1 - easeInOut(animationProgress));
	}

	const Vector2f pos = Vector2f(10.0f, 0) + animationOffset;

	Texture& seedBank = getTexture(TEXTURE_TYPE::SEEDBANK);

	uiSprite.setPosition(pos);
	uiSprite.setTexture(seedBank);
	resizeSpriteToTex(uiSprite, seedBank);
	uiSprite.setScale(Vector2f(1.0f, 1.0f));

	window.draw(uiSprite);
	uiSprite.setScale(Vector2f(1.0f, 1.0f));

	Text text;
	text.setFont(*font);
	text.setString(std::to_string(sunsMoney));
	text.setFillColor(Color());
	text.setOrigin(Vector2f(text.getGlobalBounds().getSize()) / 2.0f);
	text.setPosition(pos + Vector2f(72, 136));

	window.draw(text);

	Vector2u seedPacketSize = getTexture(TEXTURE_TYPE::SEEDPACKET).getSize();

	int removePlant = -1;

	int i = -1;
	for (auto& plant : currentLevel->selectedPlants) {
		i++;
		Vector2f spritePos = pos + Vector2f(155.0f + i * (seedPacketSize.x + 10), 24.0f);

		Color packetColor = Color::White;
		if (sunsMoney < plant.price || plant.currentSeedCooldown > 0) {
			packetColor = Color(100, 100, 100, 255);
		}

		Sprite sprite = DrawSeedPacket(window, plant, spritePos, true, 0.9f, packetColor);

		if (plant.currentSeedCooldown > 0.0f && plant.seedCooldown > 0.0f) {
			float progress = plant.currentSeedCooldown / plant.seedCooldown;
			float h = sprite.getGlobalBounds().height * 0.9f;
			float w = sprite.getGlobalBounds().width * 0.9f;

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
	if (currentLevel->clock.getElapsedTime().asSeconds() < 2.45f) {
		return;
	}

	DrawSeeds(window);

	Vector2f animationOffset = {};

	animationOffset.x = -750.0f * easeInOut(1.0f - std::min((currentLevel->clock.getElapsedTime().asSeconds() - 2.25f) / 1.0f, 1.0f));

	Sprite bgSprite;
	bgSprite.setTexture(getTexture(TEXTURE_TYPE::SEED_CHOOSER));
	bgSprite.setPosition(Vector2f(10, 183.0f) + animationOffset);
	bgSprite.setScale(Vector2f(1.5f, 1.5f));

	window.draw(bgSprite);

	Text text;
	text.setFont(*font);
	text.setString("Choose your plants");
	text.setFillColor(Color(255, 255, 255, 192));
	text.setPosition(Vector2f(240, 195.0f) + animationOffset);

	window.draw(text);

	text.setFillColor(Color());

	Vector2u seedPacketSize = getTexture(TEXTURE_TYPE::SEEDPACKET).getSize();

	int i = -1;
	for (auto& plant : currentLevel->plant_types) {
		if (!isEntityUnlocked[plant.type]) continue;

		i++;
		Vector2f spritePos = Vector2f(50.0f, 250.0f) + animationOffset + Vector2f((i % 6) * (seedPacketSize.x + 5.0f), 10.0f + (i / 6) * 150.0f);

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

		Sprite sprite = DrawSeedPacket(window, plant, spritePos, true, 0.9f, color);

		if (!disabled && mouseJustClicked[0] && currentLevel->selectedPlants.size() < 5 && sprite.getGlobalBounds().contains(Vector2f(mousePos))) {
			uiInteracted = true;
			currentLevel->selectedPlants.push_back(plant);
		}
	}

	Sprite startButton;

	startButton.setTexture(getTexture(TEXTURE_TYPE::SEED_CHOOSER_BUTTON));
	startButton.setScale(Vector2f(1.5f, 1.5f));
	startButton.setPosition(Vector2f(250.0f, 780.0f) + animationOffset);

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
	bool unlockedPlant = currentLevel->plantReward.plantType != ENTITY_TYPE::NONE;

	Text text;
	text.setFont(*font);
	text.setString("You unlocked a new plant!");
	text.setFillColor(Color(255, 255, 255, 255));
	text.setPosition(Vector2f(960, 130.0f));
	text.setCharacterSize(64);
	text.setOrigin(text.getLocalBounds().getSize() / 2.0f);

	if (unlockedPlant) window.draw(text);

	Vector2u seedPacketSize = getTexture(TEXTURE_TYPE::SEEDPACKET).getSize();

	float diff = currentLevel->clock.getElapsedTime().asSeconds() - currentLevel->winTime.asSeconds();

	screenFade(window, 0.5f * std::min(diff / 1.0f, 1.0f));

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

	if (unlockedPlant) window.draw(packetBg);

	if (unlockedPlant) DrawSeedPacket(window, plants[currentLevel->plantReward.plantType], Vector2f(960, 540) - Vector2f(seedPacketSize) * scale / 2.0f, false, scale);

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
	int level = currentLevel->lvlNum;

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
		int c = (int)(255 * currentLevel->clock.getElapsedTime().asSeconds());
		ground.setColor(Color(c, c, c, 255));
	} else if (currentLevel->clock.getElapsedTime().asSeconds() < 2.0f) {
		float progress = (currentLevel->clock.getElapsedTime().asSeconds() - 1.0f) / 1.0f;
		if (progress < 0) progress = 0;
		if (progress > 1) progress = 1;

		progress = easeInOut(progress);

		ground.setTextureRect(IntRect(maxPan * progress, 0, visibleWidth, actualTexHeight));
	} else if (!currentLevel->arePlantsSelected) {
		ground.setTextureRect(IntRect(maxPan, 0, visibleWidth, actualTexHeight));
	} else if (currentLevel->clock.getElapsedTime().asSeconds() - currentLevel->plantsSelectedTime.asSeconds() < 0.75f) {
		float progress = (currentLevel->clock.getElapsedTime().asSeconds() - currentLevel->plantsSelectedTime.asSeconds()) / 0.75f;
		if (progress < 0) progress = 0;
		if (progress > 1) progress = 1;

		progress = easeInOut(progress);

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

		screenFade(window, progress * 0.5f);
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

		screenFade(window, progress);
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
	cout << "======== Booster Activated! Remaining Boosters: " << curBoosterCnt << endl;
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
		plant.attackClock.restart();
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

void DrawLevelStats(RenderWindow& window) {
	float animationProgress = (currentLevel->clock.getElapsedTime().asSeconds() - currentLevel->plantsSelectedTime.asSeconds() - 4.0f) / 0.25f;
	if (animationProgress > 1) animationProgress = 1;
	Vector2f animationOffset = Vector2f(-200, 0) * (1 - easeInOut(animationProgress));

	Text text;

	text.setFont(*font);
	text.setFillColor(Color::White);
	text.setOutlineColor(Color::Black);
	text.setOutlineThickness(4);
	text.setCharacterSize(48);

	Sprite sprite;

	sprite.setTexture(getTexture(TEXTURE_TYPE::COIN));
	sprite.setOrigin(0, getTexture(TEXTURE_TYPE::COIN).getSize().y);
	sprite.setPosition(Vector2f(0, windowHeight) + Vector2f(20, -20) + animationOffset);
	sprite.setScale(
		100.0 / getTexture(TEXTURE_TYPE::COIN).getSize().x,
		100.0 / getTexture(TEXTURE_TYPE::COIN).getSize().y
	);

	text.setString(std::to_string(coinsMoney) + " Coins");
	text.setOrigin(0, text.getLocalBounds().height);
	text.setFillColor(Color::Yellow);
	text.setPosition(sprite.getPosition() + Vector2f(108, -28));

	window.draw(sprite);
	window.draw(text);

	// Boosts

	sprite.setTexture(getTexture(TEXTURE_TYPE::BOOSTER));
	sprite.setOrigin(0, getTexture(TEXTURE_TYPE::BOOSTER).getSize().y);
	sprite.setPosition(Vector2f(0, windowHeight) + Vector2f(20, -120) + animationOffset);
	sprite.setScale(
		100.0 / getTexture(TEXTURE_TYPE::BOOSTER).getSize().x,
		100.0 / getTexture(TEXTURE_TYPE::BOOSTER).getSize().y
	);

	text.setString(std::to_string(-1) + " Boosts");
	text.setOrigin(0, text.getLocalBounds().height);
	text.setFillColor(Color::Green);
	text.setPosition(sprite.getPosition() + Vector2f(108, -28));

	window.draw(sprite);
	window.draw(text);


	// Progress Bar

	{
		float progressValue = (float)deadZombieCount / currentLevel->targetZombie;
		progressValue = 0.045f + (progressValue * (1 - 0.045f * 2));

		Sprite progress;
		progress.setTexture(getTexture(TEXTURE_TYPE::PROGRESS_EMPTY));
		progress.setOrigin(progress.getLocalBounds().getSize());
		progress.setPosition(Vector2f(windowWidth - 100, windowHeight - 5) + (Vector2f(0, 75) * (1 - easeInOut(animationProgress))));
		progress.setScale(0.35f, 0.35f);

		window.draw(progress);

		Vector2u texSize = getTexture(TEXTURE_TYPE::PROGRESS_FULL).getSize();

		progress.setTexture(getTexture(TEXTURE_TYPE::PROGRESS_FULL));
		progress.setTextureRect(IntRect{ 0, 0, int(texSize.x * progressValue), (int)texSize.y });

		window.draw(progress);

		sprite.setTexture(getTexture(TEXTURE_TYPE::PROGRESS_HEAD));
		sprite.setOrigin(Vector2f(getTexture(TEXTURE_TYPE::PROGRESS_HEAD).getSize()) / 2.0f);
		sprite.setScale(0.25f, 0.25f);
		sprite.setPosition(Vector2f(progress.getGlobalBounds().left + progress.getGlobalBounds().width, progress.getPosition().y - 35));

		window.draw(sprite);
	}
}

void UpdateFallenItems(RenderWindow& window) {
	int i = 0;
	for (auto& item : fallenItems) {
		float time = item.clock.getElapsedTime().asSeconds();
		if (time > 3.5f) continue;

		Vector2f offsetPos;

		offsetPos.x = 75 * time;
		offsetPos.y = -100 * time + 0.5f * 500 * time * time;

		if (offsetPos.x > 75.0f) offsetPos.x = 75.0f;
		if (offsetPos.y > 150.0f) offsetPos.y = 150.0f;

		Sprite sprite;

		sprite.setTexture(textures[item.texture][0]);
		sprite.setPosition(item.fallPosition + offsetPos);
		sprite.setOrigin(sprite.getGlobalBounds().getSize() / 2.0f);

		sprite.setRotation(std::min(time, 1.0f) * 270);

		if (time > 2.5f) {
			float progress = (time - 2.5f) / 1.0f;
			if (progress > 1) progress = 1;
			sprite.setColor(Color(255, 255, 255, (int)(255 * (1 - progress))));
		}

		window.draw(sprite);
		i++;
	}
}

void UpdateLevel(RenderWindow& window) {
	if (currentLevel == nullptr) return;
	// Drawing
	window.clear(Color::Black);
	// 
	ground.setColor(Color::White);
	screenAnimation(window);

	zombieSprite.setColor(Color::White);
	plantSprite.setColor(Color::White);

	if (currentLevel == nullptr) return;
	int level = currentLevel->lvlNum;
	ground.setTexture(textures[TEXTURE_TYPE::LEVEL_BACKGROUND][level]);
	window.draw(ground);

	if (currentLevel->arePlantsSelected && (currentLevel->clock.getElapsedTime().asSeconds() - currentLevel->plantsSelectedTime.asSeconds()) >= 0.95f) UpdateLawnmowers(window);

	if (!currentLevel->arePlantsSelected || currentLevel->clock.getElapsedTime().asSeconds() - currentLevel->plantsSelectedTime.asSeconds() < 3.75f) {
		float diff = currentLevel->clock.getElapsedTime().asSeconds() - currentLevel->plantsSelectedTime.asSeconds();

		float progress = (diff - 0.75f) / 5.0f + 0.15f;

		if (!currentLevel->arePlantsSelected) progress = 0;

		ground.setTexture(textures[TEXTURE_TYPE::LEVEL_BACKGROUND_DIRT][level]);

		bool rolling = currentLevel->arePlantsSelected && diff >= 0.75f;

		int dirtLeft = 0;

		if (rolling) {
			int right = ground.getGlobalBounds().left + ground.getGlobalBounds().width;

			ground.setOrigin(1400 - 1400 * progress, 0);
			ground.setPosition(right, 0);
			ground.setTextureRect(IntRect(1400 * progress, 0, 1400 - 1400 * progress, 600));

			dirtLeft = ground.getGlobalBounds().left;
		}

		window.draw(ground);

		ground.setTextureRect(IntRect(0, 0, 1400, 600));
		ground.setOrigin(0, 0);
		ground.setPosition(Vector2f(0, 0));

		if (rolling && (1400 * progress) >= 250 && (1400 * progress) <= 1000) {
			if (level != 4) {
				Sprite roll;
				Texture& rollTex = textures[TEXTURE_TYPE::GRID_ROLL][0];
				roll.setTexture(rollTex);
				roll.setOrigin(rollTex.getSize().x / 2, 0);
				roll.setPosition(Vector2f(dirtLeft, currentLevel->offsetY - 75));
				float scale = 1.0f;
				scale = (currentLevel->cellHeight * gridRows + 125) / rollTex.getSize().y;
				roll.setScale(Vector2f(scale, scale));
				window.draw(roll);
			}
		}
	}

	if (isGameOver) {
		ground.setTexture(textures[TEXTURE_TYPE::LEVEL_BACKGROUND_OPENDOOR][level]);
		window.draw(ground);
	}

	//fake zombies
	if (!currentLevel->arePlantsSelected || currentLevel->clock.getElapsedTime().asSeconds() - currentLevel->plantsSelectedTime.asSeconds() < 3.75f) {


		float previewDiff = currentLevel->clock.getElapsedTime().asSeconds();
		float camProgress = 0.0f;

		if (previewDiff >= 1.0f && previewDiff < 2.0f) {
			camProgress = easeInOut((previewDiff - 1.0f) / 1.0f);//حته بتاعت يوتيوبر كبير (:
		} else if (previewDiff >= 2.0f && !currentLevel->arePlantsSelected) {
			camProgress = 1.0f;
		} else if (currentLevel->arePlantsSelected) {
			float pan = (previewDiff - currentLevel->plantsSelectedTime.asSeconds()) / 0.75f;
			camProgress = 1.0f - easeInOut(min(pan, 1.0f));
		}



		float targetX = 1250.0f;


		float startX = (windowWidth + 200.0f) - (camProgress * ((windowWidth + 200.0f) - targetX));


		if (currentLevel->zombie_types.size() > 0) {

			float offsetX[5] = { 0, 90, 40, 130, 70 };

			for (int raw = 0; raw < 4; raw++) {
				for (int col = 0; col < 3; col++) {

					int zomIndex = (raw + col) % currentLevel->zombie_types.size();
					Entity dummyZombie = currentLevel->zombie_types[zomIndex];

					Texture& zomTex = textures[getTextureType(dummyZombie.type)][0];
					int frames = dummyZombie.animations[0].frames;

					if (frames > 0) {
						int frameWidth = zomTex.getSize().x / frames;

						int frame = (int)(previewDiff * 15 + raw * 3 + col * 7) % frames;

						zombieSprite.setTexture(zomTex);
						zombieSprite.setTextureRect(IntRect(frame * frameWidth, 0, frameWidth, zomTex.getSize().y));
						zombieSprite.setScale(Vector2f(dummyZombie.scale, dummyZombie.scale));
						zombieSprite.setColor(Color::White);

						float actualHeight = zomTex.getSize().y * dummyZombie.scale;
						float zomY = currentLevel->offsetY + (raw * currentLevel->cellHeight) - (actualHeight - currentLevel->cellHeight);
						float zomX = startX + offsetX[raw] + (col * 130);

						zombieSprite.setPosition(zomX, zomY);
						window.draw(zombieSprite);
					}
				}
			}
		}
	}

	DrawZombies(window);
	DrawPlantsGrid(window);

	if (isGameOver) {
		ground.setTexture(textures[TEXTURE_TYPE::LEVEL_BACKGROUND_HOUSE][level]);
		window.draw(ground);

		gameOverAnimation(window);
	}
	if (currentLevel == nullptr) return;

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
	} else if (currentLevel->clock.getElapsedTime().asSeconds() - currentLevel->plantsSelectedTime.asSeconds() < 4.0f) {
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

	UpdateAndDrawBullets(window);
	UpdateSuns(window);
	UpdateBooster(window);
	UpdateFallenItems(window);
	DrawSeeds(window);
	DrawDebugGrid(window);

	if (screenFlashAlpha > 0) {
		screenFlashAlpha -= delta_time * 600.0f;
		if (screenFlashAlpha < 0) screenFlashAlpha = 0;

		screenFlashOverlay.setSize(Vector2f(windowWidth, windowHeight));
		screenFlashOverlay.setFillColor(Color(200, 230, 255, (Uint8)screenFlashAlpha));
		window.draw(screenFlashOverlay);
	}

	UI_DrawClick(window);
	UI_Interaction(window);

	DrawLevelStats(window);

	if (deadZombieCount >= currentLevel->targetZombie) {
		isGameWon = true;
	}

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

bool selectingLevel = false;
bool inStore = false;
Time startMenuItemTime = Time::Zero;
Time startMainMenuTime = Time::Zero;

void DrawLevelSelector(RenderWindow& window) {
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

	FloatRect backRc;
	backRc.left = 714;
	backRc.top = 934;
	backRc.width = 370;
	backRc.height = 100;

	if (backRc.contains(Vector2f(mousePos))) {
		sprite.setTexture(getTexture(TEXTURE_TYPE::LEVEL_SELECTOR_BACKGROUND_BACK));
		window.draw(sprite);

		if (mouseJustClicked[0]) {
			selectingLevel = false;
			startMainMenuTime = gameClock.getElapsedTime();
		}
	}

	{
		Text title;

		title.setFont(*font);
		title.setFillColor(Color::White);
		title.setOutlineColor(Color::Black);
		title.setOutlineThickness(4);
		title.setCharacterSize(86);

		title.setString("Select Level");

		title.setOrigin(title.getLocalBounds().width / 2.0f, title.getLocalBounds().height / 2.0f);
		title.setPosition(windowWidth / 2.0f, 100.0f);

		window.draw(title);
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

	Vector2f initialPos = Vector2f(windowSize / 2.0f) - Vector2f(0, 75.0f);

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

		Texture& iconTex = textures[TEXTURE_TYPE::LEVEL_ICON][i + 1];
		sprite.setTexture(iconTex);
		sprite.setTextureRect({ 0, 0, (int)iconTex.getSize().x, (int)iconTex.getSize().y });
		float fitScaleX = 370.0f / iconTex.getSize().x;
		float fitScaleY = 320.0f / iconTex.getSize().y;
		sprite.setScale(Vector2f(fitScaleX * (scale.x / 1.5f), fitScaleY * (scale.y / 1.5f)));
		sprite.setPosition(pos + Vector2f(0, -32));
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
		playButton.setScale(Vector2f(0.6f, 0.6f));
		playButton.setColor(Color::White);

		if (playButton.getGlobalBounds().contains(Vector2f(mousePos))) {
			playButton.setColor(Color(128, 128, 128, 255));

			if (mouseJustClicked[0]) {
				startLevel(selectedLevel + 1);
			}
		}

		window.draw(playButton);
	}

	{
		float progress = 1 - ((gameClock.getElapsedTime() - startMenuItemTime).asSeconds() - 0.25f) / 0.25f;
		if (progress < 0) progress = 0;
		screenFade(window, progress);
	}
}

void DrawStore(RenderWindow& window) {
	Sprite sprite;

	Texture bgTex = getTexture(TEXTURE_TYPE::MAINMENU_BACKGROUND);
	sprite.setTexture(bgTex);
	sprite.setPosition(Vector2f(0, 0));
	sprite.setScale(Vector2f(
		windowWidth / (float)bgTex.getSize().x,
		windowHeight / (float)bgTex.getSize().y
	));

	window.draw(sprite);

	Texture storeTex = getTexture(TEXTURE_TYPE::STORE_WINDOW);
	sprite.setTexture(storeTex);
	sprite.setPosition(Vector2f(windowWidth, windowHeight) / 2.0f);
	sprite.setScale(Vector2f(
		windowWidth / (float)storeTex.getSize().x,
		windowHeight / (float)storeTex.getSize().y
	));
	sprite.setOrigin(Vector2f(storeTex.getSize()) / 2.0f);

	window.draw(sprite);

	Sprite button;
	Texture buttonTex = getTexture(TEXTURE_TYPE::WOOD_BUTTON);
	Vector2f buttonTexSize = Vector2f(buttonTex.getSize());

	button.setTexture(buttonTex);
	button.setScale(0.225f, 0.225f);

	button.setOrigin(buttonTexSize / 2.0f);

	Text text;

	text.setFont(*font);
	text.setFillColor(Color::White);
	text.setOutlineColor(Color::Black);
	text.setOutlineThickness(4);
	text.setCharacterSize(48);

	text.setString("Shop");
	text.setCharacterSize(64);
	text.setOrigin(text.getLocalBounds().width / 2.0f, text.getLocalBounds().height / 2.0f);
	text.setFillColor(Color::Yellow);
	text.setPosition(Vector2f(windowWidth / 2.0f, 200));

	window.draw(text);
	text.setCharacterSize(48);

	button.setColor(Color::White);
	text.setFillColor(Color::White);
	button.setPosition(Vector2f(windowWidth / 2.0f, windowHeight) + Vector2f(0, -70));
	text.setString(std::string("Back"));
	text.setFillColor(Color::Red);
	text.setOrigin(text.getLocalBounds().width / 2.0f, text.getLocalBounds().height / 2.0f);
	text.setPosition(button.getPosition());

	if (button.getGlobalBounds().contains(Vector2f(mousePos))) {
		button.setColor(Color(128, 128, 128, 255));
		text.setFillColor(text.getFillColor() * Color(128, 128, 128, 255));

		if (mouseJustClicked[0]) {
			inStore = false;
			startMainMenuTime = gameClock.getElapsedTime();
		}
	}

	window.draw(button);
	window.draw(text);

	{
		float progress = 1 - ((gameClock.getElapsedTime() - startMenuItemTime).asSeconds() - 0.25f) / 0.25f;
		if (progress < 0) progress = 0;
		screenFade(window, progress);
	}
}

void DrawMainMenu(RenderWindow& window) {
	if ((gameClock.getElapsedTime() - startMenuItemTime).asSeconds() > 0.25f) {
		if (selectingLevel) {
			DrawLevelSelector(window);
			return;
		}

		if (inStore) {
			DrawStore(window);
			return;
		}
	}

	bool buttonsEnabled = true;
	if (selectingLevel || inStore) buttonsEnabled = false;

	Sprite sprite;

	Texture bgTex = getTexture(TEXTURE_TYPE::MAINMENU_BACKGROUND);
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
		logoSprite.setPosition(windowWidth / 2.0f, 225.0f);
		logoSprite.setScale(0.5f, 0.5f);

		window.draw(logoSprite);
	}

	Sprite button;
	Texture buttonTex = getTexture(TEXTURE_TYPE::WOOD_BUTTON);
	Vector2f buttonTexSize = Vector2f(buttonTex.getSize());

	button.setTexture(buttonTex);
	button.setScale(0.225f, 0.225f);

	button.setOrigin(buttonTexSize / 2.0f);

	Text text;

	text.setFont(*font);
	text.setFillColor(Color::White);
	text.setOutlineColor(Color::Black);
	text.setOutlineThickness(4);
	text.setCharacterSize(48);

	button.setColor(Color::White);
	text.setFillColor(Color::White);
	button.setPosition(Vector2f(windowWidth, windowHeight) / 2.0f + Vector2f(0, -50));
	text.setString(std::string("Play"));
	text.setFillColor(Color::Green);
	text.setOrigin(text.getLocalBounds().width / 2.0f, text.getLocalBounds().height / 2.0f);
	text.setPosition(button.getPosition());

	if (buttonsEnabled && button.getGlobalBounds().contains(Vector2f(mousePos))) {
		button.setColor(Color(128, 128, 128, 255));
		text.setFillColor(text.getFillColor() * Color(128, 128, 128, 255));

		if (mouseJustClicked[0]) {
			selectingLevel = true;
			startMenuItemTime = gameClock.getElapsedTime();
		}
	}

	window.draw(button);
	window.draw(text);

	button.setColor(Color::White);
	text.setFillColor(Color::White);
	button.setPosition(Vector2f(windowWidth, windowHeight) / 2.0f + Vector2f(0, 50));
	text.setString(std::string("Store"));
	text.setFillColor(Color::Yellow);
	text.setOrigin(text.getLocalBounds().width / 2.0f, text.getLocalBounds().height / 2.0f);
	text.setPosition(button.getPosition());

	if (buttonsEnabled && button.getGlobalBounds().contains(Vector2f(mousePos))) {
		button.setColor(Color(128, 128, 128, 255));
		text.setFillColor(text.getFillColor() * Color(128, 128, 128, 255));

		if (mouseJustClicked[0]) {
			inStore = true;
			startMenuItemTime = gameClock.getElapsedTime();
		}
	}

	window.draw(button);
	window.draw(text);

	button.setColor(Color::White);
	text.setFillColor(Color::White);
	button.setPosition(Vector2f(windowWidth, windowHeight) / 2.0f + Vector2f(0, 150));
	text.setString(std::string("Quit Game"));
	text.setFillColor(Color::Red);
	text.setOrigin(text.getLocalBounds().width / 2.0f, text.getLocalBounds().height / 2.0f);
	text.setPosition(button.getPosition());

	if (buttonsEnabled && button.getGlobalBounds().contains(Vector2f(mousePos))) {
		button.setColor(Color(128, 128, 128, 255));
		text.setFillColor(text.getFillColor() * Color(128, 128, 128, 255));

		if (mouseJustClicked[0]) {
			window.close();
		}
	}

	window.draw(button);
	window.draw(text);

	if (selectingLevel || inStore) {
		float progress = (gameClock.getElapsedTime() - startMenuItemTime).asSeconds() / 0.25f;
		if (progress > 1) progress = 1;
		screenFade(window, progress);
	}

	{
		float progress = 1 - ((gameClock.getElapsedTime() - startMainMenuTime).asSeconds()) / 0.5f;
		if (progress < 0) progress = 0;
		screenFade(window, progress);
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

int level_1_music(Music& music) {
	if (!music.openFromFile("assets/audio/06. Main Music 00.ogg")) {
		std::cout << "Error loading file!" << std::endl;
		return -1;
	}
	music.setLoop(true);
	music.play();
	return 0;
}

int level_2_music(Music& music) {
	if (!music.openFromFile("assets/audio/11. Main Music 05.ogg")) {
		std::cout << "Error loading file!" << std::endl;
		return -1;
	}
	music.setLoop(true);
	music.play();
	return 0;
}

int level_3_music(Music& music) {
	if (!music.openFromFile("assets/audio/14. Main Music 08.ogg")) {
		std::cout << "Error loading file!" << std::endl;
		return -1;
	}
	music.setLoop(true);
	music.play();
	return 0;
}

int level_4_music(Music& music) {
	if (!music.openFromFile("assets/audio/15. Main Music 09.ogg")) {
		std::cout << "Error loading file!" << std::endl;
		return -1;
	}
	music.setLoop(true);
	music.play();
	return 0;
}

int level_5_music(Music& music) {
	if (!music.openFromFile("assets/audio/17. Main Music 11.ogg")) {
		std::cout << "Error loading file!" << std::endl;
		return -1;
	}
	music.setLoop(true);
	music.play();
	return 0;
}

int level_6_music(Music& music) {
	if (!music.openFromFile("assets/audio/18. Main Music 12.ogg")) {
		std::cout << "Error loading file!" << std::endl;
		return -1;
	}
	music.setLoop(true);
	music.play();
	return 0;
}

int huge_wave_music(Music& music) {
	if (!music.openFromFile("assets/audio/03. hugewave.ogg")) {
		std::cout << "Error loading file!" << std::endl;
		return -1;
	}
	music.setLoop(false);
	music.play();
	return 0;
}

int after_huge_wave_music(Music& music) {
	if (!music.openFromFile("assets/audio/SFX siren.ogg")) {
		std::cout << "Error loading file!" << std::endl;
		return -1;
	}
	music.setLoop(false);
	music.play();
	return 0;
}

int pause_music(Music& music) {
	if (!music.openFromFile("assets/audio/24. pause.ogg")) {
		std::cout << "Error loading file!" << std::endl;
		return -1;
	}
	music.setLoop(false);
	music.play();
	return 0;
}

int win_music(Music& music) {
	if (!music.openFromFile("assets/audio/26. winmusic.ogg")) {
		std::cout << "Error loading file!" << std::endl;
		return -1;
	}
	music.setLoop(false);
	music.play();
	return 0;
}

int eating_music(Music& music) {
	if (!music.openFromFile("assets/audio/SFX chomp.ogg")) {
		std::cout << "Error loading file!" << std::endl;
		return -1;
	}

	int playCount = 0;
	const int maxPlays = 3;

	music.play();

	while (playCount < maxPlays) {
		if (music.getStatus() == sf::SoundSource::Stopped) {
			playCount++;
			if (playCount < maxPlays) {
				music.play();
			}
		}
	}

	return 0;
}

int small_red_car_music(Music& music) {
	if (!music.openFromFile("assets/audio/SFX lawnmower.ogg")) {
		std::cout << "Error loading file!" << std::endl;
		return -1;
	}
	music.setLoop(false);
	music.play();
	return 0;
}

int zombies_are_coming_music(Music& music) {
	if (!music.openFromFile("assets/audio/Voices awooga.ogg")) {
		std::cout << "Error loading file!" << std::endl;
		return -1;
	}
	music.setLoop(false);
	music.play();
	return 0;
}

int before_lose_music_music(Music& music) {
	if (!music.openFromFile("assets/audio/Voices scream.ogg")) {
		std::cout << "Error loading file!" << std::endl;
		return -1;
	}
	music.setLoop(false);
	music.play();
	return 0;
}

int lose_music(Music& music) {
	if (!music.openFromFile("assets/audio/05. losemusic.ogg")) {
		std::cout << "Error loading file!" << std::endl;
		return -1;
	}
	music.setLoop(false);
	music.play();
	return 0;
}


int main() {
	srand(time(0));

	screenFlashOverlay.setPosition(0, 0);
	screenFlashOverlay.setSize(Vector2f(windowWidth, windowHeight));

	Music music;	// can't be global - OpenAL must be loaded first
	RenderWindow window(VideoMode(windowWidth, windowHeight), "Plants vs Zombies");
	bool fullscreen = false;
	Vector2i oldPos;
	Vector2u oldSize;

	window.setSize(Vector2u(1280, 720));
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
	Entity iceShroom = defineIceShroom();

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
	Entity zombiePharaoh = definePharaohZombie();
	Entity zombieShieldGuard = defineShieldGuardZombie();

	// Other Textures
	textures[TEXTURE_TYPE::LOGO][Default_Texture_Index].loadFromFile("assets/logo.png");
	textures[TEXTURE_TYPE::GREEN_BUTTON][Default_Texture_Index].loadFromFile("assets/mainmenu/green_button.png");
	textures[TEXTURE_TYPE::WOOD_BUTTON][Default_Texture_Index].loadFromFile("assets/mainmenu/wood_button.png");
	textures[TEXTURE_TYPE::MAINMENU_BACKGROUND][Default_Texture_Index].loadFromFile("assets/mainmenu/mainmenu_background.png");
	textures[TEXTURE_TYPE::STORE_WINDOW][Default_Texture_Index].loadFromFile("assets/mainmenu/store_window.png");
	textures[TEXTURE_TYPE::LEVEL_SELECTOR_BACKGROUND][Default_Texture_Index].loadFromFile("assets/mainmenu/level_selector_background.png");
	textures[TEXTURE_TYPE::LEVEL_SELECTOR_BACKGROUND_BACK][Default_Texture_Index].loadFromFile("assets/mainmenu/level_selector_background_back.png");
	textures[TEXTURE_TYPE::LEVEL_OPTION_BACKGROUND][Default_Texture_Index].loadFromFile("assets/mainmenu/level_option_background.png");
	textures[TEXTURE_TYPE::LEVEL_PLAY_BUTTON][Default_Texture_Index].loadFromFile("assets/mainmenu/play_button.png");
	textures[TEXTURE_TYPE::GRID_ROLL][Default_Texture_Index].loadFromFile("assets/grid_roll.png");
	textures[TEXTURE_TYPE::LUSTER][Default_Texture_Index].loadFromFile("assets/luster.png");
	textures[TEXTURE_TYPE::SEED_CHOOSER][Default_Texture_Index].loadFromFile("assets/seedchooser.png");
	textures[TEXTURE_TYPE::SEED_CHOOSER_BUTTON][Default_Texture_Index].loadFromFile("assets/seedchooser_button.png");
	textures[TEXTURE_TYPE::SUN][Default_Texture_Index].loadFromFile("assets/sun.png");
	textures[TEXTURE_TYPE::COIN][Default_Texture_Index].loadFromFile("assets/coin.png");
	textures[TEXTURE_TYPE::SEEDBANK][Default_Texture_Index].loadFromFile("assets/seedbank.png");
	textures[TEXTURE_TYPE::SEEDPACKET][Default_Texture_Index].loadFromFile("assets/seedpacket.png");
	textures[TEXTURE_TYPE::SHOVEL][Default_Texture_Index].loadFromFile("assets/shovel.png");
	textures[TEXTURE_TYPE::MAGIC_SHOVEL][Default_Texture_Index].loadFromFile("assets/relocate.png");
	textures[TEXTURE_TYPE::BOOSTER][Default_Texture_Index].loadFromFile("assets/booster.png");
	textures[TEXTURE_TYPE::PROGRESS_EMPTY][Default_Texture_Index].loadFromFile("assets/progress_empty.png");
	textures[TEXTURE_TYPE::PROGRESS_FULL][Default_Texture_Index].loadFromFile("assets/progress_full.png");
	textures[TEXTURE_TYPE::PROGRESS_HEAD][Default_Texture_Index].loadFromFile("assets/progress_head.png");
	textures[TEXTURE_TYPE::ITEM_CONE][Default_Texture_Index].loadFromFile("assets/item_cone.png");
	textures[TEXTURE_TYPE::ITEM_NEWSPAPER][Default_Texture_Index].loadFromFile("assets/item_newspaper.png");
	textures[TEXTURE_TYPE::ITEM_PHAROAH_CASKET][Default_Texture_Index].loadFromFile("assets/item_pharoah_casket.png");

	// Font
	font = new Font();
	font->loadFromFile("assets/impact.ttf");

	// General Plants to any level

	vector<Entity> mainPlantTypes = { sunflower, repeaterPlant, normalPlant, wallnut, chomper, icePeashooter, miniShroom, POTATOMINE, doomShroom, CHEERY_BOMB, iceShroom };//, shroom, sunshroom };

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
		isEntityUnlocked[iceShroom.type] =
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
//---------------------------------------
	// Level 2: Night
	levels[2].lvlNum = 2;
	levels[2].plant_types = mainPlantTypes;
	levels[2].zombie_types = { zombie, zombieConehead, zombieNewspaper, zombieAllStar, zombieShieldGuard };
	levels[2].targetZombie = 25;
	levels[2].startingSuns = 100;
	levels[2].offsetX = 450;
	levels[2].offsetY = 135;
	levels[2].cellWidth = 150;
	levels[2].cellHeight = 170;
	levels[2].plantReward = shroom;
	levels[2].coinReward = 200;
	setupLevelLawnmowers(levels[2]);

	//------------------------------------------
	// Level 3: Egypt
	levels[3].lvlNum = 3;
	levels[3].plant_types = mainPlantTypes;
	levels[3].plant_types.push_back(shroom);
	levels[3].zombie_types = { zombieTorch, zombieRa, zombieMummy, zombiePharaoh };
	levels[3].targetZombie = 30;
	levels[3].addReward(500);
	levels[3].addReward(wallnut);
	levels[3].offsetX = 450;
	levels[3].offsetY = 135;
	levels[3].cellWidth = 150;
	levels[3].cellHeight = 170;
	levels[3].plantReward = wallnut;
	levels[3].coinReward = 300;
	setupLevelLawnmowers(levels[3]);

	//------------------------------------------
	// Level 4: Wild West
	levels[4].lvlNum = 4;
	levels[4].plant_types = mainPlantTypes;
	levels[4].zombie_types = { zombie, zombieConehead, zombieNewspaper, zombieAllStar };
	levels[4].targetZombie = 30;
	levels[4].waveLimit = 12;
	levels[4].waveTimer = 15;
	levels[4].startingSuns = 3000;
	levels[4].isUnlocked = true;
	levels[4].offsetX = 450;
	levels[4].offsetY = 135;
	levels[4].cellWidth = 150;
	levels[4].cellHeight = 170;
	levels[4].plantReward = repeaterPlant;
	levels[4].coinReward = 400;
	setupLevelLawnmowers(levels[4]);
	//------------------------------------------

	// Level 5: Squaid Game
	levels[5].lvlNum = 5;
	levels[5].plant_types = mainPlantTypes;
	levels[5].zombie_types = { zombie, zombieConehead, zombiePharaoh, zombieAllStar, zombieBig };
	levels[5].targetZombie = 40;
	levels[5].waveLimit = 15;
	levels[5].waveTimer = 18;
	levels[5].startingSuns = 50;
	levels[5].isUnlocked = true;
	levels[5].offsetX = 450;
	levels[5].offsetY = 135;
	levels[5].cellWidth = 150;
	levels[5].cellHeight = 170;
	setupLevelLawnmowers(levels[5]);



	// setting levels background textures //bravooo
	textures[TEXTURE_TYPE::LEVEL_BACKGROUND][0].loadFromFile("assets/levels/level1.png");
	textures[TEXTURE_TYPE::LEVEL_BACKGROUND_HOUSE][0].loadFromFile("assets/levels/level1_house.png");
	for (int lvl = 1; lvl <= number_levels; lvl++) {
		cout << "Loading level " << lvl << " files" << endl;
		levels[lvl].lvlNum = lvl;

		textures[TEXTURE_TYPE::LEVEL_BACKGROUND][lvl].loadFromFile("assets/levels/level" + to_string(lvl) + ".png");
		textures[TEXTURE_TYPE::LEVEL_BACKGROUND_HOUSE][lvl].loadFromFile("assets/levels/level" + to_string(lvl) + "_house.png");
		textures[TEXTURE_TYPE::LEVEL_BACKGROUND_OPENDOOR][lvl].loadFromFile("assets/levels/level" + to_string(lvl) + "_opendoor.png");
		textures[TEXTURE_TYPE::LEVEL_BACKGROUND_DIRT][lvl].loadFromFile("assets/levels/level" + to_string(lvl) + "_dirt.png");
		textures[TEXTURE_TYPE::LEVEL_ICON][lvl].loadFromFile("assets/levels/level" + to_string(lvl) + "_icon.png");

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

	gameClock.restart();
	while (window.isOpen()) {
		sf::Time dtTime = DeltaTimeClock.restart();
		delta_time = dtTime.asSeconds();
		mousePos = Mouse::getPosition(window);

		mousePos.x *= (float)windowWidth / window.getSize().x;
		mousePos.y *= (float)windowHeight / window.getSize().y;

		uiInteracted = false;
		mouseJustClicked[0] = false;
		mouseJustClicked[1] = false;

		while (window.pollEvent(event)) {
			if (event.type == Event::Closed) {
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
			} else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::F11) {
				fullscreen = !fullscreen;
				if (fullscreen) {
					oldPos = window.getPosition();
					oldSize = window.getSize();
				}
				window.close();
				if (fullscreen) {
					window.create(VideoMode::getDesktopMode(), "Plants vs Zombies", Style::Fullscreen);
				} else {
					window.create(VideoMode(windowWidth, windowHeight), "Plants vs Zombies");
					window.setPosition(oldPos);
					window.setSize(oldSize);
				}
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