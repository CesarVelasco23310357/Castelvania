#include "CGame.hpp"
#include <iostream>
#include <cmath>
#include "CMusica.hpp"

// Constructor
CGame::CGame() 
    : m_gameState(GameState::MENU), m_isRunning(false), m_fontLoaded(false),
      m_currentLevelIndex(0), m_inputCooldown(0.0f), m_playerSpeed(150.0f),
      m_jumpForce(12.0f), m_attackRange(50.0f), m_attackDamage(25), m_totalScore(0),
      m_levelsCompleted(0), m_totalPlayTime(0.0f),
      m_musica(nullptr) { 
    
    // Inicializar array de teclas
    for (int i = 0; i < sf::Keyboard::KeyCount; i++) {
        m_keyPressed[i] = false;
    }
}

// Destructor
CGame::~CGame() {
    cleanup();
}

// MAIN GAME LOOP
void CGame::run() {
    initialize();
    
    while (m_isRunning && m_window.isOpen()) {
        float deltaTime = m_clock.restart().asSeconds();
        
        // Limitar deltaTime para evitar saltos grandes
        if (deltaTime > 0.1f) {
            deltaTime = 0.1f;
        }
        
        handleEvents();
        handleInput(deltaTime);
        update(deltaTime);
        render();
    }
    
    cleanup();
}

// GAME STATE MANAGEMENT
void CGame::initialize() {
    initializeWindow();
    loadResources();
    setupGameSettings();
    setupUI();
    
    // Sistema de fisicas
    initializePhysics();
   
    initializeMusic();
    
    createLevels();
    
    m_isRunning = true;
    m_gameState = GameState::MENU;
    
    std::cout << "CGame: Sistema inicializado exitosamente." << std::endl;
}

void CGame::initializeMusic() {
    m_musica = std::make_unique<CMusica>();
    
    if (m_musica && m_musica->initialize()) {
        m_musica->playMenuMusic();
    } else {
        std::cerr << "Error: No se pudo inicializar el sistema de musica" << std::endl;
        m_musica.reset(); 
    }
}

void CGame::initializePhysics() {
    m_physics = std::make_unique<CPhysics>();
    
    if (!m_physics) {
        std::cerr << "Error: No se pudo inicializar el sistema de fisicas" << std::endl;
    }
}

void CGame::cleanup() {
    if (m_window.isOpen()) {
        m_window.close();
    }
    
    m_player.reset();
    m_levels.clear();
    
    if (m_musica) {
        m_musica->cleanup();
        m_musica.reset();
    }
    
    m_physics.reset();
}

bool CGame::isRunning() const {
    return m_isRunning;
}

// GETTERS
GameState CGame::getGameState() const {
    return m_gameState;
}

int CGame::getCurrentLevel() const {
    return m_currentLevelIndex + 1; // +1 porque los indices empiezan en 0
}

int CGame::getTotalScore() const {
    return m_totalScore;
}

float CGame::getTotalPlayTime() const {
    return m_totalPlayTime;
}

CPhysics* CGame::getPhysics() const {
    return m_physics.get();
}

// GAME CONTROL
void CGame::startNewGame() {
    // Reset estadisticas
    m_totalScore = 0;
    m_levelsCompleted = 0;
    m_totalPlayTime = 0.0f;
    m_currentLevelIndex = 0;
    
    // Crear jugador
    createPlayer();
    
    // Cargar primer nivel
    if (!m_levels.empty()) {
        loadLevel(0);
        m_gameState = GameState::PLAYING;
    } else {
        std::cerr << "Error: No hay niveles disponibles!" << std::endl;
        m_gameState = GameState::GAME_OVER;
    }
}

void CGame::pauseGame() {
    if (m_gameState == GameState::PLAYING) {
        m_gameState = GameState::PAUSED;
    }
}

void CGame::resumeGame() {
    if (m_gameState == GameState::PAUSED) {
        m_gameState = GameState::PLAYING;
    }
}

void CGame::restartLevel() {
    if (getActiveLevel()) {
        getActiveLevel()->resetLevel();
        if (m_player) {
            m_player->setPosition(100.0f, 400.0f);
            m_player->setHealth(m_player->getMaxHealth());
            if (m_physics && m_player->getPhysicsBody()) {
                m_player->updatePhysicsPosition();
            }
        }
        m_gameState = GameState::PLAYING;
    }
}

void CGame::nextLevel() {
    m_levelsCompleted++;
    m_currentLevelIndex++;
    
    if (m_currentLevelIndex < static_cast<int>(m_levels.size())) {
        loadLevel(m_currentLevelIndex);
        m_gameState = GameState::PLAYING;
    } else {
        m_gameState = GameState::VICTORY;
    }
}

void CGame::endGame() {
    m_gameState = GameState::MENU;
}

// CORE GAME LOOP METHODS
void CGame::handleEvents() {
    sf::Event event;
    while (m_window.pollEvent(event)) {
        switch (event.type) {
            case sf::Event::Closed:
                m_isRunning = false;
                break;
                
            case sf::Event::KeyPressed:
                // Manejar ESC para cerrar/pausar
                if (event.key.code == sf::Keyboard::Escape) {
                    if (m_gameState == GameState::PLAYING) {
                        pauseGame();
                    } else if (m_gameState == GameState::PAUSED) {
                        resumeGame();
                    } else {
                        m_isRunning = false;
                    }
                }
                break;
                
            default:
                break;
        }
    }
}

