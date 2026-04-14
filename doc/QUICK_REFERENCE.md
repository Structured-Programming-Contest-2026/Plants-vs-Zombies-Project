# Code Architecture Cheat Sheet

**Quick Reference for AI Models & Developers**

---

## File Organization (main.cpp)

```
Lines 1-100:  #includes, #defines, globals
Lines 100-200:   Enums (ENTITY_CATEGORY, ENTITY_TYPE, TEXTURE_TYPE)
Lines 200-300:   Structs (Animation, Entity, Level, Sun)
Lines 300-400:   Global game state (entities, money, suns, textures)
Lines 400-600:   Helper functions (getTexture, gridCell, getEntity)
Lines 600-1100:  Define functions (defineNormalPlant, defineRaZombie, etc.)
Lines 1100-1500: Drawing/Update functions (DrawZombies, DrawPlantsGrid, etc.)
Lines 1500-1800: UI and level management (DrawUI, UpdateLevel, DrawMainMenu)
Lines 1800-2000: main() - initialization and game loop
```

---

## Key Functions by Purpose

### Entity Creation
```cpp
Entity defineNormalPlant()      // Line ~650
Entity defineShroomPlant()      // Line ~670
Entity defineSunflower() // Line ~690
Entity defineWallnut()          // Line ~750
Entity defineChomperPlant()     // Line ~730
Entity defineNormalZombie()     // Line ~600
Entity defineJapaneseZombie()   // Line ~620
Entity defineBigZombie()        // Line ~640
Entity defineTorchlightZombie() // Line ~790
Entity defineRaZombie()         // Line ~815
```

### Zombie Logic
```cpp
void DrawZombies()        // Line ~1125
// Contains: Death handling, Animation, Ra ability, Movement, Combat
```

### Plant Logic
```cpp
void DrawPlantsGrid()        // Line ~1040
// Contains: Wallnut health states, Chomper behavior, Attacks, Sunflower suns
```

### Bullet Logic
```cpp
void UpdateAndDrawBullets()         // Line ~1280
// Contains: Bullet movement, Collision, Zombie damage
```

### Utilities
```cpp
TEXTURE_TYPE getTextureType()       // Line ~270
// Maps ENTITY_TYPE to TEXTURE_TYPE

Entity& getEntity(Vector2i gridPos) // Line ~345
// Finds plant at grid position

Vector2i gridCell(Vector2f pos)     // Line ~340
// Converts world position to grid cell

bool isSunAlive(const Sun& sun)     // Line ~495
// Checks if sun exists and is valid

bool anySunAlive()      // Line ~355
// Checks if any sun exists
```

---

## Global State Variables

```cpp
vector<Entity> entities;       // All game objects (zombies, plants, bullets)
int money; // Current player money
Sun suns[MAX_SUNS];     // Max 16 suns on screen
Entity* currentLevel;     // Pointer to active level
```

---

## Data Structure Quick Reference

### Animation (Sprite Frame Management)
```cpp
struct Animation {
 int frames;     // Total frames in spritesheet
float frameTimer;    // Seconds per frame
    int currentFrame;      // Current frame (0-based)
  int cycles;  // Times looped
    Clock clock;           // Timer since last frame
};
```

### Entity (All game objects)
```cpp
struct Entity {
    ENTITY_CATEGORY category;  // Type: ZOMBIE, PLANT, Bullet
    ENTITY_TYPE type;     // Specific: ZOMBIE_NORMAL, PLANT_CHOMPER, etc.
    
    // Stats
    int health, attack, price, scale;
    
    // Animation (10 animations max)
    int num_animations;
    int currentAnimationIndex;
    Animation animations[10];
    Animation currentAnimation;
    int AnimationForCycles;
    function<void(Entity&)> afterAnimation;
    
// Ability
  bool isUsingAbility;
    Clock abilityClock;
    float abilityDuration;
    
    // Position
    Vector2f position;
    float x_vel;
    int gridRow, gridCol;
    
    // State
    bool isDead;
    Clock attackClock;
    int attackTimer;
    int id;  // Unique identifier
};
```

---

## Common Code Locations

### To Add New Zombie:
1. Line ~50: Add enum to `ENTITY_TYPE`
2. Line ~70: Add enum to `TEXTURE_TYPE`
3. Line ~600-820: Create `defineNewZombie()` function
4. Line ~270: Add case to `getTextureType()` switch
5. Line ~1700: Create template: `Entity newZombie = defineNewZombie()`
6. Line ~1725: Add to level: `levels[X].zombie_types.push_back(...)`
7. Line ~1125: Add ability logic in `DrawZombies()` if needed

