#include "CPlayer.hpp"
#include <iostream>

// Constructor
CPlayer::CPlayer(const std::string& name) 
    : m_name(name), m_health(100), m_maxHealth(100), 
      m_position(0.0f, 0.0f), m_speed(150.0f), m_color(sf::Color::Blue),
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

// ===================================================
// NUEVOS MÉTODOS PARA CONFIGURAR SPRITES MANUALMENTE
// ===================================================

void CPlayer::setIdleSprite(int startX, int startY, int frameWidth, int frameHeight, int frameCount) {
    // Estas variables ahora son const, pero podrías hacerlas miembros no-const si necesitas cambiarlas en runtime
    std::cout << "🎭 Configurando IDLE sprite: (" << startX << "," << startY << ") " 
              << frameWidth << "x" << frameHeight << " [" << frameCount << " frames]" << std::endl;
    // Para cambiar valores en runtime, necesitarías hacer estas variables miembros no-const
    // Por ahora, este método sirve para mostrar cómo sería la interfaz
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
                // Debug: confirmar que estamos en HURT
                // std::cout << "🩸 DEBUG: En estado HURT, frame: " << m_currentFrame << std::endl;
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