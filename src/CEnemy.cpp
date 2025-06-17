#include "CEnemy.hpp"
#include "CPhysics.hpp"
#include <iostream>
#include <cmath>

// Constructor
CEnemy::CEnemy(EnemyType type, float x, float y) 
    : m_enemyType(type), m_position(x, y), m_currentCooldown(0.0f),
      m_physics(nullptr),           // ← NUEVO: Referencia al sistema de físicas
      m_physicsBody(nullptr),       // ← NUEVO: Cuerpo físico
      m_physicsEnabled(false),      // ← NUEVO: Estado de físicas
      m_isGrounded(false),          // ← NUEVO: Estado en el suelo
      m_canFly(false),              // ← NUEVO: Capacidad de volar
      m_jumpForce(0.0f),            // ← NUEVO: Fuerza de salto
      m_flyForce(0.0f),             // ← NUEVO: Fuerza de vuelo
      m_movementForce(0.0f),        // ← NUEVO: Fuerza de movimiento
      m_lastDirectionChange(0.0f),  // ← NUEVO: Timer para cambio de dirección
      m_movementDirection(1) {      // ← NUEVO: Dirección inicial (derecha)
    
    // Configurar el tipo de enemigo
    setupEnemyType(type);
    
    // Configurar el sprite
    m_sprite.setSize(sf::Vector2f(28.0f, 28.0f));
    m_sprite.setFillColor(m_color);
    m_sprite.setPosition(m_position);
    m_originalColor = m_color;
    
    std::cout << "Enemigo " << m_type << " creado en posición (" 
              << x << ", " << y << ")" << std::endl;
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
    return m_sprite.getGlobalBounds();
}

float CEnemy::getDetectionRange() const {
    return m_detectionRange;
}

float CEnemy::getAttackRange() const {
    return m_attackRange;
}

// ===================================
// NUEVO: Getters para físicas
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

// SETTERS
void CEnemy::setPosition(float x, float y) {
    m_position.x = x;
    m_position.y = y;
    m_sprite.setPosition(m_position);
}