### To Add New Plant:
1. Line ~50: Add enum to `ENTITY_TYPE`
2. Line ~70: Add enum to `TEXTURE_TYPE`
3. Line ~650-750: Create `defineNewPlant()` function
4. Line ~270: Add case to `getTextureType()` switch
5. Line ~1700: Create template: `Entity newPlant = defineNewPlant()`
6. Line ~1725: Add to level: `levels[X].plant_types.push_back(...)`
7. Line ~1040: Add behavior logic in `DrawPlantsGrid()` if needed

### To Add Ability:
1. Line ~1125 (for zombies) or Line ~1040 (for plants)
2. Add 3-part logic: TRIGGER ? ACTIVE ? END
3. Use `Clock` and `isUsingAbility` for timing
4. Use `DoAnimation()` for animation control
5. Use `afterAnimation` callback if chaining animations

---

## Animation Quick Reference

### Setting Up Animations
```cpp
entity.num_animations = 3;      // Total animation types
entity.animations[0].frames = 20;           // Animation 0: 20 frames
entity.animations[0].frameTimer = 0.1f;     // Each frame: 0.1 seconds
entity.animations[1].frames = 15;           // Animation 1: 15 frames
entity.animations[1].frameTimer = 0.08f;
entity.setCurrentAnimation(0);         // Start with animation 0
```

### Playing Animations
```cpp
entity.DoAnimation(1, 0);     // Play animation 0 once
entity.DoAnimation(10, 1); // Play animation 1 ten times
entity.DoAnimation(-1, 0);    // Play animation 0 infinitely
```

### Animation Callbacks
```cpp
entity.afterAnimation = [](Entity& e) {
 // Executes when DoAnimation() completes
    e.DoAnimation(-1, 0);  // Chain to next animation
};
```

---

## Ability Pattern Template

```cpp
// In DrawZombies() or DrawPlantsGrid()

if (entity.type == ENTITY_TYPE::YOUR_CHARACTER && entity.health > 0) {
    
    // PHASE 1: TRIGGER
    if (!entity.isUsingAbility && triggerCondition()) {
        entity.isUsingAbility = true;
        entity.abilityClock.restart();
     entity.x_vel = 0;  // Stop movement
   entity.DoAnimation(-1, abilityAnimIndex);
// ... any setup ...
    }
    
    // PHASE 2: ACTIVE (runs every frame while ability is active)
    if (entity.isUsingAbility) {
 // ... execute ability behavior ...
 }
    
    // PHASE 3: END
    if (entity.isUsingAbility && 
        (entity.abilityClock.getElapsedTime().asSeconds() >= entity.abilityDuration ||
         endConditionMet())) {
      entity.isUsingAbility = false;
        entity.x_vel = normalSpeed;
        entity.setCurrentAnimation(0);
        // ... cleanup ...
    }
}
```

---

## Texture Loading Pattern

```cpp
// Location: Inside defineCharacter() function

// Single animation
Texture tex;
tex.loadFromFile("assets/filename.png");
textures[TEXTURE_TYPE::CHARACTER_NAME][0] = tex;

// Multiple animations
textures[TEXTURE_TYPE::CHARACTER_NAME][0].loadFromFile("assets/walk.png");
textures[TEXTURE_TYPE::CHARACTER_NAME][1].loadFromFile("assets/attack.png");
textures[TEXTURE_TYPE::CHARACTER_NAME][2].loadFromFile("assets/death.png");

// After defining:
Entity character{};
character.num_animations = 3;  // Match number of textures loaded
```

---

## Game Loop Flow

```
main()
  ?
while (window.isOpen())
  ?? DeltaTime calculation
  ?? Input handling
  ?? if (currentLevel)
  ?   ?? UpdateLevel()
?  ?? Spawn timers (CheckTimer_AddZombie, CheckTimer_AddSun)
  ?     ?? DrawPlantsGrid() ? Plant updates + animation + drawing
  ?       ?? DrawZombies() ? Zombie updates + ability + animation + drawing
  ?       ?? UpdateAndDrawBullets() ? Bullet movement + collision
  ?       ?? UpdateSuns() ? Sun falling + drawing
  ?       ?? DrawUI() ? Buttons + money display
  ?       ?? Game over check
  ?  else
  ?   ?? DrawMainMenu()
  ?       ?? Level selection
  ?
  ?? window.display() ? Render to screen
```

