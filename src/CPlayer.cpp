#include "CPlayer.hpp"
#include "CPhysics.hpp"
#include <iostream>

// Constructor
CPlayer::CPlayer(const std::string& name) 
    : name(name), health(100), maxHealth(100), 
      position(0.0f, 0.0f), speed(150.0f), color(sf::Color::Blue),
      physics(nullptr),           // Referencia al sistema de fisicas
      physicsBody(nullptr),       // Cuerpo fisico
      physicsEnabled(false),      // Estado de fisicas
      grounded(false),            // Estado en el suelo
      jumpForce(DEFAULT_JUMP_FORCE), // Fuerza de salto
      texturesLoaded(false), currentState(PlayerState::IDLE),
      currentFrame(0), animationTimer(0.0f), animationSpeed(IDLE_ANIMATION_SPEED),
      hurtTimer(0.0f), hurt(false) {
    
    
    // Configurar el sprite del jugador (fallback)
    sprite.setSize(sf::Vector2f(32.0f, 32.0f));
    sprite.setFillColor(color);
    sprite.setPosition(position);
    
    
    // Intentar cargar texturas
    loadPlayerTextures();
    
    std::cout << "Jugador " << name << " creado exitosamente." << std::endl;
}

// Destructor
CPlayer::~CPlayer() {
    // El sistema de fisicas se encarga de limpiar los cuerpos automaticamente
    std::cout << "Jugador " << name << " destruido.\n";
}

// GETTERS
const std::string& CPlayer::getName() const {
    return name;
}

int CPlayer::getHealth() const {
    return health;
}

int CPlayer::getMaxHealth() const {
    return maxHealth;
}

sf::Vector2f CPlayer::getPosition() const {
    return position;
}

float CPlayer::getSpeed() const {
    return speed;
}

sf::FloatRect CPlayer::getBounds() const {
    return sprite.getGlobalBounds();
}

// Getters para fisicas
bool CPlayer::isGrounded() const {
    return grounded;
}

bool CPlayer::isJumping() const {
    return currentState == PlayerState::JUMPING;
}

bool CPlayer::isFalling() const {
    return currentState == PlayerState::FALLING;
}

b2Body* CPlayer::getPhysicsBody() const {
    return physicsBody;
}

sf::Vector2f CPlayer::getVelocity() const {
    if (!physicsEnabled || !physicsBody) {
        return sf::Vector2f(0.0f, 0.0f);
    }
    
    b2Vec2 velocity = physicsBody->GetLinearVelocity();
    return CPhysics::b2VecToSFML(velocity);
}

// SETTERS
void CPlayer::setPosition(float x, float y) {
    position.x = x;
    position.y = y;
    sprite.setPosition(position);
    
    // Tambien actualizar sprite con textura si esta cargada
    if (texturesLoaded) {
        playerSprite.setPosition(position);
    }
}

void CPlayer::setPosition(const sf::Vector2f& position) {
    this->position = position;
    sprite.setPosition(this->position);
    
    // Tambien actualizar sprite con textura si esta cargada
    if (texturesLoaded) {
        playerSprite.setPosition(this->position);
    }
}

void CPlayer::setHealth(int health) {
    this->health = health;
    // Asegurar que la salud no exceda el maximo ni sea menor a 0
    if (this->health > maxHealth) {
        this->health = maxHealth;
    } else if (this->health < 0) {
        this->health = 0;
    }
}

void CPlayer::setSpeed(float speed) {
    if (speed >= 0) {
        this->speed = speed;
    }
}

// Setter para fuerza de salto
void CPlayer::setJumpForce(float force) {
    if (force > 0) {
        jumpForce = force;
    }
}

// Inicializar fisicas del jugador
void CPlayer::initializePhysics(CPhysics* physics) {
    this->physics = physics;
    
    // Crear cuerpo fisico del jugador
    physicsBody = this->physics->createPlayerBody(position.x, position.y, this);
    
    if (physicsBody) {
        physicsEnabled = true;
        
        // Configurar propiedades iniciales
        updatePhysicsPosition();
    } else {
        std::cerr << "Error: No se pudo crear el cuerpo fisico del jugador" << std::endl;
        physicsEnabled = false;
    }
}

