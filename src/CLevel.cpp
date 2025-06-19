#include "CLevel.hpp"
#include "CPhysics.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <limits>

// Constructor
CLevel::CLevel(int levelNumber) 
    : levelNumber(levelNumber), state(LevelState::LOADING),
      physics(nullptr),        
      levelSize(800.0f, 600.0f), levelTime(0.0f), spawnTimer(0.0f),
      totalEnemies(0), enemiesKilled(0), texturesLoaded(false),
      loaded(false), completionTime(0.0f) {
    
    levelName = "Nivel " + std::to_string(levelNumber);
    boundaries = sf::FloatRect(0.0f, 0.0f, levelSize.x, levelSize.y);
}

// Destructor
CLevel::~CLevel() {
    // Limpiar plataformas fisicas antes de destruir
    destroyPhysicalPlatforms();
    destroyLevelBoundaries();
    unloadLevel();
}

// GETTERS
int CLevel::getLevelNumber() const {
    return levelNumber;
}

const std::string& CLevel::getLevelName() const {
    return levelName;
}

LevelState CLevel::getState() const {
    return state;
}

sf::Vector2f CLevel::getLevelSize() const {
    return levelSize;
}

sf::FloatRect CLevel::getBoundaries() const {
    return boundaries;
}

float CLevel::getLevelTime() const {
    return levelTime;
}

int CLevel::getTotalEnemies() const {
    return totalEnemies;
}

int CLevel::getEnemiesKilled() const {
    return enemiesKilled;
}

int CLevel::getEnemiesAlive() const {
    int aliveCount = 0;
    for (const auto& enemy : enemies) {
        if (enemy && enemy->isAlive()) {
            aliveCount++;
        }
    }
    return aliveCount;
}

float CLevel::getCompletionPercentage() const {
    if (totalEnemies == 0) return 100.0f;
    return (static_cast<float>(enemiesKilled) / static_cast<float>(totalEnemies)) * 100.0f;
}

bool CLevel::isLoaded() const {
    return loaded;
}

bool CLevel::isCompleted() const {
    return state == LevelState::COMPLETED;
}

const std::vector<PhysicalPlatform>& CLevel::getPlatforms() const {
    return platforms;
}

size_t CLevel::getPlatformCount() const {
    return platforms.size();
}

// SETTERS
void CLevel::setState(LevelState state) {
    if (this->state != state) {
        this->state = state;
        
        if (state == LevelState::COMPLETED) {
            completionTime = levelTime;
        }
    }
}

void CLevel::setLevelSize(float width, float height) {
    levelSize.x = width;
    levelSize.y = height;
    boundaries = sf::FloatRect(0.0f, 0.0f, width, height);
    createLevelGeometry();
}

void CLevel::initializePhysics(CPhysics* physics) {
    if (!physics) {
        std::cerr << "Error: Sistema de fisicas nulo para el nivel" << std::endl;
        return;
    }
    
    this->physics = physics;
    
    // Crear plataformas fisicas especificas del nivel
    createPhysicalPlatforms();
    
    // Crear limites invisibles
    createLevelBoundaries();
}

void CLevel::createPhysicalPlatforms() {
    if (!physics) {
        std::cerr << "Error: No se puede crear plataformas sin sistema de fisicas" << std::endl;
        return;
    }
    
    // Limpiar plataformas existentes
    clearPhysicalPlatforms();
    
    // Configurar plataformas especificas por nivel
    setupPhysicalPlatformsForLevel();
}

void CLevel::createLevelBoundaries() {
    if (!physics) return;
    
    // Limpiar limites existentes
    destroyLevelBoundaries();
    
    // SOLO CREAR LIMITES BASICOS - NADA MAS
    float wallThickness = 10.0f;  // Mas delgados
    
    // Muro izquierdo (fuera de pantalla)
    b2Body* leftWall = physics->createWall(-wallThickness, 0.0f, wallThickness, levelSize.y);
    if (leftWall) wallBodies.push_back(leftWall);
    
    // Muro derecho (fuera de pantalla)
    b2Body* rightWall = physics->createWall(levelSize.x, 0.0f, wallThickness, levelSize.y);
    if (rightWall) wallBodies.push_back(rightWall);
}

