#include "CGame.hpp"
#include <iostream>
#include <cmath>
#include "CMusica.hpp"

// Constructor
CGame::CGame() 
    : gameState(GameState::MENU), running(false), fontLoaded(false),
      currentLevelIndex(0), inputCooldown(0.0f), playerSpeed(150.0f),
      jumpForce(12.0f), attackRange(50.0f), attackDamage(25), totalScore(0),
      levelsCompleted(0), totalPlayTime(0.0f),
      musica(nullptr) { 
    
    // Inicializar array de teclas
    for (int i = 0; i < sf::Keyboard::KeyCount; i++) {
        keyPressed[i] = false;
    }
}

// Destructor
CGame::~CGame() {
    cleanup();
}

// MAIN GAME LOOP
void CGame::run() {
    initialize();
    
    while (running && window.isOpen()) {
        float deltaTime = clock.restart().asSeconds();
        
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
    
    running = true;
    gameState = GameState::MENU;
    
    std::cout << "CGame: Sistema inicializado exitosamente." << std::endl;
}

void CGame::initializeMusic() {
    musica = std::make_unique<CMusica>();
    
    if (musica && musica->initialize()) {
        musica->playMenuMusic();
    } else {
        std::cerr << "Error: No se pudo inicializar el sistema de musica" << std::endl;
        musica.reset(); 
    }
}

void CGame::initializePhysics() {
    physics = std::make_unique<CPhysics>();
    
    if (!physics) {
        std::cerr << "Error: No se pudo inicializar el sistema de fisicas" << std::endl;
    }
}

void CGame::cleanup() {
    if (window.isOpen()) {
        window.close();
    }
    
    player.reset();
    levels.clear();
    
    if (musica) {
        musica->cleanup();
        musica.reset();
    }
    
    physics.reset();
}

bool CGame::isRunning() const {
    return running;
}

// GETTERS
GameState CGame::getGameState() const {
    return gameState;
}

int CGame::getCurrentLevel() const {
    return currentLevelIndex + 1; // +1 porque los indices empiezan en 0
}

int CGame::getTotalScore() const {
    return totalScore;
}

float CGame::getTotalPlayTime() const {
    return totalPlayTime;
}

CPhysics* CGame::getPhysics() const {
    return physics.get();
}

// GAME CONTROL
void CGame::startNewGame() {
    // Reset estadisticas
    totalScore = 0;
    levelsCompleted = 0;
    totalPlayTime = 0.0f;
    currentLevelIndex = 0;
    
    // Crear jugador
    createPlayer();
    
    // Cargar primer nivel
    if (!levels.empty()) {
        loadLevel(0);
        gameState = GameState::PLAYING;
    } else {
        std::cerr << "Error: No hay niveles disponibles!" << std::endl;
        gameState = GameState::GAME_OVER;
    }
}

void CGame::pauseGame() {
    if (gameState == GameState::PLAYING) {
        gameState = GameState::PAUSED;
    }
}

void CGame::resumeGame() {
    if (gameState == GameState::PAUSED) {
        gameState = GameState::PLAYING;
    }
}

void CGame::restartLevel() {
    if (getActiveLevel()) {
        getActiveLevel()->resetLevel();
        if (player) {
            player->setPosition(100.0f, 400.0f);
            player->setHealth(player->getMaxHealth());
            if (physics && player->getPhysicsBody()) {
                player->updatePhysicsPosition();
            }
        }
        gameState = GameState::PLAYING;
    }
}

void CGame::nextLevel() {
    levelsCompleted++;
    currentLevelIndex++;
    
    if (currentLevelIndex < static_cast<int>(levels.size())) {
        loadLevel(currentLevelIndex);
        gameState = GameState::PLAYING;
    } else {
        gameState = GameState::VICTORY;
    }
}

void CGame::endGame() {
    gameState = GameState::MENU;
}

// CORE GAME LOOP METHODS
void CGame::handleEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        switch (event.type) {
            case sf::Event::Closed:
                running = false;
                break;
                
            case sf::Event::KeyPressed:
                // Manejar ESC para cerrar/pausar
                if (event.key.code == sf::Keyboard::Escape) {
                    if (gameState == GameState::PLAYING) {
                        pauseGame();
                    } else if (gameState == GameState::PAUSED) {
                        resumeGame();
                    } else {
                        running = false;
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
    if (inputCooldown > 0.0f) {
        inputCooldown -= deltaTime;
    }
    
    switch (gameState) {
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
                if (gameState == GameState::LEVEL_COMPLETED) {
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
    if (gameState == GameState::PLAYING) {
        totalPlayTime += deltaTime;
    }
    
    switch (gameState) {
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
    if (!physics) {
        std::cerr << "Warning: Sistema de fisicas no inicializado" << std::endl;
        return;
    }
    
    physics->update(deltaTime);
    
    // Sincronizar posiciones fisicas con visuales
    if (player && player->getPhysicsBody()) {
        player->syncPositionFromPhysics();
    }
}

void CGame::updateMusic(float deltaTime) {
    if (!musica) return;
    
    musica->update(deltaTime);
    handleMusicStateChanges();
}

void CGame::render() {
    window.clear(sf::Color::Black);
    
    switch (gameState) {
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
    
    window.display();
}

// INPUT PROCESSING
void CGame::processMenuInput() {
    if (isKeyJustPressed(sf::Keyboard::Enter) || isKeyJustPressed(sf::Keyboard::Space)) {
        startNewGame();
    }
}

void CGame::processGameInput(float deltaTime) {
    if (!player) return;
    
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
    bool wasPressed = keyPressed[key];
    keyPressed[key] = currentState;
    
    return currentState && !wasPressed && inputCooldown <= 0.0f;
}

// GAME LOGIC
void CGame::updateGameplay(float deltaTime) {
    if (!player || !getActiveLevel()) return;
    
    // Actualizar jugador
    player->update(deltaTime);
    updatePlayerBounds();
    
    // Actualizar nivel actual
    getActiveLevel()->update(deltaTime, player->getPosition());
    
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
    if (!player || !getActiveLevel()) return;
    
    // Verificar que el jugador esta vivo
    if (!player->isAlive()) return;
    
    CLevel* level = getActiveLevel();
    sf::FloatRect playerBounds = player->getBounds();
    
    // Buscar enemigo mas cercano para colision
    CEnemy* closestEnemy = level->getClosestEnemyToPosition(
        player->getPosition(), 40.0f);
    
    if (closestEnemy && closestEnemy->isAlive() && 
        closestEnemy->getBounds().intersects(playerBounds)) {
        
        // Verificar que no estemos ya en estado hurt para evitar spam de dano
        if (!player->isHurt()) {
            int damage = closestEnemy->attack();
            if (damage > 0) {
                player->takeDamage(damage);
                
                // Anadir un pequeno cooldown para evitar dano continuo
                inputCooldown = 0.5f; // 0.5 segundos de cooldown
            }
        }
    }
}

void CGame::checkAttackCollisions() {
    // Esta funcion se llamara cuando el jugador ataque
    // Por ahora es un placeholder para el sistema de ataque
}

void CGame::updateGameState() {
    if (!player || !getActiveLevel()) return;
    
    // Verificar si el jugador murio
    if (!player->isAlive()) {
        gameState = GameState::GAME_OVER;
        return;
    }
    
    // Verificar si el nivel se completo
    if (getActiveLevel()->isCompleted()) {
        gameState = GameState::LEVEL_COMPLETED;
        totalScore += 1000; // Bonus por completar nivel
    }
}

// LEVEL MANAGEMENT
void CGame::loadLevel(int levelIndex) {
    if (levelIndex < 0 || levelIndex >= static_cast<int>(levels.size())) {
        std::cerr << "Error: indice de nivel invalido: " << levelIndex << std::endl;
        return;
    }
    
    // Descargar nivel anterior si existe
    if (currentLevelIndex >= 0 && currentLevelIndex < static_cast<int>(levels.size())) {
        if (levels[currentLevelIndex]) {
            levels[currentLevelIndex]->unloadLevel();
        }
    }
    
    currentLevelIndex = levelIndex;
    
    // Verificar que el nivel existe
    if (!levels[levelIndex]) {
        std::cerr << "Error: Nivel " << levelIndex << " es nulo" << std::endl;
        gameState = GameState::GAME_OVER;
        return;
    }
    
    // Configurar fisicas del nivel ANTES de cargar
    if (physics) {
        levels[levelIndex]->initializePhysics(physics.get());
    } else {
        std::cerr << "Warning: Cargando nivel sin sistema de fisicas" << std::endl;
    }
    
    try {
        levels[levelIndex]->loadLevel();
        levels[levelIndex]->startLevel();
        
    } catch (const std::exception& e) {
        std::cerr << "Error al cargar nivel: " << e.what() << std::endl;
        gameState = GameState::GAME_OVER;
        return;
    }
    
    // Reposicionar jugador para este nivel especificamente
    if (player) {
        float playerX = 100.0f;
        float playerY = 400.0f;  
        
        player->setPosition(playerX, playerY);
        player->setHealth(player->getMaxHealth());
        
        // Sincronizar con fisicas
        if (physics && player->getPhysicsBody()) {
            player->updatePhysicsPosition();
        }
    }
}

void CGame::createLevels() {
    // Crear 3 niveles principales
    for (int i = 1; i <= 3; i++) {
        levels.push_back(std::make_unique<CLevel>(i));
    }
}

CLevel* CGame::getActiveLevel() {
    if (currentLevelIndex >= 0 && currentLevelIndex < static_cast<int>(levels.size())) {
        return levels[currentLevelIndex].get();
    }
    return nullptr;
}

const CLevel* CGame::getActiveLevel() const {
    if (currentLevelIndex >= 0 && currentLevelIndex < static_cast<int>(levels.size())) {
        return levels[currentLevelIndex].get();
    }
    return nullptr;
}

// PLAYER MANAGEMENT
void CGame::createPlayer() {
    std::string playerName = "Heroe"; // Nombre por defecto para evitar bloqueo de UI
    
    player = std::make_unique<CPlayer>(playerName);
    
    float startX = 400.0f;  // Centro de pantalla
    float startY = 350.0f;  // ARRIBA del suelo negro
    
    player->setPosition(startX, startY);
    player->setSpeed(playerSpeed);
    player->setJumpForce(jumpForce);
    
    // Inicializar fisicas del jugador
    if (physics) {
        player->initializePhysics(physics.get());
        
        // Verificar que el cuerpo fisico se creo correctamente
        if (!player->getPhysicsBody()) {
            std::cerr << "ERROR: Cuerpo fisico del jugador NO se creo" << std::endl;
        }
    } else {
        std::cerr << "ERROR: Sistema de fisicas no disponible" << std::endl;
    }
}

void CGame::handleMusicInput() {
    if (!musica) return;
    
    if (isKeyJustPressed(sf::Keyboard::M)) {
        musica->toggleSilencio();
    }
    
    if (isKeyJustPressed(sf::Keyboard::Equal)) { 
        float currentVolume = musica->getMasterVolumen();
        musica->setMasterVolumen(currentVolume + 10.0f);
    }
    
    if (isKeyJustPressed(sf::Keyboard::Hyphen)) { 
        float currentVolume = musica->getMasterVolumen();
        musica->setMasterVolumen(currentVolume - 10.0f);
    }
    
    // F9 = Debug de musica
    if (isKeyJustPressed(sf::Keyboard::F9)) {
        printMusicInfo();
    }
}

void CGame::handleMusicStateChanges() {
    if (!musica) return;
    
    static GameState lastState = GameState::MENU; // Para detectar cambios
    
    if (gameState != lastState) {
        switch (gameState) {
            case GameState::MENU:
            case GameState::GAME_OVER:
            case GameState::VICTORY:
                if (musica->getCurrentMusicType() != MusicType::MENU) {
                    musica->fadeToMenuMusic(1.5f);
                }
                break;
                
            case GameState::PLAYING:
                // Cambiar a musica del gameplay
                if (musica->getCurrentMusicType() != MusicType::GAMEPLAY) {
                    musica->fadeToGameplayMusic(1.5f);
                }
                break;
                
            case GameState::PAUSED:
                // Pausar musica actual
                musica->pauseMusic();
                break;
                
            case GameState::LEVEL_COMPLETED:
                // Mantener musica del gameplay pero bajar volumen
                // (opcional - puedes quitar esto si no lo quieres)
                break;
        }
        
        lastState = gameState;
    }
    
    // Si se reanuda desde pausa
    if (gameState == GameState::PLAYING && musica->isPaused()) {
        musica->resumeMusic();
    }
}

void CGame::handlePlayerMovement(float deltaTime) {
    if (!player) return;
    
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
        if (physics && player->getPhysicsBody()) {
            // Usar fisicas
            float forceX = moveDirection * 15.0f;
            physics->applyForce(player.get(), forceX, 0.0f);
            
            // Limitar velocidad maxima
            b2Body* body = player->getPhysicsBody();
            b2Vec2 velocity = body->GetLinearVelocity();
            
            if (std::abs(velocity.x) > 8.0f) {
                velocity.x = (velocity.x > 0) ? 8.0f : -8.0f;
                body->SetLinearVelocity(velocity);
            }
        } else {
            // Fallback sin fisicas
            float moveDistance = playerSpeed * deltaTime;
            sf::Vector2f currentPos = player->getPosition();
            sf::Vector2f newPos = currentPos;
            
            newPos.x += moveDirection * moveDistance;
            
            if (getActiveLevel() && getActiveLevel()->isPositionInBounds(newPos)) {
                player->setPosition(newPos);
            }
        }
    } else {
        // Parar movimiento horizontal
        if (physics && player->getPhysicsBody()) {
            b2Body* body = player->getPhysicsBody();
            b2Vec2 velocity = body->GetLinearVelocity();
            velocity.x *= 0.85f;  // Freno gradual
            body->SetLinearVelocity(velocity);
        }
    }
    
    // Actualizar animacion
    player->setRunning(isMoving);
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
    if (player && player->getPhysicsBody()) {
        sf::Vector2f pos = player->getPosition();
        b2Vec2 velocity = player->getPhysicsBody()->GetLinearVelocity();
        
        std::cout << "\nJUGADOR:" << std::endl;
        std::cout << "   Posicion: (" << pos.x << ", " << pos.y << ")" << std::endl;
        std::cout << "   Velocidad: (" << velocity.x << ", " << velocity.y << ")" << std::endl;
        std::cout << "   En suelo: " << (player->isGrounded() ? "Si" : "NO") << std::endl;
    }
    
    std::cout << "======================\n" << std::endl;
}

void CGame::handlePlayerJump() {
    if (!player) {
        std::cerr << "Warning: Intento de salto sin jugador" << std::endl;
        return;
    }
    
    if (!physics) {
        std::cerr << "Warning: Intento de salto sin sistema de fisicas" << std::endl;
        return;
    }
    
    if (!player->getPhysicsBody()) {
        std::cerr << "Warning: Jugador sin cuerpo fisico" << std::endl;
        return;
    }
    
    bool isGrounded = player->isGrounded();
    bool isAlive = player->isAlive();
    
    if (isGrounded && isAlive) {
        player->jump();
    }
}

void CGame::handlePlayerAttack() {
    if (!player || !getActiveLevel()) return;
    
    player->attack();
    
    // Buscar enemigos en rango de ataque
    CEnemy* targetEnemy = getActiveLevel()->getClosestEnemyToPosition(
        player->getPosition(), attackRange);
    
    if (targetEnemy) {
        targetEnemy->takeDamage(attackDamage);
        totalScore += 10; // Puntos por atacar
        
        if (!targetEnemy->isAlive()) {
            totalScore += 50; // Bonus por eliminar enemigo
        }
    }
}

void CGame::updatePlayerBounds() {
    // Asegurar que el jugador no salga de los limites
    if (!player || !getActiveLevel()) return;
    
    sf::FloatRect levelBounds = getActiveLevel()->getBoundaries();
    sf::Vector2f playerPos = player->getPosition();
    
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
        player->setPosition(playerPos);
        if (physics && player->getPhysicsBody()) {
            player->updatePhysicsPosition();
        }
    }
}

// METODOS DE GESTION DE FISICAS
void CGame::syncPlayerWithPhysics() {
    if (player && physics) {
        player->syncPositionFromPhysics();
    }
}

void CGame::createPhysicsWorld() {
    if (physics) {
        std::cout << "Mundo de fisicas ya creado en initializePhysics()" << std::endl;
    }
}

void CGame::createLevelPlatforms() {
    if (getActiveLevel() && physics) {
        std::cout << "Plataformas del nivel cargadas automaticamente" << std::endl;
    }
}

void CGame::addPlayerToPhysics() {
    if (player && physics) {
        player->initializePhysics(physics.get());
    }
}

void CGame::addEnemyToPhysics(CEnemy* enemy) {
    if (enemy && physics) {
        enemy->initializePhysics(physics.get());
    }
}

void CGame::removeEnemyFromPhysics(CEnemy* enemy) {
    if (enemy && physics) {
        physics->destroyBody(enemy);
    }
}

// UI AND RENDERING
void CGame::setupUI() {
    // Intentar cargar fuente
    if (font.loadFromFile("arial.ttf")) {
        fontLoaded = true;
    } else {
        fontLoaded = false;
    }
    
    // Configurar textos
    if (fontLoaded) {
        titleText.setFont(font);
        statusText.setFont(font);
        instructionsText.setFont(font);
        levelText.setFont(font);
        healthText.setFont(font);
        scoreText.setFont(font);
    }
    
    // Configurar barra de salud
    healthBarBackground.setSize(sf::Vector2f(200.0f, 20.0f));
    healthBarBackground.setPosition(10.0f, 10.0f);
    healthBarBackground.setFillColor(sf::Color(64, 64, 64));
    healthBarBackground.setOutlineThickness(2.0f);
    healthBarBackground.setOutlineColor(sf::Color::White);
    
    healthBar.setSize(sf::Vector2f(200.0f, 20.0f));
    healthBar.setPosition(10.0f, 10.0f);
    healthBar.setFillColor(sf::Color::Green);
}

void CGame::updateUI() {
    updateHealthBar();
    
    // Actualizar textos con informacion actual
    levelText.setString("Nivel: " + std::to_string(getCurrentLevel()));
    scoreText.setString("Puntuacion: " + std::to_string(totalScore));
    
    if (player) {
        healthText.setString("Salud: " + std::to_string(player->getHealth()) + 
                              "/" + std::to_string(player->getMaxHealth()));
    }
}

void CGame::renderMenu() {
    // RENDERIZAR IMAGEN DE FONDO PRIMERO
    if (titleScreenTexture.getSize().x > 0) {
        window.draw(titleScreenSprite);
    } 
    
    // RENDERIZAR TEXTOS ENCIMA DE LA IMAGEN
    titleText.setString("PRESIONA ENTER PARA COMENZAR");
    titleText.setCharacterSize(32);
    titleText.setFillColor(sf::Color::White);
    titleText.setOutlineThickness(2.0f);
    titleText.setOutlineColor(sf::Color::Black);
    centerText(titleText, 450.0f);
    window.draw(titleText);
    
    // Instrucciones de controles (MODIFICADO para incluir musica)
    statusText.setString("A/D = Mover | W/ESPACIO = Saltar | ENTER = Atacar | M = Musica | +/- = Volumen");
    statusText.setCharacterSize(16);
    statusText.setFillColor(sf::Color::Yellow);
    statusText.setOutlineThickness(1.0f);
    statusText.setOutlineColor(sf::Color::Black);
    centerText(statusText, 520.0f);
    window.draw(statusText);
}

void CGame::renderGame() {
    // Renderizar nivel actual
    if (getActiveLevel()) {
        getActiveLevel()->render(window);
    }
    
    // Renderizar jugador
    if (player) {
        player->render(window);
    }
    
    // Renderizar HUD
    renderHUD();
}

void CGame::renderPauseScreen() {
    // Crear overlay semi-transparente
    sf::RectangleShape overlay;
    overlay.setSize(sf::Vector2f(800.0f, 600.0f));
    overlay.setFillColor(sf::Color(0, 0, 0, 128));
    window.draw(overlay);
    
    titleText.setString("PAUSADO");
    titleText.setCharacterSize(36);
    titleText.setFillColor(sf::Color::Yellow);
    centerText(titleText, 250.0f);
    window.draw(titleText);
    
    instructionsText.setString("ESC = Continuar, R = Reiniciar Nivel");
    instructionsText.setCharacterSize(20);
    instructionsText.setFillColor(sf::Color::White);
    centerText(instructionsText, 350.0f);
    window.draw(instructionsText);
}

void CGame::renderGameOver() {
    titleText.setString("GAME OVER");
    titleText.setCharacterSize(48);
    titleText.setFillColor(sf::Color::Red);
    centerText(titleText, 200.0f);
    window.draw(titleText);
    
    statusText.setString("Puntuacion Final: " + std::to_string(totalScore));
    statusText.setCharacterSize(24);
    statusText.setFillColor(sf::Color::White);
    centerText(statusText, 300.0f);
    window.draw(statusText);
    
    instructionsText.setString("Presiona ESPACIO para volver al menu");
    instructionsText.setCharacterSize(20);
    instructionsText.setFillColor(sf::Color::Yellow);
    centerText(instructionsText, 400.0f);
    window.draw(instructionsText);
}

void CGame::renderVictory() {
    titleText.setString("VICTORIA!");
    titleText.setCharacterSize(48);
    titleText.setFillColor(sf::Color::Green);
    centerText(titleText, 200.0f);
    window.draw(titleText);
    
    statusText.setString("Has completado todos los niveles!");
    statusText.setCharacterSize(24);
    statusText.setFillColor(sf::Color::White);
    centerText(statusText, 280.0f);
    window.draw(statusText);
    
    scoreText.setString("Puntuacion Final: " + std::to_string(totalScore));
    scoreText.setCharacterSize(24);
    scoreText.setFillColor(sf::Color::Yellow);
    centerText(scoreText, 320.0f);
    window.draw(scoreText);
    
    instructionsText.setString("Presiona ESPACIO para volver al menu");
    instructionsText.setCharacterSize(20);
    instructionsText.setFillColor(sf::Color::Yellow);
    centerText(instructionsText, 420.0f);
    window.draw(instructionsText);
}

void CGame::renderLevelCompleted() {
    // Crear overlay semi-transparente
    sf::RectangleShape overlay;
    overlay.setSize(sf::Vector2f(800.0f, 600.0f));
    overlay.setFillColor(sf::Color(0, 0, 0, 128));
    window.draw(overlay);
    
    titleText.setString("NIVEL COMPLETADO!");
    titleText.setCharacterSize(36);
    titleText.setFillColor(sf::Color::Green);
    centerText(titleText, 250.0f);
    window.draw(titleText);
    
    instructionsText.setString("Presiona ESPACIO para continuar");
    instructionsText.setCharacterSize(20);
    instructionsText.setFillColor(sf::Color::White);
    centerText(instructionsText, 350.0f);
    window.draw(instructionsText);
}

void CGame::renderUI() {
    renderHUD();
}

void CGame::renderHUD() {
    // Renderizar barra de salud
    window.draw(healthBarBackground);
    window.draw(healthBar);
    
    // Renderizar textos del HUD
    levelText.setPosition(10.0f, 40.0f);
    levelText.setCharacterSize(16);
    levelText.setFillColor(sf::Color::White);
    window.draw(levelText);
    
    scoreText.setPosition(10.0f, 60.0f);
    scoreText.setCharacterSize(16);
    scoreText.setFillColor(sf::Color::Yellow);
    window.draw(scoreText);
    
    // Informacion del nivel actual
    if (getActiveLevel()) {
        sf::Text enemyText;
        if (fontLoaded) enemyText.setFont(font);
        enemyText.setString("Enemigos: " + std::to_string(getActiveLevel()->getEnemiesAlive()));
        enemyText.setPosition(10.0f, 80.0f);
        enemyText.setCharacterSize(16);
        enemyText.setFillColor(sf::Color::Cyan);
        window.draw(enemyText);
    }
}

void CGame::renderPhysicsDebug() {
    // Este metodo se puede usar para renderizar informacion de debug de fisicas
    // Por ejemplo, dibujar los contornos de los cuerpos fisicos
    if (physics) {
        // Implementar visualizacion de debug si es necesario
    }
}

// UTILITY METHODS
void CGame::centerText(sf::Text& text, float y) {
    sf::FloatRect textBounds = text.getLocalBounds();
    text.setPosition((800.0f - textBounds.width) / 2.0f, y);
}

void CGame::updateHealthBar() {
    if (!player) return;
    
    float healthPercentage = static_cast<float>(player->getHealth()) / 
                           static_cast<float>(player->getMaxHealth());
    
    healthBar.setSize(sf::Vector2f(200.0f * healthPercentage, 20.0f));
    healthBar.setFillColor(getHealthBarColor(healthPercentage));
}

void CGame::debugFullPhysicsState() {  
    // Estado del jugador
    if (player) {
        sf::Vector2f pos = player->getPosition();
        sf::Vector2f vel = player->getVelocity();
        
        std::cout << "\nJUGADOR:" << std::endl;
        std::cout << "   Posicion: (" << pos.x << ", " << pos.y << ")" << std::endl;
        std::cout << "   Velocidad: (" << vel.x << ", " << vel.y << ")" << std::endl;
        std::cout << "   En suelo: " << (player->isGrounded() ? "Si" : "NO") << std::endl;
        std::cout << "   Puede saltar: " << (physics->canJump(player.get()) ? "Si" : "NO") << std::endl;
        
        if (player->getPhysicsBody()) {
            b2Vec2 physicsPos = player->getPhysicsBody()->GetPosition();
            b2Vec2 physicsVel = player->getPhysicsBody()->GetLinearVelocity();
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
    if (physics) {
        std::cout << "\nDETALLES DE FISICAS:" << std::endl;
        physics->debugPrint();
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
    if (!player || !physics) return;
    
    // Posicionar al jugador en una posicion segura arriba del suelo
    float safeX = 100.0f;
    float safeY = 300.0f;  
    
    // Detener toda velocidad
    if (player->getPhysicsBody()) {
        player->getPhysicsBody()->SetLinearVelocity(b2Vec2(0, 0));
        player->getPhysicsBody()->SetAngularVelocity(0);
    }
    
    // Reposicionar
    player->setPosition(safeX, safeY);
    player->updatePhysicsPosition();
    
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
    window.create(sf::VideoMode(800, 600), "Castelvania", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(true);
}

void CGame::loadResources() {
    // CARGAR IMAGEN DE TITULO
    if (titleScreenTexture.loadFromFile("assets/title_screen.png")) {
        // Configurar sprite de titulo
        titleScreenSprite.setTexture(titleScreenTexture);
        
        // ESCALAR IMAGEN PARA AJUSTARSE A LA VENTANA (800x600)
        sf::Vector2u textureSize = titleScreenTexture.getSize();
        
        // Calcular escalado para ajustar a 800x600 manteniendo aspecto
        float scaleX = 800.0f / textureSize.x;
        float scaleY = 600.0f / textureSize.y;
        
        // Usar el menor escalado para que quepa completa
        float finalScale = std::min(scaleX, scaleY);
        titleScreenSprite.setScale(finalScale, finalScale);
        
        // Centrar la imagen
        sf::FloatRect spriteBounds = titleScreenSprite.getGlobalBounds();
        float centerX = (800.0f - spriteBounds.width) / 2.0f;
        float centerY = (600.0f - spriteBounds.height) / 2.0f;
        titleScreenSprite.setPosition(centerX, centerY);
        
    } else {
        std::cerr << "Error: No se pudo cargar assets/title_screen.png" << std::endl;
    }
}

void CGame::setupGameSettings() {
    // Configurar ajustes del juego
    playerSpeed = 150.0f;
    jumpForce = 18.0f;
    attackRange = 50.0f;
    attackDamage = 25;
}

// DEBUG
void CGame::printGameState() const {
    std::cout << "=== Estado del Juego ===" << std::endl;
    std::cout << "Estado: " << gameStateToString(gameState) << std::endl;
    std::cout << "Nivel actual: " << getCurrentLevel() << std::endl;
    std::cout << "Puntuacion: " << totalScore << std::endl;
    std::cout << "Tiempo de juego: " << totalPlayTime << "s" << std::endl;
    std::cout << "=======================" << std::endl;
}

void CGame::printPlayerPosition() const {
    if (player) {
        sf::Vector2f pos = player->getPosition();
        std::cout << "Posicion del jugador: (" << pos.x << ", " << pos.y << ")" << std::endl;
    }
}

void CGame::printPhysicsInfo() const {
    if (physics) {
        physics->debugPrint();
        
        if (player) {
            player->printPhysicsStatus();
        }
        
        if (getActiveLevel()) {
            getActiveLevel()->printPhysicsInfo();
        }
    } 
}

void CGame::debugPositions() {
    if (player) {
        sf::Vector2f pos = player->getPosition();
        std::cout << "Jugador: (" << pos.x << ", " << pos.y << ")" << std::endl;
        std::cout << "Suelo en Y=450, jugador " << (pos.y < 450 ? "ARRIBA" : "ABAJO") << std::endl;
    }
}

CMusica* CGame::getMusica() const {
    return musica.get();
}

void CGame::printMusicInfo() const {
    if (musica) {
        musica->printAudioStatus();
        musica->printVolumeInfo();
    }
}