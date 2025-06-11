#include <iostream>
#include <string>
#include <vector>
#include <limits>

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
        // Lógica de ataque (placeholder)
        std::cout << name << " ataca!\n";
    }

    void showStatus() const {
        std::cout << "Jugador: " << name << " | Salud: " << health << " | Posicion: (" << x << "," << y << ")\n";
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
        // Lógica de movimiento enemigo (placeholder)
    }

    void showStatus() const {
        std::cout << "Enemigo: " << type << " | Salud: " << health << " | Posicion: (" << x << "," << y << ")\n";
    }
};

// Clase para el nivel
class Level {
public:
    int number;
    std::vector<Enemy> enemies;

    Level(int num) : number(num) {}

    void load() {
        // Cargar enemigos y obstáculos (ejemplo simple)
        enemies.clear();
        enemies.emplace_back("Murcielago", 2, 2);
        enemies.emplace_back("Esqueleto", 4, 1);
    }

    void showEnemies() const {
        for (const auto& enemy : enemies) {
            enemy.showStatus();
        }
    }
};

// Clase principal del juego
class Game {
public:
    Player player;
    std::vector<Level> levels;
    int currentLevel;

    Game(const std::string& playerName) : player(playerName), currentLevel(0) {
        // Crear niveles de ejemplo
        levels.emplace_back(1);
        levels.emplace_back(2);
    }

    void start() {
        // Inicializar niveles, jugador, etc.
        for (auto& level : levels) {
            level.load();
        }
        currentLevel = 0;
        std::cout << "Juego iniciado. ¡Buena suerte, " << player.name << "!\n";
    }

    void showState() {
        std::cout << "\n--- Estado del Juego ---\n";
        // Previene acceso fuera de rango
        if (currentLevel >= 0 && currentLevel < levels.size()) {
            std::cout << "Nivel actual: " << levels[currentLevel].number << "\n";
            player.showStatus();
            levels[currentLevel].showEnemies();
        } else {
            std::cout << "Nivel actual inválido!\n";
        }
        std::cout << "-----------------------\n";
    }

    void update(char input) {
        // Procesar entrada simple: w/a/s/d para mover, q para salir, f para atacar
        switch (input) {
            case 'w': player.move(0, 1); break;
            case 's': player.move(0, -1); break;
            case 'a': player.move(-1, 0); break;
            case 'd': player.move(1, 0); break;
            case 'f': player.attack(); break;
            default: break;
        }
    }
};

int main() {
    std::cout << "Bienvenido a Castelvania!" << std::endl;
    std::string nombre;
    std::cout << "Ingresa tu nombre: ";
    std::getline(std::cin, nombre);

    Game game(nombre);
    game.start();

    char input;
    bool running = true;
    while (running) {
        game.showState();
        std::cout << "Comandos: w/a/s/d = mover, f = atacar, q = salir\n";
        std::cout << "Tu accion: ";
        std::cin >> input;
        if (input == 'q') {
            running = false;
            std::cout << "Gracias por jugar!\n";
        } else {
            game.update(input);
        }
        // Limpiar buffer de entrada
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    return 0;
}