---

## When to Use What

### `DoAnimation(1, idx)` - Single Play
```cpp
// Play once, then reset to animation 0
zombie.DoAnimation(1, 2);  // Death animation
```

### `DoAnimation(10, idx)` - Multiple Cycles
```cpp
// Play 10 times, then reset to 0
entity.DoAnimation(10, 2);  // Digest animation
```

### `DoAnimation(-1, idx)` - Infinite Loop
```cpp
// Loop forever until manually changed
entity.DoAnimation(-1, 0);  // Idle animation
```

### `afterAnimation` Callback
```cpp
// For complex sequences
entity.DoAnimation(1, 1);  // Play animation 1 once
entity.afterAnimation = [](Entity& e) {
    e.DoAnimation(-1, 0);  // When done, play animation 0 forever
};
```

---

## Common Conditionals

### Find entity at grid position
```cpp
Entity& plant = getEntity({row, col});
if (plant.category == ENTITY_CATEGORY::PLANT) {
    // Found plant
}
```

### Loop through living zombies
```cpp
for (Entity& zombie : entities) {
    if (zombie.category != ENTITY_CATEGORY::ZOMBIE) continue;
    if (zombie.isDead) continue;
    // ... use zombie ...
}
```

### Check distance between two entities
```cpp
float dx = zombie.position.x - plant.position.x;
float dy = zombie.position.y - plant.position.y;
float dist = sqrt(dx * dx + dy * dy);
if (dist < 100.0f) {
    // Close enough
}
```

### Check if time passed
```cpp
if (attackClock.getElapsedTime().asSeconds() >= 3.0f) {
  attackClock.restart();
    // Execute every 3 seconds
}
```

---

## Entity State Changes

```cpp
// Birth (creation in level)
Entity newZombie = level.zombie_types[index];
newZombie.position = Vector2f(x, y);
newZombie.id = nextEntityId++;
entities.push_back(newZombie);

// Combat/Damage
entity.health -= damageAmount;

// Death Start
if (entity.health <= 0 && !entity.isDead) {
    entity.isDead = true;
    entity.DoAnimation(1, deathAnimIndex);
}

// Death End (in callback)
entity.category = ENTITY_CATEGORY::NONE;  // Mark for removal

// Removal (next frame, loops will skip)
for (Entity& e : entities) {
    if (e.category == ENTITY_CATEGORY::NONE) continue;  // Skipped
}
```

---

## Frame Rate Independent Movement

```cpp
// Use delta_time for smooth motion
entity.position.x -= entity.x_vel * delta_time;

// For 60 FPS with x_vel = 0.05:
// Movement per frame = 0.05 * (1/60) ? 0.0008 pixels
// For ability pulling suns:
sun.position.x += (dx / dist) * 300.0f * delta_time;
```

---

## Animation Frame Calculation

```cpp
// Texture layout: Horizontal strip of frames
// Each frame width = total_width / num_frames

Texture& tex = textures[TEXTURE_TYPE::ZOMBIE_NORMAL][0];
float frameWidthF = tex.getSize().x / entity.currentAnimation.frames;
int frameWidth = round(frameWidthF);
int frameHeight = tex.getSize().y;

// Crop rect for current frame
IntRect rect = {
    entity.currentAnimation.currentFrame * frameWidth,  // X
    0,         // Y
    frameWidth,    // Width
    frameHeight          // Height
};
sprite.setTextureRect(rect);
```

---

## Debugging Checklist

- [ ] Is `num_animations` set before using animations?
- [ ] Are textures loaded before entity creation?
- [ ] Is `getTextureType()` case added for new entity type?
- [ ] Is entity added to a level in `main()`?
- [ ] Is clock restarted before timing?
- [ ] Does animation index exist (< num_animations)?
- [ ] Is `setCurrentAnimation()` called at setup?
- [ ] Does callback properly chain animations?
- [ ] Is ability cleanup done (reset x_vel, scale, etc.)?
- [ ] Are dead entities skipped in loops (continue)?

---

**For AI Model Assistance**: This reference should be consulted when:
- User asks to add a new character
- User wants to add an ability
- User reports animation issues
- User needs to modify timing/duration
- User wants to optimize code