void CGame::handleInput(float deltaTime) {
    // Actualizar cooldown de input
    if (m_inputCooldown > 0.0f) {
        m_inputCooldown -= deltaTime;
    }
    
    switch (m_gameState) {
        case GameState::MENU:
            processMenuInput();
            break;
        case GameState::PLAYING:
            processGameInput(deltaTime);
            break;
        case GameState::PAUSED:
            processPauseInput();
            break;
        case GameState::LEVEL_COMPLETED:
        case GameState::GAME_OVER:
        case GameState::VICTORY:
            // Presionar cualquier tecla para continuar
            if (isKeyJustPressed(sf::Keyboard::Space) || isKeyJustPressed(sf::Keyboard::Enter)) {
                if (m_gameState == GameState::LEVEL_COMPLETED) {
                    nextLevel();
                } else {
                    endGame();
                }
            }
            break;
    }
}

void CGame::update(float deltaTime) {
    // Actualizar tiempo total de juego
    if (m_gameState == GameState::PLAYING) {
        m_totalPlayTime += deltaTime;
    }
    
    switch (m_gameState) {
        case GameState::PLAYING:
            updateGameplay(deltaTime);
            updatePhysics(deltaTime);
            break;
        default:
            break;
    }
    
    updateMusic(deltaTime);
    updateUI();
}

void CGame::updatePhysics(float deltaTime) {
    if (!m_physics) {
        std::cerr << "Warning: Sistema de fisicas no inicializado" << std::endl;
        return;
    }
    
    m_physics->update(deltaTime);
    
    // Sincronizar posiciones fisicas con visuales
    if (m_player && m_player->getPhysicsBody()) {
        m_player->syncPositionFromPhysics();
    }
}

void CGame::updateMusic(float deltaTime) {
    if (!m_musica) return;
    
    m_musica->update(deltaTime);
    handleMusicStateChanges();
}

void CGame::render() {
    m_window.clear(sf::Color::Black);
    
    switch (m_gameState) {
        case GameState::MENU:
            renderMenu();
            break;
        case GameState::PLAYING:
            renderGame();
            break;
        case GameState::PAUSED:
            renderGame(); 
            renderPauseScreen();
            break;
        case GameState::LEVEL_COMPLETED:
            renderGame(); 
            renderLevelCompleted();
            break;
        case GameState::GAME_OVER:
            renderGameOver();
            break;
        case GameState::VICTORY:
            renderVictory();
            break;
    }
    
    m_window.display();
}

// INPUT PROCESSING
void CGame::processMenuInput() {
    if (isKeyJustPressed(sf::Keyboard::Enter) || isKeyJustPressed(sf::Keyboard::Space)) {
        startNewGame();
    }
}

void CGame::processGameInput(float deltaTime) {
    if (!m_player) return;
    
    handlePlayerMovement(deltaTime);
    handleMusicInput();
    
    if (isKeyJustPressed(sf::Keyboard::I)) {
        debugPositions();
    }   
    
    if (isKeyJustPressed(sf::Keyboard::W) || isKeyJustPressed(sf::Keyboard::Space)) {
        handlePlayerJump();
    }
    
    if (isKeyJustPressed(sf::Keyboard::Enter)) {
        handlePlayerAttack();
    }
    
    if (isKeyJustPressed(sf::Keyboard::R)) {
        restartLevel();
    }
    
    // DEBUG CONTROLS
    if (isKeyJustPressed(sf::Keyboard::P)) {
        debugFullPhysicsState();
    }
    
    // Controles de debug de plataformas
    if (isKeyJustPressed(sf::Keyboard::F1)) {
        debugShowPlatformPositions();
    }
    
    if (isKeyJustPressed(sf::Keyboard::F2)) {
        adjustPlatformOffset(0.0f, -5.0f);
    }
    
    if (isKeyJustPressed(sf::Keyboard::F3)) {
        adjustPlatformOffset(0.0f, 5.0f);
    }
    
    if (isKeyJustPressed(sf::Keyboard::F4)) {
        adjustPlatformOffset(-5.0f, 0.0f);
    }
    
    if (isKeyJustPressed(sf::Keyboard::F5)) {
        adjustPlatformOffset(5.0f, 0.0f);
    }
    
    if (isKeyJustPressed(sf::Keyboard::F6)) {
        resetPlatformOffsets();
    }
    
    if (isKeyJustPressed(sf::Keyboard::F7)) {
        CLevel* activeLevel = getActiveLevel();
        if (activeLevel) {
            activeLevel->adjustPlatformThickness(10.0f);   
        }
    }
    
    if (isKeyJustPressed(sf::Keyboard::F8)) {
        CLevel* activeLevel = getActiveLevel();
        if (activeLevel) {
            activeLevel->adjustPlatformThickness(-10.0f);  
        }
    }
    
    if (isKeyJustPressed(sf::Keyboard::T)) {
        debugPlatformInfo();
    }
    
    if (isKeyJustPressed(sf::Keyboard::Y)) {
        forcePlayerRepositioning();
    }
}

void CGame::processPauseInput() {
    if (isKeyJustPressed(sf::Keyboard::R)) {
        restartLevel();
    }
}

