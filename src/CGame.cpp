#include "CGame.hpp"
#include <iostream>
#include <cmath>

// Constructor
CGame::CGame() 
    : m_gameState(GameState::MENU), m_isRunning(false), m_fontLoaded(false),
      m_currentLevelIndex(0), m_inputCooldown(0.0f), m_playerSpeed(150.0f),
      m_jumpForce(12.0f),         // ← NUEVO: Fuerza de salto
      m_attackRange(50.0f), m_attackDamage(25), m_totalScore(0),
      m_levelsCompleted(0), m_totalPlayTime(0.0f) {
    
    // Inicializar array de teclas
    for (int i = 0; i < sf::Keyboard::KeyCount; i++) {
        m_keyPressed[i] = false;
    }
    
    std::cout << "CGame creado." << std::endl;
}

// Destructor
CGame::~CGame() {
    cleanup();
    std::cout << "CGame destruido." << std::endl;
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
    std::cout << "Inicializando juego..." << std::endl;
    
    initializeWindow();
    loadResources();
    setupGameSettings();
    setupUI();
    
    // *** NUEVO: Inicializar sistema de físicas ***
    initializePhysics();
    
    createLevels();
    
    m_isRunning = true;
    m_gameState = GameState::MENU;
    
    std::cout << "Juego inicializado exitosamente." << std::endl;
}

// ===============================================
// NUEVO: Inicialización del sistema de físicas
// ===============================================
void CGame::initializePhysics() {
    std::cout << "🔧 Inicializando sistema de físicas..." << std::endl;
    
    m_physics = std::make_unique<CPhysics>();
    
    if (m_physics) {
        std::cout << "✅ Sistema de físicas Box2D inicializado" << std::endl;
        m_physics->debugPrint();
    } else {
        std::cerr << "❌ Error: No se pudo inicializar el sistema de físicas" << std::endl;
    }
}

void CGame::cleanup() {
    if (m_window.isOpen()) {
        m_window.close();
    }
    
    m_player.reset();
    m_levels.clear();
    
    // *** NUEVO: Limpiar sistema de físicas ***
    m_physics.reset();
    
    std::cout << "Recursos del juego liberados." << std::endl;
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

// ===============================================
// NUEVO: Getter para acceder al sistema de físicas
// ===============================================
CPhysics* CGame::getPhysics() const {
    return m_physics.get();
}

// GAME CONTROL
void CGame::startNewGame() {
    std::cout << "Iniciando nuevo juego..." << std::endl;
    
    // Reset estadísticas
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
        std::cout << "Juego pausado." << std::endl;
    }
}

void CGame::resumeGame() {
    if (m_gameState == GameState::PAUSED) {
        m_gameState = GameState::PLAYING;
        std::cout << "Juego reanudado." << std::endl;
    }
}

void CGame::restartLevel() {
    if (getActiveLevel()) {
        getActiveLevel()->resetLevel();
        if (m_player) {
            m_player->setPosition(100.0f, 100.0f);
            m_player->setHealth(m_player->getMaxHealth());
            // *** NUEVO: Sincronizar con físicas ***
            if (m_physics) {
                m_player->updatePhysicsPosition();
            }
        }
        m_gameState = GameState::PLAYING;
        std::cout << "Nivel reiniciado." << std::endl;
    }
}

void CGame::nextLevel() {
    m_levelsCompleted++;
    m_currentLevelIndex++;
    
    if (m_currentLevelIndex < static_cast<int>(m_levels.size())) {
        loadLevel(m_currentLevelIndex);
        m_gameState = GameState::PLAYING;
        std::cout << "Avanzando al nivel " << getCurrentLevel() << std::endl;
    } else {
        m_gameState = GameState::VICTORY;
        std::cout << "¡Todos los niveles completados! ¡Victoria!" << std::endl;
    }
}

void CGame::endGame() {
    m_gameState = GameState::MENU;
    std::cout << "Regresando al menú principal." << std::endl;
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
            updatePhysics(deltaTime);    // ← NUEVO: Actualizar físicas
            break;
        default:
            // Otros estados no necesitan update constante
            break;
    }
    
    updateUI();
}

