#include "CEnemy.hpp"
#include "CPhysics.hpp"
#include <iostream>
#include <cmath>

// Constructor
CEnemy::CEnemy(EnemyType type, float x, float y) 
    : m_enemyType(type), m_position(x, y), m_currentCooldown(0.0f),
      m_physics(nullptr),           // Referencia al sistema de fisicas
      m_physicsBody(nullptr),       // Cuerpo fisico
      m_physicsEnabled(false),      // Estado de fisicas
      m_isGrounded(false),          // Estado en el suelo
      m_canFly(false),              // Capacidad de volar
      m_jumpForce(0.0f),            // Fuerza de salto
      m_flyForce(0.0f),             // Fuerza de vuelo
      m_movementForce(0.0f),        // Fuerza de movimiento
      m_lastDirectionChange(0.0f),  // Timer para cambio de direccion
      m_movementDirection(1),       // Direccion inicial (derecha)
      // ===================================
      // NUEVO: Inicializacion del sistema de sprites
      // ===================================
      m_texturesLoaded(false),      // Estado de texturas
      m_currentState(EnemyState::IDLE), // Estado inicial
      m_currentFrame(0),            // Frame inicial
      m_animationTimer(0.0f),       // Timer de animacion
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
    std::cout << " Cargando sprites para enemigo " << m_type << "..." << std::endl;
    loadEnemyTextures();
    
    std::cout << "Enemigo " << m_type << " creado en posicion (" 
              << x << ", " << y << ") con sprites: " 
              << (m_texturesLoaded ? "CARGADOS" : "FALLBACK") << std::endl;
}

