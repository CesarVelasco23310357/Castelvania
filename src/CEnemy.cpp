#include "CEnemy.hpp"
#include <iostream>
#include <cmath>

// Constructor
CEnemy::CEnemy(EnemyType type, float x, float y) 
    : m_enemyType(type), m_position(x, y), m_currentCooldown(0.0f) {
    
    // Configurar el tipo de enemigo
    setupEnemyType(type);
    
    // Configurar el sprite
    m_sprite.setSize(sf::Vector2f(28.0f, 28.0f));
    m_sprite.setFillColor(m_color);
    m_sprite.setPosition(m_position);
    m_originalColor = m_color;
    
    std::cout << "Enemigo " << m_type << " creado en posición (" 
              << x << ", " << y << ")\n";
}

// Destructor
CEnemy::~CEnemy() {
    std::cout << "Enemigo " << m_type << " destruido.\n";
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

// MÉTODOS DE GAMEPLAY
void CEnemy::moveTowards(const sf::Vector2f& targetPosition, float deltaTime) {
    if (!isAlive()) return;
    
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

// IA BÁSICA
void CEnemy::updateAI(const sf::Vector2f& playerPosition, float deltaTime) {
    if (!isAlive()) return;
    
    float distanceToPlayer = calculateDistance(m_position, playerPosition);
    
    // Si el jugador está en rango de detección
    if (distanceToPlayer <= m_detectionRange) {
        // Si está en rango de ataque, atacar
        if (distanceToPlayer <= m_attackRange && canAttack()) {
            attack();
        } else {
            // Si no está en rango de ataque, moverse hacia él
            moveTowards(playerPosition, deltaTime);
        }
    }
}

// MÉTODOS SFML
void CEnemy::update(float deltaTime) {
    // Actualizar cooldown de ataque
    if (m_currentCooldown > 0.0f) {
        m_currentCooldown -= deltaTime;
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