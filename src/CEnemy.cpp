#include "CEnemy.hpp"
#include "CPhysics.hpp"
#include <iostream>
#include <cmath>

// Constructor
CEnemy::CEnemy(EnemyType type, float x, float y) 
    : enemyType(type), position(x, y), currentCooldown(0.0f),
      physics(nullptr),           // Referencia al sistema de fisicas
      physicsBody(nullptr),       // Cuerpo fisico
      physicsEnabled(false),      // Estado de fisicas
      grounded(false),            // Estado en el suelo
      flyCapable(false),          // Capacidad de volar
      jumpForce(0.0f),            // Fuerza de salto
      flyForce(0.0f),             // Fuerza de vuelo
      movementForce(0.0f),        // Fuerza de movimiento
      lastDirectionChange(0.0f),  // Timer para cambio de direccion
      movementDirection(1),       // Direccion inicial (derecha)
      texturesLoaded(false),      // Estado de texturas
      currentState(EnemyState::IDLE), // Estado inicial
      currentFrame(0),            // Frame inicial
      animationTimer(0.0f),       // Timer de animacion
      animationSpeed(0.3f),       // Velocidad por defecto
      moving(false) {             // Estado de movimiento
    
    // Configurar el tipo de enemigo
    setupEnemyType(type);
    
    // Configurar el sprite (fallback)
    sprite.setSize(sf::Vector2f(28.0f, 28.0f));
    sprite.setFillColor(color);
    sprite.setPosition(position);
    originalColor = color;
    
    // Cargar texturas del enemigo
    loadEnemyTextures();
}

// Destructor
CEnemy::~CEnemy() {
    // El sistema de fisicas se encarga de limpiar los cuerpos automaticamente
}

// GETTERS
const std::string& CEnemy::getType() const {
    return type;
}

EnemyType CEnemy::getEnemyType() const {
    return enemyType;
}

int CEnemy::getHealth() const {
    return health;
}

int CEnemy::getMaxHealth() const {
    return maxHealth;
}

int CEnemy::getDamage() const {
    return damage;
}

sf::Vector2f CEnemy::getPosition() const {
    return position;
}

float CEnemy::getSpeed() const {
    return speed;
}

sf::FloatRect CEnemy::getBounds() const {
    if (texturesLoaded) {
        return enemySprite.getGlobalBounds();
    }
    return sprite.getGlobalBounds();
}

float CEnemy::getDetectionRange() const {
    return detectionRange;
}

float CEnemy::getAttackRange() const {
    return attackRange;
}

// Getters para fisicas
bool CEnemy::isGrounded() const {
    return grounded;
}

bool CEnemy::canFly() const {
    return flyCapable;
}

b2Body* CEnemy::getPhysicsBody() const {
    return physicsBody;
}

sf::Vector2f CEnemy::getVelocity() const {
    if (!physicsEnabled || !physicsBody) {
        return sf::Vector2f(0.0f, 0.0f);
    }
    
    b2Vec2 velocity = physicsBody->GetLinearVelocity();
    return CPhysics::b2VecToSFML(velocity);
}

int CEnemy::getMovementDirection() const {
    return movementDirection;
}

// Getters para animacion
EnemyState CEnemy::getCurrentState() const {
    return currentState;
}

bool CEnemy::isMoving() const {
    return moving;
}

bool CEnemy::hasTextures() const {
    return texturesLoaded;
}

// SETTERS
void CEnemy::setPosition(float x, float y) {
    position.x = x;
    position.y = y;
    sprite.setPosition(position);
    
    // Actualizar tambien el sprite con textura
    if (texturesLoaded) {
        enemySprite.setPosition(position);
    }
}

void CEnemy::setPosition(const sf::Vector2f& position) {
    this->position = position;
    sprite.setPosition(this->position);
    
    if (texturesLoaded) {
        enemySprite.setPosition(this->position);
    }
}

void CEnemy::setHealth(int health) {
    this->health = health;
    if (this->health > maxHealth) {
        this->health = maxHealth;
    } else if (this->health < 0) {
        this->health = 0;
    }
}

// Setter para controlar animacion
void CEnemy::setMoving(bool moving) {
    if (this->moving != moving) {
        this->moving = moving;
        updateAnimationState();
    }
}

