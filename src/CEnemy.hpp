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

// ===================================
// NUEVO: Estados de animación para enemigos
// ===================================
enum class EnemyState {
    IDLE,      // Parado (frame único)
    MOVING     // En movimiento (animación)
};

class CEnemy {
private:
    // Atributos del enemigo
    std::string type;
    EnemyType enemyType;
    int health;
    int maxHealth;
    int damage;
    sf::Vector2f position;
    float speed;
    
    // ===================================
    // Sistema de físicas
    // ===================================
    CPhysics* physics;             
    b2Body* physicsBody;            
    bool physicsEnabled;            
    bool grounded;                // ← Cambiado para evitar conflicto con getter isGrounded()
    bool flyCapable;              // ← Cambiado para evitar conflicto con getter canFly()
    
    // IA y comportamiento
    float detectionRange;
    float attackRange;
    float attackCooldown;
    float currentCooldown;
    
    // ===================================
    // Comportamiento de IA con físicas
    // ===================================
    float jumpForce;               
    float flyForce;               
    float movementForce;            
    float lastDirectionChange;      
    int movementDirection;          
    
    // Gráficos SFML (fallback)
    sf::RectangleShape sprite;
    sf::Color color;
    sf::Color originalColor;
    
    // ===================================
    // NUEVO: Sistema de sprites y animación
    // ===================================
    sf::Texture enemyTexture;      
    sf::Sprite enemySprite;          
    bool texturesLoaded;             
    
    // Sistema de animación
    EnemyState currentState;       
    int currentFrame;                
    float animationTimer;            
    float animationSpeed;            
    bool moving;                   // ← Cambiado para evitar conflicto con getter isMoving()
    
    // ===================================
    // CONFIGURACIÓN DE SPRITES POR TIPO DE ENEMIGO
    // ===================================
    
    // ZOMBIE - zombie.png
    // Fila 1: 1 frame IDLE, Fila 2: 4 frames MOVING
struct ZombieSprites {
    static const int IDLE_START_X = 45;
    static const int IDLE_START_Y = 0;
    static const int IDLE_FRAME_COUNT = 1;
    static const int IDLE_FRAME_WIDTH = 177;
    static const int IDLE_FRAME_HEIGHT = 158;
    
    static const int MOVING_START_X = 0;
    static const int MOVING_START_Y = 158;
    static const int MOVING_FRAME_COUNT = 4;
    static const int MOVING_FRAME_WIDTH = 174;
    static const int MOVING_FRAME_HEIGHT = 158;
    
    // ✨ NUEVO: Factor de escalado
    static constexpr float SCALE_X = 0.9f;  // 20% del tamaño original (más pequeño)
    static constexpr float SCALE_Y = 0.9f;  // 20% del tamaño original
};
    
    // SKELETON - skeleton.png  
    // Fila 1: 1 frame IDLE, Fila 2: 5 frames MOVING
    struct SkeletonSprites {
    static const int IDLE_START_X = 40;
    static const int IDLE_START_Y = 0;
    static const int IDLE_FRAME_COUNT = 1;
    static const int IDLE_FRAME_WIDTH = 550;
    static const int IDLE_FRAME_HEIGHT = 186;
    
    static const int MOVING_START_X = 40;
    static const int MOVING_START_Y = 186;
    static const int MOVING_FRAME_COUNT = 5;
    static const int MOVING_FRAME_WIDTH = 130;
    static const int MOVING_FRAME_HEIGHT = 186;
    
    // ✨ NUEVO: Factor de escalado (más pequeño porque está muy grande)
    static constexpr float SCALE_X = 0.7f;  // 15% del tamaño original
    static constexpr float SCALE_Y = 0.7f;  // 15% del tamaño original
};
    
    // MURCIELAGO - murcielago.png
    // Solo 1 fila: 5 frames MOVING (siempre en movimiento)
struct MurcielagoSprites {
    static const int IDLE_START_X = 0;
    static const int IDLE_START_Y = 0;
    static const int IDLE_FRAME_COUNT = 5;
    static const int IDLE_FRAME_WIDTH = 106;
    static const int IDLE_FRAME_HEIGHT = 127;
    
    static const int MOVING_START_X = 0;
    static const int MOVING_START_Y = 0;
    static const int MOVING_FRAME_COUNT = 5;
    static const int MOVING_FRAME_WIDTH = 106;
    static const int MOVING_FRAME_HEIGHT = 127;
    