void CEnemy::setPosition(const sf::Vector2f& position) {
    m_position = position;
    m_sprite.setPosition(m_position);
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
// NUEVO: Inicializar físicas del enemigo
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
// NUEVO: Configurar físicas según el tipo
// ===================================
void CEnemy::setupPhysicsForType() {
    if (!m_physicsEnabled || !m_physicsBody) return;
    
    switch (m_enemyType) {
        case EnemyType::MURCIELAGO:
            m_canFly = true;
            m_flyForce = MURCIELAGO_FLY_FORCE;
            m_movementForce = DEFAULT_MOVEMENT_FORCE;
            // Los murciélagos tienen menor gravedad
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
// NUEVO: Sincronizar posición desde físicas
// ===================================
void CEnemy::syncPositionFromPhysics() {
    if (!m_physicsEnabled || !m_physicsBody) return;
    
    // Obtener posición del cuerpo físico
    b2Vec2 physicsPos = m_physicsBody->GetPosition();
    sf::Vector2f newPos = CPhysics::metersToPixels(physicsPos);
    
    // Actualizar posición visual
    m_position = newPos;
    m_sprite.setPosition(m_position);
    
    // Actualizar estados basados en físicas
    updatePhysicsState();
}

// ===================================
// NUEVO: Actualizar posición en físicas
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
    // Calcular dirección hacia el objetivo
    sf::Vector2f direction = targetPosition - m_position;
    float distance = calculateDistance(m_position, targetPosition);
    
    // Si está muy cerca, no moverse más
    if (distance <= m_attackRange) {
        return;
    }
    
    // Normalizar la dirección y aplicar velocidad
    if (distance > 0) {
        direction.x /= distance;
        direction.y /= distance;
        
        // Mover hacia el objetivo
        sf::Vector2f movement = direction * m_speed * deltaTime;
        m_position += movement;
        m_sprite.setPosition(m_position);
    }
}

// ===================================
// NUEVO: Movimiento con físicas
// ===================================
void CEnemy::moveWithPhysics(const sf::Vector2f& targetPosition, float deltaTime) {
    if (!m_physicsEnabled || !m_physicsBody || !isAlive()) return;
    
    float distance = calculateDistance(m_position, targetPosition);
    
    // Si está muy cerca, no moverse más
    if (distance <= m_attackRange) {
        return;
    }
    
    // Calcular dirección hacia el objetivo
    sf::Vector2f direction = targetPosition - m_position;
    float moveDirection = 0.0f;
    
    if (std::abs(direction.x) > 10.0f) { // Umbral mínimo para movimiento
        moveDirection = (direction.x > 0) ? 1.0f : -1.0f;
    }
    
    // Aplicar movimiento según el tipo
    if (m_canFly && m_enemyType == EnemyType::MURCIELAGO) {
        handleMurcieelagoAI(targetPosition, deltaTime);
    } else {
        // Movimiento terrestre
        applyMovementForce(moveDirection);
        
        // Los esqueletos pueden saltar hacia el objetivo
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
        
        // Efecto visual al recibir daño
        if (m_health > 0) {
            m_sprite.setFillColor(sf::Color::Red);
        } else {
            m_sprite.setFillColor(sf::Color::Black);
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
// NUEVO: Saltar (esqueletos principalmente)
// ===================================
void CEnemy::jump() {
    if (!m_physicsEnabled || !m_physicsBody || !m_isGrounded || m_jumpForce <= 0.0f) {
        return;
    }
    
    // Aplicar impulso hacia arriba
    m_physics->applyImpulse(this, 0.0f, -m_jumpForce);
    m_isGrounded = false;
    
    std::cout << "🦘 " << m_type << " salta! Fuerza: " << m_jumpForce << std::endl;
}

// ===================================
// NUEVO: Volar (murciélagos)
// ===================================
void CEnemy::fly() {
    if (!m_physicsEnabled || !m_physicsBody || !m_canFly) return;
    
    // Aplicar fuerza hacia arriba para contrarrestar la gravedad
    m_physics->applyForce(this, 0.0f, -m_flyForce);
}

// ===================================
// NUEVO: Patrullar automáticamente
// ===================================
void CEnemy::patrol() {
    if (!m_physicsEnabled || !m_physicsBody) return;
    
    // Aplicar movimiento en la dirección actual
    applyMovementForce(static_cast<float>(m_movementDirection) * 0.5f);
}

// ===================================
// NUEVO: Seguir objetivo con físicas
// ===================================
void CEnemy::followTarget(const sf::Vector2f& target, float deltaTime) {
    moveWithPhysics(target, deltaTime);
}

// ===================================
// NUEVO: IA específica para murciélagos (vuelan)
// ===================================
void CEnemy::handleMurcieelagoAI(const sf::Vector2f& playerPosition, float deltaTime) {
    if (!m_physicsEnabled || !m_physicsBody) return;
    
    sf::Vector2f direction = playerPosition - m_position;
    float distance = calculateDistance(m_position, playerPosition);
    
    if (distance <= m_detectionRange && distance > m_attackRange) {
        // Volar hacia el jugador
        float forceX = (direction.x > 0) ? m_movementForce : -m_movementForce;
        float forceY = (direction.y > 0) ? m_flyForce : -m_flyForce;
        
        // Aplicar fuerza de vuelo
        m_physics->applyForce(this, forceX * 0.5f, forceY * 0.3f);
        
        // Limitar velocidad máxima
        b2Vec2 velocity = m_physicsBody->GetLinearVelocity();
        if (velocity.Length() > 8.0f) {
            velocity.Normalize();
            velocity *= 8.0f;
            m_physicsBody->SetLinearVelocity(velocity);
        }
    }
}

// ===================================
// NUEVO: IA específica para esqueletos
// ===================================
void CEnemy::handleEsqueletoAI(const sf::Vector2f& playerPosition, float deltaTime) {
    if (!m_physicsEnabled || !m_physicsBody) return;
    
    sf::Vector2f direction = playerPosition - m_position;
    float distance = calculateDistance(m_position, playerPosition);
    
    if (distance <= m_detectionRange && distance > m_attackRange) {
        // Movimiento horizontal hacia el jugador
        float moveDirection = (direction.x > 0) ? 1.0f : -1.0f;
        applyMovementForce(moveDirection);
        
        // Saltar si hay obstáculos o si el jugador está más alto
        if (m_isGrounded && (direction.y < -30.0f || std::abs(direction.x) < 50.0f)) {
            jump();
        }
    } else if (distance > m_detectionRange) {
        // Patrullar cuando no detecta al jugador
        patrol();
    }
}

// ===================================
// NUEVO: IA específica para zombies
// ===================================
void CEnemy::handleZombieAI(const sf::Vector2f& playerPosition, float deltaTime) {
    if (!m_physicsEnabled || !m_physicsBody) return;
    
    sf::Vector2f direction = playerPosition - m_position;
    float distance = calculateDistance(m_position, playerPosition);
    
    if (distance <= m_detectionRange && distance > m_attackRange) {
        // Movimiento lento hacia el jugador
        float moveDirection = (direction.x > 0) ? 1.0f : -1.0f;
        applyMovementForce(moveDirection * 0.7f); // Movimiento más lento
    } else if (distance > m_detectionRange) {
        // Patrullar lentamente
        patrol();
    }
}

// ===================================
// NUEVO: Aplicar fuerza de movimiento
// ===================================
void CEnemy::applyMovementForce(float direction) {
    if (!m_physicsEnabled || !m_physicsBody || direction == 0.0f) return;
    
    // Obtener velocidad actual
    b2Vec2 velocity = m_physicsBody->GetLinearVelocity();
    
    // Limitar velocidad horizontal máxima según el tipo
    float maxVelocity = (m_enemyType == EnemyType::ZOMBIE) ? 2.0f : 4.0f;
    
    if (std::abs(velocity.x) < maxVelocity) {
        float force = direction * m_movementForce;
        m_physics->applyForce(this, force, 0.0f);
    }
}

// ===================================
// NUEVO: Verificar estado en el suelo
// ===================================
void CEnemy::checkGroundState() {
    if (!m_physicsEnabled || !m_physicsBody) {
        m_isGrounded = true; // Asumir que está en el suelo sin físicas
        return;
    }
    
    // Verificar velocidad vertical
    b2Vec2 velocity = m_physicsBody->GetLinearVelocity();
    
    // Está en el suelo si la velocidad vertical es muy pequeña
    m_isGrounded = std::abs(velocity.y) < 0.5f;
}

// ===================================
// NUEVO: Actualizar estado basado en físicas
// ===================================
void CEnemy::updatePhysicsState() {
    if (!m_physicsEnabled) return;
    
    checkGroundState();
}

// ===================================
// NUEVO: Actualizar dirección de movimiento
// ===================================
void CEnemy::updateMovementDirection(float deltaTime) {
    m_lastDirectionChange += deltaTime;
    
    // Cambiar dirección cada cierto tiempo
    if (m_lastDirectionChange >= DIRECTION_CHANGE_TIME) {
        m_movementDirection = (rand() % 3) - 1; // -1, 0, o 1
        m_lastDirectionChange = 0.0f;
    }
}

// IA BÁSICA
void CEnemy::updateAI(const sf::Vector2f& playerPosition, float deltaTime) {
    if (!isAlive()) return;
    
    float distanceToPlayer = calculateDistance(m_position, playerPosition);
    
    // Actualizar dirección de patrullaje
    updateMovementDirection(deltaTime);
    
    // Si el jugador está en rango de detección
    if (distanceToPlayer <= m_detectionRange) {
        // Si está en rango de ataque, atacar
        if (distanceToPlayer <= m_attackRange && canAttack()) {
            attack();
        } else {
            // Moverse hacia el jugador según el tipo
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
                // Fallback al movimiento tradicional
                moveTowards(playerPosition, deltaTime);
            }
        }
    } else {
        // Patrullar cuando no detecta al jugador
        if (m_physicsEnabled) {
            patrol();
        }
    }
}

// MÉTODOS SFML
void CEnemy::update(float deltaTime) {
    // Actualizar cooldown de ataque
    if (m_currentCooldown > 0.0f) {
        m_currentCooldown -= deltaTime;
    }
    
    // *** NUEVO: Actualizar estado físico ***
    if (m_physicsEnabled) {
        updatePhysicsState();
    }
    
    // Restaurar color original después de recibir daño
    if (isAlive() && m_sprite.getFillColor() == sf::Color::Red) {
        m_sprite.setFillColor(m_originalColor);
    }
}

void CEnemy::render(sf::RenderWindow& window) {
    if (isAlive()) {
        window.draw(m_sprite);
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
    std::cout << "========================\n";
}

// ===================================
// NUEVO: Debug de físicas
// ===================================
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
            break;
            
        default:
            // Valores por defecto
            m_health = 50;
            m_maxHealth = 50;
            m_damage = 15;
            m_speed = 70.0f;
            m_detectionRange = 100.0f;
            m_attackRange = 40.0f;
            m_attackCooldown = 1.5f;
            m_color = sf::Color::Red;
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