void CGame::debugShowPlatformPositions() {
    const auto& platforms = getActiveLevel()->getPlatforms();
    std::cout << "Total de plataformas: " << platforms.size() << std::endl;
    
    for (size_t i = 0; i < platforms.size(); i++) {
        const auto& platform = platforms[i];
        
        // Posicion visual
        sf::Vector2f visualPos = platform.floorSprite.getPosition();
        sf::Vector2f visualSize = platform.size;
        
        // Posicion fisica
        if (platform.physicsBody) {
            b2Vec2 physicsPos = platform.physicsBody->GetPosition();
            float physicsCenterX = physicsPos.x * 30.0f;  
            float physicsCenterY = physicsPos.y * 30.0f;
            
            // Diferencia
            float diffX = visualPos.x - (physicsCenterX - visualSize.x/2);
            float diffY = visualPos.y - (physicsCenterY - visualSize.y/2);
            
            std::cout << "DIFERENCIA:" << std::endl;
            std::cout << "   X: " << diffX << " pixeles" << std::endl;
            std::cout << "   Y: " << diffY << " pixeles" << std::endl;
            
            if (std::abs(diffX) > 2.0f || std::abs(diffY) > 2.0f) {
                std::cout << "    DESALINEACION DETECTADA!" << std::endl;
            } else {
                std::cout << "    Alineacion correcta" << std::endl;
            }
        }
    }
}

void CGame::adjustPlatformOffset(float offsetX, float offsetY) {
    if (!getActiveLevel()) return;
    
    // Mover SOLO las plataformas visuales para alinearlas con las fisicas
    auto& platforms = const_cast<std::vector<PhysicalPlatform>&>(getActiveLevel()->getPlatforms());
    
    for (auto& platform : platforms) {
        // Mover solo el sprite visual
        sf::Vector2f currentPos = platform.floorSprite.getPosition();
        platform.floorSprite.setPosition(currentPos.x + offsetX, currentPos.y + offsetY);
        
        // Tambien mover el shape de respaldo
        sf::Vector2f currentShapePos = platform.shape.getPosition();
        platform.shape.setPosition(currentShapePos.x + offsetX, currentShapePos.y + offsetY);
    }
}

void CGame::resetPlatformOffsets() {
    if (!getActiveLevel()) return;
    
    auto& platforms = const_cast<std::vector<PhysicalPlatform>&>(getActiveLevel()->getPlatforms());
    
    for (auto& platform : platforms) {
        // Resetear a la posicion original
        platform.floorSprite.setPosition(platform.position.x, platform.position.y);
        platform.shape.setPosition(platform.position.x, platform.position.y);
    }
}

bool CGame::isKeyJustPressed(sf::Keyboard::Key key) {
    bool currentState = sf::Keyboard::isKeyPressed(key);
    bool wasPressed = m_keyPressed[key];
    m_keyPressed[key] = currentState;
    
    return currentState && !wasPressed && m_inputCooldown <= 0.0f;
}

// GAME LOGIC
void CGame::updateGameplay(float deltaTime) {
    if (!m_player || !getActiveLevel()) return;
    
    // Actualizar jugador
    m_player->update(deltaTime);
    updatePlayerBounds();
    
    // Actualizar nivel actual
    getActiveLevel()->update(deltaTime, m_player->getPosition());
    
    // Verificar colisiones
    checkCollisions();
    
    // Actualizar estado del juego
    updateGameState();
}

void CGame::checkCollisions() {
    checkPlayerEnemyCollisions();
    checkAttackCollisions();
}

void CGame::checkPlayerEnemyCollisions() {
    if (!m_player || !getActiveLevel()) return;
    
    // Verificar que el jugador esta vivo
    if (!m_player->isAlive()) return;
    
    CLevel* level = getActiveLevel();
    sf::FloatRect playerBounds = m_player->getBounds();
    
    // Buscar enemigo mas cercano para colision
    CEnemy* closestEnemy = level->getClosestEnemyToPosition(
        m_player->getPosition(), 40.0f);
    
    if (closestEnemy && closestEnemy->isAlive() && 
        closestEnemy->getBounds().intersects(playerBounds)) {
        
        // Verificar que no estemos ya en estado hurt para evitar spam de dano
        if (!m_player->isHurt()) {
            int damage = closestEnemy->attack();
            if (damage > 0) {
                m_player->takeDamage(damage);
                
                // Anadir un pequeno cooldown para evitar dano continuo
                m_inputCooldown = 0.5f; // 0.5 segundos de cooldown
            }
        }
    }
}

void CGame::checkAttackCollisions() {
    // Esta funcion se llamara cuando el jugador ataque
    // Por ahora es un placeholder para el sistema de ataque
}

void CGame::updateGameState() {
    if (!m_player || !getActiveLevel()) return;
    
    // Verificar si el jugador murio
    if (!m_player->isAlive()) {
        m_gameState = GameState::GAME_OVER;
        return;
    }
    
    // Verificar si el nivel se completo
    if (getActiveLevel()->isCompleted()) {
        m_gameState = GameState::LEVEL_COMPLETED;
        m_totalScore += 1000; // Bonus por completar nivel
    }
}

// LEVEL MANAGEMENT
void CGame::loadLevel(int levelIndex) {
    if (levelIndex < 0 || levelIndex >= static_cast<int>(m_levels.size())) {
        std::cerr << "Error: indice de nivel invalido: " << levelIndex << std::endl;
        return;
    }
    
    // Descargar nivel anterior si existe
    if (m_currentLevelIndex >= 0 && m_currentLevelIndex < static_cast<int>(m_levels.size())) {
        if (m_levels[m_currentLevelIndex]) {
            m_levels[m_currentLevelIndex]->unloadLevel();
        }
    }
    
    m_currentLevelIndex = levelIndex;
    
    // Verificar que el nivel existe
    if (!m_levels[levelIndex]) {
        std::cerr << "Error: Nivel " << levelIndex << " es nulo" << std::endl;
        m_gameState = GameState::GAME_OVER;
        return;
    }
    
    // Configurar fisicas del nivel ANTES de cargar
    if (m_physics) {
        m_levels[levelIndex]->initializePhysics(m_physics.get());
    } else {
        std::cerr << "Warning: Cargando nivel sin sistema de fisicas" << std::endl;
    }
    
    try {
        m_levels[levelIndex]->loadLevel();
        m_levels[levelIndex]->startLevel();
        
    } catch (const std::exception& e) {
        std::cerr << "Error al cargar nivel: " << e.what() << std::endl;
        m_gameState = GameState::GAME_OVER;
        return;
    }
    
    // Reposicionar jugador para este nivel especificamente
    if (m_player) {
        float playerX = 100.0f;
        float playerY = 400.0f;  
        
        m_player->setPosition(playerX, playerY);
        m_player->setHealth(m_player->getMaxHealth());
        
        // Sincronizar con fisicas
        if (m_physics && m_player->getPhysicsBody()) {
            m_player->updatePhysicsPosition();
        }
    }
}

