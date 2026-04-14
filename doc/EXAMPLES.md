# Practical Examples - Copy & Modify

**Ready-to-use code snippets for common tasks**

---

## Example 1: Adding a Slow Zombie

**Concept**: A heavy zombie that moves slowly but has high health and damage

### Step 1: Add to Enums
```cpp
// In enum ENTITY_TYPE (around line 50)
enum class ENTITY_TYPE : uint8_t {
    NONE = 0,
    // ... existing ...
    ZOMBIE_SLOW,  // ? ADD THIS
    // ... rest ...
};

// In enum TEXTURE_TYPE (around line 70)
enum class TEXTURE_TYPE : uint8_t {
    NONE = 0,
    // ... existing ...
    ZOMBIE_SLOW,  // ? ADD THIS
    // ... rest ...
};
```

### Step 2: Create Definition Function
```cpp
// Add this AFTER defineTorchlightZombie() function (around line 810)

Entity defineSlowZombie() {
 // Load textures
    Texture tex;
    tex.loadFromFile("assets/slow_zombie_walk.png");
    textures[TEXTURE_TYPE::ZOMBIE_SLOW][0] = tex;
    
    tex.loadFromFile("assets/slow_zombie_death.png");
 textures[TEXTURE_TYPE::ZOMBIE_SLOW][1] = tex;
  
    // Create entity
    Entity zombie{};
  zombie.category = ENTITY_CATEGORY::ZOMBIE;
    zombie.type = ENTITY_TYPE::ZOMBIE_SLOW;
    
    // Stats: Slow but tough
 zombie.health = 250;        // Very high health
    zombie.attack = 50;         // High attack
    zombie.x_vel = 0.02f;       // HALF speed of normal
    zombie.scale = 1.3f;        // Bigger appearance
    
    // Animations: Just walk and death
    zombie.num_animations = 2;
    Animation walkAnim;
    walkAnim.frames = 16;
    walkAnim.frameTimer = 0.12f;  // Slower animation
    zombie.animations[0] = walkAnim;
    
    Animation deathAnim;
    deathAnim.frames = 45;
    deathAnim.frameTimer = 0.06f;
    zombie.animations[1] = deathAnim;
    
    zombie.setCurrentAnimation(0);
    return zombie;
}
```

### Step 3: Add Switch Case
```cpp
// In getTextureType() function (around line 270)
// Add inside the switch statement:

case ENTITY_TYPE::ZOMBIE_SLOW:
    textureType = TEXTURE_TYPE::ZOMBIE_SLOW;
    break;
```

### Step 4: Register in main()
```cpp
// In main() function, in the character loading section (around line 1700)
Entity zombie = defineNormalZombie();
Entity zombieJapanese = defineJapaneseZombie();
Entity zombieBig = defineBigZombie();
Entity zombieTorch = defineTorchlightZombie();
Entity zombieSlow = defineSlowZombie();  // ? ADD THIS

// Then add to a level (around line 1725)
levels[2].zombie_types = { zombieTorch, zombieRa, zombieSlow };  // ? ADD zombieSlow
```

---

## Example 2: Adding a Healing Plant

**Concept**: A plant that heals nearby plants instead of attacking

### Step 1: Add to Enums
```cpp
enum class ENTITY_TYPE : uint8_t {
    // ... existing ...
    PLANT_HEALER,  // ? ADD THIS
};

enum class TEXTURE_TYPE : uint8_t {
    // ... existing ...
    PLANT_HEALER,  // ? ADD THIS
};
```

### Step 2: Create Definition Function
```cpp
// Add after defineChomperPlant() (around line 745)

Entity defineHealerPlant() {
    // Load texture
    Texture tex;
    tex.loadFromFile("assets/healer_plant.png");
    textures[TEXTURE_TYPE::PLANT_HEALER][0] = tex;
    
    Entity plant{};
  plant.category = ENTITY_CATEGORY::PLANT;
  plant.type = ENTITY_TYPE::PLANT_HEALER;
    plant.health = 150;
    plant.attack = 0;        // Doesn't attack
 plant.price = 120;
    plant.scale = 1.1f;
    
    // Single animation
 plant.num_animations = 1;
    plant.animations[0].frames = 16;
    plant.animations[0].frameTimer = 0.1f;
    plant.setCurrentAnimation(0);
    
    return plant;
}
```

