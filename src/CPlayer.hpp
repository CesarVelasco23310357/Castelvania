#ifndef CPLAYER_HPP
#define CPLAYER_HPP

#include <string>
#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>  // ← NUEVO: Box2D

class CPhysics;  // Forward declaration

class CPlayer {
private:
    // Atributos del jugador
    std::string m_name;
    int m_health;
    int m_maxHealth;
    sf::Vector2f m_position;
    float m_speed;
    
    // ===================================
    // NUEVO: Sistema de físicas
    // ===================================
    CPhysics* m_physics;              // Referencia al sistema de físicas
    b2Body* m_physicsBody;            // Cuerpo físico del jugador
    bool m_physicsEnabled;            // Si las físicas están activas
    bool m_isGrounded;                // Si está en el suelo
    float m_jumpForce;                // Fuerza de salto
    
    // Gráficos SFML (fallback)
    sf::RectangleShape m_sprite;
    sf::Color m_color;
    
    // Texturas y sprites
    sf::Texture m_characterTexture;    // Sprite sheet completo del personaje
    sf::Sprite m_playerSprite;
    bool m_texturesLoaded;
    
    // Sistema de animación
    enum class PlayerState {
        IDLE,
        RUNNING, 
        ATTACKING,
        HURT,
        JUMPING,                      // ← NUEVO: Estado de salto
        FALLING                       // ← NUEVO: Estado de caída
    };
    
    PlayerState m_currentState;
    int m_currentFrame;
    float m_animationTimer;
    float m_animationSpeed;
    
    // Variables para el estado HURT
    float m_hurtTimer;
    bool m_isHurt;
    
    // ==============================================
    // CONFIGURACIÓN MANUAL DE SPRITES - Character.png
    // ==============================================
    // Ahora cada animación se define con coordenadas X,Y de inicio
    
    // IDLE - 1 frame (estático)
    static const int IDLE_START_X = 0;        
    static const int IDLE_START_Y = 0;        
    static const int IDLE_FRAME_COUNT = 1;    
    static const int IDLE_FRAME_WIDTH = 150;  
    static const int IDLE_FRAME_HEIGHT = 100; 
    
    // RUN - 6 frames  
    static const int RUN_START_X = 0;         
    static const int RUN_START_Y = 113;       
    static const int RUN_FRAME_COUNT = 6;
    static const int RUN_FRAME_WIDTH = 127;   
    static const int RUN_FRAME_HEIGHT = 100;  
    
    // ATTACK - 4 frames
    static const int ATTACK_START_X = 28;     
    static const int ATTACK_START_Y = 213;    
    static const int ATTACK_FRAME_COUNT = 4;
    static const int ATTACK_FRAME_WIDTH = 110; 
    static const int ATTACK_FRAME_HEIGHT = 100; 
    
    // HURT - 1 frame
    static const int HURT_START_X = 0;        
    static const int HURT_START_Y = 320;      
    static const int HURT_FRAME_COUNT = 1;
    static const int HURT_FRAME_WIDTH = 400;  
    static const int HURT_FRAME_HEIGHT = 100; 
    
    // ===================================
    // NUEVO: Animaciones adicionales
    // ===================================
    // JUMP - 1 frame (puede usar el mismo que RUN frame 0)
    static const int JUMP_START_X = 0;
    static const int JUMP_START_Y = 113;
    static const int JUMP_FRAME_COUNT = 1;
    static const int JUMP_FRAME_WIDTH = 127;
    static const int JUMP_FRAME_HEIGHT = 100;
    
    // FALL - 1 frame (puede usar el mismo que RUN frame 5)
    static const int FALL_START_X = 635;  // RUN frame 5
    static const int FALL_START_Y = 113;
    static const int FALL_FRAME_COUNT = 1;
    static const int FALL_FRAME_WIDTH = 127;
    static const int FALL_FRAME_HEIGHT = 100;
    