void CGame::createLevels() {
    // Crear 3 niveles principales
    for (int i = 1; i <= 3; i++) {
        m_levels.push_back(std::make_unique<CLevel>(i));
    }
}

CLevel* CGame::getActiveLevel() {
    if (m_currentLevelIndex >= 0 && m_currentLevelIndex < static_cast<int>(m_levels.size())) {
        return m_levels[m_currentLevelIndex].get();
    }
    return nullptr;
}

const CLevel* CGame::getActiveLevel() const {
    if (m_currentLevelIndex >= 0 && m_currentLevelIndex < static_cast<int>(m_levels.size())) {
        return m_levels[m_currentLevelIndex].get();
    }
    return nullptr;
}

// PLAYER MANAGEMENT
void CGame::createPlayer() {
    std::string playerName = "Heroe"; // Nombre por defecto para evitar bloqueo de UI
    
    m_player = std::make_unique<CPlayer>(playerName);
    
    float startX = 400.0f;  // Centro de pantalla
    float startY = 350.0f;  // ARRIBA del suelo negro
    
    m_player->setPosition(startX, startY);
    m_player->setSpeed(m_playerSpeed);
    m_player->setJumpForce(m_jumpForce);
    
    // Inicializar fisicas del jugador
    if (m_physics) {
        m_player->initializePhysics(m_physics.get());
        
        // Verificar que el cuerpo fisico se creo correctamente
        if (!m_player->getPhysicsBody()) {
            std::cerr << "ERROR: Cuerpo fisico del jugador NO se creo" << std::endl;
        }
    } else {
        std::cerr << "ERROR: Sistema de fisicas no disponible" << std::endl;
    }
}

void CGame::handleMusicInput() {
    if (!m_musica) return;
    
    if (isKeyJustPressed(sf::Keyboard::M)) {
        m_musica->toggleSilencio();
    }
    
    if (isKeyJustPressed(sf::Keyboard::Equal)) { 
        float currentVolume = m_musica->getMasterVolumen();
        m_musica->setMasterVolumen(currentVolume + 10.0f);
    }
    
    if (isKeyJustPressed(sf::Keyboard::Hyphen)) { 
        float currentVolume = m_musica->getMasterVolumen();
        m_musica->setMasterVolumen(currentVolume - 10.0f);
    }
    
    // F9 = Debug de musica
    if (isKeyJustPressed(sf::Keyboard::F9)) {
        printMusicInfo();
    }
}

void CGame::handleMusicStateChanges() {
    if (!m_musica) return;
    
    static GameState lastState = GameState::MENU; // Para detectar cambios
    
    if (m_gameState != lastState) {
        switch (m_gameState) {
            case GameState::MENU:
            case GameState::GAME_OVER:
            case GameState::VICTORY:
                if (m_musica->getCurrentMusicType() != MusicType::MENU) {
                    m_musica->fadeToMenuMusic(1.5f);
                }
                break;
                
            case GameState::PLAYING:
                // Cambiar a musica del gameplay
                if (m_musica->getCurrentMusicType() != MusicType::GAMEPLAY) {
                    m_musica->fadeToGameplayMusic(1.5f);
                }
                break;
                
            case GameState::PAUSED:
                // Pausar musica actual
                m_musica->pauseMusic();
                break;
                
            case GameState::LEVEL_COMPLETED:
                // Mantener musica del gameplay pero bajar volumen
                // (opcional - puedes quitar esto si no lo quieres)
                break;
        }
        
        lastState = m_gameState;
    }
    
    // Si se reanuda desde pausa
    if (m_gameState == GameState::PLAYING && m_musica->isPaused()) {
        m_musica->resumeMusic();
    }
}

void CGame::handlePlayerMovement(float deltaTime) {
    if (!m_player) return;
    
    float moveDirection = 0.0f;
    bool isMoving = false;
    
    // Detectar teclas A/D
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        moveDirection = -1.0f;
        isMoving = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        moveDirection = 1.0f;
        isMoving = true;
    }
    
    // Aplicar movimiento
    if (isMoving) {
        if (m_physics && m_player->getPhysicsBody()) {
            // Usar fisicas
            float forceX = moveDirection * 15.0f;
            m_physics->applyForce(m_player.get(), forceX, 0.0f);
            
            // Limitar velocidad maxima
            b2Body* body = m_player->getPhysicsBody();
            b2Vec2 velocity = body->GetLinearVelocity();
            
            if (std::abs(velocity.x) > 8.0f) {
                velocity.x = (velocity.x > 0) ? 8.0f : -8.0f;
                body->SetLinearVelocity(velocity);
            }
        } else {
            // Fallback sin fisicas
            float moveDistance = m_playerSpeed * deltaTime;
            sf::Vector2f currentPos = m_player->getPosition();
            sf::Vector2f newPos = currentPos;
            
            newPos.x += moveDirection * moveDistance;
            
            if (getActiveLevel() && getActiveLevel()->isPositionInBounds(newPos)) {
                m_player->setPosition(newPos);
            }
        }
    } else {
        // Parar movimiento horizontal
        if (m_physics && m_player->getPhysicsBody()) {
            b2Body* body = m_player->getPhysicsBody();
            b2Vec2 velocity = body->GetLinearVelocity();
            velocity.x *= 0.85f;  // Freno gradual
            body->SetLinearVelocity(velocity);
        }
    }
    
    // Actualizar animacion
    m_player->setRunning(isMoving);
}