// GESTION DEL NIVEL
void CLevel::loadLevel() {
    if (loaded) {
        return;
    }
    
    // Limpiar datos anteriores
    enemies.clear();
    spawnPoints.clear();
    obstacles.clear();
    
    // Resetear contadores
    levelTime = 0.0f;
    spawnTimer = 0.0f;
    enemiesKilled = 0;
    
    // Configurar nivel especifico
    setupLevelConfiguration();
    
    // Cargar texturas
    loadLevelTextures();
    
    // Crear geometria del nivel
    createLevelGeometry();
    
    // Crear plataformas fisicas si ya hay sistema de fisicas
    if (physics) {
        createPhysicalPlatforms();
        createLevelBoundaries();
    }
    
    loaded = true;
    setState(LevelState::ACTIVE);
}

void CLevel::unloadLevel() {
    if (!loaded) return;
    
    enemies.clear();
    spawnPoints.clear();
    obstacles.clear();
    
    // Limpiar plataformas fisicas
    destroyPhysicalPlatforms();
    destroyLevelBoundaries();
    
    loaded = false;
    setState(LevelState::LOADING);
}

void CLevel::resetLevel() {
    unloadLevel();
    loadLevel();
}

void CLevel::startLevel() {
    if (state == LevelState::LOADING) {
        loadLevel();
    }
    setState(LevelState::ACTIVE);
}

// GESTION DE ENEMIGOS
void CLevel::addEnemy(EnemyType type, float x, float y) {
    auto enemy = std::make_unique<CEnemy>(type, x, y);
    
    // Inicializar fisicas del enemigo si el sistema esta disponible
    if (physics) {
        enemy->initializePhysics(physics);
    }
    
    enemies.push_back(std::move(enemy));
}

void CLevel::addSpawnPoint(float x, float y, EnemyType type, float spawnTime) {
    spawnPoints.emplace_back(x, y, type, spawnTime);
    totalEnemies++;
}

void CLevel::removeDeadEnemies() {
    auto it = std::remove_if(enemies.begin(), enemies.end(),
        [this](const std::unique_ptr<CEnemy>& enemy) {
            if (enemy && !enemy->isAlive()) {
                enemiesKilled++;
                return true;
            }
            return false;
        });
    
    enemies.erase(it, enemies.end());
}

CEnemy* CLevel::getClosestEnemyToPosition(const sf::Vector2f& position, float maxRange) {
    CEnemy* closestEnemy = nullptr;
    float closestDistance = maxRange > 0 ? maxRange : std::numeric_limits<float>::max();
    
    for (const auto& enemy : enemies) {
        if (enemy && enemy->isAlive()) {
            float distance = std::sqrt(
                std::pow(enemy->getPosition().x - position.x, 2) +
                std::pow(enemy->getPosition().y - position.y, 2)
            );
            
            if (distance < closestDistance) {
                closestDistance = distance;
                closestEnemy = enemy.get();
            }
        }
    }
    
    return closestEnemy;
}

