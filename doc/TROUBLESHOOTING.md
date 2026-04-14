# Troubleshooting & FAQ Guide

**Solutions to common issues and questions**

---

## Animation Issues

### Problem: "Animation not playing / Stuck on first frame"

**Possible Causes & Solutions:**

1. **`num_animations` not set**
   ```cpp
   // ? WRONG
   Entity zombie;
   zombie.animations[0] = myAnimation;
   entities.push_back(zombie);
   
   // ? CORRECT
   Entity zombie;
   zombie.num_animations = 1;  // Must set this FIRST
   zombie.animations[0] = myAnimation;
   entities.push_back(zombie);
   ```

2. **Texture not loaded before entity creation**
   ```cpp
   // ? WRONG
   Entity zombie = defineZombie();
   textures[TEXTURE_TYPE::ZOMBIE][0].loadFromFile("assets/zombie.png");
   
   // ? CORRECT
   textures[TEXTURE_TYPE::ZOMBIE][0].loadFromFile("assets/zombie.png");
   Entity zombie = defineZombie();
   ```

3. **`currentAnimationIndex` out of bounds**
   ```cpp
   // Verify currentAnimationIndex < num_animations
   if (entity.currentAnimationIndex >= entity.num_animations) {
       cout << "ERROR: Animation index out of bounds!" << endl;
   }
   ```

4. **Texture rect calculation wrong**
   ```cpp
   // Verify frame width is calculated correctly
 float frameWidthF = tex.getSize().x / entity.currentAnimation.frames;
   cout << "Texture width: " << tex.getSize().x 
  << ", Frames: " << entity.currentAnimation.frames
   << ", Frame width: " << frameWidthF << endl;
   ```

5. **Animation clock not progressing**
   ```cpp
   // Ensure nextFrame() is called every update
   // Location: Should be in DrawZombies, DrawPlantsGrid, etc.
   
   entity.nextFrame();  // Must call this in update loop
   ```

---

### Problem: "Animation plays once then stops"

**Cause**: `AnimationForCycles` is set to 1, not looping

```cpp
// ? WRONG - Plays once, stops
entity.DoAnimation(1, 0);

// ? CORRECT - Loops infinitely
entity.DoAnimation(-1, 0);

// ? ALSO CORRECT - Play 5 times, then stop
entity.DoAnimation(5, 0);
```

---

### Problem: "Animation jerky / skipping frames"

**Possible Causes:**

1. **Frame timer too short**
   ```cpp
   // ? Might be too fast
   anim.frameTimer = 0.01f;
   
   // ? Try normal speed
   anim.frameTimer = 0.1f;
   ```

2. **Computer can't keep up with complex logic**
   - Optimize by reducing entity checks per frame
   - Use `continue` to skip dead entities early
   - Reduce particle effects if any

---

### Problem: "Callback animation doesn't chain properly"

**Cause**: Callback not properly calling next animation

```cpp
// ? WRONG
entity.DoAnimation(1, 1);
entity.afterAnimation = [](Entity& e) {
    // Callback might not execute properly
    e.DoAnimation(-1, 0);
};

// ? CORRECT - Store reference to entity
Entity& myEntity = ...;  // Reference to actual entity
myEntity.DoAnimation(1, 1);
myEntity.afterAnimation = [&myEntity](Entity& e) {
    myEntity.DoAnimation(-1, 0);  // Use reference
};
```

---

## Ability System Issues

### Problem: "Ability won't trigger / Ability never starts"

**Checklist:**

```cpp
// 1. Is isUsingAbility initialized?
if (entity.isUsingAbility == false) {  // ? Use false, not uninitialized
    // Good
}

// 2. Is the condition checking multiple things?
if (!entity.isUsingAbility &&      // Not already using
    someCondition() &&  // Your condition
    !entity.isDead) {  // Entity alive
    entity.isUsingAbility = true;
    entity.abilityClock.restart();  // ? MUST restart clock
}

// 3. Did you restart the clock?
entity.abilityClock.restart();  // ? CRITICAL

// 4. Is the ability logic in the right place?
// Must be in DrawZombies() or DrawPlantsGrid()
// During entity loop, not outside
```

---

### Problem: "Ability won't stop / Runs forever"

**Cause**: Missing or wrong end condition