void CGame::debugMovement() {
    std::cout << "\n DEBUG DE MOVIMIENTO" << std::endl;
    std::cout << "======================" << std::endl;
    
    // Test de teclas
    bool keyA = sf::Keyboard::isKeyPressed(sf::Keyboard::A);
    bool keyD = sf::Keyboard::isKeyPressed(sf::Keyboard::D);
    
    std::cout << "TECLAS:" << std::endl;
    std::cout << "   A (izquierda): " << (keyA ? "Si" : "NO") << std::endl;
    std::cout << "   D (derecha): " << (keyD ? "Si" : "NO") << std::endl;
    
    // Estado del jugador
    if (m_player && m_player->getPhysicsBody()) {
        sf::Vector2f pos = m_player->getPosition();
        b2Vec2 velocity = m_player->getPhysicsBody()->GetLinearVelocity();
        
        std::cout << "\nJUGADOR:" << std::endl;
        std::cout << "   Posicion: (" << pos.x << ", " << pos.y << ")" << std::endl;
        std::cout << "   Velocidad: (" << velocity.x << ", " << velocity.y << ")" << std::endl;
        std::cout << "   En suelo: " << (m_player->isGrounded() ? "Si" : "NO") << std::endl;
    }
    
    std::cout << "======================\n" << std::endl;
}

void CGame::handlePlayerJump() {
    if (!m_player) {
        std::cerr << "Warning: Intento de salto sin jugador" << std::endl;
        return;
    }
    
    if (!m_physics) {
        std::cerr << "Warning: Intento de salto sin sistema de fisicas" << std::endl;
        return;
    }
    
    if (!m_player->getPhysicsBody()) {
        std::cerr << "Warning: Jugador sin cuerpo fisico" << std::endl;
        return;
    }
    
    bool isGrounded = m_player->isGrounded();
    bool isAlive = m_player->isAlive();
    
    if (isGrounded && isAlive) {
        m_player->jump();
    }
}

void CGame::handlePlayerAttack() {
    if (!m_player || !getActiveLevel()) return;
    
    m_player->attack();
    
    // Buscar enemigos en rango de ataque
    CEnemy* targetEnemy = getActiveLevel()->getClosestEnemyToPosition(
        m_player->getPosition(), m_attackRange);
    
    if (targetEnemy) {
        targetEnemy->takeDamage(m_attackDamage);
        m_totalScore += 10; // Puntos por atacar
        
        if (!targetEnemy->isAlive()) {
            m_totalScore += 50; // Bonus por eliminar enemigo
        }
    }
}

void CGame::updatePlayerBounds() {
    // Asegurar que el jugador no salga de los limites
    if (!m_player || !getActiveLevel()) return;
    
    sf::FloatRect levelBounds = getActiveLevel()->getBoundaries();
    sf::Vector2f playerPos = m_player->getPosition();
    
    bool outOfBounds = false;
    
    if (playerPos.x < levelBounds.left) {
        playerPos.x = levelBounds.left;
        outOfBounds = true;
    }
    if (playerPos.x > levelBounds.left + levelBounds.width - 32.0f) {
        playerPos.x = levelBounds.left + levelBounds.width - 32.0f;
        outOfBounds = true;
    }
    if (playerPos.y < levelBounds.top) {
        playerPos.y = levelBounds.top;
        outOfBounds = true;
    }
    if (playerPos.y > levelBounds.top + levelBounds.height - 32.0f) {
        playerPos.y = levelBounds.top + levelBounds.height - 32.0f;
        outOfBounds = true;
    }
    
    if (outOfBounds) {
        m_player->setPosition(playerPos);
        if (m_physics && m_player->getPhysicsBody()) {
            m_player->updatePhysicsPosition();
        }
    }
}

// METODOS DE GESTION DE FISICAS
void CGame::syncPlayerWithPhysics() {
    if (m_player && m_physics) {
        m_player->syncPositionFromPhysics();
    }
}

void CGame::createPhysicsWorld() {
    if (m_physics) {
        std::cout << "Mundo de fisicas ya creado en initializePhysics()" << std::endl;
    }
}

void CGame::createLevelPlatforms() {
    if (getActiveLevel() && m_physics) {
        std::cout << "Plataformas del nivel cargadas automaticamente" << std::endl;
    }
}

void CGame::addPlayerToPhysics() {
    if (m_player && m_physics) {
        m_player->initializePhysics(m_physics.get());
    }
}

void CGame::addEnemyToPhysics(CEnemy* enemy) {
    if (enemy && m_physics) {
        enemy->initializePhysics(m_physics.get());
    }
}

void CGame::removeEnemyFromPhysics(CEnemy* enemy) {
    if (enemy && m_physics) {
        m_physics->destroyBody(enemy);
    }
}