// Inicializar fisicas del enemigo
void CEnemy::initializePhysics(CPhysics* physics) {
    if (!physics) {
        std::cerr << "Error: Sistema de fisicas nulo para enemigo" << std::endl;
        return;
    }
    
    this->physics = physics;
    
    // Crear cuerpo fisico del enemigo
    physicsBody = this->physics->createEnemyBody(position.x, position.y, this);
    
    if (physicsBody) {
        physicsEnabled = true;
        
        // Configurar propiedades fisicas especificas por tipo
        setupPhysicsForType();
        
        // Configurar posicion inicial
        updatePhysicsPosition();
    } else {
        std::cerr << "Error: No se pudo crear el cuerpo fisico del enemigo" << std::endl;
        physicsEnabled = false;
    }
}

// Configurar fisicas segun el tipo
void CEnemy::setupPhysicsForType() {
    if (!physicsEnabled || !physicsBody) return;
    
    switch (enemyType) {
        case EnemyType::MURCIELAGO:
            flyCapable = true;
            flyForce = MURCIELAGO_FLY_FORCE;
            movementForce = DEFAULT_MOVEMENT_FORCE;
            break;
            
        case EnemyType::ESQUELETO:
            flyCapable = false;
            jumpForce = ESQUELETO_JUMP_FORCE;
            movementForce = DEFAULT_MOVEMENT_FORCE;
            break;
            
        case EnemyType::ZOMBIE:
            flyCapable = false;
            jumpForce = 0.18f; 
            movementForce = ZOMBIE_MOVEMENT_FORCE;
            break;
            
        default:
            flyCapable = false;
            jumpForce = 0.0f;
            movementForce = DEFAULT_MOVEMENT_FORCE;
            break;
    }
}

// Sincronizar posicion desde fisicas
void CEnemy::syncPositionFromPhysics() {
    if (!physicsEnabled || !physicsBody) return;
    
    // Obtener posicion del cuerpo fisico
    b2Vec2 physicsPos = physicsBody->GetPosition();
    sf::Vector2f newPos = CPhysics::metersToPixels(physicsPos);
    
    // Actualizar posicion visual
    position = newPos;
    sprite.setPosition(position);
    
    // Actualizar tambien el sprite con textura
    if (texturesLoaded) {
        enemySprite.setPosition(position);
    }
    
    // Actualizar estados basados en fisicas
    updatePhysicsState();
}

// Actualizar posicion en fisicas
void CEnemy::updatePhysicsPosition() {
    if (!physicsEnabled || !physicsBody) return;
    
    // Convertir posicion visual a fisicas
    b2Vec2 physicsPos = CPhysics::sfmlVecToB2(position);
    physicsBody->SetTransform(physicsPos, physicsBody->GetAngle());
}

// METODOS DE GAMEPLAY
void CEnemy::moveTowards(const sf::Vector2f& targetPosition, float deltaTime) {
    if (!isAlive()) return;
    
    // Si tiene fisicas habilitadas, usar movimiento con fisicas
    if (physicsEnabled) {
        moveWithPhysics(targetPosition, deltaTime);
        return;
    }
    
    // Movimiento tradicional (fallback)
    sf::Vector2f direction = targetPosition - position;
    float distance = calculateDistance(position, targetPosition);
    
    if (distance <= attackRange) {
        setMoving(false);
        return;
    }
    
    if (distance > 0) {
        direction.x /= distance;
        direction.y /= distance;
        
        sf::Vector2f movement = direction * speed * deltaTime;
        position += movement;
        sprite.setPosition(position);
        
        // Actualizar sprite con textura y activar animacion
        if (texturesLoaded) {
            enemySprite.setPosition(position);
        }
        setMoving(true);
    }
}

// Movimiento con fisicas
void CEnemy::moveWithPhysics(const sf::Vector2f& targetPosition, float deltaTime) {
    if (!physicsEnabled || !physicsBody || !isAlive()) return;
    
    float distance = calculateDistance(position, targetPosition);
    
    if (distance <= attackRange) {
        setMoving(false);
        return;
    }
    
    sf::Vector2f direction = targetPosition - position;
    float moveDirection = 0.0f;
    
    // Deteccion de movimiento mas sensible
    if (std::abs(direction.x) > 5.0f) {
        moveDirection = (direction.x > 0) ? 1.0f : -1.0f;
        setMoving(true);
    } else {
        setMoving(false);
    }
    
    // Aplicar movimiento segun el tipo
    if (flyCapable && enemyType == EnemyType::MURCIELAGO) {
        handleMurcieelagoAI(targetPosition, deltaTime);
    } else {
        // Fuerza de movimiento aumentada
        applyMovementForce(moveDirection * 1.5f);
        
        if (enemyType == EnemyType::ESQUELETO && grounded && std::abs(direction.y) > 30.0f) {
            jump();
        }
    }
}

