#include "CPlayer.hpp"
#include "CPhysics.hpp"
#include <iostream>

// Constructor
CPlayer::CPlayer(const std::string& name) 
    : m_name(name), m_health(100), m_maxHealth(100), 
      m_position(0.0f, 0.0f), m_speed(150.0f), m_color(sf::Color::Blue),
      m_physics(nullptr),           // ← NUEVO: Referencia al sistema de físicas
      m_physicsBody(nullptr),       // ← NUEVO: Cuerpo físico
      m_physicsEnabled(false),      // ← NUEVO: Estado de físicas
      m_isGrounded(false),          // ← NUEVO: Estado en el suelo
      m_jumpForce(DEFAULT_JUMP_FORCE), // ← NUEVO: Fuerza de salto
      m_texturesLoaded(false), m_currentState(PlayerState::IDLE),
      m_currentFrame(0), m_animationTimer(0.0f), m_animationSpeed(IDLE_ANIMATION_SPEED),
      m_hurtTimer(0.0f), m_isHurt(false) {
    
    std::cout << "🔧 DEBUG: Creando jugador " << m_name << std::endl;
    
    // Configurar el sprite del jugador (fallback)
    m_sprite.setSize(sf::Vector2f(32.0f, 32.0f));
    m_sprite.setFillColor(m_color);
    m_sprite.setPosition(m_position);
    
    std::cout << "🔧 DEBUG: Intentando cargar texturas..." << std::endl;
    
    // Intentar cargar texturas
    loadPlayerTextures();
    
    std::cout << "🔧 DEBUG: Texturas cargadas: " << (m_texturesLoaded ? "SÍ" : "NO") << std::endl;
    std::cout << "Jugador " << m_name << " creado exitosamente." << std::endl;
}

// Destructor
CPlayer::~CPlayer() {
    // El sistema de físicas se encarga de limpiar los cuerpos automáticamente
    std::cout << "Jugador " << m_name << " destruido.\n";
}

// GETTERS
const std::string& CPlayer::getName() const {
    return m_name;
}

int CPlayer::getHealth() const {
    return m_health;
}

int CPlayer::getMaxHealth() const {
    return m_maxHealth;
}

sf::Vector2f CPlayer::getPosition() const {
    return m_position;
}

float CPlayer::getSpeed() const {
    return m_speed;
}

sf::FloatRect CPlayer::getBounds() const {
    return m_sprite.getGlobalBounds();
}

// ===================================
// NUEVO: Getters para físicas
// ===================================
bool CPlayer::isGrounded() const {
    return m_isGrounded;
}

bool CPlayer::isJumping() const {
    return m_currentState == PlayerState::JUMPING;
}

bool CPlayer::isFalling() const {
    return m_currentState == PlayerState::FALLING;
}

b2Body* CPlayer::getPhysicsBody() const {
    return m_physicsBody;
}

sf::Vector2f CPlayer::getVelocity() const {
    if (!m_physicsEnabled || !m_physicsBody) {
        return sf::Vector2f(0.0f, 0.0f);
    }
    
    b2Vec2 velocity = m_physicsBody->GetLinearVelocity();
    return CPhysics::b2VecToSFML(velocity);
}

// SETTERS
void CPlayer::setPosition(float x, float y) {
    m_position.x = x;
    m_position.y = y;
    m_sprite.setPosition(m_position);
    
    // También actualizar sprite con textura si está cargada
    if (m_texturesLoaded) {
        m_playerSprite.setPosition(m_position);
    }
}

void CPlayer::setPosition(const sf::Vector2f& position) {
    m_position = position;
    m_sprite.setPosition(m_position);
    
    // También actualizar sprite con textura si está cargada
    if (m_texturesLoaded) {
        m_playerSprite.setPosition(m_position);
    }
}

void CPlayer::setHealth(int health) {
    m_health = health;
    // Asegurar que la salud no exceda el máximo ni sea menor a 0
    if (m_health > m_maxHealth) {
        m_health = m_maxHealth;
    } else if (m_health < 0) {
        m_health = 0;
    }
}