// UI AND RENDERING
void CGame::setupUI() {
    // Intentar cargar fuente
    if (m_font.loadFromFile("arial.ttf")) {
        m_fontLoaded = true;
    } else {
        m_fontLoaded = false;
    }
    
    // Configurar textos
    if (m_fontLoaded) {
        m_titleText.setFont(m_font);
        m_statusText.setFont(m_font);
        m_instructionsText.setFont(m_font);
        m_levelText.setFont(m_font);
        m_healthText.setFont(m_font);
        m_scoreText.setFont(m_font);
    }
    
    // Configurar barra de salud
    m_healthBarBackground.setSize(sf::Vector2f(200.0f, 20.0f));
    m_healthBarBackground.setPosition(10.0f, 10.0f);
    m_healthBarBackground.setFillColor(sf::Color(64, 64, 64));
    m_healthBarBackground.setOutlineThickness(2.0f);
    m_healthBarBackground.setOutlineColor(sf::Color::White);
    
    m_healthBar.setSize(sf::Vector2f(200.0f, 20.0f));
    m_healthBar.setPosition(10.0f, 10.0f);
    m_healthBar.setFillColor(sf::Color::Green);
}

void CGame::updateUI() {
    updateHealthBar();
    
    // Actualizar textos con informacion actual
    m_levelText.setString("Nivel: " + std::to_string(getCurrentLevel()));
    m_scoreText.setString("Puntuacion: " + std::to_string(m_totalScore));
    
    if (m_player) {
        m_healthText.setString("Salud: " + std::to_string(m_player->getHealth()) + 
                              "/" + std::to_string(m_player->getMaxHealth()));
    }
}

void CGame::renderMenu() {
    // RENDERIZAR IMAGEN DE FONDO PRIMERO
    if (m_titleScreenTexture.getSize().x > 0) {
        m_window.draw(m_titleScreenSprite);
    } 
    
    // RENDERIZAR TEXTOS ENCIMA DE LA IMAGEN
    m_titleText.setString("PRESIONA ENTER PARA COMENZAR");
    m_titleText.setCharacterSize(32);
    m_titleText.setFillColor(sf::Color::White);
    m_titleText.setOutlineThickness(2.0f);
    m_titleText.setOutlineColor(sf::Color::Black);
    centerText(m_titleText, 450.0f);
    m_window.draw(m_titleText);
    
    // Instrucciones de controles (MODIFICADO para incluir musica)
    m_statusText.setString("A/D = Mover | W/ESPACIO = Saltar | ENTER = Atacar | M = Musica | +/- = Volumen");
    m_statusText.setCharacterSize(16);
    m_statusText.setFillColor(sf::Color::Yellow);
    m_statusText.setOutlineThickness(1.0f);
    m_statusText.setOutlineColor(sf::Color::Black);
    centerText(m_statusText, 520.0f);
    m_window.draw(m_statusText);
}

void CGame::renderGame() {
    // Renderizar nivel actual
    if (getActiveLevel()) {
        getActiveLevel()->render(m_window);
    }
    
    // Renderizar jugador
    if (m_player) {
        m_player->render(m_window);
    }
    
    // Renderizar HUD
    renderHUD();
}

void CGame::renderPauseScreen() {
    // Crear overlay semi-transparente
    sf::RectangleShape overlay;
    overlay.setSize(sf::Vector2f(800.0f, 600.0f));
    overlay.setFillColor(sf::Color(0, 0, 0, 128));
    m_window.draw(overlay);
    
    m_titleText.setString("PAUSADO");
    m_titleText.setCharacterSize(36);
    m_titleText.setFillColor(sf::Color::Yellow);
    centerText(m_titleText, 250.0f);
    m_window.draw(m_titleText);
    
    m_instructionsText.setString("ESC = Continuar, R = Reiniciar Nivel");
    m_instructionsText.setCharacterSize(20);
    m_instructionsText.setFillColor(sf::Color::White);
    centerText(m_instructionsText, 350.0f);
    m_window.draw(m_instructionsText);
}

void CGame::renderGameOver() {
    m_titleText.setString("GAME OVER");
    m_titleText.setCharacterSize(48);
    m_titleText.setFillColor(sf::Color::Red);
    centerText(m_titleText, 200.0f);
    m_window.draw(m_titleText);
    
    m_statusText.setString("Puntuacion Final: " + std::to_string(m_totalScore));
    m_statusText.setCharacterSize(24);
    m_statusText.setFillColor(sf::Color::White);
    centerText(m_statusText, 300.0f);
    m_window.draw(m_statusText);
    
    m_instructionsText.setString("Presiona ESPACIO para volver al menu");
    m_instructionsText.setCharacterSize(20);
    m_instructionsText.setFillColor(sf::Color::Yellow);
    centerText(m_instructionsText, 400.0f);
    m_window.draw(m_instructionsText);
}

void CGame::renderVictory() {
    m_titleText.setString("VICTORIA!");
    m_titleText.setCharacterSize(48);
    m_titleText.setFillColor(sf::Color::Green);
    centerText(m_titleText, 200.0f);
    m_window.draw(m_titleText);
    
    m_statusText.setString("Has completado todos los niveles!");
    m_statusText.setCharacterSize(24);
    m_statusText.setFillColor(sf::Color::White);
    centerText(m_statusText, 280.0f);
    m_window.draw(m_statusText);
    
    m_scoreText.setString("Puntuacion Final: " + std::to_string(m_totalScore));
    m_scoreText.setCharacterSize(24);
    m_scoreText.setFillColor(sf::Color::Yellow);
    centerText(m_scoreText, 320.0f);
    m_window.draw(m_scoreText);
    
    m_instructionsText.setString("Presiona ESPACIO para volver al menu");
    m_instructionsText.setCharacterSize(20);
    m_instructionsText.setFillColor(sf::Color::Yellow);
    centerText(m_instructionsText, 420.0f);
    m_window.draw(m_instructionsText);
}

