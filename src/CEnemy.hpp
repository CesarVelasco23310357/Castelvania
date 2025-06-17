#ifndef CENEMY_HPP
#define CENEMY_HPP

#include <string>
#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>  // ← NUEVO: Box2D

// Forward declaration
class CPhysics;

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
    
    // ===================================
    // NUEVO: Sistema de físicas
    // ===================================
    CPhysics* m_physics;              // Referencia al sistema de físicas
    b2Body* m_physicsBody;            // Cuerpo físico del enemigo
    bool m_physicsEnabled;            // Si las físicas están activas
    bool m_isGrounded;                // Si está en el suelo
    bool m_canFly;                    // Si puede volar (murciélagos)
    
    // IA y comportamiento
    float m_detectionRange;
    float m_attackRange;
    float m_attackCooldown;
    float m_currentCooldown;
    
    // ===================================
    // NUEVO: Comportamiento de IA con físicas
    // ===================================
    float m_jumpForce;                // Fuerza de salto para esqueletos/zombies
    float m_flyForce;                 // Fuerza de vuelo para murciélagos
    float m_movementForce;            // Fuerza de movimiento horizontal
    float m_lastDirectionChange;      // Timer para cambio de dirección
    int m_movementDirection;          // -1 = izquierda, 1 = derecha, 0 = parado
    
    // Gráficos SFML
    sf::RectangleShape m_sprite;
    sf::Color m_color;
    sf::Color m_originalColor;
    
    // ===================================
    // NUEVO: Configuración de físicas por tipo
    // ===================================
    static constexpr float MURCIELAGO_FLY_FORCE = 8.0f;
    static constexpr float ESQUELETO_JUMP_FORCE = 7.0f;
    static constexpr float ZOMBIE_MOVEMENT_FORCE = 5.0f;
    static constexpr float DEFAULT_MOVEMENT_FORCE = 6.0f;
    static constexpr float DIRECTION_CHANGE_TIME = 2.0f;  // Cambiar dirección cada 2 segundos
    
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
    
    // ===================================
    // NUEVO: Getters para físicas
    // ===================================
    bool isGrounded() const;
    bool canFly() const;
    b2Body* getPhysicsBody() const;
    sf::Vector2f getVelocity() const;
    int getMovementDirection() const;
    
    // Setters
    void setPosition(float x, float y);
    void setPosition(const sf::Vector2f& position);
    void setHealth(int health);
    
    // ===================================
    // NUEVO: Configuración de físicas
    // ===================================
    void initializePhysics(CPhysics* physics);  // Configurar físicas
    void updatePhysicsPosition();                // Sincronizar posición con físicas
    void syncPositionFromPhysics();              // Obtener posición de las físicas
    
    // Métodos de gameplay
    void moveTowards(const sf::Vector2f& targetPosition, float deltaTime);
    void moveWithPhysics(const sf::Vector2f& targetPosition, float deltaTime);  // ← NUEVO
    int attack();
    void takeDamage(int damage);
    bool isAlive() const;
    bool canAttack() const;
    bool isInRange(const sf::Vector2f& targetPosition, float range) const;
    
    // ===================================
    // NUEVO: Comportamiento con físicas
    // ===================================
    void jump();                                 // Saltar (esqueletos/zombies)
    void fly();                                  // Volar (murciélagos)
    void patrol();                               // Patrullar automáticamente
    void followTarget(const sf::Vector2f& target, float deltaTime);  // Seguir objetivo con físicas
    
    // IA básica
    void updateAI(const sf::Vector2f& playerPosition, float deltaTime);
    
    // Métodos SFML
    void update(float deltaTime);
    void render(sf::RenderWindow& window);
    
    // Debug
    void printStatus() const;
    void printPhysicsStatus() const;             // ← NUEVO: Debug de físicas
    
private:
    // Métodos privados para configurar tipos
    void setupEnemyType(EnemyType type);
    std::string enemyTypeToString(EnemyType type) const;
    float calculateDistance(const sf::Vector2f& position1, const sf::Vector2f& position2) const;
    
    // ===================================
    // NUEVO: Métodos de físicas privados
    // ===================================
    void setupPhysicsForType();                 // Configurar físicas según tipo
    void checkGroundState();                    // Verificar si está en el suelo
    void updatePhysicsState();                  // Actualizar estado según físicas
    void updateMovementDirection(float deltaTime);  // Actualizar dirección de patrullaje
    void applyMovementForce(float direction);   // Aplicar fuerza de movimiento
    void handleMurcieelagoAI(const sf::Vector2f& playerPosition, float deltaTime);  // IA específica para murciélagos
    void handleEsqueletoAI(const sf::Vector2f& playerPosition, float deltaTime);    // IA específica para esqueletos
    void handleZombieAI(const sf::Vector2f& playerPosition, float deltaTime);       // IA específica para zombies
};

#endif // CENEMY_HPP