void CPlayer::setSpeed(float speed) {
    if (speed >= 0) {
        m_speed = speed;
    }
}

// ===================================
// NUEVO: Setter para fuerza de salto
// ===================================
void CPlayer::setJumpForce(float force) {
    if (force > 0) {
        m_jumpForce = force;
        std::cout << "💪 Fuerza de salto configurada: " << m_jumpForce << std::endl;
    }
}

// ===================================
// NUEVO: Inicializar físicas del jugador
// ===================================
void CPlayer::initializePhysics(CPhysics* physics) {
    if (!physics) {
        std::cerr << "❌ Error: Sistema de físicas nulo" << std::endl;
        return;
    }
    
    std::cout << "⚙️ Inicializando físicas del jugador..." << std::endl;
    
    m_physics = physics;
    
    // Crear cuerpo físico del jugador
    m_physicsBody = m_physics->createPlayerBody(m_position.x, m_position.y, this);
    
    if (m_physicsBody) {
        m_physicsEnabled = true;
        std::cout << "✅ Cuerpo físico del jugador creado exitosamente" << std::endl;
        
        // Configurar propiedades iniciales
        updatePhysicsPosition();
    } else {
        std::cerr << "❌ Error: No se pudo crear el cuerpo físico del jugador" << std::endl;
        m_physicsEnabled = false;
    }
}

// ===================================
// NUEVO: Sincronizar posición desde físicas
// ===================================
void CPlayer::syncPositionFromPhysics() {
    if (!m_physicsEnabled || !m_physicsBody) return;
    
    // Obtener posición del cuerpo físico
    b2Vec2 physicsPos = m_physicsBody->GetPosition();
    sf::Vector2f newPos = CPhysics::metersToPixels(physicsPos);
    
    // Actualizar posición visual
    m_position = newPos;
    m_sprite.setPosition(m_position);
    
    if (m_texturesLoaded) {
        m_playerSprite.setPosition(m_position);
    }
    
    // Actualizar estados basados en físicas
    updatePhysicsState();
}

// ===================================
// NUEVO: Actualizar posición en físicas
// ===================================
void CPlayer::updatePhysicsPosition() {
    if (!m_physicsEnabled || !m_physicsBody) return;
    
    // Convertir posición visual a físicas
    b2Vec2 physicsPos = CPhysics::sfmlVecToB2(m_position);
    m_physicsBody->SetTransform(physicsPos, m_physicsBody->GetAngle());
}

// ===================================================
// MÉTODOS PARA CONFIGURAR SPRITES MANUALMENTE
// ===================================================

void CPlayer::setIdleSprite(int startX, int startY, int frameWidth, int frameHeight, int frameCount) {
    std::cout << "🎭 Configurando IDLE sprite: (" << startX << "," << startY << ") " 
              << frameWidth << "x" << frameHeight << " [" << frameCount << " frames]" << std::endl;
}

void CPlayer::setRunSprite(int startX, int startY, int frameWidth, int frameHeight, int frameCount) {
    std::cout << "🏃 Configurando RUN sprite: (" << startX << "," << startY << ") " 
              << frameWidth << "x" << frameHeight << " [" << frameCount << " frames]" << std::endl;
}

void CPlayer::setAttackSprite(int startX, int startY, int frameWidth, int frameHeight, int frameCount) {
    std::cout << "⚔️ Configurando ATTACK sprite: (" << startX << "," << startY << ") " 
              << frameWidth << "x" << frameHeight << " [" << frameCount << " frames]" << std::endl;
}

void CPlayer::setHurtSprite(int startX, int startY, int frameWidth, int frameHeight, int frameCount) {
    std::cout << "💥 Configurando HURT sprite: (" << startX << "," << startY << ") " 
              << frameWidth << "x" << frameHeight << " [" << frameCount << " frames]" << std::endl;
}