void CGame::renderLevelCompleted() {
    // Crear overlay semi-transparente
    sf::RectangleShape overlay;
    overlay.setSize(sf::Vector2f(800.0f, 600.0f));
    overlay.setFillColor(sf::Color(0, 0, 0, 128));
    m_window.draw(overlay);
    
    m_titleText.setString("NIVEL COMPLETADO!");
    m_titleText.setCharacterSize(36);
    m_titleText.setFillColor(sf::Color::Green);
    centerText(m_titleText, 250.0f);
    m_window.draw(m_titleText);
    
    m_instructionsText.setString("Presiona ESPACIO para continuar");
    m_instructionsText.setCharacterSize(20);
    m_instructionsText.setFillColor(sf::Color::White);
    centerText(m_instructionsText, 350.0f);
    m_window.draw(m_instructionsText);
}

void CGame::renderUI() {
    renderHUD();
}

void CGame::renderHUD() {
    // Renderizar barra de salud
    m_window.draw(m_healthBarBackground);
    m_window.draw(m_healthBar);
    
    // Renderizar textos del HUD
    m_levelText.setPosition(10.0f, 40.0f);
    m_levelText.setCharacterSize(16);
    m_levelText.setFillColor(sf::Color::White);
    m_window.draw(m_levelText);
    
    m_scoreText.setPosition(10.0f, 60.0f);
    m_scoreText.setCharacterSize(16);
    m_scoreText.setFillColor(sf::Color::Yellow);
    m_window.draw(m_scoreText);
    
    // Informacion del nivel actual
    if (getActiveLevel()) {
        sf::Text enemyText;
        if (m_fontLoaded) enemyText.setFont(m_font);
        enemyText.setString("Enemigos: " + std::to_string(getActiveLevel()->getEnemiesAlive()));
        enemyText.setPosition(10.0f, 80.0f);
        enemyText.setCharacterSize(16);
        enemyText.setFillColor(sf::Color::Cyan);
        m_window.draw(enemyText);
    }
}

void CGame::renderPhysicsDebug() {
    // Este metodo se puede usar para renderizar informacion de debug de fisicas
    // Por ejemplo, dibujar los contornos de los cuerpos fisicos
    if (m_physics) {
        // Implementar visualizacion de debug si es necesario
    }
}

// UTILITY METHODS
void CGame::centerText(sf::Text& text, float y) {
    sf::FloatRect textBounds = text.getLocalBounds();
    text.setPosition((800.0f - textBounds.width) / 2.0f, y);
}

void CGame::updateHealthBar() {
    if (!m_player) return;
    
    float healthPercentage = static_cast<float>(m_player->getHealth()) / 
                           static_cast<float>(m_player->getMaxHealth());
    
    m_healthBar.setSize(sf::Vector2f(200.0f * healthPercentage, 20.0f));
    m_healthBar.setFillColor(getHealthBarColor(healthPercentage));
}

void CGame::debugFullPhysicsState() {  
    // Estado del jugador
    if (m_player) {
        sf::Vector2f pos = m_player->getPosition();
        sf::Vector2f vel = m_player->getVelocity();
        
        std::cout << "\nJUGADOR:" << std::endl;
        std::cout << "   Posicion: (" << pos.x << ", " << pos.y << ")" << std::endl;
        std::cout << "   Velocidad: (" << vel.x << ", " << vel.y << ")" << std::endl;
        std::cout << "   En suelo: " << (m_player->isGrounded() ? "Si" : "NO") << std::endl;
        std::cout << "   Puede saltar: " << (m_physics->canJump(m_player.get()) ? "Si" : "NO") << std::endl;
        
        if (m_player->getPhysicsBody()) {
            b2Vec2 physicsPos = m_player->getPhysicsBody()->GetPosition();
            b2Vec2 physicsVel = m_player->getPhysicsBody()->GetLinearVelocity();
            std::cout << "   Pos. fisica: (" << physicsPos.x << ", " << physicsPos.y << ") metros" << std::endl;
            std::cout << "   Vel. fisica: (" << physicsVel.x << ", " << physicsVel.y << ") m/s" << std::endl;
        } 
    } else {
        std::cout << "\nJUGADOR: NO EXISTE" << std::endl;
    }
    
    // Estado del nivel actual
    if (getActiveLevel()) {
        std::cout << "\nNIVEL ACTUAL:" << std::endl;
        std::cout << "   Numero: " << getCurrentLevel() << std::endl;
        std::cout << "   Plataformas: " << getActiveLevel()->getPlatformCount() << std::endl;
        std::cout << "   Enemigos vivos: " << getActiveLevel()->getEnemiesAlive() << std::endl;
    } else {
        std::cout << "\nNIVEL ACTUAL: NO EXISTE" << std::endl;
    }
    
    // Estado detallado de las fisicas
    if (m_physics) {
        std::cout << "\nDETALLES DE FISICAS:" << std::endl;
        m_physics->debugPrint();
    }
    
    std::cout << "================================\n" << std::endl;
}