### Step 3: Add Switch Case
```cpp
case ENTITY_TYPE::PLANT_HEALER:
    textureType = TEXTURE_TYPE::PLANT_HEALER;
    break;
```

### Step 4: Add Healing Logic
```cpp
// In DrawPlantsGrid() function (around line 1050)
// Add BEFORE the "Animation" comment

else if (plant.type == ENTITY_TYPE::PLANT_HEALER) {
    // Healing plant: Heals nearby plants every 5 seconds
    if (plant.attackClock.getElapsedTime().asSeconds() >= 5.0f) {
        plant.attackClock.restart();
        
        // Find nearby plants to heal
        for (Entity& other : entities) {
 if (other.category != ENTITY_CATEGORY::PLANT) continue;
  if (other.type == plant.type) continue;  // Don't heal self
            
            // Check distance
float dx = other.position.x - plant.position.x;
        float dy = other.position.y - plant.position.y;
 float dist = sqrt(dx * dx + dy * dy);
       
  if (dist < 300.0f && dist > 0.0f) {  // Within range
       other.health += 50;  // Heal
                // Optional: Cap health
          if (other.health > 200) other.health = 200;
     }
        }
    }
}
```

### Step 5: Register in main()
```cpp
Entity healer = defineHealerPlant();
levels[2].plant_types = { sunflower, shroom, healer };  // ? ADD
```

---

## Example 3: Ice Zombie with Freeze Ability

**Concept**: Zombie that freezes plants, slowing their attack

### Step 1: Add to Enums
```cpp
enum class ENTITY_TYPE : uint8_t {
    // ... existing ...
    ZOMBIE_ICE,  // ? ADD
};

enum class TEXTURE_TYPE : uint8_t {
    // ... existing ...
    ZOMBIE_ICE,  // ? ADD
};
```

### Step 2: Extend Entity Struct
```cpp
// Add to Entity struct (around line 120) - one time only
int freezeDuration = 0;  // If > 0, plant is frozen
float frozenUntil = 0;
```

### Step 3: Create Definition Function
```cpp
Entity defineIceZombie() {
    textures[TEXTURE_TYPE::ZOMBIE_ICE][0].loadFromFile("assets/ice_walk.png");
    textures[TEXTURE_TYPE::ZOMBIE_ICE][1].loadFromFile("assets/ice_ability.png");
    textures[TEXTURE_TYPE::ZOMBIE_ICE][2].loadFromFile("assets/ice_death.png");
    
    Entity zombie{};
    zombie.category = ENTITY_CATEGORY::ZOMBIE;
    zombie.type = ENTITY_TYPE::ZOMBIE_ICE;
    zombie.health = 120;
    zombie.attack = 15;
    zombie.x_vel = 0.05f;
    
    zombie.num_animations = 3;
    zombie.animations[0] = {frames: 20, frameTimer: 0.1f};  // Walk
    zombie.animations[1] = {frames: 15, frameTimer: 0.1f};  // Ability
    zombie.animations[2] = {frames: 50, frameTimer: 0.05f}; // Death
    
    zombie.setCurrentAnimation(0);
    return zombie;
}
```

### Step 4: Add Switch Case
```cpp
case ENTITY_TYPE::ZOMBIE_ICE:
    textureType = TEXTURE_TYPE::ZOMBIE_ICE;
    break;
```

### Step 5: Add Ability Logic in DrawZombies()
```cpp
// In DrawZombies(), after Ra zombie logic (around line 1220)

if (zombie.type == ENTITY_TYPE::ZOMBIE_ICE && zombie.health > 0) {
    
    // TRIGGER: Random chance to freeze nearby plant
 if (!zombie.isUsingAbility && rand() % 100 < 10) {  // 10% chance
        // Find nearby plant
        Entity* targetPlant = nullptr;
  for (Entity& plant : entities) {
          if (plant.category != ENTITY_CATEGORY::PLANT) continue;
       float dx = plant.position.x - zombie.position.x;
            float dy = plant.position.y - zombie.position.y;
            float dist = sqrt(dx * dx + dy * dy);
   
      if (dist < 400.0f && dist > 0.0f) {
        targetPlant = &plant;
    break;
      }
        }
 
        if (targetPlant) {
            zombie.isUsingAbility = true;
  zombie.abilityClock.restart();
        zombie.DoAnimation(-1, 1);  // Ability animation
            zombie.x_vel = 0;  // Stop moving
         
            // Freeze the target plant
       targetPlant->freezeDuration = 3;  // 3 seconds
   targetPlant->frozenUntil = zombie.abilityClock.getElapsedTime().asSeconds() + 3;
     }
    }
    
    // END: Ability lasts 2 seconds
 if (zombie.isUsingAbility && 
   zombie.abilityClock.getElapsedTime().asSeconds() >= 2.0f) {
        zombie.isUsingAbility = false;
        zombie.x_vel = 0.05f;
     zombie.setCurrentAnimation(0);
    }
}
```

