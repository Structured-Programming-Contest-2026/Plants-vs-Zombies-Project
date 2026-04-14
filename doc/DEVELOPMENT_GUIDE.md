# Plants vs Zombies - Development Guide

**For: AI Models & Developers Understanding This Codebase**

---

## Table of Contents

1. [Core Concepts](#core-concepts)
2. [Architecture Overview](#architecture-overview)
3. [Adding New Characters](#adding-new-characters)
4. [Animation System](#animation-system)
5. [Ability System](#ability-system)
6. [Control Flow & State Management](#control-flow--state-management)
7. [Quick Reference Checklist](#quick-reference-checklist)
8. [Common Patterns](#common-patterns)

---

## Core Concepts

### The Big Picture

This is a **tower defense game** where:
- **Plants** are defensive towers that stay in place and attack
- **Zombies** are enemies that move left and must be stopped
- **Abilities** are special actions (time-limited behaviors) that either zombies or plants can perform
- **Animations** are visual sequences (sprite frames) that play when actions occur

### Key Mental Models

**Model 1: Entity Lifecycle**
```
Entity Created ? Animation Plays ? Ability Triggers ? Movement/Combat ? Death ? Removed
```

**Model 2: Time-Based Decisions**
```
Each Frame:
  1. Check if enough time passed (using Clock)
  2. If yes, trigger action
  3. Restart clock
  4. Execute behavior
```

**Model 3: State Layering**
```
Base State (IDLE)
    ?
Animation State (Which animation playing?)
    ?
Ability State (Is ability active?)
    ?
Position State (Where is entity?)
```

---

## Architecture Overview

### File Structure
```
main.cpp contains everything:
??? Enums (ENTITY_TYPE, ENTITY_CATEGORY, TEXTURE_TYPE)
??? Structs (Animation, Entity, Sun, Level)
??? Global State (entities vector, money, suns array)
??? Helper Functions (getTexture, gridCell, getEntity)
??? Character Definition Functions (defineNormalPlant, defineRaZombie, etc.)
??? Drawing/Update Functions (DrawZombies, DrawPlantsGrid, UpdateLevel)
??? main() - Game Loop
```

### Key Data Structures

#### `Animation` Struct
Tracks ONE animation sequence:
```cpp
struct Animation {
 int frames = 1;           // How many frames in spritesheet
    float frameTimer = 1.0f;  // Seconds between frame changes
    int currentFrame = 0;  // Currently displayed (0 to frames-1)
    int cycles = 0;           // How many times looped
    Clock clock;    // Timer since last frame advance
};
```

**Example**: A zombie walk animation with 20 frames, 0.1f per frame:
- Frame 0, 1, 2, ..., 19 cycle continuously
- Each frame displays for 0.1 seconds
- Takes 2 seconds total for full walk cycle

#### `Entity` Struct
Represents ANY character (zombie, plant, bullet):
```cpp
struct Entity {
    // IDENTITY
    ENTITY_CATEGORY category;  // ZOMBIE, PLANT, Bullet
    ENTITY_TYPE type;          // ZOMBIE_NORMAL, PLANT_CHOMPER, etc.
    
    // STATS
    int health;
    int attack;
    int price;
    
    // ANIMATION SYSTEM
    int num_animations;            // Total anim types (0-10)
    int currentAnimationIndex;   // Which one playing now
    Animation animations[10];      // Array of all animations
    Animation currentAnimation;      // Reference to current
    int AnimationForCycles;             // Cycle countdown
    function<void(Entity&)> afterAnimation;    // Callback when done

    // ABILITY SYSTEM
    bool isUsingAbility;
    Clock abilityClock;
    float abilityDuration;
    
    // POSITION & MOVEMENT
    Vector2f position;
    float x_vel;   // Velocity (negative = left)
    float scale; // Size multiplier
    int gridRow, gridCol;   // Grid position
    
// STATE
    bool isDead;
Clock attackClock;
    int attackTimer;
};
```

---

## Adding New Characters

### The Concept (Read First)

Adding a character is a **4-step process**:

1. **Register the type** - Tell the system this character exists
2. **Load textures** - Load image files from disk
3. **Define the template** - Create a function that returns a configured entity
4. **Add to level** - Include it in playable levels

Think of it like: Register ? Load ? Configure ? Deploy

### Step 1: Add to Enums

**Location**: Top of `main.cpp`, find the enums

```cpp
enum class ENTITY_TYPE : uint8_t {
    // ... existing types ...
    ZOMBIE_MUMMY,        // ? ADD YOUR NEW ZOMBIE HERE
    PLANT_PEASHOOTER,    // ? OR YOUR NEW PLANT HERE
};

enum class TEXTURE_TYPE : uint8_t {
    // ... existing types ...
    ZOMBIE_MUMMY,        // ? MUST MATCH THE ENTITY_TYPE
    PLANT_PEASHOOTER,
};
```

**Why?** The system uses these enums as keys to find textures and identify entity types.

### Step 2: Create Definition Function

**Location**: Below existing `define*` functions (around line 500-800)

```cpp
// For a NEW ZOMBIE with an ability
Entity defineMummyZombie() {
    // ============ STEP A: Load all textures ============
    textures[TEXTURE_TYPE::ZOMBIE_MUMMY][0].loadFromFile("assets/mummy_walk.png");
    textures[TEXTURE_TYPE::ZOMBIE_MUMMY][1].loadFromFile("assets/mummy_death.png");
    textures[TEXTURE_TYPE::ZOMBIE_MUMMY][2].loadFromFile("assets/mummy_ability.png");

    // ============ STEP B: Create entity ============
    Entity zombie{};
    zombie.category = ENTITY_CATEGORY::ZOMBIE;
zombie.type = ENTITY_TYPE::ZOMBIE_MUMMY;
  
    // ============ STEP C: Configure stats ============
    zombie.health = 150;         // More health than normal
    zombie.attack = 25;     // Standard attack
    zombie.x_vel = 0.03f;          // SLOWER speed
    zombie.scale = 1.0f;
    
    // ============ STEP D: Setup animations ============
    zombie.num_animations = 3;     // We have 3 different animations
    
    // Animation 0: Walking (default)
    Animation walkAnim;
    walkAnim.frames = 24;     // 24 frames in walk.png
    walkAnim.frameTimer = 0.08f;   // Each frame: 0.08 seconds
    zombie.animations[0] = walkAnim;
    
    // Animation 1: Death
    Animation deathAnim;
    deathAnim.frames = 40;
    deathAnim.frameTimer = 0.06f;
    zombie.animations[1] = deathAnim;
    
    // Animation 2: Ability (mummy unwrap)
    Animation abilityAnim;
    abilityAnim.frames = 20;
    abilityAnim.frameTimer = 0.1f;
    zombie.animations[2] = abilityAnim;
    
    // ============ STEP E: Set default animation ============
  zombie.setCurrentAnimation(0);
    
    return zombie;
}
```

**For a NEW PLANT:**

```cpp
Entity definePeashooterPlant() {
  // Load textures
    textures[TEXTURE_TYPE::PLANT_PEASHOOTER][0].loadFromFile("assets/pea_idle.png");
  textures[TEXTURE_TYPE::PLANT_PEASHOOTER][1].loadFromFile("assets/pea_attack.png");
    
    Entity plant{};
    plant.category = ENTITY_CATEGORY::PLANT;
    plant.type = ENTITY_TYPE::PLANT_PEASHOOTER;
  
  // Stats
    plant.health = 120;
    plant.attack = 50;       // Pea damage
    plant.price = 75;            // Cost in game
    plant.scale = 1.0f;
    
    // Animations
    plant.num_animations = 2;
    
    Animation idleAnim;
    idleAnim.frames = 12;
    idleAnim.frameTimer = 0.1f;
    plant.animations[0] = idleAnim;  // Idle
  
    Animation attackAnim;
    attackAnim.frames = 8;
    attackAnim.frameTimer = 0.05f;
    plant.animations[1] = attackAnim;  // Attack
    
    plant.setCurrentAnimation(0);
    return plant;
}
```

### Step 3: Add to Level

**Location**: In `main()` function, around line 1700

```cpp
// BEFORE: In the character creation section
Entity mummyZombie = defineMummyZombie();
Entity peashooter = definePeashooterPlant();

// THEN: In the level setup section
levels[2].zombie_types.push_back(mummyZombie);    // Add to level 2
levels[2].plant_types.push_back(peashooter);

// OR: Create new level
levels[3].plant_types = { peashooter, sunflower, wallnut };
levels[3].zombie_types = { mummyZombie, zombieTorch, zombieRa };
levels[3].startingSuns = 400;
```

### Step 4: Update Helper Function

**Location**: `getTextureType()` function, around line 270

```cpp
TEXTURE_TYPE getTextureType(ENTITY_TYPE type) {
    TEXTURE_TYPE textureType = TEXTURE_TYPE::NONE;
    switch (type) {
        // ... existing cases ...
        
        case ENTITY_TYPE::ZOMBIE_MUMMY:    // ? ADD
            textureType = TEXTURE_TYPE::ZOMBIE_MUMMY;
            break;
          
case ENTITY_TYPE::PLANT_PEASHOOTER:          // ? ADD
            textureType = TEXTURE_TYPE::PLANT_PEASHOOTER;
        break;
 
        default:
     textureType = TEXTURE_TYPE::NONE;
       break;
 }
  return textureType;
}
```

---

## Animation System

### The Concept

An animation is a **sequence of image frames shown in sequence**. Like a film strip.

```
Walk Animation:
Frame 0 (0.0s)   Frame 1 (0.1s)   Frame 2 (0.2s) ...   Frame 19 (1.9s)
  [Image]       [Image]          [Image]        [Image]

Then loops back to Frame 0 (infinite loop by default)
```

### How It Works in Code

**Phase 1: Setup (When entity is created)**
```cpp
zombie.num_animations = 3;      // Entity has 3 different animations
zombie.animations[0] = walkAnim;        // Index 0: walk animation
zombie.animations[1] = deathAnim;       // Index 1: death animation
zombie.animations[2] = abilityAnim;     // Index 2: ability animation
zombie.setCurrentAnimation(0);          // Start with walking
```

**Phase 2: Update (Every frame in DrawZombies/DrawPlantsGrid)**
```cpp
zombie.nextFrame();  // Advances frame if enough time passed
```

Inside `nextFrame()`:
```cpp
if (clock.getElapsedTime() >= frameTimer) {
    clock.restart();        // Reset timer
    currentFrame++;  // Next frame (0 ? 1 ? 2 ? ...)
    
    if (currentFrame >= frames) {  // Completed all frames?
        cycles++;           // One full loop done
        
     if (cycles >= AnimationForCycles && AnimationForCycles != -1) {
          // Animation finished, execute callback
      setCurrentAnimation(0);
            afterAnimation(*this);
        }
        currentFrame %= frames;  // Wrap around (19 ? 0)
    }
}
```

**Phase 3: Draw (Render the current frame)**
```cpp
IntRect getFrameRect(Entity& entity) {
    Texture& tex = textures[getTextureType(entity.type)][entity.currentAnimationIndex];
    
 // Calculate frame width
    float frameWidthF = tex.getSize().x / entity.currentAnimation.frames;
    int frameWidth = round(frameWidthF);
    int frameHeight = tex.getSize().y;
    
    // Crop the texture rect to show only current frame
    IntRect textureRect = {
        entity.currentAnimation.currentFrame * frameWidth,  // X position
        0,// Y position
        frameWidth,     // Width
        frameHeight // Height
    };
    
    return textureRect;
}
```

### Animation Timing

```cpp
float frameTimer = 0.1f;   // Seconds between frames

// Speed equivalents:
// 0.05f  = very fast (200 FPS feel)
// 0.1f   = normal speed (100 FPS feel)
// 0.15f  = slower
// 0.2f   = slow motion
```

### Playing Specific Animations

```cpp
// Play attack animation once
zombie.DoAnimation(1, 1);
```

Inside `DoAnimation()`:
```cpp
void DoAnimation(int forCycles = 1, int idx = 0) {
    // forCycles = how many times to play
    // -1 = infinite (loop forever)
    // 1 = play once
    // 10 = play 10 times
    
    AnimationForCycles = forCycles;
    setCurrentAnimation(idx);     // Switch to animation at index idx
}
```

### Chaining Animations

```cpp
// When Chomper attacks: Attack ? Digest ? Idle

plant.DoAnimation(1, 1);  // Play attack once

plant.afterAnimation = [](Entity& entity) {
    // Called when attack finishes
    entity.DoAnimation(10, 2);  // Play digest 10 times
    
    entity.afterAnimation = [](Entity& entity) {
        // Called when digest finishes
      entity.DoAnimation(-1, 0);  // Infinite idle
    };
};
```

**Flow:**
1. `DoAnimation(1, 1)` starts
2. Attack animation plays (13 frames × 0.1s = 1.3s)
3. Animation completes ? calls `afterAnimation` callback
4. Callback calls `DoAnimation(10, 2)` to start digest
5. Digest animation plays 10 cycles (30 frames each cycle)
6. Digest completes ? inner callback executes
7. Idle animation plays infinitely

---

## Ability System

### The Concept

An **ability** is a **time-limited special action** that:
1. Starts when a condition is met
2. Runs for X seconds (or until condition changes)
3. Has custom behavior each frame
4. Cleans up when done

**Real examples from code:**
- **Ra Zombie ability**: Collect all suns on screen, move them to self
- **Chomper ability**: Eat nearby zombie, play digest animation
- **Wallnut ability**: Change appearance based on health (visual state)

### How Abilities Work

**The Time-Based Pattern:**

```cpp
Clock abilityClock;  // Timer for ability
float abilityDuration;     // How long ability lasts
bool isUsingAbility;       // Currently active?

// ===== TRIGGER PHASE =====
// Check: Should ability start?
if (!zombie.isUsingAbility && someCondition()) {
    zombie.isUsingAbility = true;
    zombie.abilityClock.restart();    // START timer
    zombie.DoAnimation(-1, 2);        // Play ability animation
    // ... other setup ...
}

// ===== ACTIVE PHASE =====
// Execute while ability is running
if (zombie.isUsingAbility) {
    // Do something every frame
    moveSunsTowardZombie();
    // ...
}

// ===== END PHASE =====
// Check: Should ability stop?
if (zombie.isUsingAbility && 
    zombie.abilityClock.getElapsedTime().asSeconds() >= zombie.abilityDuration) {
    
    zombie.isUsingAbility = false;
    zombie.DoAnimation(-1, 0);   // Back to walk
    // ... cleanup ...
}
```

### Ra Zombie Ability - Complete Example

**The Concept**: Ra absorbs suns floating on screen to gain power

**Code Breakdown:**

```cpp
// ===== PART 1: CHECK IF ABILITY SHOULD START =====
if (zombie.type == ENTITY_TYPE::ZOMBIE_RA && zombie.health > 0) {
    
    // Start ability if:
    // - Not already using ability
  // - There are suns available
    // - Zombie hasn't reached max capacity
  // - Zombie isn't too close to end
    if (!zombie.isUsingAbility && 
        anySunAlive() && 
      zombie.storedSunValue < zombie.maxSunValue && 
      zombie.position.x < windowWidth - 300) {
        
        // Check: Is there at least one unclaimed sun?
        bool hasUnclaimedSun = false;
        for (int i = 0; i < MAX_SUNS; i++) {
  if (isSunAlive(suns[i]) && !suns[i].targeted && suns[i].position.y >= 0) {
                hasUnclaimedSun = true;
         break;
            }
        }
        
        // If yes, start the ability
        if (hasUnclaimedSun) {
zombie.isUsingAbility = true;
     zombie.abilityClock.restart(); // START TIMER
            zombie.x_vel = 0;    // Stop moving
 zombie.DoAnimation(-1, 2);         // Play ability animation (infinite)
   zombie.scale = 1.5f; // Grow larger
          
          // Mark all unclaimed suns as targets
            for (int i = 0; i < MAX_SUNS; i++) {
            if (isSunAlive(suns[i]) && !suns[i].targeted && suns[i].position.y >= 0) {
         suns[i].targeted = true;         // This sun is now tracked
        suns[i].ownerIndex = zombie.id;  // Belongs to this zombie
    suns[i].lifeClock.restart();
   }
         }
      }
    }
    
    // ===== PART 2: EXECUTE ABILITY EVERY FRAME =====
    if (zombie.isUsingAbility) {
        // Pull each targeted sun toward zombie
        for (int i = 0; i < MAX_SUNS; i++) {
          Sun& sun = suns[i];
  
      // Skip if not our target
            if (!sun.targeted || !isSunAlive(sun) || sun.ownerIndex != zombie.id) 
         continue;
            
    // Calculate direction to sun
    float dx = zombie.position.x - sun.position.x;
            float dy = zombie.position.y - sun.position.y;
            float dist = sqrt(dx * dx + dy * dy);
            
      // Move sun toward zombie
   if (dist > 1.0f) {
             sun.position.x += (dx / dist) * 300.0f * delta_time;
        sun.position.y += (dy / dist) * 300.0f * delta_time;
            }
    
            // If sun reaches zombie, collect it
            if (dist < 60.0f) {
    if (zombie.storedSunValue + 25 <= zombie.maxSunValue) {
         sun.clicked = true;   // Mark as taken
           sun.targeted = false;  // No longer tracked
 sun.ownerIndex = -1;         // Not owned
    zombie.storedSunValue += 25;  // Add to zombie's bank
   }
            }
        }
  }
    
    // ===== PART 3: CHECK IF ABILITY SHOULD END =====
    // Check if any suns still being targeted
    bool hasLiveSuns = false;
    for (int i = 0; i < MAX_SUNS; i++) {
        if (suns[i].ownerIndex == zombie.id && isSunAlive(suns[i]))
 hasLiveSuns = true;
    }
    
    // End ability if:
    // - Time ran out, OR
    // - No more suns to collect
    if (zombie.isUsingAbility && 
  (zombie.abilityClock.getElapsedTime().asSeconds() >= zombie.abilityDuration 
    || !hasLiveSuns)) {
        
   // Cleanup
        for (int i = 0; i < MAX_SUNS; i++) {
       if (suns[i].ownerIndex == zombie.id) {
   suns[i].targeted = false;
                suns[i].ownerIndex = -1;
   }
        }
        
 // Reset state
        zombie.isUsingAbility = false;
        zombie.x_vel = 0.05f;      // Resume moving
        zombie.setCurrentAnimation(0);  // Back to walk
        zombie.scale = 1.0f;            // Normal size
    }
}
```

---

## Control Flow & State Management

### The Game Loop (main execution flow)

```
while (window.isOpen()) {
    ?
    Frame Start:
    - Calculate delta_time (time since last frame)
    - Get mouse position
    - Reset interaction flags
    
    ?
    Event Processing:
    - Closed? Exit
    - Mouse clicked? Set flag
    - Keys pressed? Handle input
    
    ?
    Game Logic:
    if (currentLevel != nullptr) {
        UpdateLevel(window)  ? All game stuff happens here
    } else {
  DrawMainMenu(window) ? Show level selector
    }
    
    ?
    Display:
    - Draw everything to screen
    - Swap buffers
    
    ?
    Loop again (usually 60 FPS)
}
```

### UpdateLevel() - Where Everything Happens

```cpp
void UpdateLevel(RenderWindow& window) {
    
 // ===== SPAWN PHASE =====
    CheckTimer_AddZombie(window);  // Add zombie if time passed
    CheckTimer_AddSun(window);     // Add sun if time passed
    
    // ===== RENDERING PHASE =====
    window.clear(Color::Black);
    window.draw(ground);

    // ===== GAME OVER CHECK =====
    if (isGameOver) {
        window.draw(gameOverScreen);
    } else {
      // ===== UPDATE PHASE =====
        DrawPlantsGrid(window);// Plant logic + animation
        DrawZombies(window);            // Zombie logic + animation
UpdateAndDrawBullets(window);   // Bullet logic
        UpdateSuns(window);      // Sun falling + visual
        DrawUI(window);         // UI buttons
        
        // ===== INPUT PHASE =====
 // Mouse clicked?
        if (!uiInteracted && mouseJustClicked[0]) {
            // Try clicking sun
            // Try clicking plant spot
        }
        
    // ===== WIN/LOSE CHECK =====
        // Did zombie reach left edge?
        for (Entity& zombie : entities) {
      if (zombie.position.x <= 0) {
         isGameOver = true;
 }
        }
    }
}
```

### Entity Update Pattern (For each entity type)

**Zombies follow this pattern:**
```
for each zombie in entities {
    ?
    CHECK DEATH
    - If health <= 0 ? start death animation
    - If animation done ? remove from world
    
    ?
    ANIMATE
    - Call nextFrame() to advance animation
    - Get current frame rect from spritesheet
    
    ?
    SPECIAL ABILITY (if exists)
    - Ra: Check if should use ability
    - Ra: Execute ability (move suns)
    - Ra: Check if ability should end
    
    ?
    MOVEMENT
    - Update position based on x_vel
    - Check grid position
    
  ?
    COMBAT
    - Check if on plant
    - If yes ? attack plant
    - If no ? keep moving
    
    ?
    DRAW
    - Set sprite texture
    - Set sprite position
    - Draw to screen
}
```

---

## Quick Reference Checklist

### Adding a New Zombie Type

- [ ] Add `ZOMBIE_NAME` to `ENTITY_TYPE` enum
- [ ] Add `ZOMBIE_NAME` to `TEXTURE_TYPE` enum
- [ ] Create `defineNameZombie()` function
  - [ ] Load textures: `textures[TEXTURE_TYPE::ZOMBIE_NAME][0/1/2...].loadFromFile(...)`
  - [ ] Set `category` = `ZOMBIE`
  - [ ] Set `type` = `ZOMBIE_NAME`
  - [ ] Configure stats (health, attack, x_vel, scale)
  - [ ] Set `num_animations`
  - [ ] Configure each animation (frames, frameTimer)
  - [ ] Call `setCurrentAnimation(0)`
  - [ ] Return zombie
- [ ] Add case in `getTextureType()` function
- [ ] Create template in `main()`: `Entity zombieName = defineNameZombie()`
- [ ] Add to level: `levels[X].zombie_types.push_back(zombieName)`
- [ ] If has ability, add ability logic in `DrawZombies()` function

### Adding a New Plant Type

- [ ] Add `PLANT_NAME` to `ENTITY_TYPE` enum
- [ ] Add `PLANT_NAME` to `TEXTURE_TYPE` enum
- [ ] Create `defineNamePlant()` function
  - [ ] Load textures
  - [ ] Set `category` = `PLANT`
  - [ ] Set `type` = `PLANT_NAME`
  - [ ] Configure stats (health, attack, price, scale)
  - [ ] Set `num_animations`
  - [ ] Configure each animation
  - [ ] Call `setCurrentAnimation(0)`
  - [ ] Return plant
- [ ] Add case in `getTextureType()` function
- [ ] Create template in `main()`: `Entity plantName = defineNamePlant()`
- [ ] Add to level: `levels[X].plant_types.push_back(plantName)`
- [ ] If has special behavior, add logic in `DrawPlantsGrid()` function

### Adding an Ability to a Character

**Location**: In the drawing function (`DrawZombies()` or `DrawPlantsGrid()`)

```cpp
// ===== TRIGGER PHASE =====
if (entity.type == ENTITY_TYPE::YOUR_TYPE) {
    if (!entity.isUsingAbility && conditionMet) {
        entity.isUsingAbility = true;
        entity.abilityClock.restart();
   // ... initialize ability ...
    }
    
    // ===== ACTIVE PHASE =====
    if (entity.isUsingAbility) {
 // ... execute ability every frame ...
    }
    
    // ===== END PHASE =====
    if (entity.isUsingAbility && abilityDone) {
    entity.isUsingAbility = false;
        // ... cleanup ...
    }
}
```

---

## Common Patterns

### Pattern 1: Time-Based Trigger

```cpp
Clock myTimer;

// In update loop:
if (myTimer.getElapsedTime().asSeconds() >= 3.0f) {  // Every 3 seconds
    myTimer.restart();         // Reset
    doSomething();           // Execute
}
```

### Pattern 2: Finding Entities

```cpp
// Find first zombie in same row
for (Entity& zombie : entities) {
 if (zombie.category != ENTITY_CATEGORY::ZOMBIE) continue;  // Skip non-zombies
    if (zombie.isDead) continue;          // Skip dead
    
    if (abs(zombie.position.y - plant.position.y) < 100.0f) {
     // Found one in same row!
        break;
    }
}

// Count living zombies
int zombieCount = 0;
for (Entity& zombie : entities) {
    if (zombie.category == ENTITY_CATEGORY::ZOMBIE && zombie.health > 0) {
   zombieCount++;
    }
}
```

### Pattern 3: State-Based Animation

```cpp
// Based on health, show different animation
if (plant.type == ENTITY_TYPE::PLANT_WALLNUT) {
    int state = 0;
    if (plant.health <= 250) state = 1;    // Cracked
    if (plant.health <= 125) state = 2;    // Badly damaged
    
    if (plant.currentAnimationIndex != state) {
    plant.setCurrentAnimation(state);  // Switch animation
    }
}
```

### Pattern 4: Distance Calculation

```cpp
// Vector from A to B
float dx = targetPos.x - sourcePos.x;
float dy = targetPos.y - sourcePos.y;

// Distance
float distance = sqrt(dx * dx + dy * dy);

// Normalized direction (unit vector)
float dirX = dx / distance;
float dirY = dy / distance;

// Move toward target
position.x += dirX * speed * delta_time;
position.y += dirY * speed * delta_time;
```

### Pattern 5: Grid-Based Positioning

```cpp
// Get grid cell from world position
Vector2i gridCell = gridCell_imp(worldPosition);

// Get world position from grid
int worldX = offsetX + (col * cellWidth) + (cellWidth / 2);
int worldY = offsetY + (row * cellHeight) + (cellHeight - plantHeight);

// Check bounds
if (gridCell.x >= 0 && gridCell.x < gridRows &&
    gridCell.y >= 0 && gridCell.y < gridCols) {
    // Valid position
}
```

### Pattern 6: Animation Chaining with Callbacks

```cpp
// Play sequence: Attack ? Digest ? Idle

void startAttackSequence(Entity& plant) {
    plant.DoAnimation(1, 1);  // Attack once
    
    plant.afterAnimation = [&plant](Entity& entity) {
        plant.DoAnimation(10, 2);  // Digest 10 times
        
        plant.afterAnimation = [&plant](Entity& entity) {
         plant.DoAnimation(-1, 0);  // Idle infinite
        };
  };
}
```

### Pattern 7: Damage and Death

```cpp
// Take damage
entity.health -= damageAmount;

// Check death
if (entity.health <= 0) {
    if (!entity.isDead) {
        entity.isDead = true;
        entity.DoAnimation(1, deathAnimationIndex);  // Play death once
    
   entity.afterAnimation = [](Entity& entity) {
            entity.category = ENTITY_CATEGORY::NONE;  // Mark for removal
        };
 }
}
```

---

## For AI Model Assistance

### Instructions for AI When User Asks to:

#### "Add a new zombie"
- Ask for: name, health, attack, speed, abilities
- Follow the **Adding New Characters** section
- Verify all 4 steps are complete
- Check `getTextureType()` was updated
- Confirm in `main()` function it's created and added to a level

#### "Add an ability to [character]"
- Ask for: what does it do? how long? when triggers?
- Use the **Ability System** section as template
- Identify where logic goes (DrawZombies or DrawPlantsGrid)
- Implement 3 phases: TRIGGER ? ACTIVE ? END
- Add any required entity fields to store ability state

#### "Fix animation not playing"
- Check: Is `num_animations` set correctly?
- Check: Are textures loaded before entity creation?
- Check: Does animation index exist (< num_animations)?
- Check: Does texture exist at that index?
- Verify `setCurrentAnimation()` is called

#### "Optimize performance"
- Suggest reducing entity updates in `entities` vector
- Recommend early `continue` for dead entities
- Cache frequently accessed textures
- Use fixed arrays instead of vectors where possible

---

## Quick Debugging Tips

```cpp
// Add to see what's happening
cout << "Entity type: " << (int)entity.type
     << " | Animation: " << entity.currentAnimationIndex
     << " | Frame: " << entity.currentAnimation.currentFrame
     << " / " << entity.currentAnimation.frames << endl;

// Check if texture loaded
if (textures[type][0].getSize().x == 0) {
    cout << "ERROR: Texture not loaded!" << endl;
}

// Verify ability timing
cout << "Ability time: " << entity.abilityClock.getElapsedTime().asSeconds()
     << " / " << entity.abilityDuration << endl;
```

---

**Document Version**: 1.0
**Last Updated**: 2024
**For C++20 with SFML**