void CGame::debugPlatformInfo() {
    const auto& platforms = getActiveLevel()->getPlatforms();
    std::cout << "Total de plataformas: " << platforms.size() << std::endl;
    
    for (size_t i = 0; i < platforms.size(); i++) {
        const auto& platform = platforms[i];
        std::cout << "Plataforma " << (i+1) << ":" << std::endl;
        std::cout << "   Posicion: (" << platform.position.x << ", " << platform.position.y << ")" << std::endl;
        std::cout << "   Tamano: " << platform.size.x << "x" << platform.size.y << std::endl;
        std::cout << "   Color: ";
        
        sf::Color color = platform.color;
        if (color == sf::Color::Black) std::cout << "NEGRO (SUELO)";
        else if (color == sf::Color::Green) std::cout << "VERDE";
        else if (color == sf::Color::Yellow) std::cout << "AMARILLO";
        else if (color == sf::Color::Red) std::cout << "ROJO";
        else if (color == sf::Color::Cyan) std::cout << "CYAN";
        else std::cout << "Otro";
    }
}

void CGame::forcePlayerRepositioning() {
    if (!m_player || !m_physics) return;
    
    // Posicionar al jugador en una posicion segura arriba del suelo
    float safeX = 100.0f;
    float safeY = 300.0f;  
    
    // Detener toda velocidad
    if (m_player->getPhysicsBody()) {
        m_player->getPhysicsBody()->SetLinearVelocity(b2Vec2(0, 0));
        m_player->getPhysicsBody()->SetAngularVelocity(0);
    }
    
    // Reposicionar
    m_player->setPosition(safeX, safeY);
    m_player->updatePhysicsPosition();
    
    std::cout << "Jugador reposicionado a (" << safeX << ", " << safeY << ")" << std::endl;
}

std::string CGame::gameStateToString(GameState state) const {
    switch (state) {
        case GameState::MENU: return "Menu";
        case GameState::PLAYING: return "Jugando";
        case GameState::PAUSED: return "Pausado";
        case GameState::LEVEL_COMPLETED: return "Nivel Completado";
        case GameState::GAME_OVER: return "Game Over";
        case GameState::VICTORY: return "Victoria";
        default: return "Desconocido";
    }
}

sf::Color CGame::getHealthBarColor(float healthPercentage) const {
    if (healthPercentage > 0.6f) return sf::Color::Green;
    if (healthPercentage > 0.3f) return sf::Color::Yellow;
    return sf::Color::Red;
}

// GAME SETUP
void CGame::initializeWindow() {
    m_window.create(sf::VideoMode(800, 600), "Castelvania", sf::Style::Titlebar | sf::Style::Close);
    m_window.setFramerateLimit(60);
    m_window.setVerticalSyncEnabled(true);
}

void CGame::loadResources() {
    // CARGAR IMAGEN DE TITULO
    if (m_titleScreenTexture.loadFromFile("assets/title_screen.png")) {
        // Configurar sprite de titulo
        m_titleScreenSprite.setTexture(m_titleScreenTexture);
        
        // ESCALAR IMAGEN PARA AJUSTARSE A LA VENTANA (800x600)
        sf::Vector2u textureSize = m_titleScreenTexture.getSize();
        
        // Calcular escalado para ajustar a 800x600 manteniendo aspecto
        float scaleX = 800.0f / textureSize.x;
        float scaleY = 600.0f / textureSize.y;
        
        // Usar el menor escalado para que quepa completa
        float finalScale = std::min(scaleX, scaleY);
        m_titleScreenSprite.setScale(finalScale, finalScale);
        
        // Centrar la imagen
        sf::FloatRect spriteBounds = m_titleScreenSprite.getGlobalBounds();
        float centerX = (800.0f - spriteBounds.width) / 2.0f;
        float centerY = (600.0f - spriteBounds.height) / 2.0f;
        m_titleScreenSprite.setPosition(centerX, centerY);
        
    } else {
        std::cerr << "Error: No se pudo cargar assets/title_screen.png" << std::endl;
    }
}

void CGame::setupGameSettings() {
    // Configurar ajustes del juego
    m_playerSpeed = 150.0f;
    m_jumpForce = 18.0f;
    m_attackRange = 50.0f;
    m_attackDamage = 25;
}

// DEBUG
void CGame::printGameState() const {
    std::cout << "=== Estado del Juego ===" << std::endl;
    std::cout << "Estado: " << gameStateToString(m_gameState) << std::endl;
    std::cout << "Nivel actual: " << getCurrentLevel() << std::endl;
    std::cout << "Puntuacion: " << m_totalScore << std::endl;
    std::cout << "Tiempo de juego: " << m_totalPlayTime << "s" << std::endl;
    std::cout << "=======================" << std::endl;
}

void CGame::printPlayerPosition() const {
    if (m_player) {
        sf::Vector2f pos = m_player->getPosition();
        std::cout << "Posicion del jugador: (" << pos.x << ", " << pos.y << ")" << std::endl;
    }
}

void CGame::printPhysicsInfo() const {
    if (m_physics) {
        m_physics->debugPrint();
        
        if (m_player) {
            m_player->printPhysicsStatus();
        }
        
        if (getActiveLevel()) {
            getActiveLevel()->printPhysicsInfo();
        }
    } 
}

void CGame::debugPositions() {
    if (m_player) {
        sf::Vector2f pos = m_player->getPosition();
        std::cout << "Jugador: (" << pos.x << ", " << pos.y << ")" << std::endl;
        std::cout << "Suelo en Y=450, jugador " << (pos.y < 450 ? "ARRIBA" : "ABAJO") << std::endl;
    }
}

CMusica* CGame::getMusica() const {
    return m_musica.get();
}

void CGame::printMusicInfo() const {
    if (m_musica) {
        m_musica->printAudioStatus();
        m_musica->printVolumeInfo();
    }
}