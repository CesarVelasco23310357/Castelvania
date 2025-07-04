#ifndef CLEVEL_HPP
#define CLEVEL_HPP

#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>
#include "CEnemy.hpp"
#include <box2d/box2d.h>  // ← NUEVO: Box2D

// Forward declaration
class CPhysics;

enum class LevelState {
    LOADING,
    ACTIVE,
    COMPLETED,
    FAILED
};

struct SpawnPoint {
    sf::Vector2f position;
    EnemyType enemyType;
    float spawnTime;
    bool hasSpawned;
    
    SpawnPoint(float x, float y, EnemyType type, float time = 0.0f) 
        : position(x, y), enemyType(type), spawnTime(time), hasSpawned(false) {}
};

// ===================================
// NUEVO: Estructura para plataformas físicas
// ===================================
struct PhysicalPlatform {
    sf::RectangleShape shape;    // Representación visual de respaldo
    sf::Sprite floorSprite;      // ← NUEVO: Sprite con textura de floor
    b2Body* physicsBody;         // Cuerpo físico
    sf::Vector2f position;
    sf::Vector2f size;
    sf::Color color;
    bool hasTexture;             // ← NUEVO: Si tiene textura cargada
    
    PhysicalPlatform(float x, float y, float w, float h, sf::Color c = sf::Color::Green)
        : physicsBody(nullptr), position(x, y), size(w, h), color(c), hasTexture(false) {
        // Configurar shape de respaldo
        shape.setPosition(x, y);
        shape.setSize(sf::Vector2f(w, h));
        shape.setFillColor(c);
        shape.setOutlineThickness(2.0f);
        shape.setOutlineColor(sf::Color::Black);
    }
};
class CLevel {
private:
    // Información del nivel
    int levelNumber;
    std::string levelName;
    LevelState state;
    
    // ===================================
    // NUEVO: Sistema de físicas
    // ===================================
    CPhysics* physics;                         // Referencia al sistema de físicas
    std::vector<PhysicalPlatform> platforms;   // Plataformas con físicas
    std::vector<b2Body*> wallBodies;          // Muros invisibles (límites)
    
    // Dimensiones y límites
    sf::Vector2f levelSize;
    sf::FloatRect boundaries;
    
    // Enemigos y spawn points
    std::vector<std::unique_ptr<CEnemy>> enemies;
    std::vector<SpawnPoint> spawnPoints;
    
    // Tiempo y progreso
    float levelTime;
    float spawnTimer;
    int totalEnemies;
    int enemiesKilled;
    
    // Gráficos del nivel
    sf::RectangleShape background;
    sf::RectangleShape border;
    std::vector<sf::RectangleShape> obstacles;  // Obstáculos visuales (sin físicas)
    
    // Texturas y sprites para fondos
    sf::Texture layer1Texture;
    sf::Texture layer2Texture;
    sf::Sprite layer1Sprite;
    sf::Sprite layer2Sprite;
    sf::Texture floorTexture;
    bool texturesLoaded;
    
    // Configuración
    bool loaded;
    float completionTime;
    
public:
    // Constructor y destructor
    CLevel(int levelNumber);
    ~CLevel();
    
    // Getters
    int getLevelNumber() const;
    const std::string& getLevelName() const;
    LevelState getState() const;
    sf::Vector2f getLevelSize() const;
    sf::FloatRect getBoundaries() const;
    float getLevelTime() const;
    int getTotalEnemies() const;
    int getEnemiesKilled() const;
    int getEnemiesAlive() const;
    float getCompletionPercentage() const;
    bool isLoaded() const;
    bool isCompleted() const;
    
    // ===================================
    // NUEVO: Getters para físicas
    // ===================================
    const std::vector<PhysicalPlatform>& getPlatforms() const;
    size_t getPlatformCount() const;
    
    // Setters
    void setState(LevelState state);
    void setLevelSize(float width, float height);
    
    // ===================================
    // NUEVO: Configuración de físicas
    // ===================================
    void initializePhysics(CPhysics* physics);   // Configurar físicas del nivel
    void createPhysicalPlatforms();              // Crear plataformas con físicas
    void createLevelBoundaries();                // Crear límites invisibles
    
    // Gestión del nivel
    void loadLevel();
    void unloadLevel();
    void resetLevel();
    void startLevel();
    
    
    // Gestión de enemigos
    void addEnemy(EnemyType type, float x, float y);
    void addSpawnPoint(float x, float y, EnemyType type, float spawnTime = 0.0f);
    void removeDeadEnemies();
    CEnemy* getClosestEnemyToPosition(const sf::Vector2f& position, float maxRange = -1.0f);
    
    // ===================================
    // NUEVO: Gestión de plataformas físicas
    // ===================================
    void addPhysicalPlatform(float x, float y, float width, float height, sf::Color color = sf::Color::Green);
    void clearPhysicalPlatforms();
    
    // Gestión de obstáculos (solo visuales, sin físicas)
    void addObstacle(float x, float y, float width, float height);
    void clearObstacles();
    bool isPositionBlocked(const sf::Vector2f& position) const;
    // Verificaciones
    bool isPositionInBounds(const sf::Vector2f& position) const;
    void checkLevelCompletion();
    
    // Métodos SFML
    void update(float deltaTime, const sf::Vector2f& playerPosition);
    void render(sf::RenderWindow& window);
    void adjustPlatformThickness(float deltaThickness);  // ← NUEVA
    // ===================================
    // NUEVO: Renderizado específico
    // ===================================
    void renderPlatforms(sf::RenderWindow& window);  // Renderizar plataformas físicas
    
    // Debug
    void printLevelInfo() const;
    void printEnemyCount() const;
    void printPhysicsInfo() const;               // ← NUEVO: Info de físicas del nivel
    
private:
    // Métodos privados de configuración
    void setupLevelConfiguration();
    void createLevelGeometry();
    void loadLevelTextures();
    void spawnEnemiesFromPoints(float deltaTime);
    void updateEnemies(float deltaTime, const sf::Vector2f& playerPosition);
    void renderEnemies(sf::RenderWindow& window);
    void renderObstacles(sf::RenderWindow& window);
    std::string levelStateToString(LevelState state) const;
    
    // ===================================
    // NUEVO: Métodos privados de físicas
    // ===================================
    void setupPhysicalPlatformsForLevel();       // Configurar plataformas específicas del nivel
    void destroyPhysicalPlatforms();             // Destruir plataformas físicas
    void destroyLevelBoundaries();               // Destruir límites
    
    // Configuraciones específicas por nivel (actualizadas)
    void configureLevel1();
    void configureLevel2();
    void configureLevel3();
    void configureDefaultLevel();
    
    // ===================================
    // NUEVO: Configuraciones de plataformas por nivel
    // ===================================
    void configurePlatformsLevel1();
    void configurePlatformsLevel2();
    void configurePlatformsLevel3();
};

#endif // CLEVEL_HPP