```cpp
// ? WRONG - No way to stop
if (entity.isUsingAbility) {
    doAbilityBehavior();
    // Never sets isUsingAbility = false
}

// ? CORRECT - Has exit condition
if (entity.isUsingAbility) {
    doAbilityBehavior();
    
    // Check end condition
    if (entity.abilityClock.getElapsedTime().asSeconds() >= entity.abilityDuration) {
    entity.isUsingAbility = false;  // ? Stop ability
 // ... cleanup ...
    }
}
```

---

### Problem: "Ability triggers too often / Only once"

**Issue**: Clock management

```cpp
// ? WRONG - Triggers every frame
if (someCondition()) {
    entity.isUsingAbility = true;
    // Never set to false, so condition always true
}

// ? CORRECT - Triggers once
if (!entity.isUsingAbility && someCondition()) {
    entity.isUsingAbility = true;
 entity.abilityClock.restart();
}

// ? ALSO CORRECT - Cooldown between abilities
if (entity.abilityClock.getElapsedTime().asSeconds() >= COOLDOWN && someCondition()) {
    entity.abilityClock.restart();
    startAbility();
}
```

---

### Problem: "Ability affects wrong character"

**Cause**: Wrong entity reference

```cpp
// ? WRONG - Reference might be stale
Entity* target = &zombie;
entity.afterAnimation = [target](Entity& e) {
    target->health -= 10;  // Might not be valid
};

// ? CORRECT - Use vector index
int targetIndex = &zombie - &entities[0];
entity.afterAnimation = [targetIndex](Entity& e) {
    entities[targetIndex].health -= 10;
};

// ? BETTER - Process immediately
for (Entity& zombie : entities) {
    if (shouldFreeze) {
        zombie.freezeDuration = 3;  // Direct modification
    }
}
```

---

## Texture & Sprite Issues

### Problem: "Texture shows completely white / No texture"

**Causes & Solutions:**

1. **Texture not loaded**
   ```cpp
   // Check if load succeeded
if (!textures[TEXTURE_TYPE::MY_TYPE][0].loadFromFile("assets/file.png")) {
       cout << "Failed to load texture!" << endl;
}
   ```

2. **File path wrong**
   ```cpp
   // ? WRONG
   "assets/my_sprite.png"  // Might not exist
   
   // ? DEBUG
   cout << "Current working directory: " << filesystem::current_path() << endl;
   ```

3. **Texture[NONE] used**
   ```cpp
   // Verify getTextureType() returns valid type, not NONE
   TEXTURE_TYPE type = getTextureType(entity.type);
   if (type == TEXTURE_TYPE::NONE) {
   cout << "ERROR: Texture type is NONE!" << endl;
   }
   ```

4. **Texture overwritten**
   ```cpp
   // Make sure texture assignments don't conflict
   textures[TEXTURE_TYPE::ZOMBIE][0].loadFromFile("a.png");
   textures[TEXTURE_TYPE::ZOMBIE][0].loadFromFile("b.png");  // Overwrites!
   ```

---

### Problem: "Sprite cuts off / Texture rect wrong"

**Check frame calculation:**

```cpp
Texture& tex = textures[TEXTURE_TYPE::ZOMBIE][0];
int totalWidth = tex.getSize().x;
int frameCount = entity.currentAnimation.frames;
int frameWidth = totalWidth / frameCount;

cout << "Total: " << totalWidth 
     << " / Frames: " << frameCount 
     << " = Width: " << frameWidth << endl;

// Should be evenly divisible
if (totalWidth % frameCount != 0) {
    cout << "WARNING: Texture width not evenly divisible by frame count!" << endl;
}
```

---

### Problem: "Texture flickers / Switches randomly"

**Cause**: Wrong texture source during animation

```cpp
// ? WRONG
Texture& tex = textures[getTextureType(entity.type)][0];  // Always [0]

// ? CORRECT
Texture& tex = textures[getTextureType(entity.type)][entity.currentAnimationIndex];
```

---

## Entity/Character Issues

### Problem: "New character doesn't appear in game"

**Checklist:**

- [ ] Added to `ENTITY_TYPE` enum?
- [ ] Added to `TEXTURE_TYPE` enum?
- [ ] Created `define*()` function?
- [ ] Added case to `getTextureType()` switch?
- [ ] Created instance in `main()`?
- [ ] Added to level: `levels[X].zombie_types.push_back(...)`?
- [ ] Level is being played?

```cpp
// Test if character is in entities vector
for (Entity& e : entities) {
  if (e.type == ENTITY_TYPE::MY_NEW_CHARACTER) {
 cout << "Found my character!" << endl;
    }
}
```

---