int CEnemy::attack() {
    if (!canAttack()) {
        return 0;
    }
    
    std::cout << type << " ataca causando " << damage << " de dano!\n";
    currentCooldown = attackCooldown;
    return damage;
}

void CEnemy::takeDamage(int damage) {
    if (damage > 0) {
        health -= damage;
        if (health < 0) {
            health = 0;
        }
        
        std::cout << type << " recibe " << damage << " de dano. Salud: " 
                  << health << "/" << maxHealth << "\n";
        
        if (health > 0) {
            sprite.setFillColor(sf::Color::Red);
        } else {
            sprite.setFillColor(sf::Color::Black);
            setMoving(false);
            std::cout << type << " ha muerto!\n";
        }
    }
}

bool CEnemy::isAlive() const {
    return health > 0;
}

bool CEnemy::canAttack() const {
    return isAlive() && currentCooldown <= 0.0f;
}

bool CEnemy::isInRange(const sf::Vector2f& targetPosition, float range) const {
    return calculateDistance(position, targetPosition) <= range;
}

// Saltar (esqueletos principalmente)
void CEnemy::jump() {
    if (!physicsEnabled || !physicsBody || !grounded || jumpForce <= 0.0f) {
        return;
    }
    
    physics->applyImpulse(this, 0.0f, -jumpForce);
    grounded = false;
}

// Volar (murcielagos)
void CEnemy::fly() {
    if (!physicsEnabled || !physicsBody || !flyCapable) return;
    
    physics->applyForce(this, 0.0f, -flyForce);
}

// Patrullar automaticamente
void CEnemy::patrol() {
    if (!physicsEnabled || !physicsBody) return;
    
    // Patrullaje mas dinamico
    applyMovementForce(static_cast<float>(movementDirection) * 0.8f);
    setMoving(true);
}

// Seguir objetivo con fisicas
void CEnemy::followTarget(const sf::Vector2f& target, float deltaTime) {
    moveWithPhysics(target, deltaTime);
}

// IA especifica para murcielagos (vuelan)
void CEnemy::handleMurcieelagoAI(const sf::Vector2f& playerPosition, float deltaTime) {
    if (!physicsEnabled || !physicsBody) return;
    
    sf::Vector2f direction = playerPosition - position;
    float distance = calculateDistance(position, playerPosition);
    
    // IA de murcielago mas efectiva
    if (distance <= detectionRange * 1.5f && distance > attackRange) {
        float forceX = (direction.x > 0) ? movementForce : -movementForce;
        float forceY = (direction.y > 0) ? flyForce : -flyForce;
        
        // Fuerzas mas fuertes
        physics->applyForce(this, forceX * 0.8f, forceY * 0.5f);
        setMoving(true);
        
        // Velocidad maxima mas alta
        b2Vec2 velocity = physicsBody->GetLinearVelocity();
        if (velocity.Length() > 10.0f) {
            velocity.Normalize();
            velocity *= 10.0f;
            physicsBody->SetLinearVelocity(velocity);
        }
    }
}

// IA especifica para esqueletos
void CEnemy::handleEsqueletoAI(const sf::Vector2f& playerPosition, float deltaTime) {
    if (!physicsEnabled || !physicsBody) return;
    
    sf::Vector2f direction = playerPosition - position;
    float distance = calculateDistance(position, playerPosition);
    
    // Rango de deteccion mas amplio
    if (distance <= detectionRange * 1.2f && distance > attackRange) {
        float moveDirection = (direction.x > 0) ? 1.0f : -1.0f;
        
        // Movimiento mas agresivo
        applyMovementForce(moveDirection * 1.3f);
        setMoving(true);
        
        // Salto mas frecuente
        if (grounded && (direction.y < -20.0f || std::abs(direction.x) < 30.0f)) {
            jump();
        }
    } else if (distance > detectionRange) {
        // Patrullaje mas activo
        patrol();
    } else {
        setMoving(false);
    }
}