// Sincronizar posicion desde fisicas
void CPlayer::syncPositionFromPhysics() {
    if (!physicsEnabled || !physicsBody) return;
    
    // Obtener posicion del cuerpo fisico
    b2Vec2 physicsPos = physicsBody->GetPosition();
    sf::Vector2f newPos = CPhysics::metersToPixels(physicsPos);
    
    // Actualizar posicion visual
    position = newPos;
    sprite.setPosition(position);
    
    if (texturesLoaded) {
        playerSprite.setPosition(position);
    }
    
    // Actualizar estados basados en fisicas
    updatePhysicsState();
}

// Actualizar posicion en fisicas
void CPlayer::updatePhysicsPosition() {
    if (!physicsEnabled || !physicsBody) return;
    
    // Convertir posicion visual a fisicas
    b2Vec2 physicsPos = CPhysics::sfmlVecToB2(position);
    physicsBody->SetTransform(physicsPos, physicsBody->GetAngle());
}

// METODOS PARA CONFIGURAR SPRITES MANUALMENTE

void CPlayer::setIdleSprite(int startX, int startY, int frameWidth, int frameHeight, int frameCount) {
    std::cout << "Configurando IDLE sprite: (" << startX << "," << startY << ") " 
              << frameWidth << "x" << frameHeight << " [" << frameCount << " frames]" << std::endl;
}

void CPlayer::setRunSprite(int startX, int startY, int frameWidth, int frameHeight, int frameCount) {
    std::cout << "Configurando RUN sprite: (" << startX << "," << startY << ") " 
              << frameWidth << "x" << frameHeight << " [" << frameCount << " frames]" << std::endl;
}

void CPlayer::setAttackSprite(int startX, int startY, int frameWidth, int frameHeight, int frameCount) {
    std::cout << "Configurando ATTACK sprite: (" << startX << "," << startY << ") " 
              << frameWidth << "x" << frameHeight << " [" << frameCount << " frames]" << std::endl;
}

void CPlayer::setHurtSprite(int startX, int startY, int frameWidth, int frameHeight, int frameCount) {
    std::cout << "Configurando HURT sprite: (" << startX << "," << startY << ") " 
              << frameWidth << "x" << frameHeight << " [" << frameCount << " frames]" << std::endl;
}

// METODOS DE GAMEPLAY
void CPlayer::move(float deltaX, float deltaY) {
    // No permitir movimiento si esta en estado hurt
    if (currentState == PlayerState::HURT) {
        return;
    }
    
    position.x += deltaX;
    position.y += deltaY;
    sprite.setPosition(position);
    
    // Tambien actualizar sprite con textura si esta cargada
    if (texturesLoaded) {
        playerSprite.setPosition(position);
    }
    
    // Cambiar a estado de correr si nos estamos moviendo y no estamos atacando o heridos
    if ((deltaX != 0 || deltaY != 0) && currentState != PlayerState::ATTACKING && currentState != PlayerState::HURT) {
        setRunning(true);
    }
}

// Movimiento con fisicas
void CPlayer::moveWithPhysics(float direction) {
    if (!physicsEnabled || !physicsBody) {
        // Fallback al movimiento tradicional
        move(direction * speed * 0.016f, 0); // Aproximar deltaTime = 0.016f
        return;
    }
    
    // No permitir movimiento si esta en estado hurt
    if (currentState == PlayerState::HURT) {
        return;
    }
    
    // Aplicar fuerza horizontal
    float force = direction * MOVEMENT_FORCE;
    
    // Obtener velocidad actual
    b2Vec2 velocity = physicsBody->GetLinearVelocity();
    
    // Limitar velocidad horizontal maxima
    if (std::abs(velocity.x) < MAX_VELOCITY_X) {
        physics->applyForce(this, force, 0.0f);
    }
    
    // Actualizar estado de animacion
    updateAnimationFromPhysics();
}