### Problem: "Character freezes in place / Doesn't move"

**Possible Causes:**

1. **x_vel is 0**
   ```cpp
   if (zombie.x_vel == 0) {
   cout << "ERROR: x_vel is 0!" << endl;
   }
```

2. **Movement code missing**
   ```cpp
   // In DrawZombies(), must have:
   zombie.position.x -= zombie.x_vel;  // Move left
   ```

3. **Movement disabled by ability**
   ```cpp
   // Check if ability sets x_vel to 0
   if (zombie.isUsingAbility) {
       zombie.x_vel = 0;  // Ability might freeze
   }
   ```

---

### Problem: "Health doesn't decrease / Character immortal"

**Causes:**

1. **Collision not detected**
   ```cpp
   // Verify collision logic
   float dist = abs(zombie.position.y - plant.position.y);
   if (dist < 100.0f) {
       cout << "In range: " << dist << endl;
   } else {
       cout << "Out of range: " << dist << endl;
   }
   ```

2. **Damage amount is 0**
   ```cpp
   if (entity.attack == 0) {
       cout << "ERROR: Attack is 0!" << endl;
   }
   ```

3. **Health cap**
   ```cpp
   // Some plants have health cap
   if (other.health > 200) other.health = 200;  // Might prevent damage
   ```

---

### Problem: "Character appears in wrong position"

**Check grid calculation:**

```cpp
// Zombie position should be around:
int startX = windowWidth;  // Right side
int startY = offsetY + (row * cellheight);

// Plant position should be:
int plantX = offsetX + (col * cellWidth) + (cellWidth - plantWidth) / 2;
int plantY = offsetY + (row * cellHeight) + (cellHeight - plantHeight);

cout << "Expected plant at: (" << plantX << ", " << plantY << ")" << endl;
cout << "Actual plant at: (" << plant.position.x << ", " << plant.position.y << ")" << endl;
```

---

## Performance Issues

### Problem: "Game lags / FPS drops"

**Optimization checklist:**

```cpp
// 1. Skip dead entities early
for (Entity& e : entities) {
if (e.category == ENTITY_CATEGORY::NONE) continue;  // ? Add this
    if (e.isDead) continue;  // ? Add this
    // Process entity
}

// 2. Cache texture lookups
Texture& tex = textures[getTextureType(entity.type)][entity.currentAnimationIndex];
// Don't recalculate in nested loops

// 3. Reduce vector searches
// Instead of: for each entity, search through all other entities
// Try: Keep nearby entity references

// 4. Profile what's slow
Clock profileClock;
profileClock.restart();
// ... code to test ...
cout << "Took: " << profileClock.getElapsedTime().asMilliseconds() << "ms" << endl;
```

---

### Problem: "Memory usage increases over time"

**Likely cause**: Dead entities not removed

```cpp
// In game loop, periodically clean up
// Remove entities with category == NONE

// Option 1: Periodic cleanup
static int cleanupCounter = 0;
if (++cleanupCounter > 100) {
    entities.erase(
        remove_if(entities.begin(), entities.end(),
         [](const Entity& e) { return e.category == ENTITY_CATEGORY::NONE; }),
        entities.end()
    );
    cleanupCounter = 0;
}

// Option 2: Check vector size
cout << "Entity count: " << entities.size() << endl;
if (entities.size() > 500) {
  cout << "WARNING: Too many entities!" << endl;
}
```

---

## Input/UI Issues

### Problem: "Can't place plant / UI not responding"

**Check input handling:**

```cpp
// Verify mouse position is correct
cout << "Mouse: (" << mousePos.x << ", " << mousePos.y << ")" << endl;

// Check if uiInteracted flag is being reset
// In UpdateLevel():
uiInteracted = false;  // ? Should be reset each frame

// Check if grid cell is valid
auto [row, col] = gridCell(mousePos);
cout << "Grid cell: (" << row << ", " << col << ")" << endl;
if (row < 0 || row >= gridRows) cout << "Out of bounds!" << endl;
```

---

## Spawn/Wave Issues

### Problem: "Zombies not spawning"

**Checklist:**

1. **Level is selected**
   ```cpp
   if (currentLevel == nullptr) {
     cout << "ERROR: No level selected!" << endl;
   }
   ```

2. **Clock time passed**
   ```cpp
   cout << "Zombie spawn time: " << zombieClock.getElapsedTime().asSeconds()
  << " / " << nextZombieTimer << endl;
   ```

