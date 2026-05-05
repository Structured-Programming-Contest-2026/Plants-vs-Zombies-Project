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

// Memory
#define MAX_ENTITIES 512
#define MAX_FALLENITEMS 32
#define MAX_BULLETS 1024

#define gridraws 5
#define gridcols 9
//#define cellWidth 150
//#define cellHeight 170
#define edgespace 250
#define z_eat_p_distance 100
#define MAX_SUNS 16
#define Default_Texture_Index 0

bool DEV = true;
// Empty the storage.txt

bool gamePaused = false;
Clock gamePauseClock;
Time totalPauseTime = Time::Zero;

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
int bulletShot = 0; // boosted pomegranate
bool isGameOver = false, isGameWon = false;

// Dragon
bool f_zombotDragon = true, entered_zombotDragon = false;

bool gameWonActionExecuted = false;
bool hugeWaveTriggered = false;

void TriggerHugeWave() {
	if (hugeWaveTriggered) return;
	hugeWaveTriggered = true;

	std::cout << "HUGE WAVE\n";

	sf::Music* hw = new sf::Music();
	if (hw->openFromFile("assets/audio/03. hugewave.ogg")) {
		hw->setLoop(false);
		hw->play();
	}
}

bool firstZombieTriggered = false;

void TriggerFirstZombie() {
	if (firstZombieTriggered) return;
	firstZombieTriggered = true;

	sf::Music* sfx = new sf::Music();
	if (sfx->openFromFile("assets/audio/Voices awooga.ogg")) {
		sfx->setLoop(false);
		sfx->play();
	}
}

void ExecuteGameWonAction() {
	sf::Music* winMusic = new sf::Music();
	if (winMusic->openFromFile("assets/audio/26. winmusic.ogg")) {
		winMusic->setLoop(false);
		winMusic->play();
	}
}

int leftEdgeForZombies = 225;	// TODO: Adjust with lawnmower and stuff
int deadZombieCount = 0, createdZombiesCount = 0;
int curWaveZombieCnt = 0;	// each wave zombie counter
int lawnmower_backoff = 150,	// from beginning edge
lawnmower_endline = windowWidth; // which x pos to stop at
//Sending Zombies
float nextZombieTimer = 10; // First zombie timer - it's assigned, current value is only used for FIRST ZOMBIE
//float nextZombieTimer = 1; // For TESTING
float zombieClockOffset = 0;
float ZombieMinGapTime = 7,
//float nextZombieTimer = 1; // First zombie timer - it's assigned, current value is only used for FIRST ZOMBIE
//float ZombieMinGapTime = 1,

ZombieMaxGapTime = ZombieMinGapTime + 3,
ZombieGap_dt = 0.1;	// Level Pace
int selectedPlant = 0;
int sunsMoney = 3000;
int coinsMoney = 0;


// music request states
const int REQUEST_NONE = -1;
const int REQUEST_SELECT_MUSIC = -2; // ask main loop to play select-levels background
int requestedLevelMusic = REQUEST_NONE;
bool loseMusicPlayed = false;
int pause_music(Music& music);


sf::Music* g_music = nullptr;
sf::Music* pauseMusic = nullptr;

float wincooldowntimer = 2.f;
bool isGameWonCoolDownStarted = false;

// Clocks
Clock gameClock;
Clock zombieClock, waveClockX;
Clock sunSpawnClock;
Clock wc;
Time wcOffset;

//shovel
bool isShovelActive = false;

//relocate
bool isRelocateActive = false;
bool isRelocating = false;
int selectedPlantIndex = -1;

// buttons positions
Vector2f addBoosterBtnPos = { 0,0 };	// updated after loading textures
Vector2f activateBoosterBtnPos = { 0,0 };	// updated after loading textures

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
struct Rail {
	int column;
	int startRow;
	int endRow;
};

struct Minecart {
	int column;
	int currentRow;
	int plantId = -1;
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
	ZOMBIE_POGO,
	ZOMBIE_PROSPECTOR,
	ZOMBIE_REDHAIR,
	ZOMBIE_COWBOY,
	ZOMBIE_PIANIST,
	ZOMBIE_CHICKENWRANGLER,
	ZOMBIE_CHICKEN,
	ZOMBOT_DRAGON,
	ZOMBIE_ROMAN_CONEHEAD,
	ZOMBIE_ROMAN_BUCKETHEAD,
	ZOMBIE_ROMAN_CENTURION,
	ZOMBIE_ROMAN_BUSTHEAD,
	ZOMBIE_ROMAN_ODESSA,
	ZOMBIE_ROMAN_SHIELD,
	ZOMBIE_ROMAN_NORMAL,
	ZOMBIE_ROMAN_GARGANTUAR,
	ZOMBIE_LOSTCITY_GARGANTUAR,

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
	PLANT_SNAPDRAGON,
	PLANT_LIGHTNING_REED,
	PLANT_INFERNO,
	PLANT_RED_SHOOTER,
	PLANT_POMEGRANATE,


	NormalBullet,
	IceBullet,
	ShroomBullet,
	SnapDragonBullet,
	LightningBullet,
	ZombotDragonBullet,
	FireTornadoBullet,
	SnapDragonBoostBullet,
	FireTornadoBulletBoosted,
	PomegranateBullet,
	PomegranateBulletBoost,
	RedBullet,
	FireDragonBullet,

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
	LEVEL_RIBBON_EASY,
	LEVEL_RIBBON_MEDIUM,
	LEVEL_RIBBON_HARD,
	GRID_ROLL,
	LUSTER,
	SEED_CHOOSER,
	SEED_CHOOSER_BUTTON,
	SUN,
	COIN,
	SEEDBANK,
	SEEDPACKET,
	BOOSTER,
	LAWNMOWER,

	PROGRESS_EMPTY,
	PROGRESS_FULL,
	PROGRESS_HEAD,

	ITEM_CONE,
	ITEM_NEWSPAPER,
	ITEM_PHAROAH_CASKET,
	ITEM_JAR,
	ITEM_HEAD,

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
	ZOMBIE_POGO,
	ZOMBIE_PROSPECTOR,
	ZOMBIE_REDHAIR,
	ZOMBIE_COWBOY,
	ZOMBIE_ELECTROCUTED,//only death not real zombie
	ZOMBIE_PIANIST,
	ZOMBIE_CHICKENWRANGLER,
	ZOMBIE_CHICKEN,
	ZOMBOT_DRAGON,
	ZOMBIE_ROMAN_CONEHEAD,
	ZOMBIE_ROMAN_BUCKETHEAD,
	ZOMBIE_ROMAN_CENTURION,
	ZOMBIE_ROMAN_BUSTHEAD,
	ZOMBIE_ROMAN_ODESSA,
	ZOMBIE_ROMAN_SHIELD,
	ZOMBIE_ROMAN_NORMAL,
	ZOMBIE_ROMAN_GARGANTUAR,
	ZOMBIE_LOSTCITY_GARGANTUAR,

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
	PLANT_SNAPDRAGON,
	PLANT_LIGHTNING_REED,
	PLANT_INFERNO,
	PLANT_POMEGRANATE,
	PLANT_RED_SHOOTER,


	NormalBullet,
	IceBullet,
	ShroomBullet,
	SnapDragonBullet,
	LightningBullet,
	ZombotDragonBullet,
	FireTornadoBullet,
	SnapDragonBoostBullet,
	FireTornadoBulletBoosted,
	PomegranateBullet,
	PomegranateBulletBoost,
	RedBullet,
	FireDragonBullet,

	SHOVEL,
	MAGIC_SHOVEL,
	RAIL,
	CART,
};
enum class THEME : uint8_t {
	NONE = 0,
	DAY,
	NIGHT,
	ANCIENT,
	WILDWEST,
	LOST_CITY,
	DARK_AGES,
	JURASSIC_AGE,
	ROMAN,

	COUNT	// keep it last
};

map < ENTITY_TYPE, bool> isEntityUnlocked;

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
	case ENTITY_TYPE::ZOMBIE_POGO:
		textureType = TEXTURE_TYPE::ZOMBIE_POGO;
		break;
	case ENTITY_TYPE::ZOMBIE_PROSPECTOR:
		textureType = TEXTURE_TYPE::ZOMBIE_PROSPECTOR;
		break;
	case ENTITY_TYPE::ZOMBIE_REDHAIR:
		textureType = TEXTURE_TYPE::ZOMBIE_REDHAIR;
		break;
	case ENTITY_TYPE::ZOMBIE_COWBOY:
		textureType = TEXTURE_TYPE::ZOMBIE_COWBOY;
		break;
	case ENTITY_TYPE::ZOMBIE_PIANIST:
		textureType = TEXTURE_TYPE::ZOMBIE_PIANIST;
		break;
	case ENTITY_TYPE::ZOMBIE_CHICKENWRANGLER:
		textureType = TEXTURE_TYPE::ZOMBIE_CHICKENWRANGLER;
		break;
	case ENTITY_TYPE::ZOMBIE_CHICKEN:
		textureType = TEXTURE_TYPE::ZOMBIE_CHICKEN;
		break;
	case ENTITY_TYPE::ZOMBIE_ROMAN_CONEHEAD:
		textureType = TEXTURE_TYPE::ZOMBIE_ROMAN_CONEHEAD;
		break;
	case ENTITY_TYPE::ZOMBIE_ROMAN_BUCKETHEAD:
		textureType = TEXTURE_TYPE::ZOMBIE_ROMAN_BUCKETHEAD;
		break;
	case ENTITY_TYPE::ZOMBIE_ROMAN_CENTURION:
		textureType = TEXTURE_TYPE::ZOMBIE_ROMAN_CENTURION;
		break;
	case ENTITY_TYPE::ZOMBIE_ROMAN_BUSTHEAD:
		textureType = TEXTURE_TYPE::ZOMBIE_ROMAN_BUSTHEAD;
		break;
	case ENTITY_TYPE::ZOMBIE_ROMAN_ODESSA:
		textureType = TEXTURE_TYPE::ZOMBIE_ROMAN_ODESSA;
		break;
	case ENTITY_TYPE::ZOMBIE_ROMAN_SHIELD:
		textureType = TEXTURE_TYPE::ZOMBIE_ROMAN_SHIELD;
		break;
	case ENTITY_TYPE::ZOMBIE_ROMAN_NORMAL:
		textureType = TEXTURE_TYPE::ZOMBIE_ROMAN_NORMAL;
		break;
	case ENTITY_TYPE::ZOMBIE_ROMAN_GARGANTUAR:
		textureType = TEXTURE_TYPE::ZOMBIE_ROMAN_GARGANTUAR;
		break;
	case ENTITY_TYPE::ZOMBIE_LOSTCITY_GARGANTUAR:
		textureType = TEXTURE_TYPE::ZOMBIE_LOSTCITY_GARGANTUAR;
		break;
	case ENTITY_TYPE::ZOMBOT_DRAGON:
		textureType = TEXTURE_TYPE::ZOMBOT_DRAGON;
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
	case ENTITY_TYPE::PLANT_SNAPDRAGON:
		textureType = TEXTURE_TYPE::PLANT_SNAPDRAGON;
		break;
	case ENTITY_TYPE::PLANT_LIGHTNING_REED:
		textureType = TEXTURE_TYPE::PLANT_LIGHTNING_REED;
		break;
	case ENTITY_TYPE::PLANT_INFERNO:
		textureType = TEXTURE_TYPE::PLANT_INFERNO;
		break;
	case ENTITY_TYPE::PLANT_POMEGRANATE:
		textureType = TEXTURE_TYPE::PLANT_POMEGRANATE;
		break;
	case ENTITY_TYPE::PLANT_RED_SHOOTER:
		textureType = TEXTURE_TYPE::PLANT_RED_SHOOTER;
		break;

	case ENTITY_TYPE::FireDragonBullet:
		textureType = TEXTURE_TYPE::FireDragonBullet;
		break;
	case ENTITY_TYPE::LightningBullet:
		textureType = TEXTURE_TYPE::LightningBullet;
		break;
	case ENTITY_TYPE::FireTornadoBullet:
		textureType = TEXTURE_TYPE::FireTornadoBullet;
		break;
	case ENTITY_TYPE::FireTornadoBulletBoosted:
		textureType = TEXTURE_TYPE::FireTornadoBulletBoosted;
		break;

	case ENTITY_TYPE::ShroomBullet:
		textureType = TEXTURE_TYPE::ShroomBullet;
		break;
	case ENTITY_TYPE::ZombotDragonBullet:
		textureType = TEXTURE_TYPE::ZombotDragonBullet;
		break;

	case ENTITY_TYPE::RedBullet:
		textureType = TEXTURE_TYPE::RedBullet;
		break;
	case ENTITY_TYPE::SnapDragonBoostBullet:
		textureType = TEXTURE_TYPE::SnapDragonBoostBullet;
		break;
	case ENTITY_TYPE::PomegranateBullet:
		textureType = TEXTURE_TYPE::PomegranateBullet;
		break;
	case ENTITY_TYPE::PomegranateBulletBoost:
		textureType = TEXTURE_TYPE::PomegranateBulletBoost;
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
	float x_vel = 0.05f;
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

	float seedCooldown = 1.0f;
	float currentSeedCooldown = 0.0f;

	int num_animations = 1,
		currentAnimationIndex = 0;
	Animation animations[10];
	Animation currentAnimation;

	int AnimationForCycles = -1;

	float freezeTimer = 0.0f;
	bool isSuperFrozen = false;

	Bullet bullet;

	float x_vel = 0.04f;
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

	bool boostedBulletHitZombot = false;	// for dragon zombot

	bool hasTackled = false; // for allstar
	bool isTackling = false; // for allstar

	bool shellDestroyed = false;   // for pharaoh
	bool playingShellAnim = false; // for pharaoh
	Animation shellAnim;           // for pharaoh

	bool isBoosted = false;
	float boostAttackTimer = 0.1;

	bool isDead = false;
	function<void(Entity& entity)> afterAnimation = [](Entity&) {};	// called after animation finish (if animationForCycles != -1)

	// Note on afterAnimation : Avoid using [&localvariable] .. it is DANGEROUS might cause dangling pointer

	bool diedByElectrocution = false;//for death by lightning or fire

	// Updates width and height whenever Animation is changed - ex: To calculate zombie position properly
	void updateDimensions() {
		if (currentAnimation.frames == 0) {
			cout << "ERROR: Switching To Animation with 0 frames...";
			return;
		}

		TEXTURE_TYPE tType = getTextureType(type);
		int tIndex = currentAnimationIndex;

		if (category == ENTITY_CATEGORY::ZOMBIE && diedByElectrocution && currentAnimationIndex == 9) {
			tType = TEXTURE_TYPE::ZOMBIE_ELECTROCUTED;
			tIndex = 0;
		}

		Texture tex = getTexture(tType, tIndex);
		int w = ((float)tex.getSize().x / currentAnimation.frames) * scale;
		int h = (float)tex.getSize().y * scale;

		width = w, height = h;
	}
	void setCurrentAnimation(int idx) {
		// You MUST change both
		//cout << "set current animation idx to " << idx << " which has " << animations[idx].frames << " frames , by entity of type " << int(type) << endl;
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
		if (gamePaused) return;
		if (isSuperFrozen) return;
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
		//if (currentAnimationIndex == idx) return;

		// Add a log inside DoAnimation to verify it's actually switching:
		if (type == ENTITY_TYPE::ZOMBOT_DRAGON) {
			cout << "DoAnimation called: idx=" << idx << " currentAnimationIndex=" << currentAnimationIndex << endl;
			if (currentAnimationIndex == idx) { cout << "SKIPPED - same index\n"; return; }
		} else if (currentAnimationIndex == idx) return;

		// forCycles = (-1)  ---> Infinite / Manual change
		AnimationForCycles = forCycles;
		setCurrentAnimation(idx);
		updateDimensions();
		cout << "Doing animation " << idx << " with currentAnimationIndex=" << currentAnimationIndex << endl;
	}

	int boostCycles = 10;
	int boostAnimationIdx = 3;
	//float boostScale = 1.0;
	void boost() {	// notice that attack clock is adapted to whether you are boosted or not
		int orgAttackTimer = attackTimer;
		attackTimer = boostAttackTimer;
		isBoosted = true;
		float orgScale = scale;
		//scale = boostScale;
		DoAnimation(boostCycles, boostAnimationIdx); // ntimes/eachcycletakes + boolmod
		afterAnimation = [orgAttackTimer, orgScale](Entity& p) {
			p.attackTimer = orgAttackTimer;
			p.isBoosted = false;
			p.scale = orgScale;
			};
	}

	void (*updateEntity)(Entity& entity) = nullptr; // This function should not have problems with Global variables .. but local variables (on its definition) are probably not captured
};

struct FallenItem {
	TEXTURE_TYPE texture;
	Clock clock;
	Vector2f fallPosition;
	Color color;
};
FallenItem fallenItems[MAX_FALLENITEMS];

void SpawnFallingItem(TEXTURE_TYPE tex, Vector2f position, Color color = Color::White) {
	FallenItem item;
	item.texture = tex;
	item.clock.restart();
	item.fallPosition = position;
	item.color = color;
	for (int i = 0; i < MAX_FALLENITEMS; i++) {
		if (fallenItems[i].clock.getElapsedTime().asSeconds() >= 3.5f) {
			fallenItems[i] = item;
			break;
		}
	}
}

map<ENTITY_TYPE, Entity> plants;

Entity entities[MAX_ENTITIES];

int addEntity(Entity& entity) {
	for (int i = 0; i < MAX_ENTITIES; i++) {
		if (entities[i].category == ENTITY_CATEGORY::NONE) {
			entities[i] = entity;
			return i;
		}
	}

	return -1;
}

Entity bullets[MAX_BULLETS];
int addBullet(Entity& bullet) {
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (bullets[i].category == ENTITY_CATEGORY::NONE) {
			bullets[i] = bullet;
			return i;
		}
	}

	return -1;
}

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

struct Wave {
	int zombieCount = 0;
	float length = 0; // in seconds
	vector<Entity> zombieTypes;
	bool isHuge = false;
	Wave(int zombieCount = 0, float length = 0, vector<Entity> zombieTypes = {}, bool isHuge = false) {
		this->zombieCount = zombieCount;
		this->length = length; // in seconds
		this->zombieTypes = zombieTypes;
		this->isHuge = isHuge;
	}
	//Wave(float every = 1.f, vector<Entity> zombieTypes, bool isHuge = false) {
	//	this->zombieCount = zombieCount;
	//	this->length = length; // in seconds
	//	this->zombieTypes = zombieTypes;
	//	this->isHuge = isHuge;
	//}
};

struct Level {
	THEME theme = THEME::NONE;
	vector<Entity> zombie_types;
	vector<Entity> plant_types;
	int startingSuns = 50;
	float sunSpawnInterval = 7.0f;
	bool arePlantsSelected = false;
	bool isUnlocked = false;
	vector<Entity> selectedPlants;
	vector<Wave> waves;
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
	Clock winClock;

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
	// Sums waves zombies as targetZombie
	void setTargetZombie() {
		targetZombie = 0;
		for (auto& w : waves) targetZombie += w.zombieCount;
	}
	void addWave(int zombieCount = 0, float length = 0, vector<Entity> zombieTypes = {}, float rest = 0, bool isHuge = false) {
		waves.push_back(Wave(zombieCount, length, zombieTypes, isHuge));
		if (rest) waves.push_back(Wave(0, rest, {}, false));	// rest wave
		for (auto& type : zombieTypes) {
			bool exists = false;
			for (auto& t : zombie_types) {
				if (t.type == type.type) {
					exists = true;
					break;
				}
			}
			if (!exists) zombie_types.push_back(type);
		}
		this->setTargetZombie();
	}

	//for mincart
	vector<Rail> rails;
	vector<Minecart> carts;
	void addRail(int col, int sRow, int eRow) {
		rails.push_back({ col, sRow, eRow });
		carts.push_back({ col, sRow, -1 });
	}

};
const int number_levels = 24;
vector<Level> levels(number_levels + 1);
Level* currentLevel = nullptr;
//Wave* currentLevel->waves[currentWaveIndex] = nullptr;   // Removed .. cuz when you push_back to waves (as in zombot), you get a dangling currentWave pointer
int currentWaveIndex = 0;


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
int maxBoosterCnt = 200;	// For DEV
//int maxBoosterCnt = 20;
float boosterSpawnDuration = 10.0f;
//float boosterSpawnDuration = 1.0f;
float boosterLifeDuration = 3.0f;
//float boosterSpawnDuration = 10.0f;
//float boosterLifeDuration = 5.0f;

// Magic Shovel
int curMagicShovelCnt = 1;

//bool isBoostActive = false;
//int maxMagicShovelCnt = 20;
//float boosterSpawnDuration = 10.0f;
//float boosterSpawnDuration = 1.0f;
//float boosterLifeDuration = 3.0f;
struct ShopItem {
	int cost = 0;
	string title;
	TEXTURE_TYPE textureType;
	int* counter = nullptr;	// reference to the item counter (ex: booster count, magic shovel count, etc ..)
	int maxCount = -1;	// 0 for infinite
	bool isUnlocked = true;
	void buy(int amnt) {
		while (amnt--) {
			if (coinsMoney >= cost && (counter == nullptr || *counter < maxCount || maxCount == -1)) {
				coinsMoney -= cost;
				if (counter != nullptr) (*counter)++;
				cout << "Purchased " << title << "! Remaining coins: " << coinsMoney << endl;
			} else {
				// message 
				cout << "Not enough coins to purchase " << title << "! Required: " << cost << ", Available: " << coinsMoney << endl;
				break;
			}
		}
	}
};
struct Booster {
	Clock spawnClock;
	Clock lifeClock;
	float minSpawnDuration = boosterSpawnDuration;
	float maxLifeDuration = boosterLifeDuration;
	// Shop
	int cost = 50;

	Vector2f scale = { 0.5f, 0.5f };
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
		plant.boost();
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

// Shop items
ShopItem shopItems[2] = {
	{50, "Booster", TEXTURE_TYPE::BOOSTER, &curBoosterCnt, maxBoosterCnt, false},
	{100, "Magic Shovel", TEXTURE_TYPE::MAGIC_SHOVEL, &curMagicShovelCnt, 1000000, false}
};

// Plants - zombies - allplanttypes
Entity normalPlant, repeaterPlant, wallnut, POTATOMINE, sunflower, chomper, shroom, sunshroom, icePeashooter, miniShroom, doomShroom, CHEERY_BOMB, iceShroom, snapdragon, lightningReed, inferno, redShooterPlant, Pomegranate;
Entity normalZombie, zombieJapanese, zombieBig, zombieTorch, zombieRa, zombieMummy, zombieConehead, zombieNewspaper, zombieAllStar, zombiePharaoh, zombieShieldGuard, zombieCowboy, zombieRedhair, zombieProspector, zombiePogo, zombiePianist, zombieChickenWrangler, zombieChicken, zombotDragon, zombieRomanConehead, zombieRomanBuckethead, zombieRomanCenturion, zombieRomanBusthead, zombieRomanOdessa, zombieRomanShield, zombieRomanNormal, zombieRomanGargantuar, zombieLostCityGargantuar;
vector<Entity> allPlantTypes;

// Suns
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

	TEXTURE_TYPE texType = getTextureType(entity.type);
	int texIndex = entity.currentAnimationIndex;

	if (entity.category == ENTITY_CATEGORY::ZOMBIE && entity.diedByElectrocution && texIndex == 9) {
		texType = TEXTURE_TYPE::ZOMBIE_ELECTROCUTED;
		texIndex = 0;
	}

	Texture& tex = textures[texType][texIndex];


	float frameWidthF = float(tex.getSize().x) / float(entity.currentAnimation.frames);
	int frameWidth = round(frameWidthF);
	int frameHeight = tex.getSize().y;

	// next frame + check animation finish
	// DEBUG: NOT SURE BUT IT'S REPEATED
	 //if ((!isGameOver && (!isGameWon || currentLevel->winClock.getElapsedTime().asSeconds() < wincooldowntimer)) || entity.enteredHome) entity.nextFrame();

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

	//if (zombie.boostedBulletHitZombot && zombie.type == ENTITY_TYPE::ZOMBOT_DRAGON) return false; // for dragon - dont hit zombie twice same bullet

	zombie.health -= attack;
	zombie.lastDamaged = currentLevel->clock.getElapsedTime();

	if (zombie.type == ENTITY_TYPE::ZOMBOT_DRAGON) cout << "------------------------ damaged zombot daragon attack " << attack << " health " << zombie.health << endl;