// Saltar
void CPlayer::jump() {
    
    // No permitir salto si esta en estado hurt
    if (currentState == PlayerState::HURT) {
        return;
    }
    
    
    // Aplicar impulso hacia arriba
    physics->applyImpulse(this, 0.0f, -jumpForce);
    
    // Cambiar estado de animacion
    startJump();
    grounded = false; // Se actualizara en checkGroundState()
    
}

void CPlayer::attack() {
    // No permitir ataque si esta en estado hurt
    if (currentState == PlayerState::HURT) {
        return;
    }
    
    std::cout << name << " realiza un ataque!\n";
    
    // Cambiar a estado de ataque
    startAttack();
}

void CPlayer::setRunning(bool running) {
    // No cambiar estado si esta herido o atacando
    if (currentState == PlayerState::HURT || currentState == PlayerState::ATTACKING) {
        return;
    }
    
    if (running) {
        if (currentState != PlayerState::RUNNING) {
            currentState = PlayerState::RUNNING;
            currentFrame = 0;
            animationTimer = 0.0f;
            animationSpeed = RUN_ANIMATION_SPEED;
        }
    } else if (currentState == PlayerState::RUNNING) {
        // Cambiar de RUNNING a IDLE
        currentState = PlayerState::IDLE;
        currentFrame = 0;
        animationTimer = 0.0f;
        animationSpeed = IDLE_ANIMATION_SPEED;
    }
}

void CPlayer::startAttack() {
    currentState = PlayerState::ATTACKING;
    currentFrame = 0;
    animationTimer = 0.0f;
    animationSpeed = ATTACK_ANIMATION_SPEED;
}

bool CPlayer::isAttacking() const {
    return currentState == PlayerState::ATTACKING;
}

void CPlayer::startHurt() {
    currentState = PlayerState::HURT;
    currentFrame = 0;
    animationTimer = 0.0f;
    animationSpeed = HURT_ANIMATION_SPEED;
    hurtTimer = HURT_DURATION;
    hurt = true;
    
    // Forzar actualizacion del sprite inmediatamente
    if (texturesLoaded) {
        updateSpriteFrame();
    }
}

// Iniciar animacion de salto
void CPlayer::startJump() {
    currentState = PlayerState::JUMPING;
    currentFrame = 0;
    animationTimer = 0.0f;
    animationSpeed = JUMP_ANIMATION_SPEED;
}

// Iniciar animacion de caida
void CPlayer::startFall() {
    currentState = PlayerState::FALLING;
    currentFrame = 0;
    animationTimer = 0.0f;
    animationSpeed = FALL_ANIMATION_SPEED;
}

bool CPlayer::isHurt() const {
    return hurt;
}

void CPlayer::takeDamage(int damage) {
    if (damage > 0) {
        health -= damage;
        if (health < 0) {
            health = 0;
        }
        
        std::cout << name << " recibe " << damage << " de dano. Salud: " 
                  << health << "/" << maxHealth << "\n";
        
        // ACTIVAR ESTADO HURT AL RECIBIR DANO
        if (health > 0) {
            startHurt(); 
        } else {
            sprite.setFillColor(sf::Color::Black);
            std::cout << name << " ha muerto!\n";
        }
    }
}

bool CPlayer::isAlive() const {
    return health > 0;
}

// Verificar si esta en el suelo
void CPlayer::checkGroundState() {
    if (!physicsEnabled || !physicsBody) {
        grounded = true; // Asumir que esta en el suelo sin fisicas
        return;
    }
    
    // Verificar velocidad vertical
    b2Vec2 velocity = physicsBody->GetLinearVelocity();
    
    // Esta en el suelo si la velocidad vertical es muy pequenia
    grounded = std::abs(velocity.y) < 0.5f;
}