// ===============================================
// NUEVO: Actualización del mundo físico
// ===============================================
void CGame::updatePhysics(float deltaTime) {
    if (m_physics) {
        m_physics->update(deltaTime);
        
        // Sincronizar posiciones físicas con visuales
        if (m_player) {
            m_player->syncPositionFromPhysics();
        }
        
        // Sincronizar enemigos
        if (getActiveLevel()) {
            // Los enemigos se sincronizan automáticamente en su update()
        }
    }
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
            renderGame(); // Renderizar juego de fondo
            renderPauseScreen();
            break;
        case GameState::LEVEL_COMPLETED:
            renderGame(); // Renderizar juego de fondo
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
    
    // *** NUEVO: Salto con W o barra espaciadora ***
    if (isKeyJustPressed(sf::Keyboard::W) || isKeyJustPressed(sf::Keyboard::Space)) {
        handlePlayerJump();
    }
    
    // Ataque con ENTER (cambiado para evitar conflicto con Space)
    if (isKeyJustPressed(sf::Keyboard::Enter)) {
        handlePlayerAttack();
    }
    
    // Restart nivel (para debug)
    if (isKeyJustPressed(sf::Keyboard::R)) {
        restartLevel();
    }
}

void CGame::processPauseInput() {
    if (isKeyJustPressed(sf::Keyboard::R)) {
        restartLevel();
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
    
    CLevel* level = getActiveLevel();
    sf::FloatRect playerBounds = m_player->getBounds();
    
    // Buscar enemigo más cercano para colisión
    CEnemy* closestEnemy = level->getClosestEnemyToPosition(
        m_player->getPosition(), 40.0f);
    
    if (closestEnemy && closestEnemy->getBounds().intersects(playerBounds)) {
        // El jugador toca un enemigo - recibe daño
        int damage = closestEnemy->attack();
        if (damage > 0) {
            m_player->takeDamage(damage);
            std::cout << "¡El jugador recibe " << damage << " de daño!" << std::endl;
        }
    }
}

void CGame::checkAttackCollisions() {
    // Esta función se llamará cuando el jugador ataque
    // Por ahora es un placeholder para el sistema de ataque
}

void CGame::updateGameState() {
    if (!m_player || !getActiveLevel()) return;
    
    // Verificar si el jugador murió
    if (!m_player->isAlive()) {
        m_gameState = GameState::GAME_OVER;
        std::cout << "¡Game Over!" << std::endl;
        return;
    }
    
    // Verificar si el nivel se completó
    if (getActiveLevel()->isCompleted()) {
        m_gameState = GameState::LEVEL_COMPLETED;
        m_totalScore += 1000; // Bonus por completar nivel
        std::cout << "¡Nivel " << getCurrentLevel() << " completado!" << std::endl;
    }
}

// LEVEL MANAGEMENT
void CGame::loadLevel(int levelIndex) {
    if (levelIndex < 0 || levelIndex >= static_cast<int>(m_levels.size())) {
        std::cerr << "Error: Índice de nivel inválido: " << levelIndex << std::endl;
        return;
    }
    
    std::cout << "Cargando nivel " << (levelIndex + 1) << "..." << std::endl;
    
    // Descargar nivel anterior si existe
    if (m_currentLevelIndex < static_cast<int>(m_levels.size())) {
        m_levels[m_currentLevelIndex]->unloadLevel();
    }
    
    m_currentLevelIndex = levelIndex;
    
    // *** NUEVO: Configurar físicas del nivel ***
    if (m_physics) {
        m_levels[levelIndex]->initializePhysics(m_physics.get());
    }
    
    m_levels[levelIndex]->loadLevel();
    m_levels[levelIndex]->startLevel();
    
    // Resetear posición del jugador
    if (m_player) {
        m_player->setPosition(100.0f, 100.0f);
        if (m_physics) {
            m_player->updatePhysicsPosition(); // *** NUEVO: Sincronizar con físicas ***
        }
    }
}

void CGame::createLevels() {
    std::cout << "Creando niveles..." << std::endl;
    
    // Crear 3 niveles principales
    for (int i = 1; i <= 3; i++) {
        m_levels.push_back(std::make_unique<CLevel>(i));
    }
    
    std::cout << "Creados " << m_levels.size() << " niveles." << std::endl;
}

CLevel* CGame::getActiveLevel() {
    if (m_currentLevelIndex >= 0 && m_currentLevelIndex < static_cast<int>(m_levels.size())) {
        return m_levels[m_currentLevelIndex].get();
    }
    return nullptr;
}

// PLAYER MANAGEMENT
void CGame::createPlayer() {
    std::string playerName;
    std::cout << "Ingresa tu nombre: ";
    std::getline(std::cin, playerName);
    
    if (playerName.empty()) {
        playerName = "Héroe";
    }
    
    m_player = std::make_unique<CPlayer>(playerName);
    m_player->setPosition(100.0f, 100.0f);
    m_player->setSpeed(m_playerSpeed);
    m_player->setJumpForce(m_jumpForce);    // *** NUEVO: Configurar fuerza de salto ***
    
    // *** NUEVO: Inicializar físicas del jugador ***
    if (m_physics) {
        m_player->initializePhysics(m_physics.get());
        std::cout << "✅ Jugador agregado al sistema de físicas" << std::endl;
    }
    
    std::cout << "Jugador creado en posición: (100, 100)" << std::endl;
}

void CGame::handlePlayerMovement(float deltaTime) {
    if (!m_player) return;
    
    float moveDirection = 0.0f;
    bool isMoving = false;
    
    // Movimiento horizontal con WASD (ahora con físicas)
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        moveDirection = -1.0f;
        isMoving = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        moveDirection = 1.0f;
        isMoving = true;
    }
    
    // *** NUEVO: Aplicar movimiento con físicas ***
    if (isMoving) {
        if (m_physics) {
            m_player->moveWithPhysics(moveDirection);
        } else {
            // Fallback al movimiento tradicional
            float moveDistance = m_playerSpeed * deltaTime;
            sf::Vector2f currentPos = m_player->getPosition();
            sf::Vector2f newPos = currentPos;
            
            newPos.x += moveDirection * moveDistance;
            
            // Verificar límites del nivel
            if (getActiveLevel() && getActiveLevel()->isPositionInBounds(newPos)) {
                m_player->setPosition(newPos);
            }
        }
    }
    
    // Actualizar estado de correr
    m_player->setRunning(isMoving);
}