	//cout << "from `attackzombie` Zombie attacked! health now: " << zombie.health << endl;
	if (zombie.health <= 0 && !withDyingAnimation) {
		zombie.category = ENTITY_CATEGORY::NONE;
		zombie.isDead = true, deadZombieCount++, ZombieGap_dt += 0.05;
		cout << "Zombie died! total dead: " << deadZombieCount << " isDead = " << zombie.isDead << endl;
		return true;
	}
	// Arise a Booster if time is good
	if (zombie.health <= 0) CheckTimer_AddBooster(true, zombie.position);


	//if (zombie.type == ENTITY_TYPE::ZOMBOT_DRAGON) zombie.boostedBulletHitZombot = true;	// for dragon
	return true;
}

// Reset Level
void resetLevel() {

	delta_time = 1.0f;
	uiInteracted = false;
	nextEntityId = 0; // for ra
	isGameOver = false, isGameWon = false;
	isGameWonCoolDownStarted = false;
	gameWonActionExecuted = false;
	deadZombieCount = 0, createdZombiesCount = 0;
	firstZombieTriggered = false;
	//Sending Zombies
	nextZombieTimer = 5; // First zombie timer - it's assigned, current value is only used for FIRST ZOMBIE
	//ZombieMinGapTime = 7
	//	ZombieMaxGapTime = ZombieMinGapTime + 3,
	//	ZombieGap_dt = 0.1;	// Level Pace
	curWaveZombieCnt = 0;

	f_zombotDragon = true, entered_zombotDragon = false;

	selectedPlant = 0;

	sunsMoney = currentLevel->startingSuns;

	//curBoosterCnt = 0;
	isBoostActive = false;

	for (int i = 0; i < gridRows; i++) {
		currentLevel->lawnmowers[i].setRow(i, currentLevel->offsetX, currentLevel->offsetY, currentLevel->cellHeight);
	}
	for (int i = 0; i < MAX_ENTITIES; i++) {
		entities[i] = Entity{};
	}
	for (int i = 0; i < MAX_BULLETS; i++) {
		bullets[i] = Entity{};
	}
	zombieClock.restart();
	waveClockX.restart();
	sunSpawnClock.restart();
	for (int i = 0; i < MAX_FALLENITEMS; i++) {
		fallenItems[i] = FallenItem{};
	}

	currentLevel->plantsSelectedTime = Time::Zero;
	currentLevel->arePlantsSelected = false;
	currentLevel->selectedPlants.clear();

	totalPauseTime = Time::Zero;
	wcOffset = Time::Zero;
	zombieClockOffset = 0;
	loseMusicPlayed = false;
	gamePaused = false;
}

void startLevel(int level) {
	// Set currentLevel and other global variables based on the selected level
	if (level < levels.size()) {
		cout << "selecting lvl " << level << " who whas " << levels[level].zombie_types.size() << endl;
		currentLevel = new Level(levels[level]);	// Note: level is 1 indexed
		currentWaveIndex = 0;

		sunsMoney = currentLevel->startingSuns;
		isGameOver = false, isGameWon = false;
		Texture& groundTex = textures[TEXTURE_TYPE::LEVEL_BACKGROUND][int(levels[level].theme)];
		//cout << "txt size " << groundTex.getSize().x << endl;
		// 
		//ground.setTexture(textures[TEXTURE_TYPE::LEVEL_BACKGROUND][(currentLevel ? currentLevel->lvlNum : 0)]);
		ground.setTexture(textures[TEXTURE_TYPE::LEVEL_BACKGROUND][(currentLevel ? int(currentLevel->theme) : 0)]);

		//ground.setTextureRect({ 0, 0, windowWidth, windowHeight });	// PROBLEM - Doesnt take the whole texture size, but only the part of it that corresponds to the window size, so if texture is smaller than window, it will be repeated, if it's bigger, it will be cropped
		ground.setTextureRect({ 0, 0, (int)groundTex.getSize().x, (int)groundTex.getSize().y });
		ground.setScale
		(
			(float)windowWidth / (float)groundTex.getSize().x,
			(float)windowHeight / (float)groundTex.getSize().y
		);

		//For different independent Lawnmower
		Texture& mowerTex = textures[TEXTURE_TYPE::LAWNMOWER][int(levels[level].theme)];
		//Texture& mowerTex = textures[TEXTURE_TYPE::LAWNMOWER][level];
		lawnmowerSprite.setTexture(mowerTex);
		lawnmowerSprite.setTextureRect(IntRect(0, 0, (int)mowerTex.getSize().x, (int)mowerTex.getSize().y));

		currentLevel->clock.restart();
		currentLevel->lvlNum = level;
	}
}