// IA especifica para zombies
void CEnemy::handleZombieAI(const sf::Vector2f& playerPosition, float deltaTime) {
    if (!physicsEnabled || !physicsBody) return;
    
    sf::Vector2f direction = playerPosition - position;
    float distance = calculateDistance(position, playerPosition);
    
    // IA de zombie mas persistente
    if (distance <= detectionRange * 1.3f && distance > attackRange) {
        float moveDirection = (direction.x > 0) ? 1.0f : -1.0f;
        
        // Zombies menos lentos
        applyMovementForce(moveDirection * 1.0f);
        setMoving(true);
    } else if (distance > detectionRange) {
        patrol();
    } else {
        setMoving(false);
    }
}

// Aplicar fuerza de movimiento
void CEnemy::applyMovementForce(float direction) {
    if (!physicsEnabled || !physicsBody || direction == 0.0f) return;
    
    b2Vec2 velocity = physicsBody->GetLinearVelocity();
    
    // Velocidades maximas mas altas y diferenciadas
    float maxVelocity;
    switch (enemyType) {
        case EnemyType::ZOMBIE:
            maxVelocity = 3.0f;
            break;
        case EnemyType::ESQUELETO:
            maxVelocity = 5.0f;
            break;
        case EnemyType::MURCIELAGO:
            maxVelocity = 6.0f;
            break;
        default:
            maxVelocity = 4.0f;
            break;
    }
    
    // Fuerza base mas alta
    if (std::abs(velocity.x) < maxVelocity) {
        float force = direction * movementForce * 1.2f;
        physics->applyForce(this, force, 0.0f);
    }
    
    // Ayuda adicional si esta muy lento
    if (std::abs(velocity.x) < 0.5f && direction != 0.0f) {
        // Impulso adicional si esta casi parado
        float boostForce = direction * movementForce * 2.0f;
        physics->applyForce(this, boostForce, 0.0f);
    }
}

// Verificar estado en el suelo
void CEnemy::checkGroundState() {
    if (!physicsEnabled || !physicsBody) {
        grounded = true;
        return;
    }
    
    b2Vec2 velocity = physicsBody->GetLinearVelocity();
    grounded = std::abs(velocity.y) < 0.5f;
}

// Actualizar estado basado en fisicas
void CEnemy::updatePhysicsState() {
    if (!physicsEnabled) return;
    
    checkGroundState();
}

// Actualizar direccion de movimiento
void CEnemy::updateMovementDirection(float deltaTime) {
    lastDirectionChange += deltaTime;
    
    if (lastDirectionChange >= DIRECTION_CHANGE_TIME) {
        movementDirection = (rand() % 3) - 1;
        lastDirectionChange = 0.0f;
    }
}