// Actualizar estado basado en fisicas
void CPlayer::updatePhysicsState() {
    if (!physicsEnabled) return;
    
    checkGroundState();
    updateAnimationFromPhysics();
}

// Actualizar animacion basada en fisicas
void CPlayer::updateAnimationFromPhysics() {
    if (!physicsEnabled || !physicsBody) return;
    
    // No cambiar animacion si esta herido o atacando
    if (currentState == PlayerState::HURT || currentState == PlayerState::ATTACKING) {
        return;
    }
    
    b2Vec2 velocity = physicsBody->GetLinearVelocity();
    
    // Determinar estado basado en velocidad y posicion
    if (!grounded) {
        if (velocity.y < -0.5f) {
            // Subiendo (saltando)
            if (currentState != PlayerState::JUMPING) {
                startJump();
            }
        } else if (velocity.y > 0.5f) {
            // Cayendo
            if (currentState != PlayerState::FALLING) {
                startFall();
            }
        }
    } else {
        // En el suelo
        if (std::abs(velocity.x) > 0.5f) {
            // Moviendose horizontalmente
            setRunning(true);
        } else {
            // Parado
            setRunning(false);
        }
    }
}

// Limitar velocidad horizontal
void CPlayer::limitHorizontalVelocity() {
    if (!physicsEnabled || !physicsBody) return;
    
    b2Vec2 velocity = physicsBody->GetLinearVelocity();
    
    if (std::abs(velocity.x) > MAX_VELOCITY_X) {
        velocity.x = (velocity.x > 0) ? MAX_VELOCITY_X : -MAX_VELOCITY_X;
        physicsBody->SetLinearVelocity(velocity);
    }
}

// METODOS SFML
void CPlayer::update(float deltaTime) {
    // ACTUALIZAR TIMER DE HURT
    if (hurt) {
        hurtTimer -= deltaTime;
        if (hurtTimer <= 0.0f) {
            // Terminar estado hurt, volver a idle
            hurt = false;
            currentState = PlayerState::IDLE;
            currentFrame = 0;
            animationTimer = 0.0f;
            animationSpeed = IDLE_ANIMATION_SPEED;
        }
    }
    
    // Actualizar estado fisico
    if (physicsEnabled) {
        updatePhysicsState();
        limitHorizontalVelocity();
    }
    
    // Actualizar animacion
    updateAnimation(deltaTime);
    
    // Restaurar color original despues de recibir dano (para el fallback)
    if (isAlive() && sprite.getFillColor() == sf::Color::Red) {
        sprite.setFillColor(color);
    }
}

void CPlayer::render(sf::RenderWindow& window) {
    if (isAlive()) {
        if (texturesLoaded) {
            // Renderizar con textura y animacion
            window.draw(playerSprite);
        } else {
            // Fallback: renderizar rectangulo de color
            window.draw(sprite);
        }
    }
}

// DEBUG
void CPlayer::printStatus() const {
    std::cout << "=== Estado del Jugador ===\n";
    std::cout << "Nombre: " << name << "\n";
    std::cout << "Salud: " << health << "/" << maxHealth << "\n";
    std::cout << "Posicion: (" << position.x << ", " << position.y << ")\n";
    std::cout << "Velocidad: " << speed << "\n";
    std::cout << "Estado: " << (isAlive() ? "Vivo" : "Muerto") << "\n";
    std::cout << "Texturas: " << (texturesLoaded ? "Cargadas" : "No cargadas") << "\n";
    std::cout << "Animacion: ";
    switch(currentState) {
        case PlayerState::IDLE: std::cout << "Idle"; break;
        case PlayerState::RUNNING: std::cout << "Corriendo"; break;
        case PlayerState::ATTACKING: std::cout << "Atacando"; break;
        case PlayerState::HURT: std::cout << "Herido"; break;
        case PlayerState::JUMPING: std::cout << "Saltando"; break;
        case PlayerState::FALLING: std::cout << "Cayendo"; break;
    }
    std::cout << " (Frame: " << currentFrame << ")\n";
    if (hurt) {
        std::cout << "Hurt Timer: " << hurtTimer << "s restantes\n";
    }
    std::cout << "========================\n";
}

