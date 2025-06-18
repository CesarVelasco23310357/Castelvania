#include "CEnemy.hpp"
#include "CPhysics.hpp"
#include <iostream>
#include <cmath>

// Constructor
CEnemy::CEnemy(EnemyType type, float x, float y) 
    : m_enemyType(type), m_position(x, y), m_currentCooldown(0.0f),
      m_physics(nullptr),           // Referencia al sistema de físicas
      m_physicsBody(nullptr),       // Cuerpo físico
      m_physicsEnabled(false),      // Estado de físicas
      m_isGrounded(false),          // Estado en el suelo
      m_canFly(false),              // Capacidad de volar
      m_jumpForce(0.0f),            // Fuerza de salto
      m_flyForce(0.0f),             // Fuerza de vuelo
      m_movementForce(0.0f),        // Fuerza de movimiento
      m_lastDirectionChange(0.0f),  // Timer para cambio de dirección
      m_movementDirection(1),       // Dirección inicial (derecha)
      // ===================================
      // NUEVO: Inicialización del sistema de sprites
      // ===================================
      m_texturesLoaded(false),      // Estado de texturas
      m_currentState(EnemyState::IDLE), // Estado inicial
      m_currentFrame(0),            // Frame inicial
      m_animationTimer(0.0f),       // Timer de animación
      m_animationSpeed(0.3f),       // Velocidad por defecto
      m_isMoving(false) {           // Estado de movimiento
    
    // Configurar el tipo de enemigo
    setupEnemyType(type);
    
    // Configurar el sprite (fallback)
    m_sprite.setSize(sf::Vector2f(28.0f, 28.0f));
    m_sprite.setFillColor(m_color);
    m_sprite.setPosition(m_position);
    m_originalColor = m_color;
    
    // ===================================
    // NUEVO: Cargar texturas del enemigo
    // ===================================
    std::cout << "🎨 Cargando sprites para enemigo " << m_type << "..." << std::endl;
    loadEnemyTextures();
    
    std::cout << "Enemigo " << m_type << " creado en posición (" 
              << x << ", " << y << ") con sprites: " 
              << (m_texturesLoaded ? "CARGADOS" : "FALLBACK") << std::endl;
}

// Destructor
CEnemy::~CEnemy() {
    // El sistema de físicas se encarga de limpiar los cuerpos automáticamente
    std::cout << "Enemigo " << m_type << " destruido." << std::endl;
}

// GETTERS
const std::string& CEnemy::getType() const {
    return m_type;
}

EnemyType CEnemy::getEnemyType() const {
    return m_enemyType;
}

int CEnemy::getHealth() const {
    return m_health;
}

int CEnemy::getMaxHealth() const {
    return m_maxHealth;
}

int CEnemy::getDamage() const {
    return m_damage;
}

sf::Vector2f CEnemy::getPosition() const {
    return m_position;
}

float CEnemy::getSpeed() const {
    return m_speed;
}

sf::FloatRect CEnemy::getBounds() const {
    if (m_texturesLoaded) {
        return m_enemySprite.getGlobalBounds();
    }
    return m_sprite.getGlobalBounds();
}

float CEnemy::getDetectionRange() const {
    return m_detectionRange;
}

float CEnemy::getAttackRange() const {
    return m_attackRange;
}

// ===================================
// Getters para físicas
// ===================================
bool CEnemy::isGrounded() const {
    return m_isGrounded;
}

bool CEnemy::canFly() const {
    return m_canFly;
}

b2Body* CEnemy::getPhysicsBody() const {
    return m_physicsBody;
}

sf::Vector2f CEnemy::getVelocity() const {
    if (!m_physicsEnabled || !m_physicsBody) {
        return sf::Vector2f(0.0f, 0.0f);
    }
    
    b2Vec2 velocity = m_physicsBody->GetLinearVelocity();
    return CPhysics::b2VecToSFML(velocity);
}

int CEnemy::getMovementDirection() const {
    return m_movementDirection;
}

// ===================================
// NUEVO: Getters para animación
// ===================================
EnemyState CEnemy::getCurrentState() const {
    return m_currentState;
}

bool CEnemy::isMoving() const {
    return m_isMoving;
}