// IA BASICA
void CEnemy::updateAI(const sf::Vector2f& playerPosition, float deltaTime) {
    if (!isAlive()) return;
    
    float distanceToPlayer = calculateDistance(position, playerPosition);
    
    updateMovementDirection(deltaTime);
    
    // Logica de IA mas agresiva
    if (distanceToPlayer <= detectionRange * 1.2f) {
        if (distanceToPlayer <= attackRange && canAttack()) {
            attack();
            setMoving(false);
        } else {
            // Siempre usar fisicas si esta disponible
            if (physicsEnabled) {
                switch (enemyType) {
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
        // Patrullaje cuando no detecta al jugador
        if (physicsEnabled) {
            patrol();
        } else {
            setMoving(false);
        }
    }
}

// METODOS SFML
void CEnemy::update(float deltaTime) {
    if (currentCooldown > 0.0f) {
        currentCooldown -= deltaTime;
    }
    
    if (physicsEnabled) {
        updatePhysicsState();
    }
    
    // Actualizar animacion
    if (texturesLoaded) {
        updateAnimation(deltaTime);
    }
    
    if (isAlive() && sprite.getFillColor() == sf::Color::Red) {
        sprite.setFillColor(originalColor);
    }
}

void CEnemy::render(sf::RenderWindow& window) {
    if (isAlive()) {
        if (texturesLoaded) {
            // Renderizar con textura y animacion
            window.draw(enemySprite);
        } else {
            // Fallback: renderizar rectangulo de color
            window.draw(sprite);
        }
    }
}

// DEBUG
void CEnemy::printStatus() const {
    std::cout << "=== Estado del Enemigo ===\n";
    std::cout << "Tipo: " << type << "\n";
    std::cout << "Salud: " << health << "/" << maxHealth << "\n";
    std::cout << "Dano: " << damage << "\n";
    std::cout << "Posicion: (" << position.x << ", " << position.y << ")\n";
    std::cout << "Velocidad: " << speed << "\n";
    std::cout << "Rango deteccion: " << detectionRange << "\n";
    std::cout << "Rango ataque: " << attackRange << "\n";
    std::cout << "Estado: " << (isAlive() ? "Vivo" : "Muerto") << "\n";
    std::cout << "Texturas: " << (texturesLoaded ? "Cargadas" : "No cargadas") << "\n";
    std::cout << "Animacion: " << (currentState == EnemyState::IDLE ? "IDLE" : "MOVING") << "\n";
    std::cout << "Frame actual: " << currentFrame << "\n";
    std::cout << "========================\n";
}

void CEnemy::printPhysicsStatus() const {
    std::cout << "=== FISICAS DEL ENEMIGO " << type << " ===" << std::endl;
    std::cout << "Fisicas habilitadas: " << (physicsEnabled ? "Si" : "NO") << std::endl;
    std::cout << "En el suelo: " << (grounded ? "Si" : "NO") << std::endl;
    std::cout << "Puede volar: " << (flyCapable ? "Si" : "NO") << std::endl;
    std::cout << "Direccion de movimiento: " << movementDirection << std::endl;
    
    if (physicsEnabled && physicsBody) {
        b2Vec2 pos = physicsBody->GetPosition();
        b2Vec2 vel = physicsBody->GetLinearVelocity();
        
        std::cout << "Posicion fisica: (" << pos.x << ", " << pos.y << ") metros" << std::endl;
        std::cout << "Velocidad: (" << vel.x << ", " << vel.y << ") m/s" << std::endl;
        
        sf::Vector2f pixelPos = CPhysics::metersToPixels(pos);
        std::cout << "Posicion en pixeles: (" << pixelPos.x << ", " << pixelPos.y << ")" << std::endl;
    }
    
    std::cout << "============================" << std::endl;
}

// Debug de sprites
void CEnemy::printSpriteStatus() const {
    std::cout << "=== SPRITES DEL ENEMIGO " << type << " ===" << std::endl;
    std::cout << "Texturas cargadas: " << (texturesLoaded ? "Si" : "NO") << std::endl;
    std::cout << "Estado actual: " << (currentState == EnemyState::IDLE ? "IDLE" : "MOVING") << std::endl;
    std::cout << "Frame actual: " << currentFrame << std::endl;
    std::cout << "En movimiento: " << (moving ? "Si" : "NO") << std::endl;
    std::cout << "Velocidad animacion: " << animationSpeed << std::endl;
    
    if (texturesLoaded) {
        sf::IntRect rect = getCurrentFrameRect();
        std::cout << "Rectangulo actual: (" << rect.left << "," << rect.top 
                  << ") " << rect.width << "x" << rect.height << std::endl;
        std::cout << "Archivo de textura: " << getTextureFileName() << std::endl;
    }
    
    std::cout << "===============================" << std::endl;
}

// METODOS PRIVADOS
void CEnemy::setupEnemyType(EnemyType type) {
    this->type = enemyTypeToString(type);
    
    switch (type) {
        case EnemyType::MURCIELAGO:
            health = 30;
            maxHealth = 30;
            damage = 10;
            speed = 120.0f;
            detectionRange = 200.0f;
            attackRange = 35.0f;
            attackCooldown = 1.0f;
            color = sf::Color::Magenta;
            animationSpeed = MURCIELAGO_ANIMATION_SPEED;
            break;
            
        case EnemyType::ESQUELETO:
            health = 60;
            maxHealth = 60;
            damage = 20;
            speed = 80.0f;
            detectionRange = 160.0f;
            attackRange = 40.0f;
            attackCooldown = 1.5f;
            color = sf::Color::White;
            animationSpeed = SKELETON_ANIMATION_SPEED;
            break;
            
        case EnemyType::ZOMBIE:
            health = 100;
            maxHealth = 100;
            damage = 30;
            speed = 50.0f;
            detectionRange = 140.0f;
            attackRange = 45.0f;
            attackCooldown = 2.0f;
            color = sf::Color::Green;
            animationSpeed = ZOMBIE_ANIMATION_SPEED;
            break;
            
        default:
            health = 50;
            maxHealth = 50;
            damage = 15;
            speed = 70.0f;
            detectionRange = 120.0f;
            attackRange = 40.0f;
            attackCooldown = 1.5f;
            color = sf::Color::Red;
            animationSpeed = 0.3f;
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

// Metodos de sprites y animacion

void CEnemy::loadEnemyTextures() {
    
    std::string textureFile = getTextureFileName();
    std::string fullPath = "assets/" + textureFile;
    
    if (!enemyTexture.loadFromFile(fullPath)) {
        std::cerr << "Error: No se pudo cargar " << fullPath << std::endl;
        std::cerr << "   Usando fallback (rectangulo de color)" << std::endl;
        texturesLoaded = false;
        return;
    }
    
    sf::Vector2u textureSize = enemyTexture.getSize();
    
    // Configurar sprite inicial
    texturesLoaded = true;
    enemySprite.setTexture(enemyTexture);
    enemySprite.setPosition(position);
    
    // Aplicar escalado segun el tipo de enemigo
    switch (enemyType) {
        case EnemyType::ZOMBIE:
            enemySprite.setScale(ZombieSprites::SCALE_X, ZombieSprites::SCALE_Y);
            break;
            
        case EnemyType::ESQUELETO:
            enemySprite.setScale(SkeletonSprites::SCALE_X, SkeletonSprites::SCALE_Y);
            break;
            
        case EnemyType::MURCIELAGO:
            enemySprite.setScale(MurcielagoSprites::SCALE_X, MurcielagoSprites::SCALE_Y);
            break;
    }
    
    // Configurar frame inicial (IDLE)
    updateSpriteFrame();
}

void CEnemy::updateAnimation(float deltaTime) {
    if (!texturesLoaded) return;
    
    animationTimer += deltaTime;
    
    // Determinar el numero de frames segun el estado actual
    int frameCount = 1;
    
    switch (enemyType) {
        case EnemyType::ZOMBIE:
            frameCount = (currentState == EnemyState::IDLE) ? ZombieSprites::IDLE_FRAME_COUNT : ZombieSprites::MOVING_FRAME_COUNT;
            break;
        case EnemyType::ESQUELETO:
            frameCount = (currentState == EnemyState::IDLE) ? SkeletonSprites::IDLE_FRAME_COUNT : SkeletonSprites::MOVING_FRAME_COUNT;
            break;
        case EnemyType::MURCIELAGO:
            frameCount = MurcielagoSprites::MOVING_FRAME_COUNT;
            break;
    }
    
    // Actualizar frame si es necesario
    if (animationTimer >= animationSpeed) {
        animationTimer = 0.0f;
        
        if (frameCount > 1) {
            currentFrame = (currentFrame + 1) % frameCount;
        } else {
            currentFrame = 0;
        }
        
        updateSpriteFrame();
    }
}

void CEnemy::updateSpriteFrame() {
    if (!texturesLoaded) return;
    
    sf::IntRect frameRect = getCurrentFrameRect();
    enemySprite.setTextureRect(frameRect);
}

sf::IntRect CEnemy::getCurrentFrameRect() const {
    int startX = 0, startY = 0;
    int frameWidth = 32, frameHeight = 32;
    
    switch (enemyType) {
        case EnemyType::ZOMBIE:
            if (currentState == EnemyState::IDLE) {
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
            if (currentState == EnemyState::IDLE) {
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
    
    int x = startX + (currentFrame * frameWidth);
    int y = startY;
    
    return sf::IntRect(x, y, frameWidth, frameHeight);
}

void CEnemy::updateAnimationState() {
    if (enemyType == EnemyType::MURCIELAGO) {
        currentState = EnemyState::MOVING;
    } else {
        if (moving) {
            if (currentState != EnemyState::MOVING) {
                currentState = EnemyState::MOVING;
                currentFrame = 0;
                animationTimer = 0.0f;
            }
        } else {
            if (currentState != EnemyState::IDLE) {
                currentState = EnemyState::IDLE;
                currentFrame = 0;
                animationTimer = 0.0f;
            }
        }
    }
}

std::string CEnemy::getTextureFileName() const 
{
    switch (enemyType) 
    {
        case EnemyType::ZOMBIE: return "zombie.png";
        case EnemyType::ESQUELETO: return "skeleton.png";
        case EnemyType::MURCIELAGO: return "murcielago.png";
        default: return "unknown.png";
    }
}