void CPlayer::printSpriteConfig() const {
    std::cout << "========== CONFIGURACION DE SPRITES ==========\n";
    std::cout << "IDLE:   (" << IDLE_START_X << "," << IDLE_START_Y << ") " 
              << IDLE_FRAME_WIDTH << "x" << IDLE_FRAME_HEIGHT << " [" << IDLE_FRAME_COUNT << " frame - ESTATICO]\n";
    std::cout << "RUN:    (" << RUN_START_X << "," << RUN_START_Y << ") " 
              << RUN_FRAME_WIDTH << "x" << RUN_FRAME_HEIGHT << " [" << RUN_FRAME_COUNT << " frames - ANIMADO]\n";
    std::cout << "ATTACK: (" << ATTACK_START_X << "," << ATTACK_START_Y << ") " 
              << ATTACK_FRAME_WIDTH << "x" << ATTACK_FRAME_HEIGHT << " [" << ATTACK_FRAME_COUNT << " frames - ANIMADO]\n";
    std::cout << "HURT:   (" << HURT_START_X << "," << HURT_START_Y << ") " 
              << HURT_FRAME_WIDTH << "x" << HURT_FRAME_HEIGHT << " [" << HURT_FRAME_COUNT << " frame - ESTATICO]\n";
    std::cout << "JUMP:   (" << JUMP_START_X << "," << JUMP_START_Y << ") " 
              << JUMP_FRAME_WIDTH << "x" << JUMP_FRAME_HEIGHT << " [" << JUMP_FRAME_COUNT << " frame - ESTATICO]\n";
    std::cout << "FALL:   (" << FALL_START_X << "," << FALL_START_Y << ") " 
              << FALL_FRAME_WIDTH << "x" << FALL_FRAME_HEIGHT << " [" << FALL_FRAME_COUNT << " frame - ESTATICO]\n";
    std::cout << "==============================================\n";
}

void CPlayer::debugCurrentFrame() const {
    std::cout << "DEBUG FRAME ACTUAL:" << std::endl;
    std::cout << "   Estado: ";
    switch(currentState) {
        case PlayerState::IDLE: std::cout << "IDLE"; break;
        case PlayerState::RUNNING: std::cout << "RUNNING"; break;
        case PlayerState::ATTACKING: std::cout << "ATTACKING"; break;
        case PlayerState::HURT: std::cout << "HURT"; break;
        case PlayerState::JUMPING: std::cout << "JUMPING"; break;
        case PlayerState::FALLING: std::cout << "FALLING"; break;
    }
    std::cout << std::endl;
    std::cout << "   Frame actual: " << currentFrame << std::endl;
    
    if (texturesLoaded) {
        sf::IntRect rect = getCurrentFrameRect();
        std::cout << "   Rectangulo de textura: (" << rect.left << "," << rect.top 
                  << ") " << rect.width << "x" << rect.height << std::endl;
    }
    std::cout << "   Is Hurt: " << (hurt ? "Si" : "NO") << std::endl;
    if (hurt) {
        std::cout << "   Hurt Timer: " << hurtTimer << "s restantes" << std::endl;
    }
}

// Debug de fisicas
void CPlayer::printPhysicsStatus() const {
    std::cout << "=== FISICAS DEL JUGADOR ===" << std::endl;
    std::cout << "Fisicas habilitadas: " << (physicsEnabled ? "Si" : "NO") << std::endl;
    std::cout << "En el suelo: " << (grounded ? "Si" : "NO") << std::endl;
    std::cout << "Fuerza de salto: " << jumpForce << std::endl;
    
    if (physicsEnabled && physicsBody) {
        b2Vec2 pos = physicsBody->GetPosition();
        b2Vec2 vel = physicsBody->GetLinearVelocity();
        
        std::cout << "Posicion fisica: (" << pos.x << ", " << pos.y << ") metros" << std::endl;
        std::cout << "Velocidad: (" << vel.x << ", " << vel.y << ") m/s" << std::endl;
        
        sf::Vector2f pixelPos = CPhysics::metersToPixels(pos);
        std::cout << "Posicion en pixeles: (" << pixelPos.x << ", " << pixelPos.y << ")" << std::endl;
    }
    
    std::cout << "==========================" << std::endl;
}