void CLevel::addPhysicalPlatform(float x, float y, float width, float height, sf::Color color) {
    if (!physics) {
        std::cerr << "Error: No se puede agregar plataforma sin sistema de fisicas" << std::endl;
        return;
    }
    
    // PASO 1: CREAR PLATAFORMA BASE
    PhysicalPlatform platform(x, y, width, height, color);
    
    // PASO 2: CONFIGURAR SPRITE VISUAL MAS GRUESO
    if (floorTexture.getSize().x > 0) {
        // HACER LA PARTE VISUAL MAS GRUESA HACIA ABAJO
        float visualThickness = 40.0f;  // Grosor visual fijo (puedes cambiar este valor)
        
        // Si la plataforma fisica ya es gruesa, usar su altura
        float finalVisualHeight = std::max(height, visualThickness);
        
        // Configurar sprite con textura
        platform.floorSprite.setTexture(floorTexture);
        
        // POSICIONAR: La parte SUPERIOR del visual coincide con la fisica
        platform.floorSprite.setPosition(x, y);  // Misma posicion superior
        
        // ESCALAR: Ajustar a nuevo tamano visual
        sf::Vector2u textureSize = floorTexture.getSize(); // 336x112
        float scaleX = width / textureSize.x;                // Ancho igual
        float scaleY = finalVisualHeight / textureSize.y;    // Altura aumentada
        
        platform.floorSprite.setScale(scaleX, scaleY);
        platform.hasTexture = true;
        
        // ACTUALIZAR TAMBIEN EL SHAPE DE RESPALDO
        platform.shape.setPosition(x, y);
        platform.shape.setSize(sf::Vector2f(width, finalVisualHeight));
        platform.shape.setFillColor(color);
        platform.shape.setOutlineThickness(2.0f);
        platform.shape.setOutlineColor(sf::Color::White);
        
    } else {
        // Fallback: usar rectangulo de color tambien mas grueso
        float visualThickness = 40.0f;
        float finalVisualHeight = std::max(height, visualThickness);
        
        platform.shape.setPosition(x, y);
        platform.shape.setSize(sf::Vector2f(width, finalVisualHeight));
        platform.shape.setFillColor(color);
        platform.shape.setOutlineThickness(2.0f);
        platform.shape.setOutlineColor(sf::Color::White);
        platform.hasTexture = false;
    }
    
    // PASO 3: CREAR CUERPO FISICO (TAMANO ORIGINAL - NO CAMBIAR)
    platform.physicsBody = physics->createPlatform(x, y, width, height);  // Fisica original
    
    if (platform.physicsBody) {
        platforms.push_back(platform);
    } else {
        std::cerr << "ERROR: No se pudo crear cuerpo fisico" << std::endl;
    }
}

void CLevel::clearPhysicalPlatforms() {
    // No necesitamos destruir los cuerpos manualmente aqui porque
    // CPhysics se encarga de eso cuando se destruye el mundo
    platforms.clear();
}

// GESTION DE OBSTACULOS (solo visuales, sin fisicas)
void CLevel::addObstacle(float x, float y, float width, float height) {
    sf::RectangleShape obstacle;
    obstacle.setPosition(x, y);
    obstacle.setSize(sf::Vector2f(width, height));
    obstacle.setFillColor(sf::Color(64, 64, 64)); // Gris oscuro
    obstacle.setOutlineThickness(2.0f);
    obstacle.setOutlineColor(sf::Color::Black);
    
    obstacles.push_back(obstacle);
}

void CLevel::clearObstacles() {
    obstacles.clear();
}

bool CLevel::isPositionBlocked(const sf::Vector2f& position) const {
    for (const auto& obstacle : obstacles) {
        if (obstacle.getGlobalBounds().contains(position)) {
            return true;
        }
    }
    return false;
}

// VERIFICACIONES
bool CLevel::isPositionInBounds(const sf::Vector2f& position) const {
    return boundaries.contains(position);
}

void CLevel::checkLevelCompletion() {
    if (state != LevelState::ACTIVE) return;
    
    // Verificar si todos los enemigos han sido spawneados
    bool allSpawned = true;
    for (const auto& spawnPoint : spawnPoints) {
        if (!spawnPoint.hasSpawned) {
            allSpawned = false;
            break;
        }
    }
    
    // Si todos fueron spawneados y no hay enemigos vivos, nivel completado
    if (allSpawned && getEnemiesAlive() == 0) {
        setState(LevelState::COMPLETED);
    }
}

// METODOS SFML
void CLevel::update(float deltaTime, const sf::Vector2f& playerPosition) {
    if (state != LevelState::ACTIVE) return;
    
    levelTime += deltaTime;
    
    // Spawn enemigos segun tiempo
    spawnEnemiesFromPoints(deltaTime);
    
    // Actualizar enemigos existentes
    updateEnemies(deltaTime, playerPosition);
    
    // Remover enemigos muertos
    removeDeadEnemies();
    
    // Verificar completacion del nivel
    checkLevelCompletion();
}