// Destructor
CEnemy::~CEnemy() {
    // El sistema de fisicas se encarga de limpiar los cuerpos automaticamente
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
// Getters para fisicas
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
// NUEVO: Getters para animacion
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
// NUEVO: Setter para controlar animacion
// ===================================
void CEnemy::setMoving(bool moving) {
    if (m_isMoving != moving) {
        m_isMoving = moving;
        updateAnimationState();
    }
}

// ===================================
// Inicializar fisicas del enemigo
// ===================================
void CEnemy::initializePhysics(CPhysics* physics) {
    if (!physics) {
        std::cerr << " Error: Sistema de fisicas nulo para enemigo" << std::endl;
        return;
    }
    
    std::cout << "⚙️ Inicializando fisicas del enemigo " << m_type << "..." << std::endl;
    
    m_physics = physics;
    
    // Crear cuerpo fisico del enemigo
    m_physicsBody = m_physics->createEnemyBody(m_position.x, m_position.y, this);
    
    if (m_physicsBody) {
        m_physicsEnabled = true;
        
        // Configurar propiedades fisicas especificas por tipo
        setupPhysicsForType();
        
        // Configurar posicion inicial
        updatePhysicsPosition();
        
        std::cout << " Cuerpo fisico del enemigo " << m_type << " creado exitosamente" << std::endl;
    } else {
        std::cerr << " Error: No se pudo crear el cuerpo fisico del enemigo" << std::endl;
        m_physicsEnabled = false;
    }
}

// ===================================
// Configurar fisicas según el tipo
// ===================================
void CEnemy::setupPhysicsForType() {
    if (!m_physicsEnabled || !m_physicsBody) return;
    
    switch (m_enemyType) {
        case EnemyType::MURCIELAGO:
            m_canFly = true;
            m_flyForce = MURCIELAGO_FLY_FORCE;
            m_movementForce = DEFAULT_MOVEMENT_FORCE;
            break;
            
        case EnemyType::ESQUELETO:
            m_canFly = false;
            m_jumpForce = ESQUELETO_JUMP_FORCE;
            m_movementForce = DEFAULT_MOVEMENT_FORCE;
            break;
            
        case EnemyType::ZOMBIE:
            m_canFly = false;
            m_jumpForce = 0.18f; 
            m_movementForce = ZOMBIE_MOVEMENT_FORCE;
            break;
            
        default:
            m_canFly = false;
            m_jumpForce = 0.0f;
            m_movementForce = DEFAULT_MOVEMENT_FORCE;
            break;
    }
}

// ===================================
// Sincronizar posicion desde fisicas
// ===================================
void CEnemy::syncPositionFromPhysics() {
    if (!m_physicsEnabled || !m_physicsBody) return;
    
    // Obtener posicion del cuerpo fisico
    b2Vec2 physicsPos = m_physicsBody->GetPosition();
    sf::Vector2f newPos = CPhysics::metersToPixels(physicsPos);
    
    // Actualizar posicion visual
    m_position = newPos;
    m_sprite.setPosition(m_position);
    
    // ===================================
    // NUEVO: Actualizar también el sprite con textura
    // ===================================
    if (m_texturesLoaded) {
        m_enemySprite.setPosition(m_position);
    }
    
    // Actualizar estados basados en fisicas
    updatePhysicsState();
}

// ===================================
// Actualizar posicion en fisicas
// ===================================
void CEnemy::updatePhysicsPosition() {
    if (!m_physicsEnabled || !m_physicsBody) return;
    
    // Convertir posicion visual a fisicas
    b2Vec2 physicsPos = CPhysics::sfmlVecToB2(m_position);
    m_physicsBody->SetTransform(physicsPos, m_physicsBody->GetAngle());
}

// MÉTODOS DE GAMEPLAY
void CEnemy::moveTowards(const sf::Vector2f& targetPosition, float deltaTime) {
    if (!isAlive()) return;
    
    // Si tiene fisicas habilitadas, usar movimiento con fisicas
    if (m_physicsEnabled) {
        moveWithPhysics(targetPosition, deltaTime);
        return;
    }
    
    // Movimiento tradicional (fallback)
    sf::Vector2f direction = targetPosition - m_position;
    float distance = calculateDistance(m_position, targetPosition);
    
    if (distance <= m_attackRange) {
        setMoving(false);  // ← NUEVO: Parar animacion
        return;
    }
    
    if (distance > 0) {
        direction.x /= distance;
        direction.y /= distance;
        
        sf::Vector2f movement = direction * m_speed * deltaTime;
        m_position += movement;
        m_sprite.setPosition(m_position);
        
        // ===================================
        // NUEVO: Actualizar sprite con textura y activar animacion
        // ===================================
        if (m_texturesLoaded) {
            m_enemySprite.setPosition(m_position);
        }
        setMoving(true);  // ← NUEVO: Activar animacion de movimiento
    }
}

// ===================================
// Movimiento con fisicas
// ===================================
void CEnemy::moveWithPhysics(const sf::Vector2f& targetPosition, float deltaTime) {
    if (!m_physicsEnabled || !m_physicsBody || !isAlive()) return;
    
    float distance = calculateDistance(m_position, targetPosition);
    
    if (distance <= m_attackRange) {
        setMoving(false);
        return;
    }
    
    sf::Vector2f direction = targetPosition - m_position;
    float moveDirection = 0.0f;
    
    // ===================================
    // CORREGIDO: Deteccion de movimiento mas sensible
    // ===================================
    if (std::abs(direction.x) > 5.0f) {  // Reducido de 10.0f a 5.0f
        moveDirection = (direction.x > 0) ? 1.0f : -1.0f;
        setMoving(true);
    } else {
        setMoving(false);
    }
    
    // Aplicar movimiento según el tipo
    if (m_canFly && m_enemyType == EnemyType::MURCIELAGO) {
        handleMurcieelagoAI(targetPosition, deltaTime);
    } else {
        // ===================================
        // CORREGIDO: Fuerza de movimiento aumentada
        // ===================================
        applyMovementForce(moveDirection * 1.5f);  // Multiplicador adicional
        
        if (m_enemyType == EnemyType::ESQUELETO && m_isGrounded && std::abs(direction.y) > 30.0f) {
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
            setMoving(false);  // ← NUEVO: Parar animacion al morir
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
    
}

// ===================================
// Volar (murciélagos)
// ===================================
void CEnemy::fly() {
    if (!m_physicsEnabled || !m_physicsBody || !m_canFly) return;
    
    m_physics->applyForce(this, 0.0f, -m_flyForce);
}

// ===================================
// Patrullar automaticamente
// ===================================
void CEnemy::patrol() {
    if (!m_physicsEnabled || !m_physicsBody) return;
    
    // ===================================
    // CORREGIDO: Patrullaje mas dinamico
    // ===================================
    applyMovementForce(static_cast<float>(m_movementDirection) * 0.8f);  // Aumentado de 0.5f
    setMoving(true);
}

// ===================================
// Seguir objetivo con fisicas
// ===================================
void CEnemy::followTarget(const sf::Vector2f& target, float deltaTime) {
    moveWithPhysics(target, deltaTime);
}

// ===================================
// IA especifica para murciélagos (vuelan)
// ===================================
void CEnemy::handleMurcieelagoAI(const sf::Vector2f& playerPosition, float deltaTime) {
    if (!m_physicsEnabled || !m_physicsBody) return;
    
    sf::Vector2f direction = playerPosition - m_position;
    float distance = calculateDistance(m_position, playerPosition);
    
    // ===================================
    // CORREGIDO: IA de murciélago mas efectiva
    // ===================================
    if (distance <= m_detectionRange * 1.5f && distance > m_attackRange) {  // 50% mas de rango
        float forceX = (direction.x > 0) ? m_movementForce : -m_movementForce;
        float forceY = (direction.y > 0) ? m_flyForce : -m_flyForce;
        
        // ===================================
        // CORREGIDO: Fuerzas mas fuertes
        // ===================================
        m_physics->applyForce(this, forceX * 0.8f, forceY * 0.5f);  // Aumentado de 0.5f y 0.3f
        setMoving(true);
        
        // ===================================
        // CORREGIDO: Velocidad maxima mas alta
        // ===================================
        b2Vec2 velocity = m_physicsBody->GetLinearVelocity();
        if (velocity.Length() > 10.0f) {  // Aumentado de 8.0f
            velocity.Normalize();
            velocity *= 10.0f;
            m_physicsBody->SetLinearVelocity(velocity);
        }
    }
}

// ===================================
// IA especifica para esqueletos
// ===================================
void CEnemy::handleEsqueletoAI(const sf::Vector2f& playerPosition, float deltaTime) {
    if (!m_physicsEnabled || !m_physicsBody) return;
    
    sf::Vector2f direction = playerPosition - m_position;
    float distance = calculateDistance(m_position, playerPosition);
    
    // ===================================
    // CORREGIDO: Rango de deteccion mas amplio
    // ===================================
    if (distance <= m_detectionRange * 1.2f && distance > m_attackRange) {  // 20% mas de rango
        float moveDirection = (direction.x > 0) ? 1.0f : -1.0f;
        
        // ===================================
        // CORREGIDO: Movimiento mas agresivo
        // ===================================
        applyMovementForce(moveDirection * 1.3f);  // Multiplicador adicional
        setMoving(true);
        
        // ===================================
        // CORREGIDO: Salto mas frecuente
        // ===================================
        if (m_isGrounded && (direction.y < -20.0f || std::abs(direction.x) < 30.0f)) {  // Mas sensible
            jump();
        }
    } else if (distance > m_detectionRange) {
        // ===================================
        // CORREGIDO: Patrullaje mas activo
        // ===================================
        patrol();
    } else {
        setMoving(false);
    }
}

// ===================================
// IA especifica para zombies
// ===================================
void CEnemy::handleZombieAI(const sf::Vector2f& playerPosition, float deltaTime) {
    if (!m_physicsEnabled || !m_physicsBody) return;
    
    sf::Vector2f direction = playerPosition - m_position;
    float distance = calculateDistance(m_position, playerPosition);
    
    // ===================================
    // CORREGIDO: IA de zombie mas persistente
    // ===================================
    if (distance <= m_detectionRange * 1.3f && distance > m_attackRange) {  // 30% mas de rango
        float moveDirection = (direction.x > 0) ? 1.0f : -1.0f;
        
        // ===================================
        // CORREGIDO: Zombies menos lentos
        // ===================================
        applyMovementForce(moveDirection * 1.0f);  // Era 0.7f, ahora 1.0f
        setMoving(true);
    } else if (distance > m_detectionRange) {
        patrol();
    } else {
        setMoving(false);
    }
}

// ===================================
// Aplicar fuerza de movimiento
// ===================================
void CEnemy::applyMovementForce(float direction) {
    if (!m_physicsEnabled || !m_physicsBody || direction == 0.0f) return;
    
    b2Vec2 velocity = m_physicsBody->GetLinearVelocity();
    
    // ===================================
    // CORREGIDO: Velocidades maximas mas altas y diferenciadas
    // ===================================
    float maxVelocity;
    switch (m_enemyType) {
        case EnemyType::ZOMBIE:
            maxVelocity = 3.0f;  // Aumentado de 2.0f
            break;
        case EnemyType::ESQUELETO:
            maxVelocity = 5.0f;  // Aumentado de 4.0f
            break;
        case EnemyType::MURCIELAGO:
            maxVelocity = 6.0f;  // Aumentado de 4.0f
            break;
        default:
            maxVelocity = 4.0f;
            break;
    }
    
    // ===================================
    // CORREGIDO: Fuerza base mas alta
    // ===================================
    if (std::abs(velocity.x) < maxVelocity) {
        float force = direction * m_movementForce * 1.2f;  // Multiplicador adicional
        m_physics->applyForce(this, force, 0.0f);
    }
    
    // ===================================
    // NUEVO: Ayuda adicional si esta muy lento
    // ===================================
    if (std::abs(velocity.x) < 0.5f && direction != 0.0f) {
        // Impulso adicional si esta casi parado
        float boostForce = direction * m_movementForce * 2.0f;
        m_physics->applyForce(this, boostForce, 0.0f);
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
// Actualizar estado basado en fisicas
// ===================================
void CEnemy::updatePhysicsState() {
    if (!m_physicsEnabled) return;
    
    checkGroundState();
}

// ===================================
// Actualizar direccion de movimiento
// ===================================
void CEnemy::updateMovementDirection(float deltaTime) {
    m_lastDirectionChange += deltaTime;
    
    if (m_lastDirectionChange >= DIRECTION_CHANGE_TIME) {
        m_movementDirection = (rand() % 3) - 1;
        m_lastDirectionChange = 0.0f;
    }
}

// IA BaSICA
void CEnemy::updateAI(const sf::Vector2f& playerPosition, float deltaTime) {
    if (!isAlive()) return;
    
    float distanceToPlayer = calculateDistance(m_position, playerPosition);
    
    updateMovementDirection(deltaTime);
    
    // ===================================
    // CORREGIDO: Logica de IA mas agresiva
    // ===================================
    if (distanceToPlayer <= m_detectionRange * 1.2f) {  // 20% mas de rango base
        if (distanceToPlayer <= m_attackRange && canAttack()) {
            attack();
            setMoving(false);
        } else {
            // ===================================
            // CORREGIDO: Siempre usar fisicas si esta disponible
            // ===================================
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
                // Fallback sin fisicas
                moveTowards(playerPosition, deltaTime);
            }
        }
    } else {
        // ===================================
        // CORREGIDO: Patrullaje cuando no detecta al jugador
        // ===================================
        if (m_physicsEnabled) {
            patrol();
        } else {
            setMoving(false);
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
    // NUEVO: Actualizar animacion
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
            // NUEVO: Renderizar con textura y animacion
            // ===================================
            window.draw(m_enemySprite);
        } else {
            // Fallback: renderizar rectangulo de color
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
    std::cout << "Posicion: (" << m_position.x << ", " << m_position.y << ")\n";
    std::cout << "Velocidad: " << m_speed << "\n";
    std::cout << "Rango deteccion: " << m_detectionRange << "\n";
    std::cout << "Rango ataque: " << m_attackRange << "\n";
    std::cout << "Estado: " << (isAlive() ? "Vivo" : "Muerto") << "\n";
    std::cout << "Texturas: " << (m_texturesLoaded ? "Cargadas" : "No cargadas") << "\n";
    std::cout << "Animacion: " << (m_currentState == EnemyState::IDLE ? "IDLE" : "MOVING") << "\n";
    std::cout << "Frame actual: " << m_currentFrame << "\n";
    std::cout << "========================\n";
}

void CEnemy::printPhysicsStatus() const {
    std::cout << "=== FiSICAS DEL ENEMIGO " << m_type << " ===" << std::endl;
    std::cout << "Fisicas habilitadas: " << (m_physicsEnabled ? "Si" : "NO") << std::endl;
    std::cout << "En el suelo: " << (m_isGrounded ? "Si" : "NO") << std::endl;
    std::cout << "Puede volar: " << (m_canFly ? "Si" : "NO") << std::endl;
    std::cout << "Direccion de movimiento: " << m_movementDirection << std::endl;
    
    if (m_physicsEnabled && m_physicsBody) {
        b2Vec2 pos = m_physicsBody->GetPosition();
        b2Vec2 vel = m_physicsBody->GetLinearVelocity();
        
        std::cout << "Posicion fisica: (" << pos.x << ", " << pos.y << ") metros" << std::endl;
        std::cout << "Velocidad: (" << vel.x << ", " << vel.y << ") m/s" << std::endl;
        
        sf::Vector2f pixelPos = CPhysics::metersToPixels(pos);
        std::cout << "Posicion en pixeles: (" << pixelPos.x << ", " << pixelPos.y << ")" << std::endl;
    }
    
    std::cout << "============================" << std::endl;
}

// ===================================
// NUEVO: Debug de sprites
// ===================================
void CEnemy::printSpriteStatus() const {
    std::cout << "=== SPRITES DEL ENEMIGO " << m_type << " ===" << std::endl;
    std::cout << "Texturas cargadas: " << (m_texturesLoaded ? "Si" : "NO") << std::endl;
    std::cout << "Estado actual: " << (m_currentState == EnemyState::IDLE ? "IDLE" : "MOVING") << std::endl;
    std::cout << "Frame actual: " << m_currentFrame << std::endl;
    std::cout << "En movimiento: " << (m_isMoving ? "Si" : "NO") << std::endl;
    std::cout << "Velocidad animacion: " << m_animationSpeed << std::endl;
    
    if (m_texturesLoaded) {
        sf::IntRect rect = getCurrentFrameRect();
        std::cout << "Rectangulo actual: (" << rect.left << "," << rect.top 
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
            m_detectionRange = 200.0f;  // Aumentado de 150.0f
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
            m_detectionRange = 160.0f;  // Aumentado de 120.0f
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
            m_detectionRange = 140.0f;  // Aumentado de 100.0f
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
            m_detectionRange = 120.0f;
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
// NUEVO: Métodos de sprites y animacion
// ===================================

void CEnemy::loadEnemyTextures() {
    
    std::string textureFile = getTextureFileName();
    std::string fullPath = "assets/" + textureFile;
    
    std::cout << "Intentando cargar: " << fullPath << std::endl;
    
    if (!m_enemyTexture.loadFromFile(fullPath)) {
        std::cerr << " Error: No se pudo cargar " << fullPath << std::endl;
        std::cerr << "   Usando fallback (rectangulo de color)" << std::endl;
        m_texturesLoaded = false;
        return;
    }
    
    sf::Vector2u textureSize = m_enemyTexture.getSize();
    std::cout << " " << textureFile << " cargado (" << textureSize.x << "x" << textureSize.y << ")" << std::endl;
    
    // Configurar sprite inicial
    m_texturesLoaded = true;
    m_enemySprite.setTexture(m_enemyTexture);
    m_enemySprite.setPosition(m_position);
    
    // ✨ NUEVO: Aplicar escalado según el tipo de enemigo
    switch (m_enemyType) {
        case EnemyType::ZOMBIE:
            m_enemySprite.setScale(ZombieSprites::SCALE_X, ZombieSprites::SCALE_Y);
            break;
            
        case EnemyType::ESQUELETO:
            m_enemySprite.setScale(SkeletonSprites::SCALE_X, SkeletonSprites::SCALE_Y);
            break;
            
        case EnemyType::MURCIELAGO:
            m_enemySprite.setScale(MurcielagoSprites::SCALE_X, MurcielagoSprites::SCALE_Y);
            break;
    }
    
    // Configurar frame inicial (IDLE)
    updateSpriteFrame();
    
    // Imprimir configuracion del sprite

    
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

std::string CEnemy::getTextureFileName() const 
{
    switch (m_enemyType) 
    {
        case EnemyType::ZOMBIE: return "zombie.png";
        case EnemyType::ESQUELETO: return "skeleton.png";
        case EnemyType::MURCIELAGO: return "murcielago.png";
        default: return "unknown.png";
    }
}