bool CEnemy::hasTextures() const {
    return m_texturesLoaded;
}

// SETTERS
void CEnemy::setPosition(float x, float y) {
    m_position.x = x;
    m_position.y = y;
    m_sprite.setPosition(m_position);
    
    // ===================================
    // NUEVO: Actualizar también el sprite con textura
    // ===================================
    if (m_texturesLoaded) {
        m_enemySprite.setPosition(m_position);
    }
}

void CEnemy::setPosition(const sf::Vector2f& position) {
    m_position = position;
    m_sprite.setPosition(m_position);
    
    if (m_texturesLoaded) {
        m_enemySprite.setPosition(m_position);
    }
}

void CEnemy::setHealth(int health) {
    m_health = health;
    if (m_health > m_maxHealth) {
        m_health = m_maxHealth;
    } else if (m_health < 0) {
        m_health = 0;
    }
}

// ===================================
// NUEVO: Setter para controlar animación
// ===================================
void CEnemy::setMoving(bool moving) {
    if (m_isMoving != moving) {
        m_isMoving = moving;
        updateAnimationState();
    }
}

// ===================================
// Inicializar físicas del enemigo
// ===================================
void CEnemy::initializePhysics(CPhysics* physics) {
    if (!physics) {
        std::cerr << "❌ Error: Sistema de físicas nulo para enemigo" << std::endl;
        return;
    }
    
    std::cout << "⚙️ Inicializando físicas del enemigo " << m_type << "..." << std::endl;
    
    m_physics = physics;
    
    // Crear cuerpo físico del enemigo
    m_physicsBody = m_physics->createEnemyBody(m_position.x, m_position.y, this);
    
    if (m_physicsBody) {
        m_physicsEnabled = true;
        
        // Configurar propiedades físicas específicas por tipo
        setupPhysicsForType();
        
        // Configurar posición inicial
        updatePhysicsPosition();
        
        std::cout << "✅ Cuerpo físico del enemigo " << m_type << " creado exitosamente" << std::endl;
    } else {
        std::cerr << "❌ Error: No se pudo crear el cuerpo físico del enemigo" << std::endl;
        m_physicsEnabled = false;
    }
}

// ===================================
// Configurar físicas según el tipo
// ===================================
void CEnemy::setupPhysicsForType() {
    if (!m_physicsEnabled || !m_physicsBody) return;
    
    switch (m_enemyType) {
        case EnemyType::MURCIELAGO:
            m_canFly = true;
            m_flyForce = MURCIELAGO_FLY_FORCE;
            m_movementForce = DEFAULT_MOVEMENT_FORCE;
            std::cout << "🦇 Murciélago configurado para volar" << std::endl;
            break;
            
        case EnemyType::ESQUELETO:
            m_canFly = false;
            m_jumpForce = ESQUELETO_JUMP_FORCE;
            m_movementForce = DEFAULT_MOVEMENT_FORCE;
            std::cout << "💀 Esqueleto configurado para saltar" << std::endl;
            break;
            
        case EnemyType::ZOMBIE:
            m_canFly = false;
            m_jumpForce = 0.0f; // Los zombies no saltan
            m_movementForce = ZOMBIE_MOVEMENT_FORCE;
            std::cout << "🧟 Zombie configurado para caminar" << std::endl;
            break;
            
        default:
            m_canFly = false;
            m_jumpForce = 0.0f;
            m_movementForce = DEFAULT_MOVEMENT_FORCE;
            break;
    }
}

// ===================================
// Sincronizar posición desde físicas
// ===================================
void CEnemy::syncPositionFromPhysics() {
    if (!m_physicsEnabled || !m_physicsBody) return;
    
    // Obtener posición del cuerpo físico
    b2Vec2 physicsPos = m_physicsBody->GetPosition();
    sf::Vector2f newPos = CPhysics::metersToPixels(physicsPos);
    
    // Actualizar posición visual
    m_position = newPos;
    m_sprite.setPosition(m_position);
    
    // ===================================
    // NUEVO: Actualizar también el sprite con textura
    // ===================================
    if (m_texturesLoaded) {
        m_enemySprite.setPosition(m_position);
    }
    
    // Actualizar estados basados en físicas
    updatePhysicsState();
}