void CLevel::render(sf::RenderWindow& window) {
    if (!loaded) return;
    
    // Renderizar fondos
    if (texturesLoaded) {
        window.draw(layer1Sprite);
        window.draw(layer2Sprite);
    } else {
        window.draw(background);
    }
    
    // Renderizar plataformas con texturas
    renderPlatforms(window);
    
    // COMENTAR/ELIMINAR ESTA LINEA PARA QUITAR LOS CUADRADOS GRISES:
    // renderObstacles(window);  // <- COMENTA ESTA LINEA
    
    // Renderizar enemigos
    renderEnemies(window);
    
    // Renderizar borde del nivel
    window.draw(border);
}

void CLevel::renderPlatforms(sf::RenderWindow& window) {
    for (const auto& platform : platforms) {
        if (platform.hasTexture) {
            // Renderizar con textura de floor.png
            window.draw(platform.floorSprite);
        } else {
            // Fallback: renderizar rectangulo de color
            window.draw(platform.shape);
        }
    }
}

// DEBUG
void CLevel::printLevelInfo() const {
    std::cout << "=== Informacion del Nivel ===" << std::endl;
    std::cout << "Numero: " << levelNumber << std::endl;
    std::cout << "Nombre: " << levelName << std::endl;
    std::cout << "Estado: " << levelStateToString(state) << std::endl;
    std::cout << "Tiempo: " << levelTime << "s" << std::endl;
    std::cout << "Tamano: " << levelSize.x << "x" << levelSize.y << std::endl;
    std::cout << "Enemigos totales: " << totalEnemies << std::endl;
    std::cout << "Enemigos eliminados: " << enemiesKilled << std::endl;
    std::cout << "Enemigos vivos: " << getEnemiesAlive() << std::endl;
    std::cout << "Progreso: " << getCompletionPercentage() << "%" << std::endl;
    std::cout << "===========================" << std::endl;
}

void CLevel::printEnemyCount() const {
    std::cout << "Enemigos en " << levelName << ": " 
              << getEnemiesAlive() << " vivos, " 
              << enemiesKilled << " eliminados" << std::endl;
}

void CLevel::printPhysicsInfo() const {
    if (!platforms.empty()) {
        std::cout << "--- Detalle de plataformas ---" << std::endl;
        for (size_t i = 0; i < platforms.size(); i++) {
            const auto& platform = platforms[i];
            std::cout << "  " << (i+1) << ". Pos: (" << platform.position.x << "," << platform.position.y 
                      << ") Tamano: " << platform.size.x << "x" << platform.size.y << std::endl;
        }
    }
}

// METODOS PRIVADOS
void CLevel::setupLevelConfiguration() {
    switch (levelNumber) {
        case 1:
            configureLevel1();
            break;
        case 2:
            configureLevel2();
            break;
        case 3:
            configureLevel3();
            break;
        default:
            configureDefaultLevel();
            break;
    }
}

void CLevel::createLevelGeometry() {
    // Si las texturas estan cargadas, usar sprites; si no, usar rectangulos de color
    if (texturesLoaded) {
        // Configurar sprites de fondo
        layer1Sprite.setTexture(layer1Texture);
        layer2Sprite.setTexture(layer2Texture);
        
        // Escalar las imagenes para que cubran toda la pantalla
        sf::Vector2u layer1Size = layer1Texture.getSize();
        sf::Vector2u layer2Size = layer2Texture.getSize();
        
        float scaleX1 = levelSize.x / layer1Size.x;
        float scaleY1 = levelSize.y / layer1Size.y;
        layer1Sprite.setScale(scaleX1, scaleY1);
        
        float scaleX2 = levelSize.x / layer2Size.x;
        float scaleY2 = levelSize.y / layer2Size.y;
        layer2Sprite.setScale(scaleX2, scaleY2);
        
    } else {
        // Fallback: configurar fondo de color solido MAS VISIBLE
        background.setSize(levelSize);
        background.setPosition(0.0f, 0.0f);
        background.setFillColor(sf::Color(50, 50, 100)); // Azul oscuro mas visible
    }
    
    // Configurar borde
    border.setSize(levelSize);
    border.setPosition(0.0f, 0.0f);
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineThickness(4.0f);
    border.setOutlineColor(sf::Color::Yellow);
}