### Step 6: Apply Freeze Effect in DrawPlantsGrid()
```cpp
// At the START of plant attacking system (around line 1055)

// Check if plant is frozen
bool isFrozen = false;
if (plant.freezeDuration > 0) {
    if (plant.frozenUntil > plant.attackClock.getElapsedTime().asSeconds()) {
        isFrozen = true;
    } else {
        plant.freezeDuration = 0;  // Unfreeze
    }
}

// If frozen, skip attack logic
if (isFrozen) continue;  // ? Skip to next plant

// ... rest of plant attack logic ...
```

---

## Example 4: Egg Zombie that Splits

**Concept**: Zombie that splits into 2 smaller zombies when it dies

### Add to Enums
```cpp
enum class ENTITY_TYPE : uint8_t {
    // ... existing ...
    ZOMBIE_EGG,  // ? Egg zombie
    ZOMBIE_BABY, // ? Baby zombies it spawns
};

enum class TEXTURE_TYPE : uint8_t {
    // ... existing ...
  ZOMBIE_EGG,
    ZOMBIE_BABY,
};
```

### Create Definition Functions
```cpp
Entity defineBabyZombie() {
    textures[TEXTURE_TYPE::ZOMBIE_BABY][0].loadFromFile("assets/baby_zombie.png");
    textures[TEXTURE_TYPE::ZOMBIE_BABY][1].loadFromFile("assets/baby_zombie_death.png");
    
    Entity zombie{};
    zombie.category = ENTITY_CATEGORY::ZOMBIE;
    zombie.type = ENTITY_TYPE::ZOMBIE_BABY;
    zombie.health = 30;  // Low health
 zombie.attack = 5;    // Low attack
    zombie.x_vel = 0.05f;       // Normal speed
    zombie.scale = 0.6f;        // Small size
    
    zombie.num_animations = 2;
    zombie.animations[0] = {frames: 16, frameTimer: 0.1f};
    zombie.animations[1] = {frames: 30, frameTimer: 0.05f};
    zombie.setCurrentAnimation(0);
    return zombie;
}

Entity defineEggZombie() {
    textures[TEXTURE_TYPE::ZOMBIE_EGG][0].loadFromFile("assets/egg_zombie.png");
    textures[TEXTURE_TYPE::ZOMBIE_EGG][1].loadFromFile("assets/egg_zombie_death.png");
    
    Entity zombie{};
    zombie.category = ENTITY_CATEGORY::ZOMBIE;
    zombie.type = ENTITY_TYPE::ZOMBIE_EGG;
    zombie.health = 100;        // Medium health
    zombie.attack = 20;
    zombie.x_vel = 0.05f;
    zombie.scale = 1.2f;
    
    zombie.num_animations = 2;
    zombie.animations[0] = {frames: 18, frameTimer: 0.1f};
    zombie.animations[1] = {frames: 40, frameTimer: 0.06f};
    zombie.setCurrentAnimation(0);
    return zombie;
}
```

### Add Splitting Logic in DrawZombies() Death Handler
```cpp
// In DrawZombies(), in the death callback (around line 1175)
// BEFORE: if (zombie.type == ENTITY_TYPE::ZOMBIE_RA && zombie.storedSunValue > 0)

// ADD THIS:
if (zombie.type == ENTITY_TYPE::ZOMBIE_EGG) {
    // Split into 2 baby zombies
    for (int i = 0; i < 2; i++) {
        Entity babyZombie = defineBabyZombie();
        babyZombie.position = Vector2f(
  zombie.position.x + (i == 0 ? -50 : 50),
            zombie.position.y
        );
        babyZombie.id = nextEntityId++;
        entities.push_back(babyZombie);
    }
}
```

---

## Example 5: Explosive Plant Attack

**Concept**: Plant that shoots an area-of-effect explosive projectile