    // VELOCIDADES DE ANIMACIÓN
    // ==============================================
    static constexpr float IDLE_ANIMATION_SPEED = 0.8f;
    static constexpr float RUN_ANIMATION_SPEED = 0.12f;
    static constexpr float ATTACK_ANIMATION_SPEED = 0.08f;
    static constexpr float HURT_ANIMATION_SPEED = 0.3f;
    static constexpr float JUMP_ANIMATION_SPEED = 0.1f;  // ← NUEVO
    static constexpr float FALL_ANIMATION_SPEED = 0.1f;  // ← NUEVO
    
    // CONFIGURACIÓN DE HURT
    // ====================
    static constexpr float HURT_DURATION = 0.5f;
    
    // ===================================
    // NUEVO: Configuración de físicas
    // ===================================
    static constexpr float DEFAULT_JUMP_FORCE = 10.0f;
    static constexpr float MOVEMENT_FORCE = 8.0f;
    static constexpr float MAX_VELOCITY_X = 5.0f;
    
public:
    // Constructor
    CPlayer(const std::string& name);
    
    // Destructor
    ~CPlayer();
    
    // Getters
    const std::string& getName() const;
    int getHealth() const;
    int getMaxHealth() const;
    sf::Vector2f getPosition() const;
    float getSpeed() const;
    sf::FloatRect getBounds() const;
    
    // ===================================
    // NUEVO: Getters para físicas
    // ===================================
    bool isGrounded() const;
    bool isJumping() const;
    bool isFalling() const;
    b2Body* getPhysicsBody() const;
    sf::Vector2f getVelocity() const;
    
    // Setters
    void setPosition(float x, float y);
    void setPosition(const sf::Vector2f& position);
    void setHealth(int health);
    void setSpeed(float speed);
    void setJumpForce(float force);              // ← NUEVO
    
    // ===================================
    // NUEVO: Configuración de físicas
    // ===================================
    void initializePhysics(CPhysics* physics);  // Configurar físicas
    void updatePhysicsPosition();                // Sincronizar posición con físicas
    void syncPositionFromPhysics();              // Obtener posición de las físicas
    
    // Métodos para configurar sprites manualmente
    void setIdleSprite(int startX, int startY, int frameWidth, int frameHeight, int frameCount);
    void setRunSprite(int startX, int startY, int frameWidth, int frameHeight, int frameCount);
    void setAttackSprite(int startX, int startY, int frameWidth, int frameHeight, int frameCount);
    void setHurtSprite(int startX, int startY, int frameWidth, int frameHeight, int frameCount);
    
    // Métodos de gameplay
    void move(float deltaX, float deltaY);
    void moveWithPhysics(float direction);       // ← NUEVO: Movimiento con físicas
    void jump();                                 // ← NUEVO: Saltar
    void attack();
    void takeDamage(int damage);
    bool isAlive() const;
    
    // Estados de animación
    void setRunning(bool running);
    void startAttack();
    void startHurt();
    void startJump();                            // ← NUEVO
    void startFall();                            // ← NUEVO
    bool isAttacking() const;
    bool isHurt() const;
    
    // Métodos SFML
    void update(float deltaTime);
    void render(sf::RenderWindow& window);
    
    // Debug
    void printStatus() const;
    void printSpriteConfig() const; 
    void debugCurrentFrame() const; 
    void printPhysicsStatus() const;             // ← NUEVO: Debug de físicas
    
private:
    // Métodos privados
    void loadPlayerTextures();
    void updateAnimation(float deltaTime);
    void updateSpriteFrame();
    void updatePhysicsState();                   // ← NUEVO: Actualizar estado según físicas
    sf::IntRect getCurrentFrameRect() const;
    
    // ===================================
    // NUEVO: Métodos de físicas privados
    // ===================================
    void checkGroundState();                     // Verificar si está en el suelo
    void updateAnimationFromPhysics();           // Actualizar animación según físicas
    void limitHorizontalVelocity();             // Limitar velocidad horizontal
};

#endif // CPLAYER_HPP