void nextLevel() {
	if (currentLevel == nullptr) {
		currentLevel = &levels[1];
		currentWaveIndex = 0;
	} else if (currentLevel->lvlNum < number_levels) {
		currentLevel = &levels[currentLevel->lvlNum + 1];
		currentWaveIndex = 0;
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
	bullet.attack = 30;
	bullet.x_vel = -580.0f;


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
	bullet.x_vel = -580.0f;

	return bullet;
}
Entity defineShroomBullet() {
	Texture tex;
	tex.loadFromFile("assets/ShroomBullet.png");
	textures[TEXTURE_TYPE::ShroomBullet][Default_Texture_Index] = tex;

	Entity bullet{};
	bullet.scale = 0.2;
	bullet.category = ENTITY_CATEGORY::Bullet;
	bullet.type = ENTITY_TYPE::ShroomBullet;
	bullet.attack = 5;
	bullet.x_vel = -580.0f;

	return bullet;
}
Entity defineSnapDragonBullet() {
	Texture tex;
	tex.loadFromFile("assets/fire 38-150-1.png");
	textures[TEXTURE_TYPE::SnapDragonBullet][Default_Texture_Index] = tex;

	Entity fire{};
	fire.category = ENTITY_CATEGORY::Bullet;
	fire.type = ENTITY_TYPE::SnapDragonBullet;
	fire.attack = 500;
	fire.x_vel = -200.0f;
	fire.scale = 1.0f;

	fire.num_animations = 1;
	fire.animations[0].frames = 1;
	//fire.animations[0].frames = 20;
	fire.animations[0].frameTimer = 10.f;
	fire.setCurrentAnimation(0);

	return fire;
}
Entity defineSnapDragonBoostedBullet() {
	Texture tex;
	tex.loadFromFile("assets/snapdragon-bullet-boost.png");
	textures[TEXTURE_TYPE::SnapDragonBoostBullet][Default_Texture_Index] = tex;

	Entity dragon{};
	dragon.category = ENTITY_CATEGORY::Bullet;
	dragon.type = ENTITY_TYPE::SnapDragonBoostBullet;
	dragon.attack = 500;
	dragon.x_vel = -350;
	dragon.scale = 1.5f;

	dragon.num_animations = 1;
	dragon.animations[0].frames = 14;
	dragon.animations[0].frameTimer = 0.1f;
	dragon.setCurrentAnimation(0);

	return dragon;
}

Entity defineZombotDragonBullet() {
	textures[TEXTURE_TYPE::ZombotDragonBullet][Default_Texture_Index].loadFromFile("assets/zombot-dragon-bullet.png");

	Entity dragonBullet{};
	dragonBullet.category = ENTITY_CATEGORY::Bullet;
	dragonBullet.type = ENTITY_TYPE::ZombotDragonBullet;
	dragonBullet.attack = 10000;
	dragonBullet.x_vel = 2.0f;
	dragonBullet.scale = 1.0f;

	dragonBullet.num_animations = 1;
	dragonBullet.animations[0].frames = 13;
	dragonBullet.animations[0].frameTimer = 0.1f;
	dragonBullet.setCurrentAnimation(0);

	return dragonBullet;
}

Entity defineFireTornadoBullet() {
	Texture tex;
	tex.loadFromFile("assets/FireTornado.png");
	textures[TEXTURE_TYPE::FireTornadoBullet][Default_Texture_Index] = tex;

	Entity tornado{};
	tornado.category = ENTITY_CATEGORY::Bullet;
	tornado.type = ENTITY_TYPE::FireTornadoBullet;
	tornado.attack = 1;
	tornado.x_vel = -250.0f;
	tornado.scale = 0.85f;

	tornado.num_animations = 1;
	tornado.animations[0].frames = 1;
	tornado.animations[0].frameTimer = 0.05f;
	tornado.setCurrentAnimation(0);

	return tornado;
}
Entity defineFireTornadoBoostedBullet() {
	Texture tex;
	tex.loadFromFile("assets/FireTornadoBoosted.png");
	textures[TEXTURE_TYPE::FireTornadoBulletBoosted][Default_Texture_Index] = tex;

	Entity tornado{};
	tornado.category = ENTITY_CATEGORY::Bullet;
	tornado.type = ENTITY_TYPE::FireTornadoBulletBoosted;
	tornado.attack = 1;
	tornado.x_vel = -175.0f;
	tornado.scale = 1.2f;

	tornado.num_animations = 1;
	tornado.animations[0].frames = 1;
	tornado.animations[0].frameTimer = 0.05f;
	tornado.setCurrentAnimation(0);

	return tornado;
}
Entity definePomegranateBullet() {
	textures[TEXTURE_TYPE::PomegranateBullet][Default_Texture_Index].loadFromFile("assets/pomegranate-bullet-normal.png");
	Entity bullet;
	bullet.category = ENTITY_CATEGORY::Bullet;
	bullet.type = ENTITY_TYPE::PomegranateBullet;
	bullet.attack = 20;
	bullet.x_vel = -650.0f;

	return bullet;
}
Entity definePomegranateBulletBoost() {
	textures[TEXTURE_TYPE::PomegranateBulletBoost][Default_Texture_Index].loadFromFile("assets/pomegranate-bullet-boost.png");
	Entity bullet;
	bullet.category = ENTITY_CATEGORY::Bullet;
	bullet.type = ENTITY_TYPE::PomegranateBulletBoost;
	bullet.attack = 30;
	bullet.x_vel = -800.0f;

	return bullet;
}
Entity defineLightningBullet() {
	Texture tex;
	tex.loadFromFile("assets/LightningBullet.png");
	textures[TEXTURE_TYPE::LightningBullet][Default_Texture_Index] = tex;

	Entity bullet{};
	bullet.category = ENTITY_CATEGORY::Bullet;
	bullet.type = ENTITY_TYPE::LightningBullet;
	bullet.attack = 15000;
	bullet.x_vel = -1500.0f;
	bullet.scale = 0.2f;

	return bullet;
}
Entity defineRedBullet() {
	Texture tex;
	tex.loadFromFile("assets/red_bullet.png");
	textures[TEXTURE_TYPE::RedBullet][Default_Texture_Index] = tex;

	Entity bullet{};
	bullet.category = ENTITY_CATEGORY::Bullet;
	bullet.type = ENTITY_TYPE::RedBullet;
	bullet.attack = 40;
	bullet.x_vel = -600.0f;
	bullet.scale = 0.2f;

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
	plant.health = 80;
	plant.attack = 30;
	plant.attackTimer = 3.0f;
	plant.boostAttackTimer = 1.2f;
	//plant.boostAttackTimer = 0.7f;	// 6 * 30 per second = 360
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

					int nbullets = 8;
					for (int i = 0; i < nbullets; i++) {
						if (i < 2) newBullet.attack *= 1.5;
						newBullet.position = Vector2f(plant.position.x + 75 * i, plant.position.y + 50);
						addBullet(newBullet);
					}

					//cout << "--- " << nbullets << " bullets at " << currentLevel->clock.getElapsedTime().asSeconds() << endl;
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
					addBullet(newBullet);
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
				addBullet(newBullet1);
				//newBullets.push_back(newBullet1);

				Entity newBullet2 = defineNormalBullet();
				newBullet2.attack = atk;
				newBullet2.position = Vector2f(pos.x + 50, pos.y + 50);
				addBullet(newBullet2);
				//newBullets.push_back(newBullet2);

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
	shroom.health = 50;
	//shroom.attack = 0;
	shroom.attack = 10;
	shroom.price = 15;
	shroom.scale = 0.8;
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
				addBullet(newBullet);
				//newBullets.push_back(newBullet);
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
	plant.scale = 1.1;

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
	SunShroom.health = 50;
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
	icePea.scale = 0.65;

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
				addBullet(newBullet);
				//newBullets.push_back(newBullet);
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
	cherry.attack = 999;
	cherry.price = 150;
	cherry.scale = 20.0f;
	cherry.seedCooldown = 5.0f;

	cherry.num_animations = 2;

	cherry.animations[0].frames = 11;
	cherry.animations[0].frameTimer = 0.1f;


	cherry.animations[1].frames = 7;
	cherry.animations[1].frameTimer = 0.08f;

	cherry.setCurrentAnimation(0);
	cherry.attackTimer = 1.2f; // Time before explosion

	plants[ENTITY_TYPE::PLANT_CHEERY_BOMB] = cherry;

	cherry.updateEntity = [](Entity& plant) {

		if (plant.attackClock.getElapsedTime().asSeconds() >= 0.5f && plant.currentAnimationIndex != 1) {
			plant.DoAnimation(1, 1);

			plant.afterAnimation = [&](Entity& p) {
				float explosionRange = 350.0f;
				// RANGE OF EXPLOSION 

				for (Entity& z : entities) {
					if (z.category == ENTITY_CATEGORY::ZOMBIE && !z.isDead) {
						float diffY = abs(z.position.y - p.position.y);
						float diffX = abs(z.position.x - p.position.x);

						// 3*3
						if (diffY < 200.0f && diffX < explosionRange)
						{
							z.diedByElectrocution = true;
							attackZombie(z, p.attack, true);

						}
					}
				}
				//DISAPPER AFTER EXPLOSION
				p.category = ENTITY_CATEGORY::NONE;
				p.isDead = true;
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
	miniShroom.scale = 0.4f;

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
					addBullet(newBullet);
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
	wallnut.health = 500;
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
	Chomper.health = 100;
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

				if (zombie.type == ENTITY_TYPE::ZOMBOT_DRAGON) continue;

				bool sameRow = abs(zombie.position.y - plant.position.y) < 100.0f;


				bool inFront = zombie.position.x > plant.position.x &&
					zombie.position.x < plant.position.x + plant.width * 0.55;

				if (sameRow && inFront && zombie.type != ENTITY_TYPE::ZOMBIE_BIG
					&& zombie.type != ENTITY_TYPE::ZOMBIE_ROMAN_BUSTHEAD
					&& zombie.type != ENTITY_TYPE::ZOMBIE_ROMAN_GARGANTUAR
					&& zombie.type != ENTITY_TYPE::ZOMBIE_LOSTCITY_GARGANTUAR) {
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


							z.scale *= 0.90f;

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
	textures[TEXTURE_TYPE::PLANT_POTATO_MINE][2].loadFromFile("assets/ex1.png");
	Entity mine{};
	mine.category = ENTITY_CATEGORY::PLANT;
	mine.type = ENTITY_TYPE::PLANT_POTATO_MINE;
	mine.health = 300;
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
	// explosion
	mine.animations[2].frames = 6;
	mine.animations[2].frameTimer = 0.001f;


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


			float triggerRange = 50.0f;

			if (diffY < 60.0f && diffX < triggerRange) {

				plant.DoAnimation(1, 2);


				plant.afterAnimation = [&](Entity& p) {
					float damageRange = 80.0f;

					for (Entity& z : entities) {
						if (z.category == ENTITY_CATEGORY::ZOMBIE && !z.isDead) {
							if (abs(z.position.y - p.position.y) < 100.0f && abs(z.position.x - p.position.x) < damageRange) {
								attackZombie(z, p.attack, true);
							}
						}
					}
					p.category = ENTITY_CATEGORY::NONE;
					};
				break;
			}
		}
		};

	return mine;
}
Entity defineDOOM_SHROOM() {
	textures[TEXTURE_TYPE::PLANT_DOOM_MINE][0].loadFromFile("assets/doom_shroom.png");
	textures[TEXTURE_TYPE::PLANT_DOOM_MINE][1].loadFromFile("assets/doom-explosion.png");

	Entity shroom{};
	shroom.category = ENTITY_CATEGORY::PLANT;
	shroom.type = ENTITY_TYPE::PLANT_DOOM_MINE;
	shroom.health = 300;
	shroom.attack = 9999;
	shroom.price = 125;
	shroom.scale = 0.4f;
	shroom.width = 120;
	shroom.height = 120;

	shroom.num_animations = 2;


	shroom.animations[0].frames = 41;
	shroom.animations[0].frameTimer = 0.1f;


	shroom.animations[1].frames = 14;
	shroom.animations[1].frameTimer = 0.01f;

	shroom.setCurrentAnimation(0);

	plants[ENTITY_TYPE::PLANT_DOOM_MINE] = shroom;
	// update
	shroom.updateEntity = [](Entity& plant) {
		for (Entity& zombie : entities) {
			if (zombie.category != ENTITY_CATEGORY::ZOMBIE || zombie.isDead) continue;


			bool sameRow = abs(zombie.position.y - plant.position.y) < 100.0f;
			bool inFront = zombie.position.x > plant.position.x &&
				zombie.position.x < plant.position.x + plant.width * 0.5;
			if (sameRow == true && inFront == true) {
				plant.DoAnimation(1, 1);
				plant.afterAnimation = [&zombie](Entity& p) {
					for (Entity& z : entities) {
						float distY = abs(z.position.y - p.position.y);
						bool inRange = (distY < 100.0f && z.position.x > p.position.x && z.position.x < p.position.x + p.width * 1.5f);

						if (z.category == ENTITY_CATEGORY::ZOMBIE && !z.isDead && inRange) {
							attackZombie(z, p.attack, false);
							p.category = ENTITY_CATEGORY::NONE;
							p.isDead = true;

						}
						//break;
					}
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
	plant.health = 80;
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
Entity defineSnapdragon() {
	textures[TEXTURE_TYPE::PLANT_SNAPDRAGON][0].loadFromFile("assets/SnapdragonIdle.png");
	textures[TEXTURE_TYPE::PLANT_SNAPDRAGON][1].loadFromFile("assets/SnapdragonAttack.png");
	textures[TEXTURE_TYPE::PLANT_SNAPDRAGON][3].loadFromFile("assets/SnapdragonBoost-ezgif.com-resize.png");
	//textures[TEXTURE_TYPE::PLANT_SNAPDRAGON][3].loadFromFile("assets/SnapdragonBoost.png");

	Entity plant;
	plant.category = ENTITY_CATEGORY::PLANT;
	plant.type = ENTITY_TYPE::PLANT_SNAPDRAGON;
	plant.health = 150;
	plant.attack = 50;
	plant.price = 150;
	plant.seedCooldown = 7.5f;
	plant.scale = 1.0f;
	plant.attackTimer = 2.0f;

	plant.num_animations = 4;

	plant.animations[0].frames = 20; // Idle
	plant.animations[0].frameTimer = 0.1f;

	plant.animations[1].frames = 20; // Attack 
	plant.animations[1].frameTimer = 0.08f;

	plant.animations[3].frames = 23; // Boost
	plant.animations[3].frameTimer = 0.1f;

	plant.setCurrentAnimation(0);

	plants[ENTITY_TYPE::PLANT_SNAPDRAGON] = plant;

	// Boost settings
	plant.boostCycles = 1;
	plant.boostAnimationIdx = 3;
	//plant.boostScale = 1.5;

	plant.updateEntity = [](Entity& p) {
		if (p.attackClock.getElapsedTime().asSeconds() >= p.attackTimer) {

			// Boost
			if (p.isBoosted) {
				p.attackClock.restart();
				Entity newBullet = defineSnapDragonBoostedBullet();
				newBullet.position = Vector2f(p.position.x + 100, p.position.y + 50);
				//newBullet.x_vel *= 2;
				addBullet(newBullet);

				p.isBoosted = false;
				//// shoot bullets
				//if (p.currentAnimation.currentFrame == p.currentAnimation.frames / 2) {
				//	p.attackClock.restart();
				//	Entity newBullet = defineNormalBullet();
				//	//Bullet newBullet = defineNormalBullet();
				//	newBullet.attack = p.attack;
				//	newBullet.x_vel *= 2;
				//	newBullet.position = Vector2f(p.position.x + 100, p.position.y + 50);
				//	bullets.push_back(newBullet);
				//	newBullet.position = Vector2f(p.position.x + 200, p.position.y + 50);
				//	bullets.push_back(newBullet);
				//	newBullet.position = Vector2f(p.position.x + 300, p.position.y + 50);
				//	bullets.push_back(newBullet);
				//}
				return;	 // to not loop zombies
			}

			// Normal

			bool zombieInRange = false;

			for (Entity& zombie : entities) {
				if (zombie.category == ENTITY_CATEGORY::ZOMBIE && zombie.health > 0) {
					float diffY = abs(zombie.position.y - p.position.y);
					float diffX = zombie.position.x - p.position.x;

					if (diffY <= 2 * currentLevel->cellHeight && diffX >= -30.0f && diffX <= 3 * currentLevel->cellWidth) {
						zombieInRange = true;
						break;
					}
				}
			}

			if (zombieInRange) {
				p.DoAnimation(1, 1);
				Entity newFire = defineSnapDragonBullet();
				newFire.position = { p.position.x + p.width, p.position.y };
				//newFire.attack = 0; // already damaged below .. but too weak
				//newFire.scale = 2;
				addBullet(newFire);
				p.attackClock.restart();

				if (p.currentAnimation.currentFrame == p.currentAnimation.frames / 2) {
					p.attackClock.restart();

					for (Entity& zombie : entities) {
						if (zombie.category == ENTITY_CATEGORY::ZOMBIE && !zombie.isDead) {
							float diffY = abs(zombie.position.y - p.position.y);
							float diffX = zombie.position.x - p.position.x;

							if (diffY <= 2 * currentLevel->cellHeight && diffX >= -30.0f && diffX <= 3 * currentLevel->cellWidth) {

								attackZombie(zombie, p.attack, true);
							}
						}
					}
				}
			}
		}
		};

	return plant;
}
Entity defineLightningReed() {
	Texture tex;
	tex.loadFromFile("assets/LightningReed.png");
	textures[TEXTURE_TYPE::PLANT_LIGHTNING_REED][Default_Texture_Index] = tex;

	Entity plant;
	plant.category = ENTITY_CATEGORY::PLANT;
	plant.type = ENTITY_TYPE::PLANT_LIGHTNING_REED;
	plant.health = 100;
	plant.attack = 50;
	plant.price = 125;
	plant.seedCooldown = 7.5f;
	plant.scale = 0.2f;
	plant.attackTimer = 2.0f;

	plant.num_animations = 1;
	plant.animations[0].frames = 18;
	plant.animations[0].frameTimer = 0.08f;

	plant.setCurrentAnimation(0);

	plants[ENTITY_TYPE::PLANT_LIGHTNING_REED] = plant;

	//it will be developed
	plant.updateEntity = [](Entity& p) {
		if (p.attackClock.getElapsedTime().asSeconds() >= p.attackTimer) {
			bool zombieInLane = false;

			for (Entity& zombie : entities) {
				if (zombie.category == ENTITY_CATEGORY::ZOMBIE && !zombie.isDead && zombie.health > 0) {
					if (abs(zombie.position.y - p.position.y) < 100.0f &&
						zombie.position.x > p.position.x &&
						zombie.position.x < windowWidth - 50.0f) {
						zombieInLane = true;
						break;
					}
				}
			}

			if (zombieInLane) {
				p.attackClock.restart();
				Entity newBullet = defineLightningBullet();
				newBullet.attack = p.attack;
				newBullet.position = Vector2f(p.position.x + 60, p.position.y - 20);
				addBullet(newBullet);
			}
		}
		};

	return plant;
}
Entity defineInfernoPlant() {
	Texture tex;
	tex.loadFromFile("assets/Inferno.png");
	textures[TEXTURE_TYPE::PLANT_INFERNO][Default_Texture_Index] = tex;
	textures[TEXTURE_TYPE::PLANT_INFERNO][2].loadFromFile("assets/Inferno-attack-170-130-34.png");
	textures[TEXTURE_TYPE::PLANT_INFERNO][3].loadFromFile("assets/Inferno-attack-boost-170-130-35.png");

	Entity plant;
	plant.category = ENTITY_CATEGORY::PLANT;
	plant.type = ENTITY_TYPE::PLANT_INFERNO;
	plant.health = 150;
	plant.attack = 10;
	plant.price = 150;
	plant.seedCooldown = 7.5f;
	plant.scale = 1.0f;
	plant.attackTimer = 6.0f;
	plant.boostAttackTimer = 1.0f;

	plant.num_animations = 3;
	// Idle
	plant.animations[0].frames = 40;
	plant.animations[0].frameTimer = 0.08f;
	// Attack
	plant.animations[2].frames = 34;
	plant.animations[2].frameTimer = 0.1f;
	// Boosted
	plant.animations[3].frames = 35;
	plant.animations[3].frameTimer = 0.1;

	plant.setCurrentAnimation(0);

	plants[ENTITY_TYPE::PLANT_INFERNO] = plant;

	plant.updateEntity = [](Entity& p) {
		float currentTimer = p.isBoosted ? p.boostAttackTimer : p.attackTimer;

		if (p.attackClock.getElapsedTime().asSeconds() >= currentTimer) {
			// Boosted
			if (p.isBoosted) {
				p.attack = 60;
				p.DoAnimation(1, 3);
				if (p.currentAnimation.currentFrame == 22) {

					p.attackClock.restart();

					Entity newBullet = defineFireTornadoBoostedBullet();
					newBullet.attack = p.attack;
					newBullet.position = Vector2f(p.position.x + 60, p.position.y - 10);
					addBullet(newBullet);
					p.isBoosted = false;
					p.attackClock.restart();
				}
				return;
			}

			// Normal 
			bool zombieInLane = false;
			for (Entity& zombie : entities) {
				if (zombie.category == ENTITY_CATEGORY::ZOMBIE && !zombie.isDead && zombie.health > 0) {
					if (abs(zombie.position.y - p.position.y) < 100.0f &&
						zombie.position.x > p.position.x &&
						zombie.position.x < windowWidth - 50.0f) {
						zombieInLane = true;
						break;
					}
				}
			}

			if (zombieInLane) {

				p.DoAnimation(1, 2);
				if (p.currentAnimation.currentFrame == 22) {

					p.attackClock.restart();

					Entity newBullet = defineFireTornadoBullet();
					newBullet.attack = p.attack;
					newBullet.position = Vector2f(p.position.x + 60, p.position.y - 20);
					addBullet(newBullet);
				}

			}
		}
		};

	return plant;
}
Entity definePomegranatePlant() {
	Texture tex;
	tex.loadFromFile("assets/pomegranate-idle.png");
	textures[TEXTURE_TYPE::PLANT_POMEGRANATE][Default_Texture_Index] = tex;
	textures[TEXTURE_TYPE::PLANT_POMEGRANATE][2].loadFromFile("assets/pomegranate-attack.png");
	textures[TEXTURE_TYPE::PLANT_POMEGRANATE][3].loadFromFile("assets/pomegranate-boost.png");

	Entity plant;
	plant.category = ENTITY_CATEGORY::PLANT;
	plant.type = ENTITY_TYPE::PLANT_POMEGRANATE;
	plant.health = 150;
	plant.price = 150;
	plant.attack = 15;
	plant.attackTimer = 1.0f;
	plant.boostAttackTimer = 0.5f;
	plant.seedCooldown = 5.0f;

	plant.num_animations = 3;
	// Idle
	plant.animations[0].frames = 50;
	plant.animations[0].frameTimer = 0.05;
	// Attack
	plant.animations[2].frames = 53;
	plant.animations[2].frameTimer = 0.03;
	// Boosted
	plant.animations[3].frames = 39;
	plant.animations[3].frameTimer = 0.03;

	plant.setCurrentAnimation(0);

	plants[ENTITY_TYPE::PLANT_POMEGRANATE] = plant;

	plant.updateEntity = [](Entity& p) {
		float currentTimer = p.isBoosted ? p.boostAttackTimer : p.attackTimer;
		float numBullets = 10;

		if (p.attackClock.getElapsedTime().asSeconds() >= currentTimer) {
			// Boosted
			if (p.isBoosted) {
				p.attackClock.restart();

				Entity newBullet = definePomegranateBulletBoost();
				newBullet.attack = p.attack;
				newBullet.position = Vector2f(p.position.x + 80, p.position.y + 70);
				if (addBullet(newBullet) != -1) {
					bulletShot++;
					if (bulletShot >= 10) {
						bulletShot = 0;
						p.afterAnimation(p);
						p.afterAnimation = [](Entity&) {};

						p.AnimationForCycles = -1;
						p.setCurrentAnimation(0);
					}
				}
				return;
			}

			//Normal
			bool zombieInLane = false;
			for (Entity& zombie : entities) {
				if (zombie.category == ENTITY_CATEGORY::ZOMBIE && !zombie.isDead && zombie.health > 0) {
					if (abs(zombie.position.y - p.position.y) < 100.0f &&
						zombie.position.x > p.position.x &&
						zombie.position.x < windowWidth - 50.0f) {
						zombieInLane = true;
						break;
					}
				}
			}

			if (zombieInLane) {
				p.DoAnimation(1, 2);
				if (p.currentAnimation.currentFrame == 22) {
					p.attackClock.restart();

					Entity newBullet = definePomegranateBullet();
					newBullet.attack = p.attack;
					newBullet.position = Vector2f(p.position.x + 80, p.position.y + 70);
					addBullet(newBullet);
				}
			}

		}
		};

	return plant;
}
Entity defineRedShooter() {
	textures[TEXTURE_TYPE::PLANT_RED_SHOOTER][0].loadFromFile("assets/RedStingerIdle.png");
	textures[TEXTURE_TYPE::PLANT_RED_SHOOTER][1].loadFromFile("assets/RedStingerAttack.png");

	Entity plant;
	plant.category = ENTITY_CATEGORY::PLANT;
	plant.type = ENTITY_TYPE::PLANT_RED_SHOOTER;
	plant.health = 150;
	plant.attack = 40;
	plant.price = 150;
	plant.scale = 0.5f;
	plant.attackTimer = 2.0f;

	plant.num_animations = 2;


	plant.animations[0].frames = 19;
	plant.animations[0].frameTimer = 0.15f;

	plant.animations[1].frames = 15;
	plant.animations[1].frameTimer = 0.15f;

	plant.setCurrentAnimation(0);
	plants[ENTITY_TYPE::PLANT_RED_SHOOTER] = plant;


	plant.updateEntity = [](Entity& p) {
		if (p.attackClock.getElapsedTime().asSeconds() >= p.attackTimer) {
			bool zombieInLane = false;
			for (Entity& zombie : entities) {
				if (zombie.category == ENTITY_CATEGORY::ZOMBIE && zombie.health > 0) {

					if (abs(zombie.position.y - p.position.y) < 100.0f &&
						zombie.position.x > p.position.x &&
						zombie.position.x < windowWidth - 50.0f) {
						zombieInLane = true;
						break;
					}
				}
			}

			if (zombieInLane) {
				p.DoAnimation(1, 1);

				if (p.currentAnimation.currentFrame == p.currentAnimation.frames / 2) {
					p.attackClock.restart();
					Entity newBullet = defineRedBullet();
					newBullet.attack = p.attack;
					newBullet.position = Vector2f(p.position.x + 80, p.position.y + 30);
					addBullet(newBullet);
				}
			}
		}
		};

	return plant;
}

// zombies
Entity defineNormalZombie() {
	textures[TEXTURE_TYPE::ZOMBIE_NORMAL][0].loadFromFile("assets/NormalZombie.png");
	textures[TEXTURE_TYPE::ZOMBIE_NORMAL][1].loadFromFile("assets/NormalZombieDeath.png");
	Entity zombie = { ENTITY_CATEGORY::ZOMBIE, ENTITY_TYPE::ZOMBIE_NORMAL };
	zombie.x_vel = 70;
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
	zombie.x_vel = 70;
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
	zombie.x_vel = 70;
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
	zombie.x_vel = 70;
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
	zombie.x_vel = 70;
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
	zombie.x_vel = 70;
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
	zombie.x_vel = 70;
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
	zombie.x_vel = 70;
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
	zombie.x_vel = 150;
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

	Entity zombie{};
	zombie.category = ENTITY_CATEGORY::ZOMBIE;
	zombie.type = ENTITY_TYPE::ZOMBIE_PHARAOH;
	zombie.health = 500;
	zombie.attack = 10;
	zombie.x_vel = 70;
	// Animations
	zombie.num_animations = 3;
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
	zombie.x_vel = 70;
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

Entity definePianistZombie() {
	textures[TEXTURE_TYPE::ZOMBIE_PIANIST][0].loadFromFile("assets/PianistZombie.png");
	textures[TEXTURE_TYPE::ZOMBIE_PIANIST][1].loadFromFile("assets/PianistZombieDeath.png");
	textures[TEXTURE_TYPE::ZOMBIE_PIANIST][2].loadFromFile("assets/PianistZombieDamaged.png");

	Entity zombie{};
	zombie.category = ENTITY_CATEGORY::ZOMBIE;
	zombie.type = ENTITY_TYPE::ZOMBIE_PIANIST;
	zombie.health = 500;
	zombie.attack = 20;
	zombie.x_vel = 20;
	// Animations
	zombie.num_animations = 3;
	Animation anim;
	// walk
	anim.frames = 30;
	anim.frameTimer = 0.05f;
	zombie.animations[0] = anim;
	// death
	anim.frames = 53;
	anim.frameTimer = 0.05f;
	zombie.animations[1] = anim;
	// damaged walk
	anim.frames = 30;
	anim.frameTimer = 0.05f;
	zombie.animations[2] = anim;

	zombie.setCurrentAnimation(0);

	return zombie;
}

Entity defineChickenWranglerZombie() {
	textures[TEXTURE_TYPE::ZOMBIE_CHICKENWRANGLER][0].loadFromFile("assets/ChickenWranglerZombie.png");
	textures[TEXTURE_TYPE::ZOMBIE_CHICKENWRANGLER][1].loadFromFile("assets/redhair_die.png");
	textures[TEXTURE_TYPE::ZOMBIE_CHICKENWRANGLER][2].loadFromFile("assets/redhair_eat.png");
	textures[TEXTURE_TYPE::ZOMBIE_CHICKENWRANGLER][3].loadFromFile("assets/redhair_walk.png");

	Entity zombie{};
	zombie.category = ENTITY_CATEGORY::ZOMBIE;
	zombie.type = ENTITY_TYPE::ZOMBIE_CHICKENWRANGLER;
	zombie.health = 300;
	zombie.attack = 10;
	zombie.x_vel = 70;

	zombie.num_animations = 4;
	Animation anim;
	// walk
	anim.frames = 44;
	anim.frameTimer = 0.05f;
	zombie.animations[0] = anim;
	// death
	anim.frames = 25;
	anim.frameTimer = 0.1f;
	zombie.animations[1] = anim;
	// eat
	anim.frames = 25;
	anim.frameTimer = 0.07f;
	zombie.animations[2] = anim;
	// no chicken walk
	anim.frames = 25;
	anim.frameTimer = 0.07f;
	zombie.animations[3] = anim;

	zombie.setCurrentAnimation(0);

	return zombie;
}

Entity defineChickenZombie() {
	textures[TEXTURE_TYPE::ZOMBIE_CHICKEN][0].loadFromFile("assets/ChickenZombie.png");
	textures[TEXTURE_TYPE::ZOMBIE_CHICKEN][1].loadFromFile("assets/ChickenZombieDeath.png");

	Entity zombie{};
	zombie.category = ENTITY_CATEGORY::ZOMBIE;
	zombie.type = ENTITY_TYPE::ZOMBIE_CHICKEN;
	zombie.health = 30;
	zombie.attack = 5;
	zombie.x_vel = 200;
	zombie.scale = 1.0f;

	zombie.num_animations = 2;
	Animation anim;
	// walk
	anim.frames = 30;
	anim.frameTimer = 0.05f;
	zombie.animations[0] = anim;
	// death
	anim.frames = 44;
	anim.frameTimer = 0.05f;
	zombie.animations[1] = anim;

	zombie.setCurrentAnimation(0);

	return zombie;
}

Entity defineProspectorZombie() {
	Entity zombie = { ENTITY_CATEGORY::ZOMBIE, ENTITY_TYPE::ZOMBIE_PROSPECTOR };
	zombie.health = 150;
	zombie.attack = 10;
	zombie.x_vel = 70;
	zombie.scale = 1.0f;

	zombie.num_animations = 4;
	Animation anim;

	anim.frames = 22;
	anim.frameTimer = 0.07f;
	zombie.animations[0] = anim;

	anim.frames = 25;
	anim.frameTimer = 0.1f;
	zombie.animations[1] = anim;

	anim.frames = 25;
	anim.frameTimer = 0.07f;
	zombie.animations[2] = anim;

	anim.frames = 25;
	anim.frameTimer = 0.05f;
	zombie.animations[3] = anim;

	zombie.setCurrentAnimation(0);
	return zombie;
}

Entity defineRedhairZombie() {
	Entity zombie = { ENTITY_CATEGORY::ZOMBIE, ENTITY_TYPE::ZOMBIE_REDHAIR };
	zombie.health = 150;
	zombie.attack = 10;
	zombie.x_vel = 70;
	zombie.scale = 1.0f;

	zombie.num_animations = 3;
	Animation anim;

	anim.frames = 25;
	anim.frameTimer = 0.07f;
	zombie.animations[0] = anim;

	anim.frames = 25;
	anim.frameTimer = 0.1f;
	zombie.animations[1] = anim;

	anim.frames = 25;
	anim.frameTimer = 0.07f;
	zombie.animations[2] = anim;

	zombie.setCurrentAnimation(0);
	return zombie;
}

Entity defineCowboyZombie() {
	Entity zombie = { ENTITY_CATEGORY::ZOMBIE, ENTITY_TYPE::ZOMBIE_COWBOY };
	zombie.health = 150;
	zombie.attack = 10;
	zombie.x_vel = 70;
	zombie.scale = 1.0f;

	zombie.num_animations = 3;
	Animation anim;

	anim.frames = 25;
	anim.frameTimer = 0.07f;
	zombie.animations[0] = anim;

	anim.frames = 24;
	anim.frameTimer = 0.1f;
	zombie.animations[1] = anim;

	anim.frames = 25;
	anim.frameTimer = 0.07f;
	zombie.animations[2] = anim;

	zombie.setCurrentAnimation(0);
	return zombie;
}

Entity defineZombotDragon() {
	textures[TEXTURE_TYPE::ZOMBOT_DRAGON][0].loadFromFile("assets/zombot-dragon-idle.png");
	//textures[TEXTURE_TYPE::ZOMBOT_DRAGON][0].loadFromFile("assets/zombot-dragon-idle-ezgif.com-resize.png");
	//textures[TEXTURE_TYPE::ZOMBOT_DRAGON][0].loadFromFile("assets/zombot-dragon-idle.png");
	textures[TEXTURE_TYPE::ZOMBOT_DRAGON][1].loadFromFile("assets/zombot-dragon-die-1-400-295-16.png");
	//textures[TEXTURE_TYPE::ZOMBOT_DRAGON][1].loadFromFile("assets/zombot-dragon-die-1.png");
	textures[TEXTURE_TYPE::ZOMBOT_DRAGON][4].loadFromFile("assets/zombot-dragon-die-2-still-400-295-1.png");
	//textures[TEXTURE_TYPE::ZOMBOT_DRAGON][4].loadFromFile("assets/zombot-dragon-die-2.png");
	textures[TEXTURE_TYPE::ZOMBOT_DRAGON][2].loadFromFile("assets/zombot-dragon-attack-400-290-14.png");
	//textures[TEXTURE_TYPE::ZOMBOT_DRAGON][2].loadFromFile("assets/zombot-dragon-attack.png");

	Entity zombie{};
	zombie.scale = 1.25f;

	zombie.category = ENTITY_CATEGORY::ZOMBIE;
	zombie.type = ENTITY_TYPE::ZOMBOT_DRAGON;
	zombie.health = 5000;
	zombie.attack = 250;
	zombie.attackTimer = 10.f;	 // just first time .. then its updated after it appears
	zombie.x_vel = 10;
	//zombie.scale = 3.f;
	// Animations
	zombie.num_animations = 5;
	Animation anim;
	// idle
	anim.frames = 17;
	anim.frameTimer = 0.1f;
	zombie.animations[0] = anim;
	// death 1
	anim.frames = 16;
	//anim.frames = 31;
	//anim.frames = 34;
	anim.frameTimer = 0.25f;
	zombie.animations[1] = anim;
	// death 2
	anim.frames = 1;
	//anim.frames = 27;
	anim.frameTimer = 5.f;
	zombie.animations[4] = anim;
	// attack
	anim.frames = 14;
	//anim.frames = 21;
	anim.frameTimer = 0.1f;
	zombie.animations[2] = anim;

	zombie.setCurrentAnimation(0);

	return zombie;
}

// Character Sound Effects
int playZombotDragonFireBall() {
	Music* music = new Music();
	if (!music->openFromFile("assets/audio/zombot-dragon-fireball.ogg")) {
		std::cout << "Error loading file!" << std::endl;
		return -1;
	}
	music->setLoop(false);
	music->play();
	return 0;
}
int playZombotDragonRoar() {
	Music* music = new Music();
	if (!music->openFromFile("assets/audio/zombot-dragon-roar.ogg")) {
		std::cout << "Error loading file!" << std::endl;
		return -1;
	}
	music->setLoop(false);
	music->play();
	return 0;
}
int playZombotDragonDeath() {
	Music* music = new Music();
	if (!music->openFromFile("assets/audio/zombot-dragon-death.ogg")) {
		std::cout << "Error loading file!" << std::endl;
		return -1;
	}
	music->setLoop(false);
	music->play();
	return 0;
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
		uiSprite.setOrigin(uiSprite.getGlobalBounds().getSize() / 2.0f);
		uiSprite.rotate(sun.lifeClock.getElapsedTime().asSeconds() * 22.5f);
		float scale = 1.0f + 0.1f * sin(sun.lifeClock.getElapsedTime().asSeconds() * 1.57f);

		uiSprite.setScale(scale, scale);

		if (sun.targeted) { // for ra
			uiSprite.setColor(Color(255, 80, 80));
		} else {
			uiSprite.setColor(Color(255, 255, 255));
		}

		window.draw(uiSprite);

		uiSprite.setRotation(0);
		uiSprite.setOrigin(0, 0);
	}
	uiSprite.setColor(Color(255, 255, 255)); // for ra
}
Entity definePogoZombie() {
	textures[TEXTURE_TYPE::ZOMBIE_POGO][0].loadFromFile("assets/PogoZombieWalk.png");      // Walk (45 frames)
	textures[TEXTURE_TYPE::ZOMBIE_POGO][1].loadFromFile("assets/PogoZombieEat.png");       // Eat (26 frames)
	textures[TEXTURE_TYPE::ZOMBIE_POGO][2].loadFromFile("assets/PogoZombieDeath.png");     // Death Walk (24 frames)
	textures[TEXTURE_TYPE::ZOMBIE_POGO][3].loadFromFile("assets/PogoZombieJump.png");      // Jump (16 frames)
	textures[TEXTURE_TYPE::ZOMBIE_POGO][4].loadFromFile("assets/PogoZombieLosePogo.png");  // Lose Pogo (23 frames)
	textures[TEXTURE_TYPE::ZOMBIE_POGO][5].loadFromFile("assets/PogoZombieDeathJump.png"); // Death Jump (19 frames)

	Entity zombie{};
	zombie.category = ENTITY_CATEGORY::ZOMBIE;
	zombie.type = ENTITY_TYPE::ZOMBIE_POGO;
	zombie.health = 200;
	zombie.attack = 10;
	zombie.x_vel = 150;
	zombie.scale = 1.0f;
	zombie.num_animations = 6;

	Animation anim;

	anim.frames = 45;
	anim.frameTimer = 0.07f;
	zombie.animations[0] = anim;

	anim.frames = 26;
	anim.frameTimer = 0.07f;
	zombie.animations[1] = anim;

	anim.frames = 24;
	anim.frameTimer = 0.07f;
	zombie.animations[2] = anim;

	anim.frames = 16;
	anim.frameTimer = 0.05f;
	zombie.animations[3] = anim;

	anim.frames = 20;
	anim.frameTimer = 0.08f;
	zombie.animations[4] = anim;

	anim.frames = 19;
	anim.frameTimer = 0.07f;
	zombie.animations[5] = anim;

	zombie.setCurrentAnimation(3);

	zombie.updateEntity = [](Entity& z) {
		if (z.health <= 150 && z.health > 0 && z.currentAnimationIndex == 3) {
			z.x_vel = 0.0f;
			z.DoAnimation(1, 4);

			z.afterAnimation = [](Entity& entity) {
				entity.setCurrentAnimation(0);
				entity.x_vel = 70;
				};
		}
		};

	return zombie;
}
Entity defineRomanConehead() {
	textures[TEXTURE_TYPE::ZOMBIE_ROMAN_CONEHEAD][0].loadFromFile("assets/roman_conehead_walk.png");
	textures[TEXTURE_TYPE::ZOMBIE_ROMAN_CONEHEAD][1].loadFromFile("assets/roman_conehead_die.png");
	textures[TEXTURE_TYPE::ZOMBIE_ROMAN_CONEHEAD][2].loadFromFile("assets/roman_conehead_walk_no_helmet.png");
	textures[TEXTURE_TYPE::ZOMBIE_ROMAN_CONEHEAD][3].loadFromFile("assets/roman_conehead_helmet_fly.png");

	Entity zombie{};
	zombie.category = ENTITY_CATEGORY::ZOMBIE;
	zombie.type = ENTITY_TYPE::ZOMBIE_ROMAN_CONEHEAD;
	zombie.health = 300;
	zombie.attack = 10;
	zombie.x_vel = 70;
	zombie.scale = 0.50f;

	zombie.num_animations = 4;
	Animation anim;
	anim.frames = 18;
	anim.frameTimer = 0.07f;

	zombie.animations[0] = anim;
	zombie.animations[1] = anim;
	zombie.animations[2] = anim;
	zombie.animations[3] = anim;

	zombie.setCurrentAnimation(0);

	zombie.updateEntity = [](Entity& z) {
		if (z.health <= 100 && z.health > 0 && !z.shellDestroyed) {
			z.shellDestroyed = true;
			z.DoAnimation(1, 3);
			z.afterAnimation = [](Entity& entity) {
				entity.setCurrentAnimation(2);
				};
		}
		};
	return zombie;
}

Entity defineRomanBuckethead() {

	textures[TEXTURE_TYPE::ZOMBIE_ROMAN_BUCKETHEAD][0].loadFromFile("assets/roman_buckethead_walk.png");
	textures[TEXTURE_TYPE::ZOMBIE_ROMAN_BUCKETHEAD][1].loadFromFile("assets/roman_buckethead_die.png");
	textures[TEXTURE_TYPE::ZOMBIE_ROMAN_BUCKETHEAD][2].loadFromFile("assets/roman_buckethead_walk_no_helmet.png");
	textures[TEXTURE_TYPE::ZOMBIE_ROMAN_BUCKETHEAD][3].loadFromFile("assets/roman_buckethead_helmet_fly.png");

	Entity zombie = defineRomanConehead();
	zombie.type = ENTITY_TYPE::ZOMBIE_ROMAN_BUCKETHEAD;
	zombie.health = 550;
	return zombie;
}

Entity defineRomanCenturion() {
	textures[TEXTURE_TYPE::ZOMBIE_ROMAN_CENTURION][0].loadFromFile("assets/roman_centurion_walk.png");
	textures[TEXTURE_TYPE::ZOMBIE_ROMAN_CENTURION][1].loadFromFile("assets/roman_centurion_die.png");
	textures[TEXTURE_TYPE::ZOMBIE_ROMAN_CENTURION][2].loadFromFile("assets/roman_centurion_walk_no_helmet.png");
	textures[TEXTURE_TYPE::ZOMBIE_ROMAN_CENTURION][3].loadFromFile("assets/roman_centurion_helmet_fly.png");

	Entity zombie = defineRomanConehead();
	zombie.type = ENTITY_TYPE::ZOMBIE_ROMAN_CENTURION;
	zombie.health = 600;
	return zombie;
}

Entity defineRomanBusthead() {
	textures[TEXTURE_TYPE::ZOMBIE_ROMAN_BUSTHEAD][0].loadFromFile("assets/roman_busthead_walk.png");
	textures[TEXTURE_TYPE::ZOMBIE_ROMAN_BUSTHEAD][1].loadFromFile("assets/roman_busthead_die.png");
	textures[TEXTURE_TYPE::ZOMBIE_ROMAN_BUSTHEAD][2].loadFromFile("assets/roman_busthead_walk_no_helmet.png");
	textures[TEXTURE_TYPE::ZOMBIE_ROMAN_BUSTHEAD][3].loadFromFile("assets/roman_busthead_helmet_fly.png");

	Entity zombie = defineRomanConehead();
	zombie.type = ENTITY_TYPE::ZOMBIE_ROMAN_BUSTHEAD;
	zombie.health = 1200;
	return zombie;
}
Entity defineRomanOdessa() {
	textures[TEXTURE_TYPE::ZOMBIE_ROMAN_ODESSA][0].loadFromFile("assets/odessa_walk.png");
	textures[TEXTURE_TYPE::ZOMBIE_ROMAN_ODESSA][1].loadFromFile("assets/odessa_die.png");
	textures[TEXTURE_TYPE::ZOMBIE_ROMAN_ODESSA][2].loadFromFile("assets/odessa_low_health.png");
	textures[TEXTURE_TYPE::ZOMBIE_ROMAN_ODESSA][3].loadFromFile("assets/odessa_eat.png");

	Entity zombie{};
	zombie.category = ENTITY_CATEGORY::ZOMBIE;
	zombie.type = ENTITY_TYPE::ZOMBIE_ROMAN_ODESSA;
	zombie.health = 300;
	zombie.attack = 10;
	zombie.x_vel = 70;
	zombie.scale = 0.50f;

	zombie.num_animations = 4;
	Animation anim;
	anim.frames = 18;
	anim.frameTimer = 0.07f;

	zombie.animations[0] = anim;
	zombie.animations[1] = anim;
	zombie.animations[2] = anim;
	zombie.animations[3] = anim;

	zombie.setCurrentAnimation(0);

	zombie.updateEntity = [](Entity& z) {
		if (z.health <= 200 && z.health > 0 && z.currentAnimationIndex == 0) {
			z.setCurrentAnimation(2);
		}
		};
	return zombie;
}

Entity defineRomanShield() {
	textures[TEXTURE_TYPE::ZOMBIE_ROMAN_SHIELD][0].loadFromFile("assets/roman_shield_walk.png");
	textures[TEXTURE_TYPE::ZOMBIE_ROMAN_SHIELD][1].loadFromFile("assets/roman_shield_die.png");

	Entity zombie{};
	zombie.category = ENTITY_CATEGORY::ZOMBIE;
	zombie.type = ENTITY_TYPE::ZOMBIE_ROMAN_SHIELD;
	zombie.health = 400;
	zombie.attack = 10;
	zombie.x_vel = 70;
	zombie.scale = 0.50f;

	zombie.num_animations = 2;
	Animation anim;
	anim.frames = 18;
	anim.frameTimer = 0.07f;
	zombie.animations[0] = anim;
	zombie.animations[1] = anim;

	zombie.setCurrentAnimation(0);
	return zombie;
}
Entity defineRomanNormal() {
	textures[TEXTURE_TYPE::ZOMBIE_ROMAN_NORMAL][0].loadFromFile("assets/roman_normal_walk.png");
	textures[TEXTURE_TYPE::ZOMBIE_ROMAN_NORMAL][1].loadFromFile("assets/roman_normal_die.png");

	Entity zombie{};
	zombie.category = ENTITY_CATEGORY::ZOMBIE;
	zombie.type = ENTITY_TYPE::ZOMBIE_ROMAN_NORMAL;
	zombie.health = 150;
	zombie.attack = 10;
	zombie.x_vel = 70;
	zombie.scale = 0.5f;

	zombie.num_animations = 2;
	Animation anim;
	anim.frames = 18;
	anim.frameTimer = 0.07f;
	zombie.animations[0] = anim;
	zombie.animations[1] = anim;

	zombie.setCurrentAnimation(0);
	return zombie;
}

Entity defineRomanGargantuar() {
	textures[TEXTURE_TYPE::ZOMBIE_ROMAN_GARGANTUAR][0].loadFromFile("assets/roman_gargantuar_walk.png");
	textures[TEXTURE_TYPE::ZOMBIE_ROMAN_GARGANTUAR][1].loadFromFile("assets/roman_gargantuar_die.png");
	textures[TEXTURE_TYPE::ZOMBIE_ROMAN_GARGANTUAR][2].loadFromFile("assets/roman_gargantuar_attack.png");

	Entity zombie{};
	zombie.category = ENTITY_CATEGORY::ZOMBIE;
	zombie.type = ENTITY_TYPE::ZOMBIE_ROMAN_GARGANTUAR;
	zombie.health = 3000;
	zombie.attack = 50;
	zombie.x_vel = 40;
	zombie.scale = 1.0f;

	zombie.num_animations = 3;
	Animation anim;
	anim.frames = 18;
	anim.frameTimer = 0.08f;

	zombie.animations[0] = anim;
	zombie.animations[1] = anim;
	zombie.animations[2] = anim;

	zombie.setCurrentAnimation(0);
	zombie.updateEntity = [](Entity& z) {
		if (z.health <= 0 && z.currentAnimationIndex == 9) {
			z.setCurrentAnimation(1);
			z.x_vel = 0;
		}
		};

	return zombie;
}
Entity defineLostCityGargantuar() {
	textures[TEXTURE_TYPE::ZOMBIE_LOSTCITY_GARGANTUAR][0].loadFromFile("assets/lostcity_gargantuar_walk.png");
	textures[TEXTURE_TYPE::ZOMBIE_LOSTCITY_GARGANTUAR][1].loadFromFile("assets/lostcity_gargantuar_die.png");
	textures[TEXTURE_TYPE::ZOMBIE_LOSTCITY_GARGANTUAR][2].loadFromFile("assets/lostcity_gargantuar_attack.png");

	Entity zombie{};
	zombie.category = ENTITY_CATEGORY::ZOMBIE;
	zombie.type = ENTITY_TYPE::ZOMBIE_LOSTCITY_GARGANTUAR;
	zombie.health = 3000;
	zombie.attack = 50;
	zombie.x_vel = 40;
	zombie.scale = 1.0f;

	zombie.num_animations = 3;
	Animation anim;
	anim.frames = 18;
	anim.frameTimer = 0.08f;

	zombie.animations[0] = anim;
	zombie.animations[1] = anim;
	zombie.animations[2] = anim;

	zombie.setCurrentAnimation(0);
	zombie.updateEntity = [](Entity& z) {
		if (z.health <= 0 && z.currentAnimationIndex == 9) {
			z.setCurrentAnimation(1);
			z.x_vel = 0;
		}
		};

	return zombie;
}

void UpdateBooster(RenderWindow& window) {
	if (!booster.isAlive) return;
	ariseSprite.setPosition(booster.position);
	ariseSprite.setTexture(textures[TEXTURE_TYPE::BOOSTER][Default_Texture_Index]);
	ariseSprite.setScale(booster.scale);
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
		if (!isGameOver && (!isGameWon || currentLevel->winClock.getElapsedTime().asSeconds() < wincooldowntimer)) plant.nextFrame(); // next frame
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
			|| (zombie.type == ENTITY_TYPE::ZOMBIE_PIANIST && zombie.currentAnimationIndex == 2)
			|| (zombie.type == ENTITY_TYPE::ZOMBIE_PROSPECTOR && zombie.currentAnimationIndex == 3)
			|| (zombie.type == ENTITY_TYPE::ZOMBIE_CHICKENWRANGLER && zombie.currentAnimationIndex == 3)
			|| (zombie.type == ENTITY_TYPE::ZOMBIE_CHICKENWRANGLER && zombie.currentAnimationIndex == 2)
			|| (zombie.type == ENTITY_TYPE::ZOMBIE_NEWSPAPER && zombie.currentAnimationIndex == 2)
			|| (zombie.type == ENTITY_TYPE::ZOMBIE_ALLSTAR && zombie.currentAnimationIndex == 2)
			|| (zombie.type == ENTITY_TYPE::ZOMBIE_PHARAOH && zombie.currentAnimationIndex == 2)
			|| (zombie.type == ENTITY_TYPE::ZOMBIE_ALLSTAR && zombie.currentAnimationIndex == 3)
			|| (zombie.type == ENTITY_TYPE::ZOMBIE_RA && zombie.isUsingAbility)
			|| ((zombie.type == ENTITY_TYPE::ZOMBIE_PROSPECTOR || zombie.type == ENTITY_TYPE::ZOMBIE_REDHAIR || zombie.type == ENTITY_TYPE::ZOMBIE_COWBOY) && zombie.currentAnimationIndex == 2))
			|| (zombie.type == ENTITY_TYPE::ZOMBIE_ROMAN_CONEHEAD && zombie.currentAnimationIndex != 1 && zombie.currentAnimationIndex != 9)
			|| (zombie.type == ENTITY_TYPE::ZOMBIE_ROMAN_BUCKETHEAD && zombie.currentAnimationIndex != 1 && zombie.currentAnimationIndex != 9)
			|| (zombie.type == ENTITY_TYPE::ZOMBIE_ROMAN_CENTURION && zombie.currentAnimationIndex != 1 && zombie.currentAnimationIndex != 9)
			|| (zombie.type == ENTITY_TYPE::ZOMBIE_ROMAN_BUSTHEAD && zombie.currentAnimationIndex != 1 && zombie.currentAnimationIndex != 9)
			|| (zombie.type == ENTITY_TYPE::ZOMBIE_ROMAN_ODESSA && zombie.currentAnimationIndex != 1 && zombie.currentAnimationIndex != 9)
			|| (zombie.type == ENTITY_TYPE::ZOMBIE_ROMAN_SHIELD && zombie.currentAnimationIndex != 1 && zombie.currentAnimationIndex != 9)
			|| (zombie.type == ENTITY_TYPE::ZOMBIE_ROMAN_NORMAL && zombie.currentAnimationIndex != 1 && zombie.currentAnimationIndex != 9)
			|| (zombie.type == ENTITY_TYPE::ZOMBIE_ROMAN_GARGANTUAR && zombie.currentAnimationIndex != 1 && zombie.currentAnimationIndex != 9)
			|| (zombie.type == ENTITY_TYPE::ZOMBIE_LOSTCITY_GARGANTUAR && zombie.currentAnimationIndex != 1 && zombie.currentAnimationIndex != 9)) {

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
			if (zombie.diedByElectrocution) {
				zombie.scale = 0.40f;
				Animation electroAnim;
				electroAnim.frames = 18;
				electroAnim.frameTimer = 0.08f;
				zombie.animations[9] = electroAnim;
				zombie.DoAnimation(1, 9);
			} else if (zombie.type == ENTITY_TYPE::ZOMBIE_POGO) {
				if (zombie.currentAnimationIndex == 3) {
					zombie.DoAnimation(1, 5);
				} else {
					zombie.DoAnimation(1, 2);
				}
			} else if (zombie.type == ENTITY_TYPE::ZOMBOT_DRAGON) {
				// Zombot Dragon Death
				cout << "---------- 1 You killed Zombot\n";	// maybe message here to kill the boss
				playZombotDragonDeath();
				float s = zombie.scale;
				zombie.DoAnimation(1, 1);
				zombie.afterAnimation = [s](Entity& zombie) {
					cout << "---------- 2 You killed Zombot\n";	// maybe message here to kill the boss
					zombie.DoAnimation(1, 4);
					zombie.afterAnimation = [s](Entity& zombie) {
						cout << "---------- 3 You killed Zombot\n";	// maybe message here to kill the boss
						zombie.scale = s;
						cout << "You killed Zombot\n";	// maybe message here to kill the boss
						//zombie.category = ENTITY_CATEGORY::NONE;
						attackZombie(zombie, 10000000, false);
						};
					};
			} else {
				zombie.DoAnimation(1, 1);

				if (zombie.type == ENTITY_TYPE::ZOMBIE_NORMAL
					|| zombie.type == ENTITY_TYPE::ZOMBIE_CONEHEAD
					|| zombie.type == ENTITY_TYPE::ZOMBIE_NEWSPAPER
					|| zombie.type == ENTITY_TYPE::ZOMBIE_ALLSTAR
					|| zombie.type == ENTITY_TYPE::ZOMBIE_MUMMY
					|| zombie.type == ENTITY_TYPE::ZOMBIE_MUMMY
					|| zombie.type == ENTITY_TYPE::ZOMBIE_RA
					|| zombie.type == ENTITY_TYPE::ZOMBIE_TORCHLIGHT
					|| zombie.type == ENTITY_TYPE::ZOMBIE_PHARAOH
					|| zombie.type == ENTITY_TYPE::ZOMBIE_COWBOY
					|| zombie.type == ENTITY_TYPE::ZOMBIE_REDHAIR
					|| zombie.type == ENTITY_TYPE::ZOMBIE_PROSPECTOR
					|| zombie.type == ENTITY_TYPE::ZOMBIE_CHICKENWRANGLER
					) {

					SpawnFallingItem(TEXTURE_TYPE::ITEM_HEAD, zombie.position + Vector2f(75, 75));
				}
			}


			if (!zombie.diedByElectrocution) {
				if (zombie.type == ENTITY_TYPE::ZOMBIE_RA) // for ra
					zombie.scale = 1.3f;
				if (zombie.type == ENTITY_TYPE::ZOMBIE_PHARAOH) // for pharaoh
					zombie.scale = 0.8f;
				if (zombie.type == ENTITY_TYPE::ZOMBIE_SHIELDGUARD) // for shield guard
					zombie.scale = 1.2f;
				if (zombie.type == ENTITY_TYPE::ZOMBIE_PIANIST) // for pianist
					zombie.scale = 1.3f;
			}

			// [1.2] After Death Animation
			zombie.afterAnimation = [](Entity& zombie) {
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
	if (zombie.type == ENTITY_TYPE::ZOMBOT_DRAGON) return;
	auto& mwr = currentLevel->lawnmowers[zombie.gridRow];
	if (!mwr.isUsed && mwr.position.x >= zombie.position.x) {
		attackZombie(zombie, 100000, true);
		mwr.isMoving = true;
	}
}
void checkZombieEnteredHome(Entity& zombie) {
	if (zombie.enteredHome) {
		if (zombie.position.y < 525) {
			zombie.position.y += 80 * delta_time;
			if (zombie.position.y > 525) zombie.position.y = 525;
		} else if (zombie.position.y > 525) {
			zombie.position.y -= 80 * delta_time;
			if (zombie.position.y < 525) zombie.position.y = 525;
		} else {
			zombie.position.x -= zombie.x_vel * delta_time;
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

			float dx = zombie.position.x - sun.position.x;//horizontal dist
			float dy = zombie.position.y - sun.position.y;//vertical dist
			float dist = sqrt(dx * dx + dy * dy);// dist between

			if (dist > 1.0f) {
				sun.position.x += (dx / dist) * 300.0f * delta_time; //dx/dist is horizontal part of dist
				sun.position.y += (dy / dist) * 300.0f * delta_time; //dy/dist is vertical part of dist
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
			zombie.x_vel = 110.0f;
			zombie.setCurrentAnimation(0);
			zombie.scale = 1.0f;
			zombie.abilityClock.restart();
		}
	}
}
void updateZombieAllStars(Entity& zombie) {
	if (!zombie.hasTackled && !zombie.isTackling) {
		bool plantAhead = false;
		for (int i = 0; i < MAX_ENTITIES; i++) {
			if (entities[i].category != ENTITY_CATEGORY::PLANT)
				continue;
			if (entities[i].isDead)
				continue;
			if (abs(entities[i].position.y - zombie.position.y) < 100.0f &&
				zombie.position.x - (entities[i].position.x + entities[i].width) <= eatDistance - 150) {
				plantAhead = true;
				break;
			}
		}

		if (plantAhead) {
			zombie.isTackling = true;
			zombie.x_vel = 0;
			zombie.DoAnimation(1, 3);
			zombie.afterAnimation = [](Entity& entity) {
				entity.x_vel = 70;
				entity.setCurrentAnimation(0);
				};
		}
	}

	if (zombie.isTackling) {
		if (zombie.currentAnimation.currentFrame >= 8) {
			for (int i = 0; i < MAX_ENTITIES; i++) {
				if (entities[i].category != ENTITY_CATEGORY::PLANT)
					continue;
				if (entities[i].isDead)
					continue;
				if (abs(entities[i].position.y - zombie.position.y) < 100.0f &&
					zombie.position.x - (entities[i].position.x + entities[i].width) <= eatDistance - 150) {
					entities[i].health = 0;
					entities[i].category = ENTITY_CATEGORY::NONE;
					entities[i].isDead = true;
					zombie.isTackling = false;
					zombie.hasTackled = true;
					zombie.health += 200;
					break;
				}
			}
		}
	}
}
void updateNewspaperZombie(Entity& zombie) {
	if (zombie.health <= 100 && zombie.currentAnimationIndex == 0) {
		zombie.setCurrentAnimation(2);
		SpawnFallingItem(TEXTURE_TYPE::ITEM_NEWSPAPER, zombie.position + Vector2f(100, 0));
		zombie.x_vel = 170.0f;
		zombie.attackTimer = 0.4f;
	}
}
void updateConeheadZombie(Entity& zombie) {
	if (zombie.health <= 100 && zombie.currentAnimationIndex == 0) {
		zombie.setCurrentAnimation(2);
		SpawnFallingItem(TEXTURE_TYPE::ITEM_CONE, zombie.position + Vector2f(100, 0));
	}
}

void updatePharaohZombie(Entity& zombie) {
	if (zombie.health <= 300 && !zombie.shellDestroyed) {
		zombie.shellDestroyed = true;
		if (zombie.currentAnimationIndex != 2) {
			zombie.setCurrentAnimation(2);
			SpawnFallingItem(TEXTURE_TYPE::ITEM_PHAROAH_CASKET, zombie.position + Vector2f(100, 0));
		}
		zombie.x_vel = 70;
	}
}

void updatePianistZombie(Entity& zombie) {

	if (zombie.currentAnimationIndex == 0) {
		zombie.scale = 1.2f;
	} else if (zombie.currentAnimationIndex == 2) {
		zombie.scale = 1.0f;
	}

	if (zombie.health <= 250 && zombie.currentAnimationIndex != 2) {
		zombie.scale = 1.0f;
		zombie.setCurrentAnimation(2);
		SpawnFallingItem(TEXTURE_TYPE::ITEM_JAR, zombie.position + Vector2f(50, -25));
	}


	for (Entity& e : entities) {
		if (e.category != ENTITY_CATEGORY::ZOMBIE || e.type != ENTITY_TYPE::ZOMBIE_PROSPECTOR || e.health <= 0 || e.isDead)
			continue;

		// doesnt work if zombie is eating or too far away
		bool isEating;
		if (e.currentAnimationIndex == 2) {
			isEating = true;
		} else {
			isEating = false;
		}

		// check for all pianists not 1
		bool nearAnyPianist = false;
		for (Entity& ent : entities) {
			if (ent.category != ENTITY_CATEGORY::ZOMBIE || ent.type != ENTITY_TYPE::ZOMBIE_PIANIST || ent.health <= 0 || ent.isDead)
				continue;
			if (abs(e.position.x - ent.position.x) <= 600.0f) {
				nearAnyPianist = true;
				break;
			}
		}

		if (isEating || !nearAnyPianist) {

			if (e.currentAnimationIndex == 3) {
				e.setCurrentAnimation(0);
				e.x_vel = 70;
			}

			continue;
		}

		if (e.currentAnimationIndex != 3 && e.currentAnimationIndex != 1) {
			e.setCurrentAnimation(3);
			e.x_vel = 0.0f;
			e.storedSunValue = e.gridRow; // storedsunvalue as a random int variable to use from entity struct and used as target row
			e.abilityClock.restart();
		}

		if (e.currentAnimationIndex == 3 && currentLevel != nullptr) {
			float targetY = currentLevel->offsetY + e.storedSunValue * currentLevel->cellHeight;
			float dy = targetY - e.position.y;

			if (abs(dy) > 4.0f) {
				float moveSpeed = 120.0f;
				e.position.y += (dy > 0 ? 1.0f : -1.0f) * moveSpeed * delta_time;   //dy condition for direction
			} else {  // zombie close enough now
				e.position.y = targetY;
				e.gridRow = e.storedSunValue;

				float switchCooldown = 3.5f + (rand() % 100) / 100.0f;
				if (e.abilityClock.getElapsedTime().asSeconds() >= switchCooldown) {
					int randRow;
					do {
						randRow = rand() % gridRows;
					} while (randRow == e.gridRow);
					e.storedSunValue = randRow;
					e.abilityClock.restart();
				}
			}
		}
	}

	if (zombie.health <= 0) {
		for (Entity& e : entities) {
			if (e.category != ENTITY_CATEGORY::ZOMBIE || e.type != ENTITY_TYPE::ZOMBIE_PROSPECTOR || e.health <= 0 || e.isDead)
				continue;

			if (e.currentAnimationIndex == 3) {
				e.setCurrentAnimation(0);
				e.x_vel = 70;
			}
		}
	}
}

void updateChickenWranglerZombie(Entity& zombie) {
	if (zombie.health <= 150 && zombie.currentAnimationIndex == 0) {
		zombie.setCurrentAnimation(3);
		currentLevel->addWave(10, 5, { zombieChicken });
	}
}

//int rowToBeBurnt = -1;
// Zombot Dragon Shot
void burnRow(int r) {
	//rowToBeBurnt = r;
}

Clock dragonJump, jumping;
bool inJump = true;
//bool inJump = false;
int D = 1, U = -1;
int prevR = 0, direction = D;
float jumpTimer = 5.f;
Vector2f gridCellPosition(int r, int c, Entity& e) {	// DONT USE
	float x = currentLevel->offsetX + (c * currentLevel->cellWidth); //+ ((currentLevel->cellWidth - e.width) / 2);
	float y = currentLevel->offsetY + (r * currentLevel->cellHeight); //+ (currentLevel->cellHeight - e.height);
	return { x,y };
}

float v = 550;
int targetRow = 3;
void updateZombotDragon(Entity& zombie) {
	if (gamePaused) return;

	Vector2f goodpos = { windowWidth - 450, windowHeight / 2 };
	if (zombie.position.x <= goodpos.x) {
		zombie.x_vel = 0; entered_zombotDragon = true;
	}

	/*if (f && zombie.position.y > gridCellPosition(0, 0, zombie).y) {
		zombie.position.y = gridCellPosition(0, 0, zombie).y;
		prevR = 0;
		f = false;
	}*/

	if (!entered_zombotDragon || zombie.currentAnimationIndex == 1 || zombie.currentAnimationIndex == 4) {
		dragonJump.restart();
		return;
	}


	// After entering, change the attackTimer
	zombie.attackTimer = 30.f;

	if (!inJump && dragonJump.getElapsedTime().asSeconds() >= jumpTimer) {
		inJump = true;
		targetRow = rand() % (gridRows);	// do not come to last row [removed to try]
		cout << "000 chose targetROW = " << targetRow << endl;
		playZombotDragonRoar();
	}
	float targetRowY = gridCellPosition(targetRow, 0, zombie).y;

	int comp = (targetRow - prevR);
	if (comp > 0) direction = D;
	else if (comp < 0) direction = U;
	else direction = 0;
	//cout << "directionw is "<<direction<<endl;

	if (inJump && (direction == 0 || zombie.position.y >= targetRowY && direction == D || zombie.position.y <= targetRowY && direction == U)) {
		// stop
		inJump = false;
		dragonJump.restart();
		prevR = targetRow;
		cout << "stopped jump\n";
	}

	else if (inJump) {
		// move
		//v += acc;
		zombie.position.y += direction * v * delta_time;
		//zombie.position.y += direction * 0.5;
		//cout << "Zombot moving to y = " << zombie.position.y << " row [x] col [y]= "<< gridCell(zombie.position).x << " "<<gridCell(zombie.position).y << endl;
		//cout << "direction is " << direction << endl;
	}

	// Attack
	if (!zombie.isDead && zombie.health > 0 && zombie.attackClock.getElapsedTime().asSeconds() >= zombie.attackTimer) {
		float s = zombie.scale;
		zombie.DoAnimation(1, 2);
		zombie.afterAnimation = [](Entity& z) {
			cout << "------- after zombot dragon attack\n";
			};
		zombie.scale = 1.25;
		// Play sound effect
		playZombotDragonFireBall();
		// 1- Shoot a bullet
		Entity newBullet = defineZombotDragonBullet();
		newBullet.position = { zombie.position.x, zombie.position.y + 50 };
		newBullet.x_vel = 300;
		addBullet(newBullet);
		// 2- Push a wave
		// Entity normalZombie, zombieJapanese, zombieBig, zombieTorch, zombieRa, zombieMummy, zombieConehead, zombieNewspaper, zombieAllStar, zombiePharaoh, zombieShieldGuard, zombieCowboy, zombieRedhair, zombieProspector, zombiePogo, zombiePianist, zombotDragon;
		currentLevel->addWave(15, 20, { normalZombie, normalZombie, normalZombie, zombieAllStar, zombieRa, zombiePharaoh, zombieProspector, zombieNewspaper, zombieRedhair }, 20, false);
		cout << "Dragon pushed a wave\n";
		cout << "waves size = " << currentLevel->waves.size() << endl;
		// Restart clock
		zombie.attackClock.restart();

	}
}
void zombieEatPlant(Entity& zombie, Entity& plant) {
	if (gamePaused) return;

	if (zombie.position.x - (plant.position.x + plant.width - eatDistance) <= 0) {
		zombie.x_vel = 0;


		if ((zombie.type == ENTITY_TYPE::ZOMBIE_PROSPECTOR ||
			zombie.type == ENTITY_TYPE::ZOMBIE_REDHAIR ||
			zombie.type == ENTITY_TYPE::ZOMBIE_CHICKENWRANGLER ||
			zombie.type == ENTITY_TYPE::ZOMBIE_COWBOY) && zombie.currentAnimationIndex == 0) {
			zombie.setCurrentAnimation(2);
		} else if (zombie.type == ENTITY_TYPE::ZOMBIE_ROMAN_GARGANTUAR || zombie.type == ENTITY_TYPE::ZOMBIE_LOSTCITY_GARGANTUAR) {
			if (zombie.currentAnimationIndex == 0) {
				zombie.setCurrentAnimation(2);
			}
		} else if (zombie.type == ENTITY_TYPE::ZOMBIE_CHICKENWRANGLER && zombie.currentAnimationIndex == 3) {
			zombie.setCurrentAnimation(2);
		} else if (zombie.type == ENTITY_TYPE::ZOMBIE_POGO && zombie.currentAnimationIndex == 0) {
			zombie.setCurrentAnimation(1);
		} else if (zombie.type == ENTITY_TYPE::ZOMBIE_ROMAN_ODESSA && (zombie.currentAnimationIndex == 0 || zombie.currentAnimationIndex == 2)) {
			zombie.setCurrentAnimation(3);
		} else if (zombie.currentAnimationIndex == 0) {
			bool isRomanWithHelmet = (
				zombie.type == ENTITY_TYPE::ZOMBIE_ROMAN_CONEHEAD ||
				zombie.type == ENTITY_TYPE::ZOMBIE_ROMAN_BUCKETHEAD ||
				zombie.type == ENTITY_TYPE::ZOMBIE_ROMAN_CENTURION ||
				zombie.type == ENTITY_TYPE::ZOMBIE_ROMAN_BUSTHEAD
				);

			if (zombie.num_animations > 3 && !isRomanWithHelmet) {
				zombie.setCurrentAnimation(3);
			}
		}

		if (zombie.attackClock.getElapsedTime().asSeconds() >= zombie.attackTimer) {
			zombie.attackClock.restart();
			plant.health -= zombie.attack;
			plant.lastDamaged = currentLevel->clock.getElapsedTime();


			if (plant.health <= 0) {
				if (zombie.health > 0) {
					if (zombie.type == ENTITY_TYPE::ZOMBIE_ROMAN_GARGANTUAR || zombie.type == ENTITY_TYPE::ZOMBIE_LOSTCITY_GARGANTUAR) {
						zombie.x_vel = 40;
					} else {
						zombie.x_vel = 70;
					}

					if (zombie.type == ENTITY_TYPE::ZOMBIE_ROMAN_ODESSA) {
						if (zombie.health <= 200)
							zombie.setCurrentAnimation(2);
						else
							zombie.setCurrentAnimation(0);
					} else if
						(zombie.type == ENTITY_TYPE::ZOMBIE_POGO) {
						zombie.setCurrentAnimation(0);
					} else if (zombie.type == ENTITY_TYPE::ZOMBIE_PROSPECTOR ||
						zombie.type == ENTITY_TYPE::ZOMBIE_REDHAIR ||
						zombie.type == ENTITY_TYPE::ZOMBIE_COWBOY) {
						zombie.setCurrentAnimation(0);
					} else if (zombie.type == ENTITY_TYPE::ZOMBIE_ROMAN_GARGANTUAR || zombie.type == ENTITY_TYPE::ZOMBIE_LOSTCITY_GARGANTUAR) {
						if (zombie.currentAnimationIndex == 2) {
							zombie.setCurrentAnimation(0);
						}
					}

					else if (zombie.type == ENTITY_TYPE::ZOMBIE_ROMAN_CONEHEAD ||
						zombie.type == ENTITY_TYPE::ZOMBIE_ROMAN_BUCKETHEAD ||
						zombie.type == ENTITY_TYPE::ZOMBIE_ROMAN_CENTURION ||
						zombie.type == ENTITY_TYPE::ZOMBIE_ROMAN_BUSTHEAD) {
						if (zombie.shellDestroyed)
							zombie.setCurrentAnimation(2);
						else zombie.setCurrentAnimation(0);
					} else {
						zombie.setCurrentAnimation(0);
					}
				}
				plant.category = ENTITY_CATEGORY::NONE;
				plant.isDead = true;
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
		if ((!isGameOver && (!isGameWon || currentLevel->winClock.getElapsedTime().asSeconds() < wincooldowntimer)) || zombie.enteredHome) zombie.nextFrame(); // next frame
		IntRect frameRect = getFrameRect(zombie);
		zombieSprite.setTextureRect(frameRect); // crop texture

		// call update function if exists
		if (zombie.updateEntity != nullptr) {
			zombie.updateEntity(zombie);
			//continue;
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
		if (zombie.type == ENTITY_TYPE::ZOMBIE_PIANIST && zombie.health > 0) { // for pianist
			updatePianistZombie(zombie);
		}
		if (zombie.type == ENTITY_TYPE::ZOMBOT_DRAGON && zombie.health > 0) { // for pianist
			updateZombotDragon(zombie);
		}
		if (zombie.type == ENTITY_TYPE::ZOMBIE_CHICKENWRANGLER && zombie.health > 0) {
			updateChickenWranglerZombie(zombie);
		}

		// zombie move
		if (!isGameOver && !isGameWon) zombie.position.x -= zombie.x_vel * delta_time;

		// check enters home
		checkZombieEnteredHome(zombie);

		// touch plant
		auto [r, c] = gridCell(zombie.position);
		if (zombie.position.x > 0 + edgeSpace) {
			// Eat
			Entity& plant = getEntity({ r, c });
			if (plant.category == ENTITY_CATEGORY::PLANT) { // touching a plant
				if (zombie.type == ENTITY_TYPE::ZOMBIE_POGO && zombie.currentAnimationIndex == 3) {
					zombie.x_vel = 170; //plants don't stop him
				} else {
					zombieEatPlant(zombie, plant);
				}
			} else if (zombie.health > 0) {
				if (!(zombie.type == ENTITY_TYPE::ZOMBIE_RA && zombie.isUsingAbility)) {
					if (zombie.isSuperFrozen) {
						zombie.x_vel = 2.8f;
					} else if (zombie.freezeTimer > 0) {
						zombie.x_vel = 28;
					} else if (zombie.type == ENTITY_TYPE::ZOMBIE_POGO) {
						if (zombie.currentAnimationIndex == 3) zombie.x_vel = 170;
						else if (zombie.currentAnimationIndex == 0) zombie.x_vel = 70;
						else zombie.x_vel = 0.0f;
					} else if (zombie.type == ENTITY_TYPE::ZOMBIE_NEWSPAPER && zombie.currentAnimationIndex == 2) {
						zombie.x_vel = 170;
					} else if (zombie.type == ENTITY_TYPE::ZOMBIE_ALLSTAR && !zombie.hasTackled && !zombie.isTackling) {
						zombie.x_vel = 260;
					} else if (zombie.type == ENTITY_TYPE::ZOMBIE_PIANIST) {
						zombie.x_vel = 20;
					} else if (zombie.type == ENTITY_TYPE::ZOMBIE_CHICKEN) {
						zombie.x_vel = 200;
					} else if (zombie.type == ENTITY_TYPE::ZOMBIE_PHARAOH && zombie.shellDestroyed) {
						zombie.x_vel = 135;
					} else {
						zombie.x_vel = 70; // no plant - has health - move normally
					}
				}
			}
		}

		int frameHeight = frameRect.getSize().y;
		float actualHeight = frameHeight * zombie.scale;
		Vector2f adjustedPosition = zombie.position;
		adjustedPosition.y -= (actualHeight - currentLevel->cellHeight);	// as if legs on ground of the cell- so head above the cell instead of legs below the cell

		zombieSprite.setPosition(adjustedPosition);
		TEXTURE_TYPE texType = getTextureType(zombie.type);
		int texIndex = zombie.currentAnimationIndex;

		if (zombie.diedByElectrocution && texIndex == 9) {
			texType = TEXTURE_TYPE::ZOMBIE_ELECTROCUTED;
			texIndex = 0;
		}
		zombieSprite.setTexture(textures[texType][texIndex]);
		zombieSprite.setScale(Vector2f(zombie.scale, zombie.scale));

		if (!isGameOver) {
			int damage = 255 - (std::max(1 - ((currentLevel->clock.getElapsedTime() - zombie.lastDamaged).asSeconds() / 0.25f), 0.0f) * 155);
			zombieSprite.setColor(zombieSprite.getColor() * Color(255, damage, damage));
		}
		// Change Origin For Zombot dragon as its huge
		Vector2f orgOrg = zombieSprite.getOrigin();
		if (zombie.type == ENTITY_TYPE::ZOMBOT_DRAGON && zombie.currentAnimationIndex == 0) zombieSprite.setOrigin(zombieSprite.getOrigin().x, -2 * zombieSprite.getGlobalBounds().height / 5);
		if (zombie.type == ENTITY_TYPE::ZOMBOT_DRAGON && zombie.currentAnimationIndex == 2) zombieSprite.setOrigin(zombieSprite.getOrigin().x + 35, -2 * zombieSprite.getGlobalBounds().height / 5 + 30);
		//if (zombie.type == ENTITY_TYPE::ZOMBOT_DRAGON) zombieSprite.setOrigin(zombieSprite.getOrigin().x, - 270);

		window.draw(zombieSprite);

		// For Zombot as its huge
		if (zombie.type == ENTITY_TYPE::ZOMBOT_DRAGON) zombieSprite.setOrigin(orgOrg);
	}
}

// Stop and Damage (not start) of Bullets along the way + Draw and Animation of bullets
void UpdateAndDrawBullets(RenderWindow& window) {
	for (Entity& bullet : bullets) {
		if (bullet.category != ENTITY_CATEGORY::Bullet) continue;

		bullet.position.x -= bullet.x_vel * delta_time; // move right

		if (bullet.position.x >= windowWidth || bullet.position.x <= currentLevel->offsetX) {
			bullet.category = ENTITY_CATEGORY::NONE;
			continue;
		}


		// Zombot Dragon Bullet (Out of zombie)
		if (bullet.type == ENTITY_TYPE::ZombotDragonBullet) {
			for (Entity& plant : entities) {
				if (plant.category != ENTITY_CATEGORY::PLANT || plant.health <= 0) continue;

				if (abs(bullet.position.y - plant.position.y) < 100.0f &&
					bullet.position.x >= plant.position.x &&
					bullet.position.x <= plant.position.x + 100.0f) {

					plant.category = ENTITY_CATEGORY::NONE;
					plant.health = 0;
					//cout << "Zombot Dragon Bullet hit plant\n";
				}
			}
		} else {

			// Plant Bullet
			for (Entity& zombie : entities) {

				if (zombie.category != ENTITY_CATEGORY::ZOMBIE || zombie.isDead || zombie.health <= 0) continue;


				bool touchZombie = abs(bullet.position.y - zombie.position.y) < 100.0f &&
					bullet.position.x >= zombie.position.x &&
					bullet.position.x <= zombie.position.x + 100.0f;
				bool snapTouchZombie = bullet.type == ENTITY_TYPE::SnapDragonBullet &&
					abs(bullet.position.y - zombie.position.y) < 100.0f &&
					bullet.position.x >= zombie.position.x &&
					bullet.position.x <= zombie.position.x + 10.0f;

				if (snapTouchZombie) {
					// Stop at 3 cells
				}
				if (touchZombie) {

					//cout << "bullet type " << int(bullet.type )<< " shot entity type " << int(zombie.type )<< endl;
					//vector<ENTITY_TYPE> unkilled_bullets = {ENTITY_TYPE::SnapDragonBoostBullet};

					if ((zombie.type == ENTITY_TYPE::ZOMBIE_SHIELDGUARD || zombie.type == ENTITY_TYPE::ZOMBIE_ROMAN_SHIELD) && bullet.type != ENTITY_TYPE::ShroomBullet) {
						bullet.category = ENTITY_CATEGORY::NONE;
						break;
					}

					if (bullet.type == ENTITY_TYPE::PomegranateBullet) {
						attackZombie(zombie, bullet.attack, true);
					}

					if (bullet.type == ENTITY_TYPE::FireTornadoBullet) {
						if (zombie.type != ENTITY_TYPE::ZOMBOT_DRAGON) zombie.position.x -= bullet.x_vel * delta_time;
						float expectedDamage = bullet.attack * delta_time;
						int tickDamage = (int)expectedDamage;
						float fractionalPart = expectedDamage - tickDamage;
						if ((rand() % 100) / 100.0f < fractionalPart) {
							tickDamage++;
						}
						if (tickDamage > 0) {
							attackZombie(zombie, tickDamage, false);
						}
						continue;
					}

					if (bullet.type == ENTITY_TYPE::FireTornadoBulletBoosted) {
						if (zombie.type != ENTITY_TYPE::ZOMBOT_DRAGON) zombie.position.x -= (bullet.x_vel - 50) * delta_time;
						float expectedDamage = bullet.attack * delta_time;
						int tickDamage = (int)expectedDamage;
						float fractionalPart = expectedDamage - tickDamage;
						if ((rand() % 100) / 100.0f < fractionalPart) {
							tickDamage++;
						}
						if (tickDamage > 0) {
							attackZombie(zombie, tickDamage, false);
						}
						continue;
					}

					if (bullet.type == ENTITY_TYPE::SnapDragonBoostBullet && zombie.type == ENTITY_TYPE::ZOMBOT_DRAGON) {
						//zombie.boostedBulletHitZombot = false;
						// to hit once only
					}

				}

				if (touchZombie || snapTouchZombie) {
					// Damage Zombie

					//if (zombie.type == ENTITY_TYPE::ZOMBOT_DRAGON) bullet.attack /= 3;		// TODO: change .. a bad fix for bullet keeps damaging the zombie

					attackZombie(zombie, bullet.attack, true);

					/*if (bullet.type == ENTITY_TYPE::SnapDragonBoostBullet && zombie.type == ENTITY_TYPE::ZOMBOT_DRAGON)
						cout << "XXXXCCCCCCCcXXXXXXX ----- zombot hit by snapboosted, health now is " << zombie.health << endl;*/

					if (bullet.type == ENTITY_TYPE::LightningBullet && zombie.health <= 0) {
						zombie.diedByElectrocution = true;
					}

					if (bullet.type == ENTITY_TYPE::IceBullet) {
						zombie.freezeTimer = 3.0f;
					}


					if (bullet.type != ENTITY_TYPE::SnapDragonBoostBullet || (bullet.type == ENTITY_TYPE::SnapDragonBoostBullet && zombie.type == ENTITY_TYPE::ZOMBOT_DRAGON)) {
						// [ if boosted snapdragon bullet touch zombot dragon it disappears, so that it damages it only once, otherwise it continues ]
						// End of the bullet
						bullet.category = ENTITY_CATEGORY::NONE;
						if (bullet.type == ENTITY_TYPE::SnapDragonBoostBullet && zombie.type == ENTITY_TYPE::ZOMBOT_DRAGON)cout << "=============== Removed bullet ----- zombot hit by snapboosted, health now is " << zombie.health << endl;
						break;
					}
				}
			}

		}//
		// Draw Bullet

		if (bullet.category == ENTITY_CATEGORY::Bullet) {
			Texture& bulletTex = textures[getTextureType(bullet.type)][bullet.currentAnimationIndex];
			bulletSprite.setTexture(bulletTex);

			if (bullet.num_animations > 0 && bullet.animations[bullet.currentAnimationIndex].frames > 0) {
				// Animation
				if (!isGameOver && (!isGameWon || currentLevel->winClock.getElapsedTime().asSeconds() < wincooldowntimer))
					bullet.nextFrame(); // next frame
				bulletSprite.setTextureRect(getFrameRect(bullet)); // crop textures
			} else {
				bulletSprite.setTextureRect(IntRect{ 0, 0, (int)bulletTex.getSize().x, (int)bulletTex.getSize().y });
			}

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
			mwr.move(200.0f * delta_time);
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

	if (zombieClock.getElapsedTime().asSeconds() + zombieClockOffset < nextZombieTimer) return;

	auto& waves = currentLevel->waves;
	cout << "waves.size() : " << waves.size() << " | currentWaveIndex : " << currentWaveIndex << endl;

	if (currentWaveIndex >= (int)waves.size()) return;
	auto& wave = waves[currentWaveIndex];

	// Advance to next wave if current is done
	if (curWaveZombieCnt >= wave.zombieCount &&
		wc.getElapsedTime().asSeconds() + wcOffset.asSeconds() >= wave.length) {
		currentWaveIndex++, curWaveZombieCnt = 0;
		wc.restart();
		wcOffset = Time::Zero;
		hugeWaveTriggered = false;
		return; // don't spawn on same tick as wave advance
	}

	if (wave.isHuge) TriggerHugeWave();
	if (createdZombiesCount == 0) TriggerFirstZombie();

	cout << "New Zombie\n";
	zombieClock.restart();
	zombieClockOffset = 0;

	if (wave.zombieCount <= 0) {
		nextZombieTimer = wave.length;
		return;
	}
	nextZombieTimer = wave.length / wave.zombieCount;

	// Spawn zombie
	Entity newZombie = wave.zombieTypes[rand() % wave.zombieTypes.size()];
	int row;
	do { row = rand() % gridraws; } while (row == currentLevel->lastRow);
	currentLevel->lastRow = row;
	newZombie.position = Vector2f(windowWidth, currentLevel->offsetY + row * currentLevel->cellHeight);
	auto [raw, column] = gridCell(newZombie.position);
	newZombie.id = nextEntityId++;
	newZombie.gridRow = raw, newZombie.gridCol = column;
	if (addEntity(newZombie) != -1) {
		createdZombiesCount++;
		curWaveZombieCnt++;
		dz++;
	}
}
void CheckTimer_AddSun(RenderWindow& window) {
	if (currentLevel == nullptr) return;

	if (sunSpawnClock.getElapsedTime().asSeconds() >= currentLevel->sunSpawnInterval) {
		cout << "New Sun\n";
		sunSpawnClock.restart();

		Sun sun{};
		sun.position = Vector2f(rand() % (windowWidth - 200) + 100, -215);
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

bool selectingLevel = false;
bool inStore = false;
Time startMenuItemTime = Time::Zero;
Time startMainMenuTime = Time::Zero;

// There is a bug in SFML that causes the program to crash when exiting
// Making this a pointer and deleting it early prevents that crash
Font* font;
Font* loudFont;

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
	if (scale >= 0.1f) window.draw(text);

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
		if ((currentLevel->arePlantsSelected && sunsMoney < plant.price) || plant.currentSeedCooldown > 0) {
			packetColor = Color(64, 64, 64, 255);
		} else if (currentLevel->arePlantsSelected && selectedPlant == i) {
			if (!isShovelActive && !isRelocateActive && !isBoostActive) {
				packetColor = Color(190, 255, 190, 255);
			}
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
						isShovelActive = false;
						isRelocateActive = false;
						isBoostActive = false;
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

	if (currentLevel->selectedPlants.empty()) {
		startButton.setColor(Color(64, 64, 64, 255));
	} else if (startButton.getGlobalBounds().contains(Vector2f(mousePos))) {
		startButton.setColor(Color(128, 128, 128, 255));
		if (mouseJustClicked[0]) {
			currentLevel->arePlantsSelected = true;
			currentLevel->plantsSelectedTime = currentLevel->clock.getElapsedTime();

			// Queue level music to be started by the main loop.
			// For level two explicitly use the level 2 music.
			if (currentLevel != nullptr) {
				if (currentLevel->lvlNum == 2) requestedLevelMusic = 2;
				else requestedLevelMusic = currentLevel->lvlNum;
			}
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
	text.setFont(*loudFont);
	text.setString("Level Rewards");
	text.setFillColor(Color(255, 255, 255, 255));
	text.setPosition(Vector2f(960, 130.0f));
	text.setCharacterSize(64);
	text.setOrigin(text.getLocalBounds().getSize() / 2.0f);
	text.setOutlineColor(Color::Black);
	text.setOutlineThickness(6);

	Vector2u seedPacketSize = getTexture(TEXTURE_TYPE::SEEDPACKET).getSize();

	float diff = currentLevel->winClock.getElapsedTime().asSeconds() - wincooldowntimer;

	screenFade(window, 0.5f * std::min((diff - 4.5f) / 1.0f, 1.0f));

	window.draw(text);

	float progress = (diff - 5.0f) / 0.5f;
	if (progress < 0) progress = 0;
	if (progress > 1) progress = 1;
	float scale = 2.0f * progress;

	Sprite packetBg;
	Texture& packetBgTex = getTexture(TEXTURE_TYPE::LUSTER);
	packetBg.setTexture(packetBgTex);
	packetBg.setPosition(Vector2f(960 - 200, 540));
	packetBg.setScale(Vector2f(scale, scale));
	packetBg.setOrigin(Vector2f(packetBgTex.getSize()) / 2.0f);
	Vector2f texSize = Vector2f(packetBgTex.getSize());
	Vector2f size = Vector2f(512, 512) + Vector2f(32, 32) * sin(diff * 1.57f);
	size *= progress;
	packetBg.setScale(Vector2f(size.x / texSize.x, size.y / texSize.y));

	packetBg.setRotation(diff * 3.14f);

	if (unlockedPlant) {
		window.draw(packetBg);
		DrawSeedPacket(window, plants[currentLevel->plantReward.plantType], packetBg.getPosition() - Vector2f(seedPacketSize) * scale / 2.0f, false, scale);
	}

	packetBg.setPosition(Vector2f(960 + 200, 540));
	if (!unlockedPlant) packetBg.setPosition(Vector2f(960, 540));
	window.draw(packetBg);

	{
		Sprite coin;
		coin.setTexture(getTexture(TEXTURE_TYPE::COIN));
		coin.setOrigin(Vector2f(getTexture(TEXTURE_TYPE::COIN).getSize()) / 2.0f);

		coin.setPosition(packetBg.getPosition() + Vector2f(0, -50));
		coin.setScale(scale * 0.5f, scale * 0.5f);

		window.draw(coin);

		// Glowing
		/*coin.setColor(Color(255, 255, 255, 32 * abs(sin(gameClock.getElapsedTime().asSeconds()))));
		window.draw(coin, sf::BlendAdd);*/

		Text text;

		text.setFont(*font);
		text.setString(std::to_string(currentLevel->coinReward.coins) + " Coins");
		text.setCharacterSize(44);
		text.setOrigin(text.getGlobalBounds().getSize() / 2.0f);
		text.setPosition(coin.getPosition() + Vector2f(0, 150));
		text.setFillColor(Color(255, 255, 0, 255 * progress));
		text.setOutlineColor(Color::Black);
		text.setOutlineThickness(6);
		text.setScale(std::max(progress, 0.01f), std::max(progress, 0.01f));

		window.draw(text);
	}

	Sprite startButton;

	startButton.setTexture(getTexture(TEXTURE_TYPE::WOOD_BUTTON));
	startButton.setOrigin(startButton.getGlobalBounds().getSize() / 2.0f);
	startButton.setScale(Vector2f(.25f, .25f));
	startButton.setPosition(Vector2f(windowWidth / 2.0f, 900) + Vector2f(192, 0));

	text.setFont(*font);
	text.setString("Next Level");
	text.setCharacterSize(44);
	text.setOrigin(text.getGlobalBounds().getSize() / 2.0f);
	text.setPosition(startButton.getPosition() + Vector2f(0, -8));
	text.setFillColor(Color::Green);
	text.setOutlineColor(Color::Black);
	text.setOutlineThickness(6);

	if (startButton.getGlobalBounds().contains(Vector2f(mousePos))) {
		startButton.setColor(Color(128, 128, 128, 255));
		text.setFillColor(text.getFillColor() * Color(128, 128, 128, 255));

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
	window.draw(text);

	startButton.setColor(Color::White);
	startButton.setPosition(Vector2f(windowWidth / 2.0f, 900) - Vector2f(192, 0));
	text.setString("Main Menu");
	text.setCharacterSize(44);
	text.setOrigin(text.getGlobalBounds().getSize() / 2.0f);
	text.setPosition(startButton.getPosition() + Vector2f(0, -8));
	text.setFillColor(Color::Red);

	if (startButton.getGlobalBounds().contains(Vector2f(mousePos))) {
		startButton.setColor(Color(128, 128, 128, 255));
		text.setFillColor(text.getFillColor() * Color(128, 128, 128, 255));

		if (mouseJustClicked[0]) {
			currentLevel->claimRewards();
			if (currentLevel->lvlNum < number_levels - 1)
				levels[currentLevel->lvlNum + 1].isUnlocked = true;

			requestedLevelMusic = REQUEST_SELECT_MUSIC;

			resetLevel();
			delete currentLevel;
			currentLevel = nullptr;
			selectingLevel = false;
			startMainMenuTime = gameClock.getElapsedTime();
		}
	}

	window.draw(startButton);
	window.draw(text);
}
void DrawWinScreen(RenderWindow& window) {
	float diff = currentLevel->winClock.getElapsedTime().asSeconds() - wincooldowntimer;
	if (diff >= 4.5f) {
		DrawRewardsScreen(window);
	}

}

// Screen Animations

void screenAnimation(RenderWindow& window) {
	int level = currentLevel->lvlNum;

	int actualTexWidth = ground.getTexture()->getSize().x;
	int actualTexHeight = ground.getTexture()->getSize().y;
	//zoom_scale
	float scale = (float)windowHeight / actualTexHeight;
	ground.setScale(scale, scale);
	int visibleWidth = windowWidth / scale;
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

bool isClicked(Vector2f pos, Vector2f size, Vector2f offset = { 0, 0 }) {
	pos += offset;
	return mousePos.x >= pos.x && mousePos.x <= pos.x + size.x &&
		mousePos.y >= pos.y && mousePos.y <= pos.y + size.y;
}

void DrawPopupText(RenderWindow& window) {
	float diff = currentLevel->clock.getElapsedTime().asSeconds() - currentLevel->plantsSelectedTime.asSeconds();
	if (!currentLevel->arePlantsSelected || diff < 0.75f) return;

	Text text;
	text.setFont(*loudFont);
	text.setPosition(Vector2f(windowWidth / 2.0f, windowHeight / 2.0f));
	text.setFillColor(Color::Red);
	text.setOutlineColor(Color::Black);
	text.setOutlineThickness(8);
	text.setCharacterSize(96);

	float showProgress = 1.0f;
	float hideProgress = 0.0f;

	if (diff < 1.75f) {
		text.setString("READY");
		showProgress = (diff - 0.75f) / 0.25f;
	} else if (diff < 2.75f) {
		text.setString("SET");
		showProgress = (diff - 1.75f) / 0.25f;
	} else if (diff < 4.75f) {
		text.setString("GO");
		showProgress = (diff - 2.75f) / 0.25f;
		hideProgress = (diff - 4.5f) / 0.25f;
	} else if (currentWaveIndex < currentLevel->waves.size() && currentLevel->waves[currentWaveIndex].isHuge && wc.getElapsedTime().asSeconds() + wcOffset.asSeconds() < 5.0f) {
		// Huge Wave

		diff = wc.getElapsedTime().asSeconds();
		showProgress = diff / 0.25f;
		hideProgress = (diff - 4.75f) / 0.25f;
		text.setCharacterSize(80);
		text.setString("A HUGE WAVE OF ZOMBIES\n            ARE APPROACHING");
	} else if (isGameWon && currentLevel->winClock.getElapsedTime().asSeconds() >= wincooldowntimer + 1) {
		diff = currentLevel->winClock.getElapsedTime().asSeconds() - (wincooldowntimer + 1);

		showProgress = diff / 0.25f;
		hideProgress = (diff - 2.75f) / 0.25f;

		text.setFillColor(Color::Green);
		text.setString("Level Complete");
	}

	if (showProgress < 0) showProgress = 0;
	if (showProgress > 1) showProgress = 1;

	if (hideProgress < 0) hideProgress = 0;
	if (hideProgress > 1) hideProgress = 1;

	text.setOrigin(text.getGlobalBounds().getSize() / 2.0f);

	float scale = 1.0f;
	Color newColor = text.getFillColor();

	if (hideProgress > 0) {
		scale = 1 - hideProgress;
		newColor.a *= 1 - hideProgress;
	} else if (showProgress > 0) {
		scale = 1 + (1 - showProgress);
		newColor.a *= showProgress;
	}

	text.setScale(scale, scale);
	text.setFillColor(newColor);
	text.setOutlineColor(Color{ 0, 0, 0, newColor.a });

	window.draw(text);
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
	if (curMagicShovelCnt <= 0) {
		// message that you dont have any magic shovels
		cout << "Not enough magic shovels!" << endl;
		return;
	}
	if (!isRelocating && clickedEntity.category == ENTITY_CATEGORY::PLANT) {
		for (int i = 0; i < MAX_ENTITIES; i++) {
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

			curMagicShovelCnt--;
		} else {
			isRelocating = false;
			selectedPlantIndex = -1;
		}
	}
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
		if (addEntity(plant) != -1) {
			sunsMoney -= plant.price;
			currentLevel->selectedPlants[selectedPlant].currentSeedCooldown = plant.seedCooldown;
		}
	}
}
void uiCollectSuns(RenderWindow& window) {
	for (int i = 0; i < MAX_SUNS; i++) {
		// Check all suns + Add clicked suns to money
		Sun& sun = suns[i];
		if (!isSunAlive(sun)) continue;

		// check mouse pos in sun rect (200x200)
		if (isClicked(sun.position - Vector2f(100, 100), { 200,200 })) {
			sunsMoney += suns[i].sunValue;

			cout << "You now have " << sunsMoney << endl;

			suns[i].clicked = true;
			suns[i].targeted = false; // for ra
			uiInteracted = true;
			break;
		}
	}
}
// boost ui
void uiBoostActive(RenderWindow& window) {
	if (isClicked(activateBoosterBtnPos, { 100, 100 }, { 0, -100 })) {
		if (mouseJustClicked[0]) {
			uiInteracted = true;
			if (curBoosterCnt > 0) {
				isBoostActive = true;
			} else {
				// maybe show some message that you cant have more boosters
			}
		}
	}

}
void uiBoostPlant(RenderWindow& window, Entity& clickedEntity) {
	// Uses the boost on the selected plant
	isBoostActive = false;
	booster.activate(clickedEntity);
	cout << "======== Booster Activated! Remaining Boosters: " << curBoosterCnt << endl;
	/*clickedEntity.boosted = true;
	clickedEntity.boostStartTime = currentLevel->clock.getElapsedTime();
					isBoostActive = false;*/
}
void uiCollectBooster(RenderWindow& window) {
	if (booster.isAlive && !uiInteracted) {
		if (isClicked(booster.position, { 200, 200 })) {
			//if (mousePos.x >= booster.position.x && mousePos.x <= booster.position.x + 200 && mousePos.y >= booster.position.y && mousePos.y <= booster.position.y + 200) {
			booster.collect();
			uiInteracted = true;
		}
	}
}

//	UI interaction - Buttons click, Collect Items, Click plants ... etc
void UI_DrawClick(RenderWindow& window) {
	Text text;
	text.setFont(*font);
	text.setCharacterSize(36);
	text.setOutlineColor(Color::Black);
	text.setOutlineThickness(2);

	//Normal_Shovel
	Texture& shovelTex = getTexture(TEXTURE_TYPE::SHOVEL);
	uiSprite.setTexture(shovelTex);
	resizeSpriteToTex(uiSprite, shovelTex);
	uiSprite.setScale(Vector2f(0.8f, 0.8f));
	uiSprite.setPosition(780.0f, 20.0f);

	if (isShovelActive) uiSprite.setColor(Color(150, 150, 150));
	else uiSprite.setColor(Color::White);

	if (uiSprite.getGlobalBounds().contains(Vector2f(mousePos))) {
		if (mouseJustClicked[0]) {
			isShovelActive = !isShovelActive;
			isRelocateActive = false;
			isBoostActive = false;
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
	uiSprite.setPosition(900.0f, 20.0f);

	if (isRelocateActive) uiSprite.setColor(Color(150, 150, 150));
	else uiSprite.setColor(Color::White);

	if (curMagicShovelCnt <= 0) {
		uiSprite.setColor(Color(100, 100, 100, 255));
	} else if (uiSprite.getGlobalBounds().contains(Vector2f(mousePos))) {
		if (mouseJustClicked[0]) {
			isRelocateActive = !isRelocateActive;
			isShovelActive = false; //NORMAL SHOVEL AND MAGIC SHOVEL DON'T MIX
			isBoostActive = false;

			if (!isRelocateActive) {
				isRelocating = false;
				selectedPlantIndex = -1;
			}
			uiInteracted = true;
		}
	}

	text.setString(std::to_string(curMagicShovelCnt));
	text.setFillColor(curMagicShovelCnt != 0 ? Color::White : Color::Red);
	text.setPosition(uiSprite.getPosition() + Vector2f(75, 75));

	window.draw(uiSprite);
	window.draw(text);

	uiSprite.setColor(Color::White);
	uiSprite.setScale(Vector2f(1.0f, 1.0f)); // return to normal scale for next use

	// Boost
	Texture& boostTex = getTexture(TEXTURE_TYPE::BOOSTER);
	uiSprite.setTexture(boostTex);
	resizeSpriteToTex(uiSprite, boostTex);
	uiSprite.setScale(Vector2f(1, 1));
	uiSprite.setPosition(1020.0f, 20.0f);

	if (isBoostActive) uiSprite.setColor(Color(150, 150, 150));
	else uiSprite.setColor(Color::White);

	if (curBoosterCnt <= 0) {
		uiSprite.setColor(Color(100, 100, 100, 255));
	} else if (uiSprite.getGlobalBounds().contains(Vector2f(mousePos))) {
		if (mouseJustClicked[0]) {
			isBoostActive = !isBoostActive;
			isShovelActive = false;
			isRelocateActive = false;

			uiInteracted = true;
		}
	}

	text.setString(std::to_string(curBoosterCnt));
	text.setFillColor(curBoosterCnt != 0 ? Color::White : Color::Red);
	text.setPosition(uiSprite.getPosition() + Vector2f(75, 75));

	window.draw(uiSprite);
	window.draw(text);

	uiSprite.setColor(Color::White);
	uiSprite.setScale(Vector2f(1.0f, 1.0f)); // return to normal scale for next use


}


void UI_Interaction(RenderWindow& window) {
	if (!uiInteracted && mouseJustClicked[0]) {
		// Collect Suns
		uiCollectSuns(window);
		// Collect booster
		uiCollectBooster(window);
		// Active boost
		uiBoostActive(window);

		// Other
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
	addBoosterBtnPos = Vector2f(0, windowHeight) + Vector2f(20, -120) + animationOffset;	// update the position of the add booster button
	activateBoosterBtnPos = Vector2f(0, windowHeight) + Vector2f(20, -120) + animationOffset;	// Temporary
	sprite.setPosition(addBoosterBtnPos);
	sprite.setScale(
		100.0 / getTexture(TEXTURE_TYPE::BOOSTER).getSize().x,
		100.0 / getTexture(TEXTURE_TYPE::BOOSTER).getSize().y
	);

	text.setString(std::to_string(curBoosterCnt) + "/" + to_string(maxBoosterCnt) + " Boosts");
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
	for (int i = 0; i < MAX_FALLENITEMS; i++) {
		FallenItem& item = fallenItems[i];
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
		sprite.setColor(item.color);

		sprite.setRotation(std::min(time, 1.0f) * 270);

		if (time > 2.5f) {
			float progress = (time - 2.5f) / 1.0f;
			if (progress > 1) progress = 1;
			sprite.setColor(sprite.getColor() * Color(255, 255, 255, (int)(255 * (1 - progress))));
		}

		window.draw(sprite);
		i++;
	}
}

void DrawRailsAndCarts(RenderWindow& window) {
	if (currentLevel == nullptr) return;

	Texture& railTex = getTexture(TEXTURE_TYPE::RAIL);
	Sprite railSprite(railTex);

	float railScaleX = currentLevel->cellWidth / (float)railTex.getSize().x;
	float railScaleY = currentLevel->cellHeight / (float)railTex.getSize().y;
	railSprite.setScale(railScaleX, railScaleY);

	for (const auto& rail : currentLevel->rails) {
		float x = currentLevel->offsetX + (rail.column * currentLevel->cellWidth);

		for (int r = rail.startRow; r <= rail.endRow; r++) {
			float y = currentLevel->offsetY + (r * currentLevel->cellHeight);
			railSprite.setPosition(x, y);
			window.draw(railSprite);
		}
	}

	Texture& cartTex = getTexture(TEXTURE_TYPE::CART);
	Sprite cartSprite(cartTex);

	float cartScaleX = currentLevel->cellWidth / (float)cartTex.getSize().x;
	float cartScaleY = currentLevel->cellHeight / (float)cartTex.getSize().y;
	cartSprite.setScale(cartScaleX, cartScaleY);

	for (const auto& cart : currentLevel->carts) {
		float x = currentLevel->offsetX + (cart.column * currentLevel->cellWidth);
		float y = currentLevel->offsetY + (cart.currentRow * currentLevel->cellHeight);
		cartSprite.setPosition(x, y);
		window.draw(cartSprite);
	}
}

void SetGamePaused(bool paused) {
	if (gamePaused == paused) return;
	if (currentLevel == nullptr) return;
	if (paused && isGameOver) return;

	if (!currentLevel->arePlantsSelected || (currentLevel->clock.getElapsedTime().asSeconds() - currentLevel->plantsSelectedTime.asSeconds() < 4.0f)) {
		return;
	}

	gamePaused = paused;

	if (gamePaused) {
		gamePauseClock.restart();

		if (g_music && g_music->getStatus() == sf::Music::Playing) {
			g_music->pause();
		}

		if (!pauseMusic) {
			pauseMusic = new sf::Music();

			if (pause_music(*pauseMusic) != 0) {
				delete pauseMusic;
				pauseMusic = nullptr;
			}
		}

	} else {
		if (pauseMusic) {
			pauseMusic->stop();
			delete pauseMusic;
			pauseMusic = nullptr;
		}

		if (g_music && g_music->getStatus() != sf::Music::Playing) {
			g_music->play();
		}

		totalPauseTime += gamePauseClock.getElapsedTime();
		wcOffset -= gamePauseClock.getElapsedTime();
		zombieClockOffset -= gamePauseClock.getElapsedTime().asSeconds();
	}
}

void DrawLevelDetails(RenderWindow& window) {
	Sprite button;
	Texture buttonTex = getTexture(TEXTURE_TYPE::WOOD_BUTTON);
	Vector2f buttonTexSize = Vector2f(buttonTex.getSize());

	button.setTexture(buttonTex);
	button.setScale(0.16875f, 0.16875f);

	button.setOrigin(buttonTexSize / 2.0f);

	Text text;

	text.setFont(*font);
	text.setFillColor(Color::White);
	text.setOutlineColor(Color::Black);
	text.setOutlineThickness(4);
	text.setCharacterSize(36);

	button.setColor(Color::White);
	text.setFillColor(Color::White);
	button.setPosition(Vector2f(windowWidth, 0) + Vector2f(-85, 50));
	text.setString("Level " + std::to_string(currentLevel->lvlNum));
	text.setFillColor(Color::White);
	text.setOrigin(text.getLocalBounds().width / 2.0f, text.getLocalBounds().height / 2.0f);
	text.setPosition(button.getPosition());

	window.draw(button);
	window.draw(text);

	button.setColor(Color::White);
	text.setFillColor(Color::White);
	button.setPosition(Vector2f(windowWidth, 0) + Vector2f(-85, 125));
	text.setString(std::string("Pause"));
	text.setFillColor(Color::White);
	text.setPosition(button.getPosition());

	bool pausingEnabled = !(!currentLevel->arePlantsSelected || (currentLevel->clock.getElapsedTime().asSeconds() - currentLevel->plantsSelectedTime.asSeconds() < 4.0f));

	if (!pausingEnabled) {
		text.setString("Main Menu");
	}

	if (button.getGlobalBounds().contains(Vector2f(mousePos))) {
		button.setColor(Color(128, 128, 128, 255));
		text.setFillColor(text.getFillColor() * Color(128, 128, 128, 255));

		if (mouseJustClicked[0]) {
			if (pausingEnabled) {
				SetGamePaused(true);
			} else {
				requestedLevelMusic = REQUEST_SELECT_MUSIC;

				resetLevel();
				delete currentLevel;
				currentLevel = nullptr;
				selectingLevel = false;
				startMainMenuTime = gameClock.getElapsedTime();
			}
		}
	}

	text.setOrigin(text.getLocalBounds().width / 2.0f, text.getLocalBounds().height / 2.0f);

	window.draw(button);
	window.draw(text);
}

void DrawPauseMenu(RenderWindow& window) {
	float progress = (gamePauseClock.getElapsedTime().asSeconds()) / 0.25f;
	if (progress < 0) progress = 0;
	if (progress > 1) progress = 1;
	screenFade(window, 0.5f * progress);

	Vector2f animationOffset;
	animationOffset.x += 2000 - (2000 * progress);

	{
		Text title;

		title.setFont(*font);
		title.setFillColor(Color::White);
		title.setOutlineColor(Color::Black);
		title.setOutlineThickness(4);
		title.setCharacterSize(86);

		title.setString("Game Paused");

		title.setOrigin(title.getLocalBounds().width / 2.0f, title.getLocalBounds().height / 2.0f);
		title.setPosition(Vector2f(windowWidth / 2.0f, 250.0f) + animationOffset);

		window.draw(title);
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
	button.setPosition(Vector2f(windowWidth, windowHeight) / 2.0f + Vector2f(0, -50) - animationOffset);
	text.setString(std::string("Resume"));
	text.setOrigin(text.getLocalBounds().width / 2.0f, text.getLocalBounds().height / 2.0f);
	text.setPosition(button.getPosition());

	if (button.getGlobalBounds().contains(Vector2f(mousePos))) {
		button.setColor(Color(128, 128, 128, 255));
		text.setFillColor(text.getFillColor() * Color(128, 128, 128, 255));

		if (mouseJustClicked[0]) {
			SetGamePaused(false);
		}
	}

	window.draw(button);
	window.draw(text);

	button.setColor(Color::White);
	text.setFillColor(Color::Yellow);
	button.setPosition(Vector2f(windowWidth, windowHeight) / 2.0f + Vector2f(0, 50) + animationOffset);
	text.setString(std::string("Restart"));
	text.setOrigin(text.getLocalBounds().width / 2.0f, text.getLocalBounds().height / 2.0f);
	text.setPosition(button.getPosition());

	if (button.getGlobalBounds().contains(Vector2f(mousePos))) {
		button.setColor(Color(128, 128, 128, 255));
		text.setFillColor(text.getFillColor() * Color(128, 128, 128, 255));

		if (mouseJustClicked[0]) {
			requestedLevelMusic = REQUEST_SELECT_MUSIC;

			resetLevel();
			int lvlNum = currentLevel->lvlNum;
			delete currentLevel;
			startLevel(lvlNum);
		}
	}

	window.draw(button);
	window.draw(text);

	button.setColor(Color::White);
	text.setFillColor(Color::Red);
	button.setPosition(Vector2f(windowWidth, windowHeight) / 2.0f + Vector2f(0, 150) - animationOffset);
	text.setString(std::string("Main Menu"));
	text.setOrigin(text.getLocalBounds().width / 2.0f, text.getLocalBounds().height / 2.0f);
	text.setPosition(button.getPosition());

	if (button.getGlobalBounds().contains(Vector2f(mousePos))) {
		button.setColor(Color(128, 128, 128, 255));
		text.setFillColor(text.getFillColor() * Color(128, 128, 128, 255));

		if (mouseJustClicked[0]) {
			requestedLevelMusic = REQUEST_SELECT_MUSIC;

			resetLevel();
			delete currentLevel;
			currentLevel = nullptr;
			selectingLevel = false;
			startMainMenuTime = gameClock.getElapsedTime();
		}
	}

	window.draw(button);
	window.draw(text);
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

	//ground.setTexture(textures[TEXTURE_TYPE::LEVEL_BACKGROUND][level]);
	ground.setTexture(textures[TEXTURE_TYPE::LEVEL_BACKGROUND][int(levels[level].theme)]);

	window.draw(ground);

	if (currentLevel->arePlantsSelected && (currentLevel->clock.getElapsedTime().asSeconds() - currentLevel->plantsSelectedTime.asSeconds()) >= 0.95f) {
		UpdateLawnmowers(window);
		DrawRailsAndCarts(window);
	}

	//fake zombies
	if (!currentLevel->arePlantsSelected || currentLevel->clock.getElapsedTime().asSeconds() - currentLevel->plantsSelectedTime.asSeconds() < 3.75f) {
		float previewDiff = currentLevel->clock.getElapsedTime().asSeconds();

		Vector2f pos;

		int actualTexWidth = ground.getTexture()->getSize().x;
		int actualTexHeight = ground.getTexture()->getSize().y;
		//zoom_scale
		float scale = (float)windowHeight / actualTexHeight;
		ground.setScale(scale, scale);
		int visibleWidth = windowWidth / scale;

		int maxPan = std::max(0, actualTexWidth - visibleWidth);
		float panRatio = (maxPan == 0) ? 0 : (ground.getTextureRect().left / (float)maxPan);
		pos.x = windowWidth - panRatio * 685;

		pos.x = windowWidth - (ground.getTextureRect().left / (float)maxPan) * 685;

		bool dragonShown = false;

		if (currentLevel->zombie_types.size() > 0) {

			float offsetX[5] = { 0, 90, 40, 130, 70 };

			for (int raw = 0; raw < 4; raw++) {
				for (int col = 0; col < 3; col++) {

					int zomIndex = (raw + col) % currentLevel->zombie_types.size();
					Entity dummyZombie = currentLevel->zombie_types[zomIndex];
					if (dummyZombie.type == ENTITY_TYPE::ZOMBOT_DRAGON) {
						if (dragonShown) continue;
						dragonShown = true;
					}

					int previewAnimIdx = 0;
					if (dummyZombie.type == ENTITY_TYPE::ZOMBIE_POGO) {
						previewAnimIdx = 3;
					}

					Texture& zomTex = textures[getTextureType(dummyZombie.type)][previewAnimIdx];
					int frames = dummyZombie.animations[previewAnimIdx].frames;


					if (frames > 0) {
						int frameWidth = zomTex.getSize().x / frames;

						int frame = (int)(previewDiff * 15 + raw * 3 + col * 7) % frames;

						zombieSprite.setTexture(zomTex);
						zombieSprite.setTextureRect(IntRect(frame * frameWidth, 0, frameWidth, zomTex.getSize().y));
						zombieSprite.setScale(Vector2f(dummyZombie.scale, dummyZombie.scale));
						zombieSprite.setColor(Color::White);

						float actualHeight = zomTex.getSize().y * dummyZombie.scale;
						float zomY = currentLevel->offsetY + (raw * currentLevel->cellHeight) - (actualHeight - currentLevel->cellHeight);
						float zomX = offsetX[raw] + (col * 130);

						if (dummyZombie.type == ENTITY_TYPE::ZOMBOT_DRAGON) {
							zomY = -250;
							zomX = 210;
						}

						zombieSprite.setPosition(pos + Vector2f(zomX, zomY));
						window.draw(zombieSprite);
					}
				}
			}
		}
	}

	if (!currentLevel->arePlantsSelected || currentLevel->clock.getElapsedTime().asSeconds() - currentLevel->plantsSelectedTime.asSeconds() < 3.75f) {
		float diff = currentLevel->clock.getElapsedTime().asSeconds() - currentLevel->plantsSelectedTime.asSeconds();

		float progress = (diff - 0.75f) / 5.0f + 0.15f;

		if (!currentLevel->arePlantsSelected) progress = 0;

		//ground.setTexture(textures[TEXTURE_TYPE::LEVEL_BACKGROUND_DIRT][level]);
		ground.setTexture(textures[TEXTURE_TYPE::LEVEL_BACKGROUND_DIRT][int(levels[level].theme)]);

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
			int currentThemeNum = int(currentLevel->theme);

			if (currentThemeNum != 4 &&
				currentThemeNum != 5 &&
				currentThemeNum != 6 &&
				currentThemeNum != 7 &&
				currentThemeNum != 8) {

				float realProgress = (progress - 0.15f) / 0.57f;

				Sprite roll;
				Texture& rollTex = textures[TEXTURE_TYPE::GRID_ROLL][0];
				roll.setTexture(rollTex);

				int frameCount = 10;

				int frameLength = rollTex.getSize().x / frameCount;
				roll.setTextureRect(IntRect(int(realProgress * frameCount) * frameLength, 0, frameLength, rollTex.getSize().y));

				roll.setOrigin(frameLength / 2, 0);
				roll.setPosition(Vector2f(dirtLeft, currentLevel->offsetY - 75));

				float scale = 1.0f;
				scale = (currentLevel->cellHeight * gridRows + 155) / rollTex.getSize().y;
				roll.setScale(Vector2f(scale, scale));
				window.draw(roll);
			}
		}

		SetGamePaused(false);
	}

	if (isGameOver) {
		//ground.setTexture(textures[TEXTURE_TYPE::LEVEL_BACKGROUND_OPENDOOR][level]);
		ground.setTexture(textures[TEXTURE_TYPE::LEVEL_BACKGROUND_OPENDOOR][int(currentLevel->theme)]);
		window.draw(ground);
	}

	DrawZombies(window);
	DrawPlantsGrid(window);

	DrawPopupText(window);

	if (isGameOver) {
		//ground.setTexture(textures[TEXTURE_TYPE::LEVEL_BACKGROUND_HOUSE][level]);
		ground.setTexture(textures[TEXTURE_TYPE::LEVEL_BACKGROUND_HOUSE][int(currentLevel->theme)]);
		window.draw(ground);

		gameOverAnimation(window);
	}
	if (currentLevel == nullptr) return;

	if (!gamePaused) DrawLevelDetails(window);

	if (isGameOver == true) {
		window.draw(gameOverScreen);
		return;
	}
	if (isGameWon == true) {
		// rewards (debug)
		if (!isGameWonCoolDownStarted) {
			currentLevel->winClock.restart();
			isGameWonCoolDownStarted = true;
		} else if (currentLevel->winClock.getElapsedTime().asSeconds() >= wincooldowntimer) {
			// run the one-time action when the win screen appears
			if (!gameWonActionExecuted) {
				ExecuteGameWonAction();
				gameWonActionExecuted = true;
			}

			DrawWinScreen(window);
			return;
		}
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
	if (zombieClock.getElapsedTime().asSeconds() >= currentLevel->clock.getElapsedTime().asSeconds()) {
		zombieClock.restart(); // DEBUG
		zombieClockOffset = 0;
	}
	if (!gamePaused) {
		CheckTimer_AddZombie(window);
		CheckTimer_AddSun(window);
		CheckTimer_AddBooster(false);
	}

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

	if (!gamePaused) {
		DrawSeeds(window);
		//DrawDebugGrid(window);

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
	} else {
		DrawPauseMenu(window);
	}
	if (currentLevel == nullptr) return;

	if (deadZombieCount >= currentLevel->targetZombie) {
		isGameWon = true;
		// request switching back to normal (select-levels) music
		requestedLevelMusic = REQUEST_SELECT_MUSIC;
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
				// request switching back to normal (select-levels) music
				requestedLevelMusic = REQUEST_SELECT_MUSIC;

				// Play lose music once when game over screen appears.
				if (!loseMusicPlayed) {
					// heap-allocated to keep playing (matches project pattern)
					sf::Music* loseMusic = new sf::Music();
					if (loseMusic->openFromFile("assets/audio/05. losemusic.ogg")) {
						loseMusic->setLoop(false);
						loseMusic->play();
						loseMusicPlayed = true;
					} else {
						delete loseMusic;
						std::cout << "Failed to load assets/audio/05. losemusic.ogg\n";
					}
				}

				break;
			}
		}
	}
}

void DrawLevelSelector(RenderWindow& window) {
	Vector2f windowSize = { windowWidth, windowHeight };

	Sprite sprite;

	static int selectedLevel = 0;
	static int previousSelectedLevel = -1;
	static Clock selectionChanged = gameClock;

	bool levelClicked = false;

	const float animationDuration = 0.5f;

	float animationProgress = std::min(selectionChanged.getElapsedTime().asSeconds(), animationDuration) / animationDuration;
	// ease in out
	animationProgress = -(cos(animationProgress * 3.14159265358979323846f) - 1) / 2;

	Texture bgTex = getTexture(TEXTURE_TYPE::LEVEL_SELECTOR_BACKGROUND);
	sprite.setTexture(bgTex);
	sprite.setPosition(Vector2f(0, 0));
	sprite.setScale(Vector2f(
		windowWidth / (float)bgTex.getSize().x,
		windowHeight / (float)bgTex.getSize().y
	));

	sprite.setColor(Color::White);
	sprite.setTexture(textures[TEXTURE_TYPE::LEVEL_SELECTOR_BACKGROUND][int(levels[selectedLevel + 1].theme)]);
	window.draw(sprite);

	if (previousSelectedLevel != -1 && animationProgress < 1) {
		sprite.setTexture(textures[TEXTURE_TYPE::LEVEL_SELECTOR_BACKGROUND][int(levels[previousSelectedLevel + 1].theme)]);
		sprite.setColor(Color(255, 255, 255, 255 * (1 - animationProgress)));
		window.draw(sprite);
	}

	FloatRect backRc;
	backRc.left = 714;
	backRc.top = 934;
	backRc.width = 370;
	backRc.height = 100;

	if (backRc.contains(Vector2f(mousePos))) {
		sprite.setTexture(textures[TEXTURE_TYPE::LEVEL_SELECTOR_BACKGROUND_BACK][int(levels[selectedLevel + 1].theme)]);
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

	Texture levelTex = getTexture(TEXTURE_TYPE::LEVEL_OPTION_BACKGROUND);
	Vector2f levelTexSize = Vector2f(levelTex.getSize());
	sprite.setTexture(levelTex);
	sprite.setOrigin(levelTexSize / 2.0f);

	float offset = selectedLevel * animationProgress + std::max(previousSelectedLevel, 0) * (1 - animationProgress);

	Vector2f initialPos = Vector2f(windowSize / 2.0f) - Vector2f(0, 75.0f);

	//For TESTING
	if (DEV) {
		for (int i = 1; i <= number_levels; i++) {
			if (levels[i].theme == THEME::NONE) {
				levels[i].isUnlocked = false; continue;
			}
			//levels[i].isUnlocked = i <= 12;
			levels[i].isUnlocked = true;
		}
	}


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
		} else if (selected) {
			scale = Vector2f(1.0f, 1.0f) * animationProgress + Vector2f(0.75f, 0.75f) * (1 - animationProgress);
		} else {
			scale = Vector2f(0.75f, 0.75f);
		}

		if (!selected && !hovered) {
			sprite.setColor(Color(128, 128, 128, 255));
		} else {
			sprite.setColor(Color::White);
		}

		Texture& iconTex = textures[TEXTURE_TYPE::LEVEL_ICON][int(levels[i + 1].theme)];
		//Texture& iconTex = textures[TEXTURE_TYPE::LEVEL_ICON][i + 1];
		sprite.setTexture(iconTex);
		sprite.setTextureRect({ 0, 0, (int)iconTex.getSize().x, (int)iconTex.getSize().y });
		float fitScaleX = 370.0f / iconTex.getSize().x;
		float fitScaleY = 320.0f / iconTex.getSize().y;
		sprite.setScale(Vector2f(fitScaleX * (scale.x / 1.5f), fitScaleY * (scale.y / 1.5f)));
		sprite.setPosition(pos + Vector2f(0, -32));
		window.draw(sprite);

		sprite.setTextureRect({ 0, 0, (int)levelTexSize.x, (int)levelTexSize.y });
		sprite.setTexture(levelTex);

		sprite.setTexture(textures[TEXTURE_TYPE::LEVEL_OPTION_BACKGROUND][int(levels[i + 1].theme)]);

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
		Sprite ribbonSprite;
		int levelNumberForMath = i + 1;

		if (levelNumberForMath % 3 == 1) {
			ribbonSprite.setTexture(textures[TEXTURE_TYPE::LEVEL_RIBBON_EASY][Default_Texture_Index]);
		} else if (levelNumberForMath % 3 == 2) {
			ribbonSprite.setTexture(textures[TEXTURE_TYPE::LEVEL_RIBBON_MEDIUM][Default_Texture_Index]);
		} else if (levelNumberForMath % 3 == 0) {
			ribbonSprite.setTexture(textures[TEXTURE_TYPE::LEVEL_RIBBON_HARD][Default_Texture_Index]);
		}

		float ribbonScale = 0.2f * scale.x;
		ribbonSprite.setScale(ribbonScale, ribbonScale);
		float ribbonWidth = ribbonSprite.getGlobalBounds().width;
		ribbonSprite.setPosition(pos.x - (ribbonWidth / 2.0f), pos.y - 250.0f * scale.y);

		if (!selected && !hovered) {
			ribbonSprite.setColor(Color(128, 128, 128, 255));
		} else {
			ribbonSprite.setColor(Color::White);
		}

		window.draw(ribbonSprite);

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

	text.setFont(*loudFont);
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
	text.setFont(*font);
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
		Sprite coin;
		coin.setTexture(getTexture(TEXTURE_TYPE::COIN));
		coin.setOrigin(Vector2f(getTexture(TEXTURE_TYPE::COIN).getSize()) / 2.0f);
		coin.setPosition(503, 320);

		coin.setScale(0.4f, 0.4f);

		window.draw(coin);

		Text text;

		text.setFont(*font);
		text.setFillColor(Color::White);
		text.setOutlineColor(Color::Black);
		text.setOutlineThickness(4);
		text.setCharacterSize(36);
		text.setString(std::to_string(coinsMoney) + " Coins");

		text.setPosition(Vector2f(590, 300));

		window.draw(text);
	}

	int i = 0;
	for (auto& item : shopItems) {
		//if (!item.isUnlocked) continue;

		Texture tex = getTexture(item.textureType);

		Sprite sprite;
		sprite.setTexture(tex);

		sprite.setScale(
			128.0f / tex.getSize().x,
			128.0f / tex.getSize().y
		);

		sprite.setPosition(Vector2f(600, 425) + float(i) * Vector2f(0, 128 + 32));

		window.draw(sprite);

		Text text;

		text.setFont(*font);
		text.setFillColor(Color::White);
		text.setOutlineColor(Color::Black);
		text.setOutlineThickness(4);
		text.setCharacterSize(48);
		text.setString(item.title);

		text.setPosition(sprite.getPosition() + Vector2f(128 + 32, 16));

		window.draw(text);

		text.setPosition(sprite.getPosition() + Vector2f(128 + 32, 80));
		text.setCharacterSize(32);
		text.setFillColor(Color::Yellow);
		text.setString(std::to_string(item.cost) + " Coins");
		window.draw(text);

		text.setPosition(sprite.getPosition() + Vector2f(128 + 196, 80));
		text.setString("Owned: " + std::to_string(*item.counter));
		text.setFillColor(Color::White);
		if (*item.counter == 0) {
			text.setFillColor(Color::Red);
		}
		window.draw(text);

		Sprite buy;

		buy.setTexture(getTexture(TEXTURE_TYPE::GREEN_BUTTON));
		buy.setPosition(sprite.getPosition() + Vector2f(512, 24));

		text.setString("Buy");
		text.setFillColor(Color::White);
		text.setPosition(buy.getPosition() + Vector2f(84, 16));
		text.setCharacterSize(48);

		if (item.isUnlocked || *item.counter >= item.maxCount) {
			text.setString("Owned");
			buy.setColor(Color(96, 96, 96, 255));
			text.setFillColor(Color(96, 96, 96, 255));
			text.setPosition(buy.getPosition() + Vector2f(56, 16));
			if (!item.isUnlocked) {
				text.setString("Max");
				text.setPosition(buy.getPosition() + Vector2f(84, 16));
			}
		} else if (item.cost > coinsMoney) {
			buy.setColor(Color(96, 96, 96, 255));
			text.setFillColor(Color(96, 96, 96, 255));
		} else if (buy.getGlobalBounds().contains(Vector2f(mousePos))) {
			buy.setColor(Color(128, 128, 128, 255));
			text.setFillColor(Color(128, 128, 128, 255));

			if (mouseJustClicked[0]) {
				item.buy(1);
			}
		}

		window.draw(buy);
		window.draw(text);

		i++;
	}

	/*
	for (auto& item : shopItems) {
		if(!item.isUnlocked) continue;
		sprite.setTexture(getTexture(item.textureType));
		item.cost;
	}
	// Buying
	shopItems[1].buy(5);
	booster.buy(2);
	*/

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
	text.setString(std::string("Shop"));
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
		msg = "Booster Count:";
		if (line.find(msg) == 0) {
			curBoosterCnt = stoi(line.substr(msg.size()));
		}
		msg = "Magic Shovel Count:";
		if (line.find(msg) == 0) {
			curMagicShovelCnt = stoi(line.substr(msg.size()));
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
	file << "\n";
	file << "Booster Count:" << curBoosterCnt << "\n";
	file << "Magic Shovel Count:" << curMagicShovelCnt << "\n";
	file << "\n";
	file.close();
}


// Music
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

int level_7_music(Music& music) {
	if (!music.openFromFile("assets/audio/level_7.ogg")) {
		std::cout << "Error loading file!" << std::endl;
		return -1;
	}
	music.setLoop(true);
	music.play();
	return 0;
}

int level_8_music(Music& music) {
	if (!music.openFromFile("assets/audio/level_8.ogg")) {
		std::cout << "Error loading file!" << std::endl;
		return -1;
	}
	music.setLoop(true);
	music.play();
	return 0;
}

int level_9_music(Music& music) {
	if (!music.openFromFile("assets/audio/level_9.ogg")) {
		std::cout << "Error loading file!" << std::endl;
		return -1;
	}
	music.setLoop(true);
	music.play();
	return 0;
}

int level_10_music(Music& music) {
	if (!music.openFromFile("assets/audio/level_10.ogg")) {
		std::cout << "Error loading file!" << std::endl;
		return -1;
	}
	music.setLoop(true);
	music.play();
	return 0;
}

int level_11_music(Music& music) {
	if (!music.openFromFile("assets/audio/level_11.ogg")) {
		std::cout << "Error loading file!" << std::endl;
		return -1;
	}
	music.setLoop(true);
	music.play();
	return 0;
}

int level_12_music(Music& music) {
	if (!music.openFromFile("assets/audio/level_12.ogg")) {
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
void playMusicForLevel(int level, Music& music) {
	switch (level) {
	case 1: level_1_music(music); break;
	case 2: level_2_music(music); break;
	case 3: level_3_music(music); break;
	case 4: level_4_music(music); break;
	case 5: level_5_music(music); break;
	case 6: level_6_music(music); break;
	case 7: level_7_music(music); break;
	case 8: level_8_music(music); break;
	case 9: level_9_music(music); break;
	case 10: level_10_music(music); break;
	case 11: level_11_music(music); break;
	case 12: level_12_music(music); break;
	default: /* fallback */ select_levels_background_music(music); break;
	}
}
void defineDayLevels() {
	//vector<Entity> mainDayPlantTypes = {normalPlant, repeaterPlant, wallnut, POTATOMINE, sunflower, snapdragon};
	vector<Entity> mainDayPlantTypes = allPlantTypes;
	for (int lvl = 1; lvl <= 3; lvl++) {
		levels[lvl].theme = THEME::DAY;
		levels[lvl].lvlNum = lvl;
		levels[lvl].plant_types = mainDayPlantTypes;
		levels[lvl].sunSpawnInterval = 8.0f;
		levels[lvl].targetZombie = 20;
		levels[lvl].waveLimit = 10; levels[1].waveTimer = 15;
		levels[lvl].startingSuns = 50;
		levels[lvl].offsetX = 450;
		levels[lvl].offsetY = 135;
		levels[lvl].cellWidth = 150;
		levels[lvl].cellHeight = 170;
		setupLevelLawnmowers(levels[lvl]);
	}
	// Level 1 Easy
	levels[1].isUnlocked = true;
	levels[1].plantReward = repeaterPlant;      levels[1].coinReward = 100;
	levels[1].addWave(2, 5, { normalZombie }, 10);
	levels[1].addWave(3, 5, { normalZombie }, 10);
	levels[1].addWave(4, 15, { normalZombie, zombieConehead }, 10);
	levels[1].addWave(11, 20, { normalZombie, zombieConehead }, 0, true);

	// Level 2 Med
	levels[2].plantReward = CHEERY_BOMB;      levels[2].coinReward = 200;
	levels[2].addWave(2, 10, { normalZombie }, 10);
	levels[2].addWave(4, 5, { normalZombie, zombieConehead }, 10);
	levels[2].addWave(6, 15, { normalZombie, normalZombie, zombieConehead }, 8);
	levels[2].addWave(10, 15, { normalZombie, normalZombie, normalZombie, zombieConehead }, 0, true);
	levels[2].addWave(12, 20, { normalZombie, zombieConehead }, 0, true);

	// Level 3 Hard
	levels[3].plantReward = sunshroom;      levels[3].coinReward = 600;
	levels[3].sunSpawnInterval = 3.f;
	levels[3].startingSuns = 1000;
	levels[3].addWave(3, 12, { normalZombie }, 5);
	//levels[3].addWave(4, 8, { normalZombie, zombieBig }, 5);
	//levels[3].addWave(5, 15, { normalZombie, zombieConehead }, 8);
	levels[3].addWave(1, 5, { zombotDragon }, 10);
	//levels[3].addWave(16, 20, { normalZombie, zombieConehead, zombieBig }, 0, true); // moved to level 6
}

void defineNightLevels() {
	//vector<Entity> mainNightPlantTypes = { normalPlant, repeaterPlant, wallnut, POTATOMINE, sunflower };
	vector<Entity> mainNightPlantTypes = allPlantTypes;

	for (int lvl = 4; lvl <= 6; lvl++) {
		levels[lvl].theme = THEME::NIGHT;
		levels[lvl].lvlNum = lvl;
		levels[lvl].plant_types = mainNightPlantTypes;
		levels[lvl].sunSpawnInterval = 99999.0f;
		levels[lvl].targetZombie = 25;
		levels[lvl].startingSuns = 100;
		levels[lvl].offsetX = 450;
		levels[lvl].offsetY = 135;
		levels[lvl].cellWidth = 150;
		levels[lvl].cellHeight = 170;
		setupLevelLawnmowers(levels[lvl]);
	}

	// Level 4
	levels[4].plantReward = shroom;       levels[4].coinReward = 200;
	levels[4].addWave(2, 10, { normalZombie }, 5);
	levels[4].addWave(3, 12, { normalZombie, zombieNewspaper }, 8);
	levels[4].addWave(5, 15, { normalZombie, zombieConehead, zombieNewspaper }, 10);
	levels[4].addWave(15, 20, { normalZombie, zombieConehead, zombieNewspaper }, 0, true);

	// Level 5
	levels[5].plantReward = iceShroom;    levels[5].coinReward = 300;
	levels[5].addWave(2, 10, { normalZombie }, 5);
	levels[5].addWave(5, 15, { normalZombie, zombieNewspaper }, 8);
	levels[5].addWave(6, 15, { zombieConehead, zombieNewspaper }, 10);
	levels[5].addWave(2, 10, { zombieAllStar }, 5);
	levels[5].addWave(15, 20, { normalZombie, zombieConehead, zombieNewspaper, zombieAllStar }, 0, true);

	// Level 6
	levels[6].plantReward = doomShroom;   levels[6].coinReward = 400;
	levels[6].startingSuns = 1500;
	levels[6].addWave(5, 15, { normalZombie, zombieBig }, 0);
	levels[6].addWave(1, 5, { zombotDragon }, 10);
	levels[6].addWave(16, 20, { normalZombie, zombieConehead, zombieBig }, 0, true);
	levels[6].addWave(3, 12, { normalZombie, zombieNewspaper }, 5);
	levels[6].addWave(5, 15, { zombieConehead, zombieNewspaper }, 8);
	/*levels[6].addWave(6, 15, {zombieNewspaper, zombieAllStar}, 10);
	levels[6].addWave(16, 20, { normalZombie, zombieConehead, zombieNewspaper, zombieAllStar }, 0, true);*/
}

void defineAncientLevels() {
	//vector<Entity> mainAncientPlantTypes = { normalPlant, repeaterPlant, wallnut, POTATOMINE, sunflower };
	vector<Entity> mainAncientPlantTypes = allPlantTypes;

	for (int lvl = 7; lvl <= 9; lvl++) {
		levels[lvl].theme = THEME::ANCIENT;
		levels[lvl].lvlNum = lvl;
		levels[lvl].plant_types = mainAncientPlantTypes;
		levels[lvl].sunSpawnInterval = 7.0f;
		levels[lvl].startingSuns = 50;
		levels[lvl].targetZombie = 30;
		levels[lvl].offsetX = 450;
		levels[lvl].offsetY = 135;
		levels[lvl].cellWidth = 150;
		levels[lvl].cellHeight = 170;
		setupLevelLawnmowers(levels[lvl]);
	}

	// Levels 7
	levels[7].plantReward = icePeashooter;     levels[7].coinReward = 300;
	levels[7].addWave(1, 5, { zombieMummy }, 10);
	levels[7].addWave(3, 15, { zombieMummy, zombieRa }, 10);
	levels[7].addWave(6, 15, { zombieMummy, zombieRa }, 10);
	levels[7].addWave(15, 20, { zombieMummy, zombieRa, zombieTorch }, 0, true);

	// Level 8
	levels[8].plantReward = icePeashooter;     levels[8].coinReward = 400;
	levels[8].addWave(3, 12, { zombieMummy }, 5);
	levels[8].addWave(5, 15, { zombieMummy, zombieRa }, 8);
	levels[8].addWave(5, 15, { zombieTorch, zombiePharaoh }, 10);
	levels[8].addWave(18, 20, { zombieMummy, zombieRa, zombieTorch, zombiePharaoh }, 0, true);

	// Level 9
	levels[9].plantReward = icePeashooter;     levels[9].coinReward = 500;
	levels[9].addWave(4, 15, { zombieMummy, zombieRa }, 5);
	levels[9].addWave(6, 15, { zombieTorch, zombiePharaoh }, 10);
	levels[9].addWave(15, 20, { zombieMummy, zombieRa, zombieTorch, zombiePharaoh }, 15, true);
	levels[9].addWave(15, 20, { zombieMummy, zombieRa, zombieTorch, zombiePharaoh }, 0, true);
}

void defineWildWestLevels() {
	//vector<Entity> mainWildWestPlantTypes = { normalPlant, repeaterPlant, wallnut, POTATOMINE, sunflower };
	vector<Entity> mainWildWestPlantTypes = allPlantTypes;

	for (int lvl = 10; lvl <= 12; lvl++) {
		levels[lvl].theme = THEME::WILDWEST;
		levels[lvl].lvlNum = lvl;
		levels[lvl].plant_types = mainWildWestPlantTypes;
		levels[lvl].startingSuns = 50;
		levels[lvl].sunSpawnInterval = 6.0f;
		levels[lvl].targetZombie = 30;
		levels[lvl].offsetX = 450;
		levels[lvl].offsetY = 135;
		levels[lvl].cellWidth = 150;
		levels[lvl].cellHeight = 170;
		setupLevelLawnmowers(levels[lvl]);
	}

	// Level 10
	levels[10].plantReward = lightningReed;      levels[10].coinReward = 400;
	levels[10].addWave(1, 12, { zombieCowboy }, 5);
	levels[10].addWave(4, 15, { zombieCowboy, zombieRedhair }, 8);
	levels[10].addWave(5, 15, { zombieCowboy, zombieProspector }, 10);
	levels[10].addWave(10, 20, { zombieCowboy, zombieRedhair, zombieProspector }, 0, true);

	// Level 11
	levels[11].plantReward = snapdragon;       levels[11].coinReward = 500;
	levels[11].addWave(3, 12, { zombieCowboy }, 5);
	levels[11].addWave(4, 15, { zombieChickenWrangler }, 8);
	levels[11].addWave(6, 15, { zombieCowboy, zombieRedhair, zombieProspector }, 10);
	levels[11].addWave(5, 15, { zombiePianist, zombieCowboy }, 10);
	levels[11].addWave(15, 20, { zombieCowboy, zombieRedhair, zombieProspector, zombieChickenWrangler, zombiePianist }, 0, true);

	// Level 12
	levels[12].plantReward = redShooterPlant;       levels[12].coinReward = 600;
	levels[12].addWave(4, 15, { zombieCowboy, zombieRedhair }, 5);
	levels[12].addWave(6, 15, { zombieChickenWrangler, zombieProspector }, 10);
	levels[12].addWave(15, 20, { zombieCowboy, zombieRedhair, zombieProspector, zombieChickenWrangler, zombiePianist }, 15, true);
	levels[12].addWave(15, 20, { zombiePianist, zombieChickenWrangler, zombieCowboy, zombieProspector }, 0, true);
}

void defineLostCityLevels() {
	//vector<Entity> mainAncientPlantTypes = { normalPlant, repeaterPlant, wallnut, POTATOMINE, sunflower };
	vector<Entity> mainLostCityPlantTypes = allPlantTypes;

	for (int lvl = 13; lvl <= 15; lvl++) {
		levels[lvl].theme = THEME::LOST_CITY;
		levels[lvl].lvlNum = lvl;
		levels[lvl].plant_types = mainLostCityPlantTypes;
		levels[lvl].targetZombie = 30;
		levels[lvl].offsetX = 450;
		levels[lvl].offsetY = 135;
		levels[lvl].cellWidth = 150;
		levels[lvl].cellHeight = 170;
		setupLevelLawnmowers(levels[lvl]);
	}

	// Level 13
	levels[13].addWave(3, 12, { normalZombie }, 5);
	levels[13].addWave(6, 15, { normalZombie, zombieShieldGuard }, 10);
	levels[13].addWave(21, 20, { normalZombie, zombieBig, zombieShieldGuard }, 0, true);

	// Level 14
	levels[14].addWave(4, 15, { normalZombie, zombieShieldGuard }, 8);
	levels[14].addWave(6, 15, { zombieBig, zombieShieldGuard }, 10);
	levels[14].addWave(25, 20, { normalZombie, zombieBig, zombieShieldGuard }, 0, true);

	// Level 15
	levels[15].addWave(5, 15, { normalZombie, zombieShieldGuard }, 8);
	levels[15].addWave(6, 15, { zombieBig, zombieShieldGuard }, 10);
	levels[15].addWave(1, 5, { zombieLostCityGargantuar }, 10);
	levels[15].addWave(28, 25, { normalZombie, zombieBig, zombieShieldGuard, zombieLostCityGargantuar }, 0, true);
}

void defineDarkAgesLevels() {
	//vector<Entity> mainAncientPlantTypes = { normalPlant, repeaterPlant, wallnut, POTATOMINE, sunflower };
	vector<Entity> mainDarkAgesPlantTypes = allPlantTypes;

	for (int lvl = 16; lvl <= 18; lvl++) {
		levels[lvl].theme = THEME::DARK_AGES;
		levels[lvl].lvlNum = lvl;
		levels[lvl].plant_types = mainDarkAgesPlantTypes;
		levels[lvl].offsetX = 450;
		levels[lvl].offsetY = 135;
		levels[lvl].cellWidth = 150;
		levels[lvl].cellHeight = 170;
		setupLevelLawnmowers(levels[lvl]);
	}

	// Level 16
	levels[16].addWave(3, 12, { zombieJapanese }, 5);
	levels[16].addWave(6, 15, { zombieJapanese, zombieShieldGuard }, 10);
	levels[16].addWave(21, 20, { zombieJapanese, zombieShieldGuard, zombiePogo }, 0, true);

	// Level 17
	levels[17].addWave(4, 15, { zombieJapanese, zombieShieldGuard }, 8);
	levels[17].addWave(6, 15, { zombieTorch, zombiePogo }, 10);
	levels[17].addWave(25, 20, { zombieJapanese, zombieShieldGuard, zombieTorch, zombiePogo }, 0, true);

	// Level 18
	levels[18].addWave(5, 15, { zombieJapanese, zombieShieldGuard }, 8);
	levels[18].addWave(8, 15, { zombieTorch, zombiePogo }, 10);
	levels[18].addWave(30, 25, { zombieJapanese, zombieShieldGuard, zombieTorch, zombiePogo }, 0, true);
}

void defineJurassicAgeLevels() {
	//vector<Entity> mainAncientPlantTypes = { normalPlant, repeaterPlant, wallnut, POTATOMINE, sunflower };
	vector<Entity> mainJurassicPlantTypes = allPlantTypes;

	for (int lvl = 19; lvl <= 21; lvl++) {
		levels[lvl].theme = THEME::JURASSIC_AGE;
		levels[lvl].lvlNum = lvl;
		levels[lvl].plant_types = mainJurassicPlantTypes;
		levels[lvl].offsetX = 450;
		levels[lvl].offsetY = 135;
		levels[lvl].cellWidth = 150;
		levels[lvl].cellHeight = 170;
		setupLevelLawnmowers(levels[lvl]);
	}

	// Level 19
	levels[19].addWave(4, 15, { zombieBig }, 5);
	levels[19].addWave(6, 15, { zombieBig, zombieAllStar }, 10);
	levels[19].addWave(20, 20, { zombieBig, zombieAllStar, zombieChickenWrangler }, 0, true);

	// Level 20
	levels[20].addWave(5, 15, { zombieBig, zombieAllStar }, 8);
	levels[20].addWave(8, 15, { zombieChickenWrangler, zombieAllStar }, 10);
	levels[20].addWave(25, 20, { zombieBig, zombieAllStar, zombieChickenWrangler }, 0, true);

	// Level 21
	levels[21].addWave(6, 15, { zombieBig, zombieAllStar }, 8);
	levels[21].addWave(2, 10, { zombieLostCityGargantuar }, 10);
	levels[21].addWave(35, 25, { zombieBig, zombieAllStar, zombieChickenWrangler, zombieLostCityGargantuar }, 0, true);
}

void defineRomanLevels() {
	//vector<Entity> mainAncientPlantTypes = { normalPlant, repeaterPlant, wallnut, POTATOMINE, sunflower };
	vector<Entity> mainRomanPlantTypes = allPlantTypes;

	for (int lvl = 22; lvl <= 24; lvl++) {
		levels[lvl].theme = THEME::ROMAN;
		levels[lvl].lvlNum = lvl;
		levels[lvl].plant_types = mainRomanPlantTypes;
		levels[lvl].sunSpawnInterval = 6.5f;
		levels[lvl].startingSuns = 100;
		levels[lvl].offsetX = 450;
		levels[lvl].offsetY = 135;
		levels[lvl].cellWidth = 150;
		levels[lvl].cellHeight = 170;
		setupLevelLawnmowers(levels[lvl]);
	}

	//Level 22
	levels[22].plantReward = Pomegranate; levels[22].coinReward = 4000;
	levels[22].addWave(3, 12, { zombieRomanNormal }, 5);
	levels[22].addWave(5, 15, { zombieRomanNormal, zombieRomanConehead }, 10);
	levels[22].addWave(7, 15, { zombieRomanNormal, zombieRomanShield }, 10);
	levels[22].addWave(20, 20, { zombieRomanNormal, zombieRomanConehead, zombieRomanShield }, 0, true);

	//Level 23
	levels[23].plantReward = inferno;     levels[23].coinReward = 5000;
	levels[23].addWave(4, 15, { zombieRomanNormal }, 5);
	levels[23].addWave(6, 15, { zombieRomanConehead, zombieRomanShield }, 10);
	levels[23].addWave(8, 15, { zombieRomanBuckethead, zombieRomanOdessa }, 10);
	levels[23].addWave(25, 20, { zombieRomanNormal, zombieRomanBuckethead, zombieRomanShield, zombieRomanOdessa }, 0, true);

	//Level 24
	levels[24].plantReward.plantType = ENTITY_TYPE::NONE;
	levels[24].coinReward = 10000;
	levels[24].addWave(5, 15, { zombieRomanConehead }, 8);
	levels[24].addWave(8, 15, { zombieRomanBuckethead, zombieRomanShield }, 10);
	levels[24].addWave(10, 15, { zombieRomanOdessa, zombieRomanCenturion }, 15);
	levels[24].addWave(20, 20, { zombieRomanBuckethead, zombieRomanCenturion, zombieRomanBusthead, zombieRomanGargantuar }, 20, true);
	levels[24].addWave(25, 20, { zombieRomanBuckethead, zombieRomanCenturion, zombieRomanBusthead, zombieRomanGargantuar }, 0, true);
}

int main() {
	srand(time(0));

	screenFlashOverlay.setPosition(0, 0);
	screenFlashOverlay.setSize(Vector2f(windowWidth, windowHeight));

	Music music;	// can't be global - OpenAL must be loaded first
	g_music = &music; // let other code (SetGamePaused) pause/resume background music
	RenderWindow window(VideoMode(windowWidth, windowHeight), "Plants vs Zombies");
	//window.setFramerateLimit(120);
	bool fullscreen = false;
	Vector2i oldPos;
	Vector2u oldSize;

	window.setSize(Vector2u(1280, 720));

	VideoMode desktop = VideoMode::getDesktopMode();
	window.setPosition(Vector2i((desktop.width - window.getSize().x) / 2.0f, (desktop.height - window.getSize().y) / 2.0f));

	Event event;

	textures[TEXTURE_TYPE::LOGO][Default_Texture_Index].loadFromFile("assets/logo.png");
	font = new Font();
	font->loadFromFile("assets/impact.ttf");

	{
		window.clear();
		Sprite logo;
		logo.setTexture(textures[TEXTURE_TYPE::LOGO][Default_Texture_Index]);
		logo.setOrigin(logo.getGlobalBounds().getSize() / 2.0f);
		logo.setPosition(windowWidth / 2.0f, windowHeight / 2.0f - 128.0f);
		logo.setScale(0.5f, 0.5f);
		window.draw(logo);

		Text text;
		text.setFont(*font);
		text.setString("Loading Game Assets");
		text.setCharacterSize(48);
		text.setOutlineThickness(8);
		text.setOrigin(text.getGlobalBounds().getSize() / 2.0f);
		text.setPosition(windowWidth / 2.0f, windowHeight / 2.0f + 256.0f);
		window.draw(text);

		window.display();
	}

	// Characters
	Texture groundTex, lawnTex;
	ground = defineGroundSprite(groundTex);
	lawnmowerSprite = defineLawnmowerSprite(lawnTex);
	//New

	// Loading Textures + Defining entities

	// Plants

	normalPlant = defineNormalPlant();
	repeaterPlant = defineRepeaterPlant();
	wallnut = defineWallnut();
	POTATOMINE = definePOTATO_MINE();
	sunflower = defineSunflower();
	lightningReed = defineLightningReed();

	chomper = defineChomperPlant();
	shroom = defineShroomPlant();
	sunshroom = defineSunShroom();
	icePeashooter = defineIcePeaShooter();
	miniShroom = defineMiniShroom();
	doomShroom = defineDOOM_SHROOM();
	CHEERY_BOMB = defineCherryBomb();
	iceShroom = defineIceShroom();
	snapdragon = defineSnapdragon();
	lightningReed = defineLightningReed();
	inferno = defineInfernoPlant();
	redShooterPlant = defineRedShooter();
	Pomegranate = definePomegranatePlant();


	// Zombies
	normalZombie = defineNormalZombie();
	zombieJapanese = defineJapaneseZombie();
	zombieBig = defineBigZombie();
	zombieTorch = defineTorchlightZombie();
	zombieRa = defineRaZombie();
	zombieMummy = defineMummyZombie();
	zombieConehead = defineConeheadZombie();
	zombieNewspaper = defineNewspaperZombie();
	zombieAllStar = defineAllStarZombie();
	zombiePharaoh = definePharaohZombie();
	zombieShieldGuard = defineShieldGuardZombie();
	zombieCowboy = defineCowboyZombie();
	zombieRedhair = defineRedhairZombie();
	zombieProspector = defineProspectorZombie();
	zombiePogo = definePogoZombie();
	zombiePianist = definePianistZombie();
	zombieChickenWrangler = defineChickenWranglerZombie();
	zombieChicken = defineChickenZombie();
	zombotDragon = defineZombotDragon();
	zombieRomanConehead = defineRomanConehead();
	zombieRomanBuckethead = defineRomanBuckethead();
	zombieRomanCenturion = defineRomanCenturion();
	zombieRomanBusthead = defineRomanBusthead();
	zombieRomanOdessa = defineRomanOdessa();
	zombieRomanShield = defineRomanShield();
	zombieRomanNormal = defineRomanNormal();
	zombieRomanGargantuar = defineRomanGargantuar();
	zombieLostCityGargantuar = defineLostCityGargantuar();

	allPlantTypes = { normalPlant, repeaterPlant, wallnut, POTATOMINE, sunflower, chomper, shroom, sunshroom, icePeashooter, miniShroom, doomShroom, CHEERY_BOMB, iceShroom, snapdragon ,lightningReed ,inferno ,redShooterPlant, Pomegranate };

	// Other Textures

	textures[TEXTURE_TYPE::GREEN_BUTTON][Default_Texture_Index].loadFromFile("assets/mainmenu/green_button.png");
	textures[TEXTURE_TYPE::WOOD_BUTTON][Default_Texture_Index].loadFromFile("assets/mainmenu/wood_button.png");
	textures[TEXTURE_TYPE::MAINMENU_BACKGROUND][Default_Texture_Index].loadFromFile("assets/mainmenu/mainmenu_background.png");
	textures[TEXTURE_TYPE::STORE_WINDOW][Default_Texture_Index].loadFromFile("assets/mainmenu/store_window.png");

	textures[TEXTURE_TYPE::LEVEL_RIBBON_EASY][Default_Texture_Index].loadFromFile("assets/mainmenu/Easy.png");
	textures[TEXTURE_TYPE::LEVEL_RIBBON_MEDIUM][Default_Texture_Index].loadFromFile("assets/mainmenu/Medium.png");
	textures[TEXTURE_TYPE::LEVEL_RIBBON_HARD][Default_Texture_Index].loadFromFile("assets/mainmenu/Hard.png");

	textures[TEXTURE_TYPE::LEVEL_SELECTOR_BACKGROUND][Default_Texture_Index].loadFromFile("assets/mainmenu/level_selector_background.png");
	textures[TEXTURE_TYPE::LEVEL_SELECTOR_BACKGROUND][1].loadFromFile("assets/mainmenu/level_selector_background.png");
	textures[TEXTURE_TYPE::LEVEL_SELECTOR_BACKGROUND][2].loadFromFile("assets/mainmenu/level_selector_background.png");
	textures[TEXTURE_TYPE::LEVEL_SELECTOR_BACKGROUND][3].loadFromFile("assets/mainmenu/level_selector_background_desert.png");
	textures[TEXTURE_TYPE::LEVEL_SELECTOR_BACKGROUND][4].loadFromFile("assets/mainmenu/level_selector_background_desert.png");
	textures[TEXTURE_TYPE::LEVEL_SELECTOR_BACKGROUND][5].loadFromFile("assets/mainmenu/level_selector_background.png");
	textures[TEXTURE_TYPE::LEVEL_SELECTOR_BACKGROUND][6].loadFromFile("assets/mainmenu/level_selector_background.png");
	textures[TEXTURE_TYPE::LEVEL_SELECTOR_BACKGROUND][7].loadFromFile("assets/mainmenu/level_selector_background_desert.png");
	textures[TEXTURE_TYPE::LEVEL_SELECTOR_BACKGROUND][8].loadFromFile("assets/mainmenu/level_selector_background_desert.png");

	textures[TEXTURE_TYPE::LEVEL_SELECTOR_BACKGROUND_BACK][Default_Texture_Index].loadFromFile("assets/mainmenu/level_selector_background_back.png");
	textures[TEXTURE_TYPE::LEVEL_SELECTOR_BACKGROUND_BACK][1].loadFromFile("assets/mainmenu/level_selector_background_back.png");
	textures[TEXTURE_TYPE::LEVEL_SELECTOR_BACKGROUND_BACK][2].loadFromFile("assets/mainmenu/level_selector_background_back.png");
	textures[TEXTURE_TYPE::LEVEL_SELECTOR_BACKGROUND_BACK][3].loadFromFile("assets/mainmenu/level_selector_background_back_desert.png");
	textures[TEXTURE_TYPE::LEVEL_SELECTOR_BACKGROUND_BACK][4].loadFromFile("assets/mainmenu/level_selector_background_back_desert.png");
	textures[TEXTURE_TYPE::LEVEL_SELECTOR_BACKGROUND_BACK][5].loadFromFile("assets/mainmenu/level_selector_background_back.png");
	textures[TEXTURE_TYPE::LEVEL_SELECTOR_BACKGROUND_BACK][6].loadFromFile("assets/mainmenu/level_selector_background_back.png");
	textures[TEXTURE_TYPE::LEVEL_SELECTOR_BACKGROUND_BACK][7].loadFromFile("assets/mainmenu/level_selector_background_back_desert.png");
	textures[TEXTURE_TYPE::LEVEL_SELECTOR_BACKGROUND_BACK][8].loadFromFile("assets/mainmenu/level_selector_background_back_desert.png");

	textures[TEXTURE_TYPE::LEVEL_OPTION_BACKGROUND][Default_Texture_Index].loadFromFile("assets/mainmenu/level_option_background.png");
	textures[TEXTURE_TYPE::LEVEL_OPTION_BACKGROUND][1].loadFromFile("assets/mainmenu/level_option_background.png");
	textures[TEXTURE_TYPE::LEVEL_OPTION_BACKGROUND][2].loadFromFile("assets/mainmenu/level_option_background.png");
	textures[TEXTURE_TYPE::LEVEL_OPTION_BACKGROUND][3].loadFromFile("assets/mainmenu/level_option_background_desert.png");
	textures[TEXTURE_TYPE::LEVEL_OPTION_BACKGROUND][4].loadFromFile("assets/mainmenu/level_option_background_desert.png");
	textures[TEXTURE_TYPE::LEVEL_OPTION_BACKGROUND][5].loadFromFile("assets/mainmenu/level_option_background_green.png");
	textures[TEXTURE_TYPE::LEVEL_OPTION_BACKGROUND][6].loadFromFile("assets/mainmenu/level_option_background_green.png");
	textures[TEXTURE_TYPE::LEVEL_OPTION_BACKGROUND][7].loadFromFile("assets/mainmenu/level_option_background_desert.png");
	textures[TEXTURE_TYPE::LEVEL_OPTION_BACKGROUND][8].loadFromFile("assets/mainmenu/level_option_background_desert.png");

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
	textures[TEXTURE_TYPE::ITEM_JAR][Default_Texture_Index].loadFromFile("assets/item_jar.png");
	textures[TEXTURE_TYPE::ITEM_HEAD][Default_Texture_Index].loadFromFile("assets/item_head.png");
	textures[TEXTURE_TYPE::RAIL][Default_Texture_Index].loadFromFile("assets/rail.png");
	textures[TEXTURE_TYPE::CART][Default_Texture_Index].loadFromFile("assets/cart.png");
	textures[TEXTURE_TYPE::ZOMBIE_PROSPECTOR][0].loadFromFile("assets/prospector_walk.png");
	textures[TEXTURE_TYPE::ZOMBIE_PROSPECTOR][1].loadFromFile("assets/prospector_die.png");
	textures[TEXTURE_TYPE::ZOMBIE_PROSPECTOR][2].loadFromFile("assets/prospector_eat.png");
	textures[TEXTURE_TYPE::ZOMBIE_PROSPECTOR][3].loadFromFile("assets/ProspectorZombieDancing.png");
	textures[TEXTURE_TYPE::ZOMBIE_REDHAIR][0].loadFromFile("assets/redhair_walk.png");
	textures[TEXTURE_TYPE::ZOMBIE_REDHAIR][1].loadFromFile("assets/redhair_die.png");
	textures[TEXTURE_TYPE::ZOMBIE_REDHAIR][2].loadFromFile("assets/redhair_eat.png");
	textures[TEXTURE_TYPE::ZOMBIE_COWBOY][0].loadFromFile("assets/cowboy_walk.png");
	textures[TEXTURE_TYPE::ZOMBIE_COWBOY][1].loadFromFile("assets/cowboy_die.png");
	textures[TEXTURE_TYPE::ZOMBIE_COWBOY][2].loadFromFile("assets/cowboy_eat.png");
	textures[TEXTURE_TYPE::ZOMBIE_ELECTROCUTED][0].loadFromFile("assets/ZombieElectrocuted.png");

	// Font

	loudFont = new Font();
	loudFont->loadFromFile("assets/loud.ttf");

	// General Plants to any level


	vector<Entity> mainPlantTypes = { sunflower, repeaterPlant, normalPlant, wallnut, chomper, icePeashooter, miniShroom, POTATOMINE, doomShroom, CHEERY_BOMB, iceShroom, inferno };//, shroom, sunshroom };

	if (DEV) {
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
			isEntityUnlocked[snapdragon.type] =
			isEntityUnlocked[lightningReed.type] =
			isEntityUnlocked[inferno.type] =
			isEntityUnlocked[redShooterPlant.type] =
			isEntityUnlocked[Pomegranate.type] =
			true;
	} else {
		isEntityUnlocked[normalPlant.type] =
			isEntityUnlocked[sunflower.type] =
			isEntityUnlocked[POTATOMINE.type] =
			true;
	}

	// Levels
	defineDayLevels();
	defineNightLevels();
	defineAncientLevels();
	defineWildWestLevels();
	defineLostCityLevels();
	defineDarkAgesLevels();
	defineJurassicAgeLevels();
	defineRomanLevels();

			// setting levels background textures //bravooo
	textures[TEXTURE_TYPE::LEVEL_BACKGROUND][0].loadFromFile("assets/levels/level1.png");
	textures[TEXTURE_TYPE::LEVEL_BACKGROUND_HOUSE][0].loadFromFile("assets/levels/level1_house.png");
	//for (int lvl = 1; lvl <= number_levels; lvl++) {
	for (int theme = 1; theme < int(THEME::COUNT); theme++) {
		cout << "Loading level " << theme << " files" << endl;
		levels[theme].lvlNum = theme;

		textures[TEXTURE_TYPE::LEVEL_BACKGROUND][theme].loadFromFile("assets/levels/level" + to_string(theme) + ".png");
		textures[TEXTURE_TYPE::LEVEL_BACKGROUND_HOUSE][theme].loadFromFile("assets/levels/level" + to_string(theme) + "_house.png");
		textures[TEXTURE_TYPE::LEVEL_BACKGROUND_OPENDOOR][theme].loadFromFile("assets/levels/level" + to_string(theme) + "_opendoor.png");
		textures[TEXTURE_TYPE::LEVEL_BACKGROUND_DIRT][theme].loadFromFile("assets/levels/level" + to_string(theme) + "_dirt.png");
		textures[TEXTURE_TYPE::LEVEL_ICON][theme].loadFromFile("assets/levels/level" + to_string(theme) + "_icon.png");
		textures[TEXTURE_TYPE::LAWNMOWER][theme].loadFromFile("assets/levels/lawnmower" + to_string(theme) + ".png");

		cout << "Loaded level " << theme << " files" << endl;
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


	loadStorage();

	Clock DeltaTimeClock;

	gameClock.restart();
	while (window.isOpen()) {
		sf::Time dtTime = DeltaTimeClock.restart();
		delta_time = dtTime.asSeconds();
		mousePos = Mouse::getPosition(window);

		if (gamePaused) delta_time = 0;

		mousePos.x *= (float)windowWidth / window.getSize().x;
		mousePos.y *= (float)windowHeight / window.getSize().y;

		uiInteracted = false;
		mouseJustClicked[0] = false;
		mouseJustClicked[1] = false;

		if (Keyboard::isKeyPressed(Keyboard::LShift)) {
			delta_time = 0.2;
		}

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
			}

			else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::G) {
				// Press 'G' to add 5 boosters
				curBoosterCnt += 5;
				if (curBoosterCnt > maxBoosterCnt) curBoosterCnt = maxBoosterCnt;

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
			} else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
				SetGamePaused(!gamePaused);
			}
		}

		if (gameClock.getElapsedTime().asSeconds() < 0.1f) {
			mouseJustClicked[0] = false;
			mouseJustClicked[1] = false;
		}

		if (requestedLevelMusic != REQUEST_NONE) {
			if (music.getStatus() == Music::Playing) music.stop();

			if (requestedLevelMusic == REQUEST_SELECT_MUSIC) {
				select_levels_background_music(music);
			} else {
				playMusicForLevel(requestedLevelMusic, music);
			}
			requestedLevelMusic = REQUEST_NONE;
		}

		if (currentLevel != nullptr) {
			UpdateLevel(window);
		} else {
			if (music.getStatus() != Music::Playing) select_levels_background_music(music);
			DrawMainMenu(window);
		}
		window.display();
	}

	delete font;
	delete loudFont;
	textures.clear();
	saveStorage();

	return 0;
}