// ===================================
// Actualizar posición en físicas
// ===================================
void CEnemy::updatePhysicsPosition() {
    if (!m_physicsEnabled || !m_physicsBody) return;
    
    // Convertir posición visual a físicas
    b2Vec2 physicsPos = CPhysics::sfmlVecToB2(m_position);
    m_physicsBody->SetTransform(physicsPos, m_physicsBody->GetAngle());
}

// MÉTODOS DE GAMEPLAY
void CEnemy::moveTowards(const sf::Vector2f& targetPosition, float deltaTime) {
    if (!isAlive()) return;
    
    // Si tiene físicas habilitadas, usar movimiento con físicas
    if (m_physicsEnabled) {
        moveWithPhysics(targetPosition, deltaTime);
        return;
    }
    
    // Movimiento tradicional (fallback)
    sf::Vector2f direction = targetPosition - m_position;
    float distance = calculateDistance(m_position, targetPosition);
    
    if (distance <= m_attackRange) {
        setMoving(false);  // ← NUEVO: Parar animación
        return;
    }
    
    if (distance > 0) {
        direction.x /= distance;
        direction.y /= distance;
        
        sf::Vector2f movement = direction * m_speed * deltaTime;
        m_position += movement;
        m_sprite.setPosition(m_position);
        
        // ===================================
        // NUEVO: Actualizar sprite con textura y activar animación
        // ===================================
        if (m_texturesLoaded) {
            m_enemySprite.setPosition(m_position);
        }
        setMoving(true);  // ← NUEVO: Activar animación de movimiento
    }
}

// ===================================
// Movimiento con físicas
// ===================================
void CEnemy::moveWithPhysics(const sf::Vector2f& targetPosition, float deltaTime) {
    if (!m_physicsEnabled || !m_physicsBody || !isAlive()) return;
    
    float distance = calculateDistance(m_position, targetPosition);
    
    if (distance <= m_attackRange) {
        setMoving(false);  // ← NUEVO: Parar animación
        return;
    }
    
    sf::Vector2f direction = targetPosition - m_position;
    float moveDirection = 0.0f;
    
    if (std::abs(direction.x) > 10.0f) {
        moveDirection = (direction.x > 0) ? 1.0f : -1.0f;
        setMoving(true);  // ← NUEVO: Activar animación
    } else {
        setMoving(false); // ← NUEVO: Parar animación
    }
    
    // Aplicar movimiento según el tipo
    if (m_canFly && m_enemyType == EnemyType::MURCIELAGO) {
        handleMurcieelagoAI(targetPosition, deltaTime);
    } else {
        applyMovementForce(moveDirection);
        
        if (m_enemyType == EnemyType::ESQUELETO && m_isGrounded && std::abs(direction.y) > 50.0f) {
            jump();
        }
    }
}

int CEnemy::attack() {
    if (!canAttack()) {
        return 0;
    }
    
    std::cout << m_type << " ataca causando " << m_damage << " de daño!\n";
    m_currentCooldown = m_attackCooldown;
    return m_damage;
}

void CEnemy::takeDamage(int damage) {
    if (damage > 0) {
        m_health -= damage;
        if (m_health < 0) {
            m_health = 0;
        }
        
        std::cout << m_type << " recibe " << damage << " de daño. Salud: " 
                  << m_health << "/" << m_maxHealth << "\n";
        
        if (m_health > 0) {
            m_sprite.setFillColor(sf::Color::Red);
        } else {
            m_sprite.setFillColor(sf::Color::Black);
            setMoving(false);  // ← NUEVO: Parar animación al morir
            std::cout << m_type << " ha muerto!\n";
        }
    }
}

bool CEnemy::isAlive() const {
    return m_health > 0;
}

bool CEnemy::canAttack() const {
    return isAlive() && m_currentCooldown <= 0.0f;
}

bool CEnemy::isInRange(const sf::Vector2f& targetPosition, float range) const {
    return calculateDistance(m_position, targetPosition) <= range;
}