// ===============================================
// NUEVO: Manejar salto del jugador
// ===============================================
void CGame::handlePlayerJump() {
    if (m_player && m_player->isGrounded()) {
        m_player->jump();
        std::cout << "🦘 ¡Jugador salta!" << std::endl;
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
    // Asegurar que el jugador no salga de los límites
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
        if (m_physics) {
            m_player->updatePhysicsPosition();
        }
    }
}

// ===============================================
// NUEVOS MÉTODOS DE GESTIÓN DE FÍSICAS
// ===============================================
void CGame::syncPlayerWithPhysics() {
    if (m_player && m_physics) {
        m_player->syncPositionFromPhysics();
    }
}

void CGame::createPhysicsWorld() {
    // Este método se puede usar para crear un mundo de físicas personalizado
    if (m_physics) {
        std::cout << "Mundo de físicas ya creado en initializePhysics()" << std::endl;
    }
}

void CGame::createLevelPlatforms() {
    // Las plataformas se crean en loadLevel() automáticamente
    if (getActiveLevel() && m_physics) {
        std::cout << "Plataformas del nivel cargadas automáticamente" << std::endl;
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
        std::cout << "Fuente cargada exitosamente." << std::endl;
    } else {
        m_fontLoaded = false;
        std::cout << "No se pudo cargar fuente personalizada, usando fuente por defecto." << std::endl;
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
    
    // Actualizar textos con información actual
    m_levelText.setString("Nivel: " + std::to_string(getCurrentLevel()));
    m_scoreText.setString("Puntuación: " + std::to_string(m_totalScore));
    
    if (m_player) {
        m_healthText.setString("Salud: " + std::to_string(m_player->getHealth()) + 
                              "/" + std::to_string(m_player->getMaxHealth()));
    }
}

void CGame::renderMenu() {
    m_titleText.setString("CASTELVANIA");
    m_titleText.setCharacterSize(48);
    m_titleText.setFillColor(sf::Color::Red);
    centerText(m_titleText, 200.0f);
    m_window.draw(m_titleText);
    
    m_instructionsText.setString("Presiona ENTER para comenzar");
    m_instructionsText.setCharacterSize(24);
    m_instructionsText.setFillColor(sf::Color::White);
    centerText(m_instructionsText, 350.0f);
    m_window.draw(m_instructionsText);
    
    m_statusText.setString("A/D = Mover, W/ESPACIO = Saltar, ENTER = Atacar, ESC = Pausar");
    m_statusText.setCharacterSize(18);
    m_statusText.setFillColor(sf::Color::Yellow);
    centerText(m_statusText, 450.0f);
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
    
    m_statusText.setString("Puntuación Final: " + std::to_string(m_totalScore));
    m_statusText.setCharacterSize(24);
    m_statusText.setFillColor(sf::Color::White);
    centerText(m_statusText, 300.0f);
    m_window.draw(m_statusText);
    
    m_instructionsText.setString("Presiona ESPACIO para volver al menú");
    m_instructionsText.setCharacterSize(20);
    m_instructionsText.setFillColor(sf::Color::Yellow);
    centerText(m_instructionsText, 400.0f);
    m_window.draw(m_instructionsText);
}

void CGame::renderVictory() {
    m_titleText.setString("¡VICTORIA!");
    m_titleText.setCharacterSize(48);
    m_titleText.setFillColor(sf::Color::Green);
    centerText(m_titleText, 200.0f);
    m_window.draw(m_titleText);
    
    m_statusText.setString("¡Has completado todos los niveles!");
    m_statusText.setCharacterSize(24);
    m_statusText.setFillColor(sf::Color::White);
    centerText(m_statusText, 280.0f);
    m_window.draw(m_statusText);
    
    m_scoreText.setString("Puntuación Final: " + std::to_string(m_totalScore));
    m_scoreText.setCharacterSize(24);
    m_scoreText.setFillColor(sf::Color::Yellow);
    centerText(m_scoreText, 320.0f);
    m_window.draw(m_scoreText);
    
    m_instructionsText.setString("Presiona ESPACIO para volver al menú");
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
    
    m_titleText.setString("¡NIVEL COMPLETADO!");
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
    
    // Información del nivel actual
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

// ===============================================
// NUEVO: Renderizado de debug de físicas
// ===============================================
void CGame::renderPhysicsDebug() {
    // Este método se puede usar para renderizar información de debug de físicas
    // Por ejemplo, dibujar los contornos de los cuerpos físicos
    if (m_physics) {
        // Implementar visualización de debug si es necesario
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

std::string CGame::gameStateToString(GameState state) const {
    switch (state) {
        case GameState::MENU: return "Menú";
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
    
    std::cout << "Ventana inicializada: 800x600" << std::endl;
}

void CGame::loadResources() {
    // Por ahora solo intentamos cargar la fuente
    // En el futuro aquí cargaríamos texturas, sonidos, etc.
    std::cout << "Cargando recursos..." << std::endl;
}

void CGame::setupGameSettings() {
    // Configurar ajustes del juego
    m_playerSpeed = 150.0f;
    m_jumpForce = 12.0f;     // ← NUEVO: Fuerza de salto
    m_attackRange = 50.0f;
    m_attackDamage = 25;
    
    std::cout << "Configuración del juego establecida." << std::endl;
    std::cout << "  - Velocidad del jugador: " << m_playerSpeed << std::endl;
    std::cout << "  - Fuerza de salto: " << m_jumpForce << std::endl;
}

// DEBUG
void CGame::printGameState() const {
    std::cout << "=== Estado del Juego ===" << std::endl;
    std::cout << "Estado: " << gameStateToString(m_gameState) << std::endl;
    std::cout << "Nivel actual: " << getCurrentLevel() << std::endl;
    std::cout << "Puntuación: " << m_totalScore << std::endl;
    std::cout << "Tiempo de juego: " << m_totalPlayTime << "s" << std::endl;
    std::cout << "=======================" << std::endl;
}

void CGame::printPlayerPosition() const {
    if (m_player) {
        sf::Vector2f pos = m_player->getPosition();
        std::cout << "Posición del jugador: (" << pos.x << ", " << pos.y << ")" << std::endl;
    }
}

// ===============================================
// NUEVO: Debug de físicas
// ===============================================
void CGame::printPhysicsInfo() const {
    std::cout << "=== INFORMACIÓN DE FÍSICAS ===" << std::endl;
    if (m_physics) {
        m_physics->debugPrint();
        
        if (m_player) {
            m_player->printPhysicsStatus();
        }
        
        if (getActiveLevel()) {
            getActiveLevel()->printPhysicsInfo();
        }
    } else {
        std::cout << "Sistema de físicas no inicializado" << std::endl;
    }
    std::cout << "==============================" << std::endl;
}