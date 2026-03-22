#include <iostream>
#define SFML_STATIC
#include <SFML/Graphics.hpp>

#define MAX_ROWS 5
#define MAX_COLS 9
#define MAX_ENTITIES (MAX_ROWS * MAX_COLS)

#define ENTITY_TYPES 1

using namespace sf;
using namespace std;

enum class ENTITY_TYPE : uint8_t {
	ZOMBIE = 0
};

struct Entity {
	ENTITY_TYPE type;
	float health;
	Vector2f position;
	uint64_t creationTime;
};

const string TEXTURE_PATH[ENTITY_TYPES] = {
	"assets/zombie.png"
};

Entity createEntity(ENTITY_TYPE type) {
	Entity entity{};
	entity.type = type;
	entity.creationTime = time(0);

	switch (type) {
	case ENTITY_TYPE::ZOMBIE:
		entity.health = 100.0f;
		break;
	}

	return entity;
}

int main() {
	srand(time(0));

	RenderWindow window(VideoMode(800, 600), "Plants vs Zombies", Style::Titlebar | Style::Close);

	Entity entities[MAX_ENTITIES] = {};
	Texture textures[1];

	for (int i = 0; i < ENTITY_TYPES; i++) {
		string path = TEXTURE_PATH[i];

		if (!textures[i].loadFromFile(path)) {
			window.close();
			return 1;
		}
	}

	Entity testZombie = createEntity(ENTITY_TYPE::ZOMBIE);
	testZombie.position = Vector2f(50, 50);
	entities[0] = testZombie;

	Sprite entitySprite;

	while (window.isOpen()) {
		Event e;
		while (window.pollEvent(e)) {
			if (e.type == Event::Closed) window.close();
		}

		for (int i = 0; i < MAX_ENTITIES; i++) {
			Entity& entity = entities[i];
			if (entity.health <= 0) continue;

			entitySprite.setTexture(textures[int(entity.type)]);
			entitySprite.setPosition(entity.position);
			entitySprite.setScale(
				128 / entitySprite.getLocalBounds().getSize().x,
				128 / entitySprite.getLocalBounds().getSize().y
			);

			window.draw(entitySprite);
		}

		window.display();
	}

	return 0;
}