// ===================================
// Saltar (esqueletos principalmente)
// ===================================
void CEnemy::jump() {
    if (!m_physicsEnabled || !m_physicsBody || !m_isGrounded || m_jumpForce <= 0.0f) {
        return;
    }
    
    m_physics->applyImpulse(this, 0.0f, -m_jumpForce);
    m_isGrounded = false;
    
    std::cout << "🦘 " << m_type << " salta! Fuerza: " << m_jumpForce << std::endl;
}

// ===================================
// Volar (murciélagos)
// ===================================
void CEnemy::fly() {
    if (!m_physicsEnabled || !m_physicsBody || !m_canFly) return;
    
    m_physics->applyForce(this, 0.0f, -m_flyForce);
}

// ===================================
// Patrullar automáticamente
// ===================================
void CEnemy::patrol() {
    if (!m_physicsEnabled || !m_physicsBody) return;
    
    applyMovementForce(static_cast<float>(m_movementDirection) * 0.5f);
    setMoving(true);  // ← NUEVO: Activar animación durante patrullaje
}

// ===================================
// Seguir objetivo con físicas
// ===================================
void CEnemy::followTarget(const sf::Vector2f& target, float deltaTime) {
    moveWithPhysics(target, deltaTime);
}

// ===================================
// IA específica para murciélagos (vuelan)
// ===================================
void CEnemy::handleMurcieelagoAI(const sf::Vector2f& playerPosition, float deltaTime) {
    if (!m_physicsEnabled || !m_physicsBody) return;
    
    sf::Vector2f direction = playerPosition - m_position;
    float distance = calculateDistance(m_position, playerPosition);
    
    if (distance <= m_detectionRange && distance > m_attackRange) {
        float forceX = (direction.x > 0) ? m_movementForce : -m_movementForce;
        float forceY = (direction.y > 0) ? m_flyForce : -m_flyForce;
        
        m_physics->applyForce(this, forceX * 0.5f, forceY * 0.3f);
        setMoving(true);  // ← NUEVO: Activar animación
        
        b2Vec2 velocity = m_physicsBody->GetLinearVelocity();
        if (velocity.Length() > 8.0f) {
            velocity.Normalize();
            velocity *= 8.0f;
            m_physicsBody->SetLinearVelocity(velocity);
        }
    }
}

// ===================================
// IA específica para esqueletos
// ===================================
void CEnemy::handleEsqueletoAI(const sf::Vector2f& playerPosition, float deltaTime) {
    if (!m_physicsEnabled || !m_physicsBody) return;
    
    sf::Vector2f direction = playerPosition - m_position;
    float distance = calculateDistance(m_position, playerPosition);
    
    if (distance <= m_detectionRange && distance > m_attackRange) {
        float moveDirection = (direction.x > 0) ? 1.0f : -1.0f;
        applyMovementForce(moveDirection);
        setMoving(true);  // ← NUEVO: Activar animación
        
        if (m_isGrounded && (direction.y < -30.0f || std::abs(direction.x) < 50.0f)) {
            jump();
        }
    } else if (distance > m_detectionRange) {
        patrol();
    } else {
        setMoving(false);  // ← NUEVO: Parar animación
    }
}

// ===================================
// IA específica para zombies
// ===================================
void CEnemy::handleZombieAI(const sf::Vector2f& playerPosition, float deltaTime) {
    if (!m_physicsEnabled || !m_physicsBody) return;
    
    sf::Vector2f direction = playerPosition - m_position;
    float distance = calculateDistance(m_position, playerPosition);
    
    if (distance <= m_detectionRange && distance > m_attackRange) {
        float moveDirection = (direction.x > 0) ? 1.0f : -1.0f;
        applyMovementForce(moveDirection * 0.7f);
        setMoving(true);  // ← NUEVO: Activar animación
    } else if (distance > m_detectionRange) {
        patrol();
    } else {
        setMoving(false);  // ← NUEVO: Parar animación
    }
}

// ===================================
// Aplicar fuerza de movimiento
// ===================================
void CEnemy::applyMovementForce(float direction) {
    if (!m_physicsEnabled || !m_physicsBody || direction == 0.0f) return;
    
    b2Vec2 velocity = m_physicsBody->GetLinearVelocity();
    float maxVelocity = (m_enemyType == EnemyType::ZOMBIE) ? 2.0f : 4.0f;
    
    if (std::abs(velocity.x) < maxVelocity) {
        float force = direction * m_movementForce;
        m_physics->applyForce(this, force, 0.0f);
    }
}

