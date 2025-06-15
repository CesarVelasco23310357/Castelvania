#ifndef CENEMY_HPP
#define CENEMY_HPP

#include <string>
#include <SFML/Graphics.hpp>

enum class EnemyType {
    MURCIELAGO,
    ESQUELETO,
    ZOMBIE
};

class CEnemy {
private:
    // Atributos del enemigo
    std::string m_type;
    EnemyType m_enemyType;
    int m_health;
    int m_maxHealth;
    int m_damage;
    sf::Vector2f m_position;
    float m_speed;
    
    // IA y comportamiento
    float m_detectionRange;
    float m_attackRange;
    float m_attackCooldown;
    float m_currentCooldown;
    
    // Gráficos SFML
    sf::RectangleShape m_sprite;
    sf::Color m_color;
    sf::Color m_originalColor;
    
public:
    // Constructor
    CEnemy(EnemyType type, float x, float y);
    
    // Destructor
    ~CEnemy();
    
    // Getters
    const std::string& getType() const;
    EnemyType getEnemyType() const;
    int getHealth() const;
    int getMaxHealth() const;
    int getDamage() const;
    sf::Vector2f getPosition() const;
    float getSpeed() const;
    sf::FloatRect getBounds() const;
    float getDetectionRange() const;
    float getAttackRange() const;
    
    // Setters
    void setPosition(float x, float y);
    void setPosition(const sf::Vector2f& position);
    void setHealth(int health);
    
    // Métodos de gameplay
    void moveTowards(const sf::Vector2f& targetPosition, float deltaTime);
    int attack();
    void takeDamage(int damage);
    bool isAlive() const;
    bool canAttack() const;
    bool isInRange(const sf::Vector2f& targetPosition, float range) const;
    
    // IA básica
    void updateAI(const sf::Vector2f& playerPosition, float deltaTime);
    
    // Métodos SFML
    void update(float deltaTime);
    void render(sf::RenderWindow& window);
    
    // Debug
    void printStatus() const;
    
private:
    // Métodos privados para configurar tipos
    void setupEnemyType(EnemyType type);
    std::string enemyTypeToString(EnemyType type) const;
    float calculateDistance(const sf::Vector2f& position1, const sf::Vector2f& position2) const;
};

#endif // CENEMY_HPP