// MÉTODOS DE GAMEPLAY
void CPlayer::move(float deltaX, float deltaY) {
    // No permitir movimiento si está en estado hurt
    if (m_currentState == PlayerState::HURT) {
        return;
    }
    
    m_position.x += deltaX;
    m_position.y += deltaY;
    m_sprite.setPosition(m_position);
    
    // También actualizar sprite con textura si está cargada
    if (m_texturesLoaded) {
        m_playerSprite.setPosition(m_position);
    }
    
    // Cambiar a estado de correr si nos estamos moviendo y no estamos atacando o heridos
    if ((deltaX != 0 || deltaY != 0) && m_currentState != PlayerState::ATTACKING && m_currentState != PlayerState::HURT) {
        setRunning(true);
    }
}

// ===================================
// NUEVO: Movimiento con físicas
// ===================================
void CPlayer::moveWithPhysics(float direction) {
    if (!m_physicsEnabled || !m_physicsBody) {
        // Fallback al movimiento tradicional
        move(direction * m_speed * 0.016f, 0); // Aproximar deltaTime = 0.016f
        return;
    }
    
    // No permitir movimiento si está en estado hurt
    if (m_currentState == PlayerState::HURT) {
        return;
    }
    
    // Aplicar fuerza horizontal
    float force = direction * MOVEMENT_FORCE;
    
    // Obtener velocidad actual
    b2Vec2 velocity = m_physicsBody->GetLinearVelocity();
    
    // Limitar velocidad horizontal máxima
    if (std::abs(velocity.x) < MAX_VELOCITY_X) {
        m_physics->applyForce(this, force, 0.0f);
    }
    
    // Actualizar estado de animación
    updateAnimationFromPhysics();
}

// ===================================
// NUEVO: Saltar
// ===================================
void CPlayer::jump() {
    if (!m_physicsEnabled || !m_physicsBody) {
        std::cout << "⚠️ Salto sin físicas no implementado" << std::endl;
        return;
    }
    
    // No permitir salto si está en estado hurt
    if (m_currentState == PlayerState::HURT) {
        return;
    }
    
    if (!m_isGrounded) {
        std::cout << "❌ No se puede saltar: jugador no está en el suelo" << std::endl;
        return;
    }
    
    // Aplicar impulso hacia arriba
    m_physics->applyImpulse(this, 0.0f, -m_jumpForce);
    
    // Cambiar estado de animación
    startJump();
    m_isGrounded = false; // Se actualizará en checkGroundState()
    
    std::cout << "🦘 ¡Salto aplicado! Fuerza: " << m_jumpForce << std::endl;
}

void CPlayer::attack() {
    // No permitir ataque si está en estado hurt
    if (m_currentState == PlayerState::HURT) {
        return;
    }
    
    std::cout << m_name << " realiza un ataque!\n";
    
    // Cambiar a estado de ataque
    startAttack();
}

void CPlayer::setRunning(bool running) {
    // No cambiar estado si está herido o atacando
    if (m_currentState == PlayerState::HURT || m_currentState == PlayerState::ATTACKING) {
        return;
    }
    
    if (running) {
        if (m_currentState != PlayerState::RUNNING) {
            m_currentState = PlayerState::RUNNING;
            m_currentFrame = 0;
            m_animationTimer = 0.0f;
            m_animationSpeed = RUN_ANIMATION_SPEED;
        }
    } else if (m_currentState == PlayerState::RUNNING) {
        // Cambiar de RUNNING a IDLE
        m_currentState = PlayerState::IDLE;
        m_currentFrame = 0;
        m_animationTimer = 0.0f;
        m_animationSpeed = IDLE_ANIMATION_SPEED;
    }
}

void CPlayer::startAttack() {
    m_currentState = PlayerState::ATTACKING;
    m_currentFrame = 0;
    m_animationTimer = 0.0f;
    m_animationSpeed = ATTACK_ANIMATION_SPEED;
}

bool CPlayer::isAttacking() const {
    return m_currentState == PlayerState::ATTACKING;
}