// ===================================
// Verificar estado en el suelo
// ===================================
void CEnemy::checkGroundState() {
    if (!m_physicsEnabled || !m_physicsBody) {
        m_isGrounded = true;
        return;
    }
    
    b2Vec2 velocity = m_physicsBody->GetLinearVelocity();
    m_isGrounded = std::abs(velocity.y) < 0.5f;
}

// ===================================
// Actualizar estado basado en físicas
// ===================================
void CEnemy::updatePhysicsState() {
    if (!m_physicsEnabled) return;
    
    checkGroundState();
}

// ===================================
// Actualizar dirección de movimiento
// ===================================
void CEnemy::updateMovementDirection(float deltaTime) {
    m_lastDirectionChange += deltaTime;
    
    if (m_lastDirectionChange >= DIRECTION_CHANGE_TIME) {
        m_movementDirection = (rand() % 3) - 1;
        m_lastDirectionChange = 0.0f;
    }
}

// IA BÁSICA
void CEnemy::updateAI(const sf::Vector2f& playerPosition, float deltaTime) {
    if (!isAlive()) return;
    
    float distanceToPlayer = calculateDistance(m_position, playerPosition);
    
    updateMovementDirection(deltaTime);
    
    if (distanceToPlayer <= m_detectionRange) {
        if (distanceToPlayer <= m_attackRange && canAttack()) {
            attack();
            setMoving(false);  // ← NUEVO: Parar animación al atacar
        } else {
            if (m_physicsEnabled) {
                switch (m_enemyType) {
                    case EnemyType::MURCIELAGO:
                        handleMurcieelagoAI(playerPosition, deltaTime);
                        break;
                    case EnemyType::ESQUELETO:
                        handleEsqueletoAI(playerPosition, deltaTime);
                        break;
                    case EnemyType::ZOMBIE:
                        handleZombieAI(playerPosition, deltaTime);
                        break;
                }
            } else {
                moveTowards(playerPosition, deltaTime);
            }
        }
    } else {
        if (m_physicsEnabled) {
            patrol();
        } else {
            setMoving(false);  // ← NUEVO: Parar animación cuando no patrulla
        }
    }
}

// MÉTODOS SFML
void CEnemy::update(float deltaTime) {
    if (m_currentCooldown > 0.0f) {
        m_currentCooldown -= deltaTime;
    }
    
    if (m_physicsEnabled) {
        updatePhysicsState();
    }
    
    // ===================================
    // NUEVO: Actualizar animación
    // ===================================
    if (m_texturesLoaded) {
        updateAnimation(deltaTime);
    }
    
    if (isAlive() && m_sprite.getFillColor() == sf::Color::Red) {
        m_sprite.setFillColor(m_originalColor);
    }
}

void CEnemy::render(sf::RenderWindow& window) {
    if (isAlive()) {
        if (m_texturesLoaded) {
            // ===================================
            // NUEVO: Renderizar con textura y animación
            // ===================================
            window.draw(m_enemySprite);
        } else {
            // Fallback: renderizar rectángulo de color
            window.draw(m_sprite);
        }
    }
}

// DEBUG
void CEnemy::printStatus() const {
    std::cout << "=== Estado del Enemigo ===\n";
    std::cout << "Tipo: " << m_type << "\n";
    std::cout << "Salud: " << m_health << "/" << m_maxHealth << "\n";
    std::cout << "Daño: " << m_damage << "\n";
    std::cout << "Posición: (" << m_position.x << ", " << m_position.y << ")\n";
    std::cout << "Velocidad: " << m_speed << "\n";
    std::cout << "Rango detección: " << m_detectionRange << "\n";
    std::cout << "Rango ataque: " << m_attackRange << "\n";
    std::cout << "Estado: " << (isAlive() ? "Vivo" : "Muerto") << "\n";
    std::cout << "Texturas: " << (m_texturesLoaded ? "Cargadas" : "No cargadas") << "\n";
    std::cout << "Animación: " << (m_currentState == EnemyState::IDLE ? "IDLE" : "MOVING") << "\n";
    std::cout << "Frame actual: " << m_currentFrame << "\n";
    std::cout << "========================\n";
}

