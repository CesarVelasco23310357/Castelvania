#ifndef CLEVEL_HPP
#define CLEVEL_HPP

#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>
#include "CEnemy.hpp"

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

class CLevel {
private:
    // Información del nivel
    int m_levelNumber;
    std::string m_levelName;
    LevelState m_state;
    
    // Dimensiones y límites
    sf::Vector2f m_levelSize;
    sf::FloatRect m_boundaries;
    
    // Enemigos y spawn points
    std::vector<std::unique_ptr<CEnemy>> m_enemies;
    std::vector<SpawnPoint> m_spawnPoints;
    
    // Tiempo y progreso
    float m_levelTime;
    float m_spawnTimer;
    int m_totalEnemies;
    int m_enemiesKilled;
    
    // Gráficos del nivel
    sf::RectangleShape m_background;
    sf::RectangleShape m_border;
    std::vector<sf::RectangleShape> m_obstacles;
    
    // Texturas y sprites para fondos
    sf::Texture m_layer1Texture;
    sf::Texture m_layer2Texture;
    sf::Sprite m_layer1Sprite;
    sf::Sprite m_layer2Sprite;
    bool m_texturesLoaded;
    
    // Configuración
    bool m_isLoaded;
    float m_completionTime;
    
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
    
    // Setters
    void setState(LevelState state);
    void setLevelSize(float width, float height);
    
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
    
    // Gestión de obstáculos
    void addObstacle(float x, float y, float width, float height);
    void clearObstacles();
    bool isPositionBlocked(const sf::Vector2f& position) const;
    
    // Verificaciones
    bool isPositionInBounds(const sf::Vector2f& position) const;
    void checkLevelCompletion();
    
    // Métodos SFML
    void update(float deltaTime, const sf::Vector2f& playerPosition);
    void render(sf::RenderWindow& window);
    
    // Debug
    void printLevelInfo() const;
    void printEnemyCount() const;
    
private:
    // Métodos privados de configuración
    void setupLevelConfiguration();
    void createLevelGeometry();
    void loadLevelTextures();  // NUEVO MÉTODO
    void spawnEnemiesFromPoints(float deltaTime);
    void updateEnemies(float deltaTime, const sf::Vector2f& playerPosition);
    void renderEnemies(sf::RenderWindow& window);
    void renderObstacles(sf::RenderWindow& window);
    std::string levelStateToString(LevelState state) const;
    
    // Configuraciones específicas por nivel
    void configureLevel1();
    void configureLevel2();
    void configureLevel3();
    void configureDefaultLevel();
};

#endif // CLEVEL_HPP