### Add to Enums
```cpp
enum class ENTITY_TYPE : uint8_t {
  // ... existing ...
    PLANT_EXPLOSIVE,  // ? ADD
  ExplosiveBullet,  // ? ADD (special bullet type)
};

enum class TEXTURE_TYPE : uint8_t {
    // ... existing ...
    PLANT_EXPLOSIVE,
    ExplosiveBullet,
};
```

### Create Definition Function
```cpp
Entity defineExplosivePlant() {
    textures[TEXTURE_TYPE::PLANT_EXPLOSIVE][0].loadFromFile("assets/explosive_plant.png");
    textures[TEXTURE_TYPE::PLANT_EXPLOSIVE][1].loadFromFile("assets/explosive_attack.png");
    
    Entity plant{};
    plant.category = ENTITY_CATEGORY::PLANT;
    plant.type = ENTITY_TYPE::PLANT_EXPLOSIVE;
    plant.health = 100;
    plant.attack = 40;  // Base damage
    plant.price = 150;
 plant.scale = 1.0f;
    
    plant.num_animations = 2;
    plant.animations[0] = {frames: 10, frameTimer: 0.1f};
    plant.animations[1] = {frames: 8, frameTimer: 0.08f};
    plant.setCurrentAnimation(0);
    
    return plant;
}

Entity defineExplosiveBullet() {
Texture tex;
    tex.loadFromFile("assets/explosive_bullet.png");
    textures[TEXTURE_TYPE::ExplosiveBullet][0] = tex;
    
    Entity bullet{};
    bullet.category = ENTITY_CATEGORY::Bullet;
    bullet.type = ENTITY_TYPE::ExplosiveBullet;
    bullet.attack = 100;         // Higher damage
    bullet.x_vel = -0.5f;        // Faster
    
    return bullet;
}
```

### Add Explosion Detection in UpdateAndDrawBullets()
```cpp
// In UpdateAndDrawBullets() (around line 1280)
// REPLACE the collision detection with:

for (Entity& zombie : entities) {
    if (zombie.category != ENTITY_CATEGORY::ZOMBIE || zombie.isDead) continue;
    
    // For explosive bullets: Check area of effect
    if (bullet.type == ENTITY_TYPE::ExplosiveBullet) {
   float dx = zombie.position.x - bullet.position.x;
        float dy = zombie.position.y - bullet.position.y;
        float dist = sqrt(dx * dx + dy * dy);
        
        if (dist < 150.0f) {  // 150 pixel radius
            zombie.health -= bullet.attack;
   bullet.category = ENTITY_CATEGORY::NONE;  // Bullet explodes
            break;
        }
    } else {
        // Normal bullet collision
        if (abs(bullet.position.y - zombie.position.y) < 100.0f &&
            bullet.position.x >= zombie.position.x &&
        bullet.position.x <= zombie.position.x + 100.0f) {
            zombie.health -= bullet.attack;
            bullet.category = ENTITY_CATEGORY::NONE;
       break;
        }
    }
}
```

### Add Firing Logic in DrawPlantsGrid()
```cpp
// After checking other plant types:

else if (plant.type == ENTITY_TYPE::PLANT_EXPLOSIVE) {
    if (plant.attackClock.getElapsedTime().asSeconds() >= 4.0f) {  // 4 second reload
    
        bool zombieInLane = false;
        for (Entity& zombie : entities) {
 if (zombie.category != ENTITY_CATEGORY::ZOMBIE) continue;
      if (zombie.health <= 0) continue;
            
       if (abs(zombie.position.y - plant.position.y) < 100.0f &&
  zombie.position.x > plant.position.x) {
      zombieInLane = true;
    break;
   }
 }
        
        if (zombieInLane) {
         plant.attackClock.restart();
            plant.DoAnimation(1, 1);  // Attack animation
            
 Entity newBullet = defineExplosiveBullet();
     newBullet.position = Vector2f(plant.position.x + 100, plant.position.y + 50);
   newBullets.push_back(newBullet);
        }
    }
}
```

---

## Example 6: Shield Plant (Blocks Zombies)

**Concept**: A plant that damages zombies passing through it

### Add to Enums
```cpp
enum class ENTITY_TYPE : uint8_t {
    // ... existing ...
    PLANT_SHIELD,  // ? ADD
};

enum class TEXTURE_TYPE : uint8_t {
    // ... existing ...
    PLANT_SHIELD,
};
```