void CEnemy::printPhysicsStatus() const {
    std::cout << "=== FÍSICAS DEL ENEMIGO " << m_type << " ===" << std::endl;
    std::cout << "Físicas habilitadas: " << (m_physicsEnabled ? "SÍ" : "NO") << std::endl;
    std::cout << "En el suelo: " << (m_isGrounded ? "SÍ" : "NO") << std::endl;
    std::cout << "Puede volar: " << (m_canFly ? "SÍ" : "NO") << std::endl;
    std::cout << "Dirección de movimiento: " << m_movementDirection << std::endl;
    
    if (m_physicsEnabled && m_physicsBody) {
        b2Vec2 pos = m_physicsBody->GetPosition();
        b2Vec2 vel = m_physicsBody->GetLinearVelocity();
        
        std::cout << "Posición física: (" << pos.x << ", " << pos.y << ") metros" << std::endl;
        std::cout << "Velocidad: (" << vel.x << ", " << vel.y << ") m/s" << std::endl;
        
        sf::Vector2f pixelPos = CPhysics::metersToPixels(pos);
        std::cout << "Posición en píxeles: (" << pixelPos.x << ", " << pixelPos.y << ")" << std::endl;
    }
    
    std::cout << "============================" << std::endl;
}

// ===================================
// NUEVO: Debug de sprites
// ===================================
void CEnemy::printSpriteStatus() const {
    std::cout << "=== SPRITES DEL ENEMIGO " << m_type << " ===" << std::endl;
    std::cout << "Texturas cargadas: " << (m_texturesLoaded ? "SÍ" : "NO") << std::endl;
    std::cout << "Estado actual: " << (m_currentState == EnemyState::IDLE ? "IDLE" : "MOVING") << std::endl;
    std::cout << "Frame actual: " << m_currentFrame << std::endl;
    std::cout << "En movimiento: " << (m_isMoving ? "SÍ" : "NO") << std::endl;
    std::cout << "Velocidad animación: " << m_animationSpeed << std::endl;
    
    if (m_texturesLoaded) {
        sf::IntRect rect = getCurrentFrameRect();
        std::cout << "Rectángulo actual: (" << rect.left << "," << rect.top 
                  << ") " << rect.width << "x" << rect.height << std::endl;
        std::cout << "Archivo de textura: " << getTextureFileName() << std::endl;
    }
    
    std::cout << "===============================" << std::endl;
}

// MÉTODOS PRIVADOS
void CEnemy::setupEnemyType(EnemyType type) {
    m_type = enemyTypeToString(type);
    
    switch (type) {
        case EnemyType::MURCIELAGO:
            m_health = 30;
            m_maxHealth = 30;
            m_damage = 10;
            m_speed = 120.0f;
            m_detectionRange = 150.0f;
            m_attackRange = 35.0f;
            m_attackCooldown = 1.0f;
            m_color = sf::Color::Magenta;
            m_animationSpeed = MURCIELAGO_ANIMATION_SPEED;
            break;
            
        case EnemyType::ESQUELETO:
            m_health = 60;
            m_maxHealth = 60;
            m_damage = 20;
            m_speed = 80.0f;
            m_detectionRange = 120.0f;
            m_attackRange = 40.0f;
            m_attackCooldown = 1.5f;
            m_color = sf::Color::White;
            m_animationSpeed = SKELETON_ANIMATION_SPEED;
            break;
            
        case EnemyType::ZOMBIE:
            m_health = 100;
            m_maxHealth = 100;
            m_damage = 30;
            m_speed = 50.0f;
            m_detectionRange = 100.0f;
            m_attackRange = 45.0f;
            m_attackCooldown = 2.0f;
            m_color = sf::Color::Green;
            m_animationSpeed = ZOMBIE_ANIMATION_SPEED;
            break;
            
        default:
            m_health = 50;
            m_maxHealth = 50;
            m_damage = 15;
            m_speed = 70.0f;
            m_detectionRange = 100.0f;
            m_attackRange = 40.0f;
            m_attackCooldown = 1.5f;
            m_color = sf::Color::Red;
            m_animationSpeed = 0.3f;
            break;
    }
}

