#include <iostream>
#define SFML_STATIC
#include <SFML/Graphics.hpp>

using namespace sf;
using namespace std;

int main() {
	srand(time(0));

	RenderWindow window(VideoMode(800, 600), "Plants vs Zombies", Style::Titlebar | Style::Close);

	while (window.isOpen()) {
		Event e;
		while (window.pollEvent(e)) {
			if (e.type == Event::Closed) window.close();
		}

		window.display();
	}

	return 0;
}