### Create Definition Function
```cpp
Entity defineShieldPlant() {
    textures[TEXTURE_TYPE::PLANT_SHIELD][0].loadFromFile("assets/shield_plant.png");
    
    Entity plant{};
    plant.category = ENTITY_CATEGORY::PLANT;
    plant.type = ENTITY_TYPE::PLANT_SHIELD;
  plant.health = 300;    // Very tough
 plant.attack = 30;     // Contact damage
    plant.price = 100;
    plant.scale = 1.3f;
    
    plant.num_animations = 1;
    plant.animations[0] = {frames: 12, frameTimer: 0.1f};
    plant.setCurrentAnimation(0);
    
    return plant;
}
```

### Add Shield Logic in DrawZombies()
```cpp
// In the zombie movement/combat section (around line 1235):

// Check for shield plant
Entity& plant = getEntity({ r, c });

// Shield plant damages zombie on contact
if (plant.category == ENTITY_CATEGORY::PLANT && 
    plant.type == ENTITY_TYPE::PLANT_SHIELD) {
    
    zombie.health -= plant.attack;
    zombie.x_vel = 0;  // Stop
    
    if (plant.attackClock.getElapsedTime().asSeconds() >= plant.attackTimer) {
        plant.attackClock.restart();
      // Could add spark effect here
    }
} 
else if (plant.category == ENTITY_CATEGORY::PLANT) {
    // Normal plant combat
    zombie.x_vel = 0;
    if (zombie.attackClock.getElapsedTime().asSeconds() >= zombie.attackTimer) {
        zombie.attackClock.restart();
      plant.health -= zombie.attack;
  // ...
    }
}
```

---

## Example 7: Spike Plant - Stacking Damage

**Concept**: Plant that does more damage the more zombies attack it

### Create Definition Function
```cpp
// Add to Entity struct (one time):
int damageStacks = 0;  // For spike plant

Entity defineSpikePlant() {
 textures[TEXTURE_TYPE::PLANT_SPIKE][0].loadFromFile("assets/spike_plant.png");
    
    Entity plant{};
    plant.category = ENTITY_CATEGORY::PLANT;
 plant.type = ENTITY_TYPE::PLANT_SPIKE;
    plant.health = 150;
    plant.attack = 10;      // Base damage (stacks)
    plant.price = 80;
    plant.scale = 1.0f;
    
    plant.num_animations = 1;
    plant.animations[0] = {frames: 8, frameTimer: 0.1f};
  plant.setCurrentAnimation(0);
    
  plant.damageStacks = 0;  // Initialize
    
    return plant;
}
```

### Add Stacking Logic
```cpp
// In DrawZombies(), when zombie attacks plant:

if (plant.type == ENTITY_TYPE::PLANT_SPIKE && 
    plant.category == ENTITY_CATEGORY::PLANT) {
    
    if (plant.attackClock.getElapsedTime().asSeconds() >= plant.attackTimer) {
        plant.attackClock.restart();
        
        // Damage increases per stack
        int damageDealt = plant.attack * (1 + plant.damageStacks);
   zombie.health -= damageDealt;
        
 plant.damageStacks++;  // Add stack
        if (plant.damageStacks > 5) plant.damageStacks = 5;  // Cap at 5
    }
}
```

---

## Tips for Modification

### Adjust Difficulty
```cpp
// Easy mode: Reduce zombie health, increase plant damage
Entity zombie = defineNormalZombie();
zombie.health = 50;  // Lower

// Hard mode: Increase zombie stats
Entity zombie = defineNormalZombie();
zombie.health = 150;
zombie.attack = 20;
```

### Change Ability Duration
```cpp
// In ability end check:
if (entity.abilityClock.getElapsedTime().asSeconds() >= 5.0f) {  // Was 1.5f
    entity.isUsingAbility = false;
}
```

### Adjust Animation Speed
```cpp
// Faster animation
anim.frameTimer = 0.05f;  // Was 0.1f

// Slower animation
anim.frameTimer = 0.2f;   // Was 0.1f
```

### Change Attack Frequency
```cpp
// Attack more often
if (plant.attackClock.getElapsedTime().asSeconds() >= 2.0f) {  // Was 3.0f

// Attack less often
if (plant.attackClock.getElapsedTime().asSeconds() >= 5.0f) {  // Was 3.0f
```

---

**All examples use C++20 and SFML**
**Remember to add texture files to assets/ folder**