    // ✨ NUEVO: Factor de escalado
    static constexpr float SCALE_X = 1.0f;  // 30% del tamaño original
    static constexpr float SCALE_Y = 1.0f;  // 30% del tamaño original
};
    
    // Velocidades de animación (más alto = más lento)
    static constexpr float ZOMBIE_ANIMATION_SPEED = 0.3f;     // Lento
    static constexpr float SKELETON_ANIMATION_SPEED = 0.2f;   // Medio
    static constexpr float MURCIELAGO_ANIMATION_SPEED = 0.15f; // Rápido
    
    // ===================================
    // Configuración de físicas por tipo
    // ===================================
     static constexpr float MURCIELAGO_FLY_FORCE = 12.0f;        // Aumentado de 8.0f
    static constexpr float ESQUELETO_JUMP_FORCE = 9.0f;         // Aumentado de 7.0f
    static constexpr float ZOMBIE_MOVEMENT_FORCE = 8.0f;        // Aumentado de 5.0f
    static constexpr float DEFAULT_MOVEMENT_FORCE = 10.0f;      // Aumentado de 6.0f
    static constexpr float DIRECTION_CHANGE_TIME = 3.0f;        // Aumentado de 2.0f (patrullaje más persistente)
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
    // Getters para físicas
    // ===================================
    bool isGrounded() const;
    bool canFly() const;
    b2Body* getPhysicsBody() const;
    sf::Vector2f getVelocity() const;
    int getMovementDirection() const;
    
    // ===================================
    // NUEVO: Getters para animación
    // ===================================
    EnemyState getCurrentState() const;
    bool isMoving() const;
    bool hasTextures() const;
    
    // Setters
    void setPosition(float x, float y);
    void setPosition(const sf::Vector2f& position);
    void setHealth(int health);
    void setMoving(bool moving);                 // ← NUEVO: Controlar animación
    
    // ===================================
    // Configuración de físicas
    // ===================================
    void initializePhysics(CPhysics* physics);  // Configurar físicas
    void updatePhysicsPosition();                // Sincronizar posición con físicas
    void syncPositionFromPhysics();              // Obtener posición de las físicas
    
    // Métodos de gameplay
    void moveTowards(const sf::Vector2f& targetPosition, float deltaTime);
    void moveWithPhysics(const sf::Vector2f& targetPosition, float deltaTime);
    int attack();
    void takeDamage(int damage);
    bool isAlive() const;
    bool canAttack() const;
    bool isInRange(const sf::Vector2f& targetPosition, float range) const;
    
    // ===================================
    // Comportamiento con físicas
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
    void printPhysicsStatus() const;             // Debug de físicas
    void printSpriteStatus() const;              // ← NUEVO: Debug de sprites
    
private:
    // Métodos privados para configurar tipos
    void setupEnemyType(EnemyType type);
    std::string enemyTypeToString(EnemyType type) const;
    float calculateDistance(const sf::Vector2f& position1, const sf::Vector2f& position2) const;
    
    // ===================================
    // Métodos de físicas privados
    // ===================================
    void setupPhysicsForType();                 // Configurar físicas según tipo
    void checkGroundState();                    // Verificar si está en el suelo
    void updatePhysicsState();                  // Actualizar estado según físicas
    void updateMovementDirection(float deltaTime);  // Actualizar dirección de patrullaje
    void applyMovementForce(float direction);   // Aplicar fuerza de movimiento
    void handleMurcieelagoAI(const sf::Vector2f& playerPosition, float deltaTime);  // IA específica para murciélagos
    void handleEsqueletoAI(const sf::Vector2f& playerPosition, float deltaTime);    // IA específica para esqueletos
    void handleZombieAI(const sf::Vector2f& playerPosition, float deltaTime);       // IA específica para zombies
    
    // ===================================
    // NUEVO: Métodos de sprites y animación
    // ===================================
    void loadEnemyTextures();                   // Cargar texturas según el tipo
    void updateAnimation(float deltaTime);      // Actualizar animación
    void updateSpriteFrame();                   // Actualizar frame del sprite
    sf::IntRect getCurrentFrameRect() const;    // Obtener rectángulo del frame actual
    void updateAnimationState();                // Actualizar estado de animación basado en movimiento
    std::string getTextureFileName() const;     // Obtener nombre del archivo de textura
};

#endif // CENEMY_HPP