void CPlayer::startHurt() {
    std::cout << "💥 DEBUG: Iniciando estado HURT" << std::endl;
    m_currentState = PlayerState::HURT;
    m_currentFrame = 0;
    m_animationTimer = 0.0f;
    m_animationSpeed = HURT_ANIMATION_SPEED;
    m_hurtTimer = HURT_DURATION;
    m_isHurt = true;
    
    // Forzar actualización del sprite inmediatamente
    if (m_texturesLoaded) {
        updateSpriteFrame();
        std::cout << "💥 DEBUG: Sprite HURT actualizado - Frame: " << m_currentFrame << std::endl;
    }
}

// ===================================
// NUEVO: Iniciar animación de salto
// ===================================
void CPlayer::startJump() {
    m_currentState = PlayerState::JUMPING;
    m_currentFrame = 0;
    m_animationTimer = 0.0f;
    m_animationSpeed = JUMP_ANIMATION_SPEED;
}

// ===================================
// NUEVO: Iniciar animación de caída
// ===================================
void CPlayer::startFall() {
    m_currentState = PlayerState::FALLING;
    m_currentFrame = 0;
    m_animationTimer = 0.0f;
    m_animationSpeed = FALL_ANIMATION_SPEED;
}

bool CPlayer::isHurt() const {
    return m_isHurt;
}

void CPlayer::takeDamage(int damage) {
    if (damage > 0) {
        m_health -= damage;
        if (m_health < 0) {
            m_health = 0;
        }
        
        std::cout << m_name << " recibe " << damage << " de daño. Salud: " 
                  << m_health << "/" << m_maxHealth << "\n";
        
        // *** ACTIVAR ESTADO HURT AL RECIBIR DAÑO ***
        if (m_health > 0) {
            std::cout << "🩸 DEBUG: Activando animación HURT..." << std::endl;
            startHurt(); // ← Activar animación de hurt
        } else {
            m_sprite.setFillColor(sf::Color::Black);
            std::cout << m_name << " ha muerto!\n";
        }
    }
}

bool CPlayer::isAlive() const {
    return m_health > 0;
}

