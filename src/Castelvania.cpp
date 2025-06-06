#include <iostream>
#include <string>
#include <vector>

// Clase para el jugador
class Player {
  public:
    std::string name;
    int health;
    int x, y;  // posición

    Player(const std::string& name) : name(name), health(100), x(0), y(0) {}

    void move(int dx, int dy) {
        x += dx;
        y += dy;
    }

    void attack() {
        // Lógica de ataque
    }
};

// Clase para enemigos
class Enemy {
  public:
    std::string type;
    int health;
    int x, y;

    Enemy(const std::string& type, int x, int y) : type(type), health(50), x(x), y(y) {}

    void move() {
        // Lógica de movimiento enemigo
    }
};

// Clase para el nivel
class Level {
  public:
    int number;
    std::vector<Enemy> enemies;

    Level(int num) : number(num) {}

    void load() {
        // Cargar enemigos y obstáculos
    }
};

// Clase principal del juego
class Game {
  public:
    Player player;
    std::vector<Level> levels;
    int currentLevel;

    Game(const std::string& playerName) : player(playerName), currentLevel(0) {}

    void start() {
        // Inicializar niveles, jugador, etc.
    }

    void update() {
        // Actualizar estado del juego
    }
};

int main() {
    std::cout << "Bienvenido a Castelvania!" << std::endl;
    // Inicialización del juego
    // Bucle principal del juego
    // Procesamiento de entrada, actualización y renderizado

    // Por ahora, solo muestra un mensaje y termina.
    return 0;
}