std::string CEnemy::enemyTypeToString(EnemyType type) const {
    switch (type) {
        case EnemyType::MURCIELAGO: return "Murcielago";
        case EnemyType::ESQUELETO: return "Esqueleto";
        case EnemyType::ZOMBIE: return "Zombie";
        default: return "Desconocido";
    }
}

float CEnemy::calculateDistance(const sf::Vector2f& position1, const sf::Vector2f& position2) const {
    float dx = position2.x - position1.x;
    float dy = position2.y - position1.y;
    return std::sqrt(dx * dx + dy * dy);
}

// ===================================
// NUEVO: Métodos de sprites y animación
// ===================================

void CEnemy::loadEnemyTextures() {
    std::cout << "🎨 Cargando texturas para " << m_type << "..." << std::endl;
    
    std::string textureFile = getTextureFileName();
    std::string fullPath = "assets/" + textureFile;
    
    std::cout << "Intentando cargar: " << fullPath << std::endl;
    
    if (!m_enemyTexture.loadFromFile(fullPath)) {
        std::cerr << "❌ Error: No se pudo cargar " << fullPath << std::endl;
        std::cerr << "   Usando fallback (rectángulo de color)" << std::endl;
        m_texturesLoaded = false;
        return;
    }
    
    sf::Vector2u textureSize = m_enemyTexture.getSize();
    std::cout << "✅ " << textureFile << " cargado (" << textureSize.x << "x" << textureSize.y << ")" << std::endl;
    
    // Configurar sprite inicial
    m_texturesLoaded = true;
    m_enemySprite.setTexture(m_enemyTexture);
    m_enemySprite.setPosition(m_position);
    
    // ✨ NUEVO: Aplicar escalado según el tipo de enemigo
    switch (m_enemyType) {
        case EnemyType::ZOMBIE:
            m_enemySprite.setScale(ZombieSprites::SCALE_X, ZombieSprites::SCALE_Y);
            std::cout << "   🧟 ZOMBIE escalado a: " << ZombieSprites::SCALE_X << "x" << ZombieSprites::SCALE_Y << std::endl;
            break;
            
        case EnemyType::ESQUELETO:
            m_enemySprite.setScale(SkeletonSprites::SCALE_X, SkeletonSprites::SCALE_Y);
            std::cout << "   💀 SKELETON escalado a: " << SkeletonSprites::SCALE_X << "x" << SkeletonSprites::SCALE_Y << std::endl;
            break;
            
        case EnemyType::MURCIELAGO:
            m_enemySprite.setScale(MurcielagoSprites::SCALE_X, MurcielagoSprites::SCALE_Y);
            std::cout << "   🦇 MURCIELAGO escalado a: " << MurcielagoSprites::SCALE_X << "x" << MurcielagoSprites::SCALE_Y << std::endl;
            break;
    }
    
    // Configurar frame inicial (IDLE)
    updateSpriteFrame();
    
    // Imprimir configuración del sprite
    switch (m_enemyType) {
        case EnemyType::ZOMBIE:
            std::cout << "   🧟 ZOMBIE - IDLE: 1 frame, MOVING: 4 frames" << std::endl;
            break;
        case EnemyType::ESQUELETO:
            std::cout << "   💀 SKELETON - IDLE: 1 frame, MOVING: 5 frames" << std::endl;
            break;
        case EnemyType::MURCIELAGO:
            std::cout << "   🦇 MURCIELAGO - Siempre animado: 5 frames" << std::endl;
            break;
    }
    
    std::cout << "✅ Sistema de sprites de " << m_type << " inicializado" << std::endl;
}