void CLevel::loadLevelTextures() {
    texturesLoaded = true; // Assume success, set to false if any fails
    
    // CARGAR TEXTURA DEL SUELO/PLATAFORMAS
    if (!floorTexture.loadFromFile("assets/floor.png")) {
        std::cerr << "Error: No se pudo cargar assets/floor.png" << std::endl;
        texturesLoaded = false;
    }
    
    // Cargar layer 1 (fondo lejano)
    if (!layer1Texture.loadFromFile("assets/layer_1.png")) {
        std::cerr << "Error: No se pudo cargar assets/layer_1.png" << std::endl;
        texturesLoaded = false;
    }
    
    // Cargar layer 2 (fondo cercano)
    if (!layer2Texture.loadFromFile("assets/layer_2.png")) {
        std::cerr << "Error: No se pudo cargar assets/layer_2.png" << std::endl;
        texturesLoaded = false;
    }
}

void CLevel::spawnEnemiesFromPoints(float deltaTime) {
    spawnTimer += deltaTime;
    
    for (auto& spawnPoint : spawnPoints) {
        if (!spawnPoint.hasSpawned && spawnTimer >= spawnPoint.spawnTime) {
            addEnemy(spawnPoint.enemyType, spawnPoint.position.x, spawnPoint.position.y);
            spawnPoint.hasSpawned = true;
        }
    }
}

void CLevel::updateEnemies(float deltaTime, const sf::Vector2f& playerPosition) {
    for (auto& enemy : enemies) {
        if (enemy && enemy->isAlive()) {
            enemy->updateAI(playerPosition, deltaTime);
            enemy->update(deltaTime);
            
            // Sincronizar posicion del enemigo con fisicas
            if (physics) {
                enemy->syncPositionFromPhysics();
            }
        }
    }
}

void CLevel::renderEnemies(sf::RenderWindow& window) {
    for (const auto& enemy : enemies) {
        if (enemy && enemy->isAlive()) {
            enemy->render(window);
        }
    }
}

void CLevel::renderObstacles(sf::RenderWindow& window) {
    for (const auto& obstacle : obstacles) {
        window.draw(obstacle);
    }
}

std::string CLevel::levelStateToString(LevelState state) const {
    switch (state) {
        case LevelState::LOADING: return "Cargando";
        case LevelState::ACTIVE: return "Activo";
        case LevelState::COMPLETED: return "Completado";
        case LevelState::FAILED: return "Fallido";
        default: return "Desconocido";
    }
}

void CLevel::setupPhysicalPlatformsForLevel() {
    switch (levelNumber) {
        case 1:
            configurePlatformsLevel1();
            break;
        case 2:
            configurePlatformsLevel2();
            break;
        case 3:
            configurePlatformsLevel3();
            break;
        default:
            // Configuracion basica para niveles generados
            addPhysicalPlatform(0.0f, 550.0f, 800.0f, 50.0f, sf::Color::Green);
            addPhysicalPlatform(300.0f, 400.0f, 200.0f, 20.0f, sf::Color::Yellow);
            break;
    }
}

void CLevel::destroyPhysicalPlatforms() {
    // Limpiar plataformas visuales y fisicas
    for (auto& platform : platforms) {
        if (platform.physicsBody && physics) {
            // Destruir cuerpo fisico especificamente
            physics->destroyBody(platform.physicsBody);
        }
    }
    
    platforms.clear();
}

void CLevel::destroyLevelBoundaries() {
    // Los cuerpos se destruyen automaticamente con el mundo de Box2D
    wallBodies.clear();
}