// ===================================
// NUEVO: Verificar si está en el suelo
// ===================================
void CPlayer::checkGroundState() {
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
void CPlayer::updatePhysicsState() {
    if (!m_physicsEnabled) return;
    
    checkGroundState();
    updateAnimationFromPhysics();
}

// ===================================
// NUEVO: Actualizar animación basada en físicas
// ===================================
void CPlayer::updateAnimationFromPhysics() {
    if (!m_physicsEnabled || !m_physicsBody) return;
    
    // No cambiar animación si está herido o atacando
    if (m_currentState == PlayerState::HURT || m_currentState == PlayerState::ATTACKING) {
        return;
    }
    
    b2Vec2 velocity = m_physicsBody->GetLinearVelocity();
    
    // Determinar estado basado en velocidad y posición
    if (!m_isGrounded) {
        if (velocity.y < -0.5f) {
            // Subiendo (saltando)
            if (m_currentState != PlayerState::JUMPING) {
                startJump();
            }
        } else if (velocity.y > 0.5f) {
            // Cayendo
            if (m_currentState != PlayerState::FALLING) {
                startFall();
            }
        }
    } else {
        // En el suelo
        if (std::abs(velocity.x) > 0.5f) {
            // Moviéndose horizontalmente
            setRunning(true);
        } else {
            // Parado
            setRunning(false);
        }
    }
}

// ===================================
// NUEVO: Limitar velocidad horizontal
// ===================================
void CPlayer::limitHorizontalVelocity() {
    if (!m_physicsEnabled || !m_physicsBody) return;
    
    b2Vec2 velocity = m_physicsBody->GetLinearVelocity();
    
    if (std::abs(velocity.x) > MAX_VELOCITY_X) {
        velocity.x = (velocity.x > 0) ? MAX_VELOCITY_X : -MAX_VELOCITY_X;
        m_physicsBody->SetLinearVelocity(velocity);
    }
}

// MÉTODOS SFML
void CPlayer::update(float deltaTime) {
    // *** ACTUALIZAR TIMER DE HURT ***
    if (m_isHurt) {
        m_hurtTimer -= deltaTime;
        if (m_hurtTimer <= 0.0f) {
            // Terminar estado hurt, volver a idle
            std::cout << "💚 DEBUG: Finalizando estado HURT, volviendo a IDLE" << std::endl;
            m_isHurt = false;
            m_currentState = PlayerState::IDLE;
            m_currentFrame = 0;
            m_animationTimer = 0.0f;
            m_animationSpeed = IDLE_ANIMATION_SPEED;
        }
    }
    
    // *** NUEVO: Actualizar estado físico ***
    if (m_physicsEnabled) {
        updatePhysicsState();
        limitHorizontalVelocity();
    }
    
    // *** ACTUALIZAR ANIMACIÓN ***
    updateAnimation(deltaTime);
    
    // Restaurar color original después de recibir daño (para el fallback)
    if (isAlive() && m_sprite.getFillColor() == sf::Color::Red) {
        m_sprite.setFillColor(m_color);
    }
}

void CPlayer::render(sf::RenderWindow& window) {
    if (isAlive()) {
        if (m_texturesLoaded) {
            // Renderizar con textura y animación
            window.draw(m_playerSprite);
        } else {
            // Fallback: renderizar rectángulo de color
            window.draw(m_sprite);
        }
    }
}

// DEBUG
void CPlayer::printStatus() const {
    std::cout << "=== Estado del Jugador ===\n";
    std::cout << "Nombre: " << m_name << "\n";
    std::cout << "Salud: " << m_health << "/" << m_maxHealth << "\n";
    std::cout << "Posición: (" << m_position.x << ", " << m_position.y << ")\n";
    std::cout << "Velocidad: " << m_speed << "\n";
    std::cout << "Estado: " << (isAlive() ? "Vivo" : "Muerto") << "\n";
    std::cout << "Texturas: " << (m_texturesLoaded ? "Cargadas" : "No cargadas") << "\n";
    std::cout << "Animación: ";
    switch(m_currentState) {
        case PlayerState::IDLE: std::cout << "Idle"; break;
        case PlayerState::RUNNING: std::cout << "Corriendo"; break;
        case PlayerState::ATTACKING: std::cout << "Atacando"; break;
        case PlayerState::HURT: std::cout << "Herido"; break;
        case PlayerState::JUMPING: std::cout << "Saltando"; break;
        case PlayerState::FALLING: std::cout << "Cayendo"; break;
    }
    std::cout << " (Frame: " << m_currentFrame << ")\n";
    if (m_isHurt) {
        std::cout << "Hurt Timer: " << m_hurtTimer << "s restantes\n";
    }
    std::cout << "========================\n";
}

void CPlayer::printSpriteConfig() const {
    std::cout << "========== CONFIGURACIÓN DE SPRITES ==========\n";
    std::cout << "🎭 IDLE:   (" << IDLE_START_X << "," << IDLE_START_Y << ") " 
              << IDLE_FRAME_WIDTH << "x" << IDLE_FRAME_HEIGHT << " [" << IDLE_FRAME_COUNT << " frame - ESTÁTICO]\n";
    std::cout << "🏃 RUN:    (" << RUN_START_X << "," << RUN_START_Y << ") " 
              << RUN_FRAME_WIDTH << "x" << RUN_FRAME_HEIGHT << " [" << RUN_FRAME_COUNT << " frames - ANIMADO]\n";
    std::cout << "⚔️ ATTACK: (" << ATTACK_START_X << "," << ATTACK_START_Y << ") " 
              << ATTACK_FRAME_WIDTH << "x" << ATTACK_FRAME_HEIGHT << " [" << ATTACK_FRAME_COUNT << " frames - ANIMADO]\n";
    std::cout << "💥 HURT:   (" << HURT_START_X << "," << HURT_START_Y << ") " 
              << HURT_FRAME_WIDTH << "x" << HURT_FRAME_HEIGHT << " [" << HURT_FRAME_COUNT << " frame - ESTÁTICO]\n";
    std::cout << "🦘 JUMP:   (" << JUMP_START_X << "," << JUMP_START_Y << ") " 
              << JUMP_FRAME_WIDTH << "x" << JUMP_FRAME_HEIGHT << " [" << JUMP_FRAME_COUNT << " frame - ESTÁTICO]\n";
    std::cout << "🍃 FALL:   (" << FALL_START_X << "," << FALL_START_Y << ") " 
              << FALL_FRAME_WIDTH << "x" << FALL_FRAME_HEIGHT << " [" << FALL_FRAME_COUNT << " frame - ESTÁTICO]\n";
    std::cout << "==============================================\n";
}

void CPlayer::debugCurrentFrame() const {
    std::cout << "🔍 DEBUG FRAME ACTUAL:" << std::endl;
    std::cout << "   Estado: ";
    switch(m_currentState) {
        case PlayerState::IDLE: std::cout << "IDLE"; break;
        case PlayerState::RUNNING: std::cout << "RUNNING"; break;
        case PlayerState::ATTACKING: std::cout << "ATTACKING"; break;
        case PlayerState::HURT: std::cout << "HURT"; break;
        case PlayerState::JUMPING: std::cout << "JUMPING"; break;
        case PlayerState::FALLING: std::cout << "FALLING"; break;
    }
    std::cout << std::endl;
    std::cout << "   Frame actual: " << m_currentFrame << std::endl;
    
    if (m_texturesLoaded) {
        sf::IntRect rect = getCurrentFrameRect();
        std::cout << "   Rectángulo de textura: (" << rect.left << "," << rect.top 
                  << ") " << rect.width << "x" << rect.height << std::endl;
    }
    std::cout << "   Is Hurt: " << (m_isHurt ? "SÍ" : "NO") << std::endl;
    if (m_isHurt) {
        std::cout << "   Hurt Timer: " << m_hurtTimer << "s restantes" << std::endl;
    }
}

// ===================================
// NUEVO: Debug de físicas
// ===================================
void CPlayer::printPhysicsStatus() const {
    std::cout << "=== FÍSICAS DEL JUGADOR ===" << std::endl;
    std::cout << "Físicas habilitadas: " << (m_physicsEnabled ? "SÍ" : "NO") << std::endl;
    std::cout << "En el suelo: " << (m_isGrounded ? "SÍ" : "NO") << std::endl;
    std::cout << "Fuerza de salto: " << m_jumpForce << std::endl;
    
    if (m_physicsEnabled && m_physicsBody) {
        b2Vec2 pos = m_physicsBody->GetPosition();
        b2Vec2 vel = m_physicsBody->GetLinearVelocity();
        
        std::cout << "Posición física: (" << pos.x << ", " << pos.y << ") metros" << std::endl;
        std::cout << "Velocidad: (" << vel.x << ", " << vel.y << ") m/s" << std::endl;
        
        sf::Vector2f pixelPos = CPhysics::metersToPixels(pos);
        std::cout << "Posición en píxeles: (" << pixelPos.x << ", " << pixelPos.y << ")" << std::endl;
    }
    
    std::cout << "==========================" << std::endl;
}

// MÉTODOS PRIVADOS
void CPlayer::loadPlayerTextures() {
    std::cout << "========================================" << std::endl;
    std::cout << "🔧 ENTRANDO A loadPlayerTextures()" << std::endl;
    std::cout << "🔧 Cargando sprite sheet con coordenadas manuales..." << std::endl;
    std::cout << "========================================" << std::endl;
    
    std::cout << "Cargando sprite sheet del personaje..." << std::endl;
    
    // ===================================================
    // CARGAR CHARACTER.PNG - Sprite sheet completo
    // ===================================================
    std::cout << "Intentando cargar: assets/Character.png" << std::endl;
    if (!m_characterTexture.loadFromFile("assets/Character.png")) {
        std::cerr << "❌ Error: No se pudo cargar assets/Character.png" << std::endl;
        m_texturesLoaded = false;
        std::cout << "⚠️  Usando rectángulo azul (textura no disponible)" << std::endl;
        return;
    }
    
    sf::Vector2u sheetSize = m_characterTexture.getSize();
    std::cout << "✅ Character.png cargado (" << sheetSize.x << "x" << sheetSize.y << ")" << std::endl;
    std::cout << "   📐 Configuración manual de sprites:" << std::endl;
    std::cout << "      • IDLE: Inicia en (" << IDLE_START_X << "," << IDLE_START_Y << ") - " 
              << IDLE_FRAME_COUNT << " frame de " << IDLE_FRAME_WIDTH << "x" << IDLE_FRAME_HEIGHT << " (ESTÁTICO)" << std::endl;
    std::cout << "      • RUN: Inicia en (" << RUN_START_X << "," << RUN_START_Y << ") - " 
              << RUN_FRAME_COUNT << " frames de " << RUN_FRAME_WIDTH << "x" << RUN_FRAME_HEIGHT << std::endl;
    std::cout << "      • ATTACK: Inicia en (" << ATTACK_START_X << "," << ATTACK_START_Y << ") - " 
              << ATTACK_FRAME_COUNT << " frames de " << ATTACK_FRAME_WIDTH << "x" << ATTACK_FRAME_HEIGHT << std::endl;
    std::cout << "      • HURT: Inicia en (" << HURT_START_X << "," << HURT_START_Y << ") - " 
              << HURT_FRAME_COUNT << " frame de " << HURT_FRAME_WIDTH << "x" << HURT_FRAME_HEIGHT << " (ESTÁTICO)" << std::endl;
    std::cout << "      • JUMP: Inicia en (" << JUMP_START_X << "," << JUMP_START_Y << ") - " 
              << JUMP_FRAME_COUNT << " frame de " << JUMP_FRAME_WIDTH << "x" << JUMP_FRAME_HEIGHT << " (ESTÁTICO)" << std::endl;
    std::cout << "      • FALL: Inicia en (" << FALL_START_X << "," << FALL_START_Y << ") - " 
              << FALL_FRAME_COUNT << " frame de " << FALL_FRAME_WIDTH << "x" << FALL_FRAME_HEIGHT << " (ESTÁTICO)" << std::endl;
    
    // ===================================================
    // CONFIGURAR SPRITE INICIAL
    // ===================================================
    m_texturesLoaded = true;
    
    // Configurar sprite inicial con textura del sprite sheet
    m_playerSprite.setTexture(m_characterTexture);
    m_playerSprite.setPosition(m_position);
    updateSpriteFrame(); // Configurar el primer frame (idle)
    
    std::cout << "🎮 Sistema de coordenadas manuales inicializado" << std::endl;
    std::cout << "   🎭  IDLE: ESTÁTICO (frame 0)" << std::endl;
    std::cout << "   🏃  RUN: ANIMADO (" << RUN_FRAME_COUNT << " frames)" << std::endl;
    std::cout << "   ⚔️  ATTACK: ANIMADO (" << ATTACK_FRAME_COUNT << " frames)" << std::endl;
    std::cout << "   💥  HURT: ESTÁTICO (frame 0)" << std::endl;
    std::cout << "   🦘  JUMP: ESTÁTICO (frame 0)" << std::endl;
    std::cout << "   🍃  FALL: ESTÁTICO (frame 0)" << std::endl;
    std::cout << "   ⚙️  Para ajustar posiciones, edita las constantes START_X/START_Y en CPlayer.hpp" << std::endl;
}

void CPlayer::updateAnimation(float deltaTime) {
    if (!m_texturesLoaded) return;
    
    m_animationTimer += deltaTime;
    
    // ===================================================
    // ACTUALIZAR FRAME DE ANIMACIÓN
    // ===================================================
    if (m_animationTimer >= m_animationSpeed) {
        m_animationTimer = 0.0f;
        
        // Avanzar frame según el estado actual
        switch (m_currentState) {
            case PlayerState::IDLE:
                // IDLE es estático - siempre frame 0
                m_currentFrame = 0;
                break;
                
            case PlayerState::RUNNING:
                m_currentFrame = (m_currentFrame + 1) % RUN_FRAME_COUNT;
                break;
                
            case PlayerState::ATTACKING:
                m_currentFrame++;
                if (m_currentFrame >= ATTACK_FRAME_COUNT) {
                    // Fin de animación de ataque, volver a idle
                    m_currentState = PlayerState::IDLE;
                    m_currentFrame = 0;
                    m_animationSpeed = IDLE_ANIMATION_SPEED;
                }
                break;
                
            case PlayerState::HURT:
                // HURT es estático - siempre frame 0
                m_currentFrame = 0;
                break;
                
            // *** NUEVO: Estados de salto y caída ***
            case PlayerState::JUMPING:
                // JUMP es estático - siempre frame 0
                m_currentFrame = 0;
                break;
                
            case PlayerState::FALLING:
                // FALL es estático - siempre frame 0
                m_currentFrame = 0;
                break;
        }
        
        updateSpriteFrame();
    }
}

void CPlayer::updateSpriteFrame() {
    if (!m_texturesLoaded) return;
    
    // ===================================================
    // CONFIGURAR RECTÁNGULO DE RECORTE DEL SPRITE SHEET
    // ===================================================
    sf::IntRect frameRect = getCurrentFrameRect();
    m_playerSprite.setTextureRect(frameRect);
}

sf::IntRect CPlayer::getCurrentFrameRect() const {
    // ===================================================
    // CALCULAR POSICIÓN EN EL SPRITE SHEET CON COORDENADAS MANUALES
    // Cada animación puede estar en cualquier parte de la imagen
    // ===================================================
    
    int startX = 0, startY = 0;
    int frameWidth = 0, frameHeight = 0;
    
    // Determinar coordenadas de inicio y dimensiones según el estado actual
    switch (m_currentState) {
        case PlayerState::IDLE:
            startX = IDLE_START_X;
            startY = IDLE_START_Y;
            frameWidth = IDLE_FRAME_WIDTH;
            frameHeight = IDLE_FRAME_HEIGHT;
            break;
            
        case PlayerState::RUNNING:
            startX = RUN_START_X;
            startY = RUN_START_Y;
            frameWidth = RUN_FRAME_WIDTH;
            frameHeight = RUN_FRAME_HEIGHT;
            break;
            
        case PlayerState::ATTACKING:
            startX = ATTACK_START_X;
            startY = ATTACK_START_Y;
            frameWidth = ATTACK_FRAME_WIDTH;
            frameHeight = ATTACK_FRAME_HEIGHT;
            break;
            
        case PlayerState::HURT:
            startX = HURT_START_X;
            startY = HURT_START_Y;
            frameWidth = HURT_FRAME_WIDTH;
            frameHeight = HURT_FRAME_HEIGHT;
            break;
            
        // *** NUEVO: Estados de salto y caída ***
        case PlayerState::JUMPING:
            startX = JUMP_START_X;
            startY = JUMP_START_Y;
            frameWidth = JUMP_FRAME_WIDTH;
            frameHeight = JUMP_FRAME_HEIGHT;
            break;
            
        case PlayerState::FALLING:
            startX = FALL_START_X;
            startY = FALL_START_Y;
            frameWidth = FALL_FRAME_WIDTH;
            frameHeight = FALL_FRAME_HEIGHT;
            break;
        
        default:
            // Por defecto, usar idle
            startX = IDLE_START_X;
            startY = IDLE_START_Y;
            frameWidth = IDLE_FRAME_WIDTH;
            frameHeight = IDLE_FRAME_HEIGHT;
            break;
    }
    
    // ===================================================
    // CALCULAR COORDENADAS DEL FRAME ACTUAL
    // NUEVA FÓRMULA: x = startX + (frame * frameWidth), y = startY
    // ===================================================
    int x = startX + (m_currentFrame * frameWidth);   // Posición horizontal desde el inicio
    int y = startY;                                   // Posición vertical fija (inicio Y)
    
    return sf::IntRect(x, y, frameWidth, frameHeight);
}