3. **Level has zombie_types**
   ```cpp
 if (currentLevel->zombie_types.empty()) {
    cout << "ERROR: Level has no zombie types!" << endl;
   }
   ```

---

### Problem: "Too many/few zombies spawning"

**Adjust spawn timers:**

```cpp
// In CheckTimer_AddZombie():
float nextZombieTimer = ZombieMinGapTime + rand() % 10;

// To spawn faster (less than 3 seconds):
nextZombieTimer = 1.0f + rand() % 3;

// To spawn slower (more than 10 seconds):
nextZombieTimer = 15.0f + rand() % 10;
```

---

## Level Issues

### Problem: "Level doesn't start when clicking Play"

**Debug:**

```cpp
// In DrawMainMenu(), check if startLevel called
if (mouseJustClicked[0]) {
    cout << "Play button clicked, starting level " << selectedLevel << endl;
}

// In startLevel():
cout << "Level " << level << " started" << endl;
cout << "Zombies: " << currentLevel->zombie_types.size() << endl;
cout << "Plants: " << currentLevel->plant_types.size() << endl;
```

---

### Problem: "Wrong plants/zombies in level"

**Check level configuration:**

```cpp
// In main(), verify levels are set up
levels[1].zombie_types = { zombie, zombieJapanese, zombieBig };
levels[1].plant_types = { sunflower, repeaterPlant, wallnut };

cout << "Level 1 has " << levels[1].zombie_types.size() << " zombie types" << endl;
cout << "Level 1 has " << levels[1].plant_types.size() << " plant types" << endl;
```

---

## Miscellaneous

### Problem: "Game crashes when exiting"

**Known issue**: SFML texture deletion order

```cpp
// Already handled in code:
delete font;         // ? Delete early
textures.clear();    // ? Clear map
```

If still crashing:
- Check for dangling pointers
- Ensure all textures deleted before window closes

---

### Problem: "Random crashes with no error message"

**Debug strategies:**

```cpp
// 1. Add safety checks
if (!entity.category) {
    cout << "WARNING: Entity category is invalid" << endl;
    return;  // Skip
}

// 2. Add bounds checking
if (entity.currentAnimationIndex >= entity.num_animations) {
cout << "ERROR: Animation index out of bounds: " 
         << entity.currentAnimationIndex << " >= " << entity.num_animations << endl;
    entity.currentAnimationIndex = 0;  // Reset
}

// 3. Validate before dereferencing pointers
if (currentLevel != nullptr) {
    // Safe to use currentLevel
}

// 4. Run in debugger
// Visual Studio: F5 to run with debugger
// Check "Debug Output" window for more info
```

---

### Problem: "Inconsistent behavior / Sometimes works, sometimes doesn't"

**Possible causes:**

1. **Uninitialized variables**
   ```cpp
   // Always initialize
   Entity e{};  // Uses default constructor
   // Better than: Entity e;  // Garbage values
   ```

2. **Random behavior**
   ```cpp
   // If using rand(), behavior is random
   if (rand() % 100 < 10) { ... }  // 10% chance
   
   // For testing, use fixed values
   if (5 < 10) { ... }  // Always true
   ```

3. **Timing-dependent bugs**
   ```cpp
   // Use stable clock checks
   cout << "Elapsed: " << clock.getElapsedTime().asSeconds() << endl;
   ```

---

## Quick Debugging Setup

```cpp
// Add to top of main.cpp for easy toggling
#define DEBUG_MODE 1

#if DEBUG_MODE
#define DEBUG_LOG(x) cout << x << endl
#else
#define DEBUG_LOG(x)
#endif

// Usage:
DEBUG_LOG("Zombie spawned at " << zombie.position.x);
```

---

## Common Error Messages & Solutions

| Error | Cause | Fix |
|-------|-------|-----|
| Assertion failed: `m_texture != NULL` | Sprite texture not set | Call `sprite.setTexture(tex)` |
| Access violation | Dereferencing null pointer | Add null check before use |
| Texture not found | File path wrong or asset missing | Verify file exists at path |
| Animation index out of bounds | Using animation that doesn't exist | Check `currentAnimationIndex < num_animations` |
| Memory leak | Entities never removed | Implement cleanup logic |
| Freeze/Hang | Infinite loop | Check loop exit conditions |

---

**Remember:**
- Always check that variables are initialized
- Always restart clocks when starting timers
- Always verify texture loads succeeded
- Always validate indices before using arrays
- Use `cout` liberally for debugging
- Test one thing at a time