// CONFIGURACIONES ESPECIFICAS POR NIVEL
void CLevel::configureLevel1() {
    // Solo unos pocos enemigos para empezar
    addSpawnPoint(200.0f, 150.0f, EnemyType::MURCIELAGO, 2.0f);
    addSpawnPoint(600.0f, 200.0f, EnemyType::ESQUELETO, 5.0f);
    addSpawnPoint(400.0f, 350.0f, EnemyType::ZOMBIE, 8.0f);
    
    // Un obstaculo simple (visual, sin fisicas)
    addObstacle(350.0f, 250.0f, 100.0f, 50.0f);
}

void CLevel::configureLevel2() {
    // Mas enemigos y variedad
    addSpawnPoint(100.0f, 100.0f, EnemyType::MURCIELAGO, 1.0f);
    addSpawnPoint(700.0f, 100.0f, EnemyType::MURCIELAGO, 2.0f);
    addSpawnPoint(300.0f, 200.0f, EnemyType::ESQUELETO, 3.0f);
    addSpawnPoint(500.0f, 200.0f, EnemyType::ESQUELETO, 5.0f);
    addSpawnPoint(400.0f, 400.0f, EnemyType::ZOMBIE, 8.0f);
    
    // Mas obstaculos visuales
    addObstacle(200.0f, 150.0f, 100.0f, 30.0f);
    addObstacle(500.0f, 150.0f, 100.0f, 30.0f);
    addObstacle(350.0f, 350.0f, 100.0f, 100.0f);
}

void CLevel::configureLevel3() {
    // Nivel dificil con muchos enemigos
    addSpawnPoint(100.0f, 100.0f, EnemyType::MURCIELAGO, 1.0f);
    addSpawnPoint(700.0f, 100.0f, EnemyType::MURCIELAGO, 1.5f);
    addSpawnPoint(100.0f, 500.0f, EnemyType::MURCIELAGO, 2.0f);
    addSpawnPoint(700.0f, 500.0f, EnemyType::MURCIELAGO, 2.5f);
    addSpawnPoint(200.0f, 200.0f, EnemyType::ESQUELETO, 3.0f);
    addSpawnPoint(600.0f, 200.0f, EnemyType::ESQUELETO, 4.0f);
    addSpawnPoint(200.0f, 400.0f, EnemyType::ZOMBIE, 5.0f);
    addSpawnPoint(600.0f, 400.0f, EnemyType::ZOMBIE, 6.0f);
    addSpawnPoint(400.0f, 300.0f, EnemyType::ZOMBIE, 10.0f); // Jefe final
    
    // Laberinto de obstaculos visuales
    addObstacle(150.0f, 150.0f, 80.0f, 20.0f);
    addObstacle(570.0f, 150.0f, 80.0f, 20.0f);
    addObstacle(300.0f, 100.0f, 20.0f, 100.0f);
    addObstacle(480.0f, 100.0f, 20.0f, 100.0f);
    addObstacle(300.0f, 400.0f, 200.0f, 20.0f);
}

void CLevel::configureDefaultLevel() {
    levelName = "Nivel " + std::to_string(levelNumber) + " - Generado";
    
    // Configuracion escalable basada en el numero de nivel
    int numEnemies = 2 + levelNumber;
    
    for (int i = 0; i < numEnemies; i++) {
        float x = 100.0f + std::fmod((i * 150.0f), 600.0f);
        float y = 100.0f + (i % 3) * 150.0f;
        EnemyType type = static_cast<EnemyType>(i % 3);
        float spawnTime = 1.0f + i * 2.0f;
        
        addSpawnPoint(x, y, type, spawnTime);
    }
    
    // Algunos obstaculos aleatorios
    for (int i = 0; i < levelNumber; i++) {
        float x = 200.0f + std::fmod((i * 200.0f), 400.0f);
        float y = 200.0f + std::fmod((i * 100.0f), 200.0f);
        addObstacle(x, y, 60.0f, 60.0f);
    }
}