void CEnemy::updateAnimation(float deltaTime) {
    if (!m_texturesLoaded) return;
    
    m_animationTimer += deltaTime;
    
    // Determinar el número de frames según el estado actual
    int frameCount = 1;
    
    switch (m_enemyType) {
        case EnemyType::ZOMBIE:
            frameCount = (m_currentState == EnemyState::IDLE) ? ZombieSprites::IDLE_FRAME_COUNT : ZombieSprites::MOVING_FRAME_COUNT;
            break;
        case EnemyType::ESQUELETO:
            frameCount = (m_currentState == EnemyState::IDLE) ? SkeletonSprites::IDLE_FRAME_COUNT : SkeletonSprites::MOVING_FRAME_COUNT;
            break;
        case EnemyType::MURCIELAGO:
            frameCount = MurcielagoSprites::MOVING_FRAME_COUNT;
            break;
    }
    
    // Actualizar frame si es necesario
    if (m_animationTimer >= m_animationSpeed) {
        m_animationTimer = 0.0f;
        
        if (frameCount > 1) {
            m_currentFrame = (m_currentFrame + 1) % frameCount;
        } else {
            m_currentFrame = 0;
        }
        
        updateSpriteFrame();
    }
}

void CEnemy::updateSpriteFrame() {
    if (!m_texturesLoaded) return;
    
    sf::IntRect frameRect = getCurrentFrameRect();
    m_enemySprite.setTextureRect(frameRect);
}

sf::IntRect CEnemy::getCurrentFrameRect() const {
    int startX = 0, startY = 0;
    int frameWidth = 32, frameHeight = 32;
    
    switch (m_enemyType) {
        case EnemyType::ZOMBIE:
            if (m_currentState == EnemyState::IDLE) {
                startX = ZombieSprites::IDLE_START_X;
                startY = ZombieSprites::IDLE_START_Y;
                frameWidth = ZombieSprites::IDLE_FRAME_WIDTH;
                frameHeight = ZombieSprites::IDLE_FRAME_HEIGHT;
            } else {
                startX = ZombieSprites::MOVING_START_X;
                startY = ZombieSprites::MOVING_START_Y;
                frameWidth = ZombieSprites::MOVING_FRAME_WIDTH;
                frameHeight = ZombieSprites::MOVING_FRAME_HEIGHT;
            }
            break;
            
        case EnemyType::ESQUELETO:
            if (m_currentState == EnemyState::IDLE) {
                startX = SkeletonSprites::IDLE_START_X;
                startY = SkeletonSprites::IDLE_START_Y;
                frameWidth = SkeletonSprites::IDLE_FRAME_WIDTH;
                frameHeight = SkeletonSprites::IDLE_FRAME_HEIGHT;
            } else {
                startX = SkeletonSprites::MOVING_START_X;
                startY = SkeletonSprites::MOVING_START_Y;
                frameWidth = SkeletonSprites::MOVING_FRAME_WIDTH;
                frameHeight = SkeletonSprites::MOVING_FRAME_HEIGHT;
            }
            break;
            
        case EnemyType::MURCIELAGO:
            startX = MurcielagoSprites::MOVING_START_X;
            startY = MurcielagoSprites::MOVING_START_Y;
            frameWidth = MurcielagoSprites::MOVING_FRAME_WIDTH;
            frameHeight = MurcielagoSprites::MOVING_FRAME_HEIGHT;
            break;
    }
    
    int x = startX + (m_currentFrame * frameWidth);
    int y = startY;
    
    return sf::IntRect(x, y, frameWidth, frameHeight);
}

void CEnemy::updateAnimationState() {
    if (m_enemyType == EnemyType::MURCIELAGO) {
        m_currentState = EnemyState::MOVING;
    } else {
        if (m_isMoving) {
            if (m_currentState != EnemyState::MOVING) {
                m_currentState = EnemyState::MOVING;
                m_currentFrame = 0;
                m_animationTimer = 0.0f;
            }
        } else {
            if (m_currentState != EnemyState::IDLE) {
                m_currentState = EnemyState::IDLE;
                m_currentFrame = 0;
                m_animationTimer = 0.0f;
            }
        }
    }
}

std::string CEnemy::getTextureFileName() const {
    switch (m_enemyType) {
        case EnemyType::ZOMBIE: return "zombie.png";
        case EnemyType::ESQUELETO: return "skeleton.png";
        case EnemyType::MURCIELAGO: return "murcielago.png";
        default: return "unknown.png";
    }
}