// METODOS PRIVADOS
void CPlayer::loadPlayerTextures() {
    
    
    // CARGAR CHARACTER.PNG - Sprite sheet completo
    std::cout << "Intentando cargar: assets/Character.png" << std::endl;
    if (!characterTexture.loadFromFile("assets/Character.png")) {
        std::cerr << "Error: No se pudo cargar assets/Character.png" << std::endl;
        texturesLoaded = false;
        return;
    }
    
    sf::Vector2u sheetSize = characterTexture.getSize();
    
    // CONFIGURAR SPRITE INICIAL
    texturesLoaded = true;
    
    // Configurar sprite inicial con textura del sprite sheet
    playerSprite.setTexture(characterTexture);
    playerSprite.setPosition(position);
    updateSpriteFrame(); // Configurar el primer frame (idle)
}

void CPlayer::updateAnimation(float deltaTime) {
    if (!texturesLoaded) return;
    
    animationTimer += deltaTime;
    
    // ACTUALIZAR FRAME DE ANIMACION
    if (animationTimer >= animationSpeed) {
        animationTimer = 0.0f;
        
        // Avanzar frame segun el estado actual
        switch (currentState) {
            case PlayerState::IDLE:
                // IDLE es estatico - siempre frame 0
                currentFrame = 0;
                break;
                
            case PlayerState::RUNNING:
                currentFrame = (currentFrame + 1) % RUN_FRAME_COUNT;
                break;
                
            case PlayerState::ATTACKING:
                currentFrame++;
                if (currentFrame >= ATTACK_FRAME_COUNT) {
                    // Fin de animacion de ataque, volver a idle
                    currentState = PlayerState::IDLE;
                    currentFrame = 0;
                    animationSpeed = IDLE_ANIMATION_SPEED;
                }
                break;
                
            case PlayerState::HURT:
                // HURT es estatico - siempre frame 0
                currentFrame = 0;
                break;
                
            // Estados de salto y caida
            case PlayerState::JUMPING:
                // JUMP es estatico - siempre frame 0
                currentFrame = 0;
                break;
                
            case PlayerState::FALLING:
                // FALL es estatico - siempre frame 0
                currentFrame = 0;
                break;
        }
        
        updateSpriteFrame();
    }
}

void CPlayer::updateSpriteFrame() {
    if (!texturesLoaded) return;
    
    // CONFIGURAR RECTANGULO DE RECORTE DEL SPRITE SHEET
    sf::IntRect frameRect = getCurrentFrameRect();
    playerSprite.setTextureRect(frameRect);
}

sf::IntRect CPlayer::getCurrentFrameRect() const {
    // CALCULAR POSICION EN EL SPRITE SHEET CON COORDENADAS MANUALES
    // Cada animacion puede estar en cualquier parte de la imagen
    
    int startX = 0, startY = 0;
    int frameWidth = 0, frameHeight = 0;
    
    // Determinar coordenadas de inicio y dimensiones segun el estado actual
    switch (currentState) {
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
            
        // Estados de salto y caida
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
    
    // CALCULAR COORDENADAS DEL FRAME ACTUAL
    // NUEVA FORMULA: x = startX + (frame * frameWidth), y = startY
    int x = startX + (currentFrame * frameWidth);   // Posicion horizontal desde el inicio
    int y = startY;                                   // Posicion vertical fija (inicio Y)
    
    return sf::IntRect(x, y, frameWidth, frameHeight);
}