void CLevel::configurePlatformsLevel1() {
    // SUELO NEGRO (mantener igual)
    float groundY = 450.0f;
    addPhysicalPlatform(0.0f, groundY, 800.0f, 150.0f, sf::Color::Black);
    
    // PLATAFORMAS MAS BAJAS Y ACCESIBLES
    // Plataforma verde (izquierda) - MAS BAJA
    addPhysicalPlatform(150.0f, 380.0f, 120.0f, 20.0f, sf::Color::Green);
    
    // Plataforma amarilla (centro) - MAS BAJA  
    addPhysicalPlatform(350.0f, 320.0f, 120.0f, 20.0f, sf::Color::Yellow);
    
    // Plataforma roja (derecha) - MAS BAJA
    addPhysicalPlatform(550.0f, 280.0f, 120.0f, 20.0f, sf::Color::Red);
    
    // Plataforma cyan (opcional, cerca del suelo)
    addPhysicalPlatform(250.0f, 400.0f, 100.0f, 20.0f, sf::Color::Cyan);
}

void CLevel::configurePlatformsLevel2() {
    // Plataforma principal (suelo)
    addPhysicalPlatform(0.0f, 550.0f, 800.0f, 50.0f, sf::Color::Black);
    
    // Plataformas flotantes mas complejas
    addPhysicalPlatform(100.0f, 450.0f, 120.0f, 20.0f, sf::Color::Yellow);
    addPhysicalPlatform(300.0f, 380.0f, 120.0f, 20.0f, sf::Color::Yellow);
    addPhysicalPlatform(500.0f, 320.0f, 120.0f, 20.0f, sf::Color::Red);
    addPhysicalPlatform(650.0f, 420.0f, 100.0f, 20.0f, sf::Color::Red);
    
    // Plataforma alta
    addPhysicalPlatform(350.0f, 200.0f, 100.0f, 20.0f, sf::Color::Red);
}

void CLevel::configurePlatformsLevel3() {
    // Plataforma principal (suelo)
    addPhysicalPlatform(0.0f, 550.0f, 800.0f, 50.0f, sf::Color::Black);
    
    // Laberinto vertical
    addPhysicalPlatform(50.0f, 480.0f, 100.0f, 15.0f, sf::Color::Yellow);
    addPhysicalPlatform(200.0f, 420.0f, 100.0f, 15.0f, sf::Color::Yellow);
    addPhysicalPlatform(350.0f, 360.0f, 100.0f, 15.0f, sf::Color::Red);
    addPhysicalPlatform(500.0f, 300.0f, 100.0f, 15.0f, sf::Color::Red);
    addPhysicalPlatform(650.0f, 240.0f, 100.0f, 15.0f, sf::Color::Red);
    
    // Plataformas de retorno
    addPhysicalPlatform(400.0f, 180.0f, 150.0f, 15.0f, sf::Color::Magenta);
    addPhysicalPlatform(150.0f, 320.0f, 80.0f, 15.0f, sf::Color::Cyan);
    addPhysicalPlatform(600.0f, 400.0f, 80.0f, 15.0f, sf::Color::Cyan);
}

void CLevel::adjustPlatformThickness(float deltaThickness) {
    for (auto& platform : platforms) {
        if (platform.hasTexture && floorTexture.getSize().x > 0) {
            // Ajustar escala Y del sprite
            sf::Vector2f currentScale = platform.floorSprite.getScale();
            sf::Vector2u textureSize = floorTexture.getSize();
            
            // Calcular nueva altura visual
            float currentVisualHeight = currentScale.y * textureSize.y;
            float newVisualHeight = std::max(20.0f, currentVisualHeight + deltaThickness);
            float newScaleY = newVisualHeight / textureSize.y;
            
            platform.floorSprite.setScale(currentScale.x, newScaleY);
            
            // Ajustar tambien el shape de respaldo
            sf::Vector2f shapeSize = platform.shape.getSize();
            platform.shape.setSize(sf::Vector2f(shapeSize.x, newVisualHeight));
        }
    }
}