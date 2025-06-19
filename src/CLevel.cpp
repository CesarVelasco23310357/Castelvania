#include "CLevel.hpp"
#include "CPhysics.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <limits>

// Constructor
CLevel::CLevel(int levelNumber) 
    : m_levelNumber(levelNumber), m_state(LevelState::LOADING),
      m_physics(nullptr),           // ← NUEVO: Referencia al sistema de físicas
      m_levelSize(800.0f, 600.0f), m_levelTime(0.0f), m_spawnTimer(0.0f),
      m_totalEnemies(0), m_enemiesKilled(0), m_texturesLoaded(false),
      m_isLoaded(false), m_completionTime(0.0f) {
    
    m_levelName = "Nivel " + std::to_string(levelNumber);
    m_boundaries = sf::FloatRect(0.0f, 0.0f, m_levelSize.x, m_levelSize.y);
    
    std::cout << "Creando " << m_levelName << std::endl;
}

// Destructor
CLevel::~CLevel() {
    // Limpiar plataformas físicas antes de destruir
    destroyPhysicalPlatforms();
    destroyLevelBoundaries();
    
    unloadLevel();
    std::cout << "Nivel " << m_levelNumber << " destruido." << std::endl;
}

// GETTERS
int CLevel::getLevelNumber() const {
    return m_levelNumber;
}

const std::string& CLevel::getLevelName() const {
    return m_levelName;
}

LevelState CLevel::getState() const {
    return m_state;
}

sf::Vector2f CLevel::getLevelSize() const {
    return m_levelSize;
}

sf::FloatRect CLevel::getBoundaries() const {
    return m_boundaries;
}

float CLevel::getLevelTime() const {
    return m_levelTime;
}

int CLevel::getTotalEnemies() const {
    return m_totalEnemies;
}

int CLevel::getEnemiesKilled() const {
    return m_enemiesKilled;
}

int CLevel::getEnemiesAlive() const {
    int aliveCount = 0;
    for (const auto& enemy : m_enemies) {
        if (enemy && enemy->isAlive()) {
            aliveCount++;
        }
    }
    return aliveCount;
}

float CLevel::getCompletionPercentage() const {
    if (m_totalEnemies == 0) return 100.0f;
    return (static_cast<float>(m_enemiesKilled) / static_cast<float>(m_totalEnemies)) * 100.0f;
}

bool CLevel::isLoaded() const {
    return m_isLoaded;
}

bool CLevel::isCompleted() const {
    return m_state == LevelState::COMPLETED;
}

// ===================================
// NUEVO: Getters para plataformas
// ===================================
const std::vector<PhysicalPlatform>& CLevel::getPlatforms() const {
    return m_platforms;
}

size_t CLevel::getPlatformCount() const {
    return m_platforms.size();
}

// SETTERS
void CLevel::setState(LevelState state) {
    if (m_state != state) {
        std::cout << m_levelName << " cambió de estado: " 
                  << levelStateToString(m_state) << " -> " 
                  << levelStateToString(state) << std::endl;
        m_state = state;
        
        if (state == LevelState::COMPLETED) {
            m_completionTime = m_levelTime;
        }
    }
}

void CLevel::setLevelSize(float width, float height) {
    m_levelSize.x = width;
    m_levelSize.y = height;
    m_boundaries = sf::FloatRect(0.0f, 0.0f, width, height);
    createLevelGeometry();
}

// ===================================
// NUEVO: Inicializar físicas del nivel
// ===================================
void CLevel::initializePhysics(CPhysics* physics) {
    if (!physics) {
        std::cerr << "❌ Error: Sistema de físicas nulo para el nivel" << std::endl;
        return;
    }
    
    std::cout << "⚙️ Inicializando físicas del " << m_levelName << "..." << std::endl;
    
    m_physics = physics;
    
    // Crear plataformas físicas específicas del nivel
    createPhysicalPlatforms();
    
    // Crear límites invisibles
    createLevelBoundaries();
    
    std::cout << "✅ Físicas del " << m_levelName << " inicializadas" << std::endl;
}

// ===================================
// NUEVO: Crear plataformas físicas
// ===================================
void CLevel::createPhysicalPlatforms() {
    if (!m_physics) {
        std::cerr << "❌ Error: No se puede crear plataformas sin sistema de físicas" << std::endl;
        return;
    }
    
    std::cout << "🟩 Creando plataformas físicas para " << m_levelName << "..." << std::endl;
    
    // Limpiar plataformas existentes
    clearPhysicalPlatforms();
    
    // Configurar plataformas específicas por nivel
    setupPhysicalPlatformsForLevel();
    
    std::cout << "✅ " << m_platforms.size() << " plataformas físicas creadas" << std::endl;
}

// ===================================
// NUEVO: Crear límites del nivel
// ===================================
void CLevel::createLevelBoundaries() {
    if (!m_physics) return;
    
    std::cout << "🧱 Creando límites limpios del nivel..." << std::endl;
    
    // Limpiar límites existentes
    destroyLevelBoundaries();
    
    // ===================================
    // SOLO CREAR LÍMITES BÁSICOS - NADA MÁS
    // ===================================
    float wallThickness = 10.0f;  // Más delgados
    
    // Muro izquierdo (fuera de pantalla)
    b2Body* leftWall = m_physics->createWall(-wallThickness, 0.0f, wallThickness, m_levelSize.y);
    if (leftWall) m_wallBodies.push_back(leftWall);
    
    // Muro derecho (fuera de pantalla)
    b2Body* rightWall = m_physics->createWall(m_levelSize.x, 0.0f, wallThickness, m_levelSize.y);
    if (rightWall) m_wallBodies.push_back(rightWall);
    
    // ===================================
    // NO CREAR NADA MÁS - Sin techo, sin muros extra
    // ===================================
    
    std::cout << "✅ SOLO " << m_wallBodies.size() << " límites básicos creados" << std::endl;
    std::cout << "   Sin techo, sin muros invisibles extra" << std::endl;
}
// GESTIÓN DEL NIVEL
void CLevel::loadLevel() {
    if (m_isLoaded) {
        std::cout << "El " << m_levelName << " ya está cargado." << std::endl;
        return;
    }
    
    std::cout << "Cargando " << m_levelName << "..." << std::endl;
    
    // Limpiar datos anteriores
    m_enemies.clear();
    m_spawnPoints.clear();
    m_obstacles.clear();
    
    // Resetear contadores
    m_levelTime = 0.0f;
    m_spawnTimer = 0.0f;
    m_enemiesKilled = 0;
    
    // Configurar nivel específico
    setupLevelConfiguration();
    
    // Cargar texturas
    loadLevelTextures();
    
    // Crear geometría del nivel
    createLevelGeometry();
    
    // *** NUEVO: Crear plataformas físicas si ya hay sistema de físicas ***
    if (m_physics) {
        createPhysicalPlatforms();
        createLevelBoundaries();
    }
    
    m_isLoaded = true;
    setState(LevelState::ACTIVE);
    
    std::cout << m_levelName << " cargado exitosamente. Total enemigos: " 
              << m_totalEnemies << ", Plataformas: " << m_platforms.size() << std::endl;
}

void CLevel::unloadLevel() {
    if (!m_isLoaded) return;
    
    std::cout << "Descargando " << m_levelName << "..." << std::endl;
    
    m_enemies.clear();
    m_spawnPoints.clear();
    m_obstacles.clear();
    
    // *** NUEVO: Limpiar plataformas físicas ***
    destroyPhysicalPlatforms();
    destroyLevelBoundaries();
    
    m_isLoaded = false;
    setState(LevelState::LOADING);
}

void CLevel::resetLevel() {
    std::cout << "Reiniciando " << m_levelName << "..." << std::endl;
    unloadLevel();
    loadLevel();
}

void CLevel::startLevel() {
    if (m_state == LevelState::LOADING) {
        loadLevel();
    }
    setState(LevelState::ACTIVE);
    std::cout << m_levelName << " iniciado!" << std::endl;
}

// GESTIÓN DE ENEMIGOS
void CLevel::addEnemy(EnemyType type, float x, float y) {
    auto enemy = std::make_unique<CEnemy>(type, x, y);
    
    // *** NUEVO: Inicializar físicas del enemigo si el sistema está disponible ***
    if (m_physics) {
        enemy->initializePhysics(m_physics);
    }
    
    m_enemies.push_back(std::move(enemy));
}

void CLevel::addSpawnPoint(float x, float y, EnemyType type, float spawnTime) {
    m_spawnPoints.emplace_back(x, y, type, spawnTime);
    m_totalEnemies++;
}

void CLevel::removeDeadEnemies() {
    auto it = std::remove_if(m_enemies.begin(), m_enemies.end(),
        [this](const std::unique_ptr<CEnemy>& enemy) {
            if (enemy && !enemy->isAlive()) {
                m_enemiesKilled++;
                return true;
            }
            return false;
        });
    
    m_enemies.erase(it, m_enemies.end());
}

CEnemy* CLevel::getClosestEnemyToPosition(const sf::Vector2f& position, float maxRange) {
    CEnemy* closestEnemy = nullptr;
    float closestDistance = maxRange > 0 ? maxRange : std::numeric_limits<float>::max();
    
    for (const auto& enemy : m_enemies) {
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

// ===================================
// CORREGIDO: Agregar plataforma física
// ===================================
void CLevel::addPhysicalPlatform(float x, float y, float width, float height, sf::Color color) {
    if (!m_physics) {
        std::cerr << "❌ Error: No se puede agregar plataforma sin sistema de físicas" << std::endl;
        return;
    }
    
    std::cout << "\n🟩 CREANDO PLATAFORMA CON TEXTURA GRUESA:" << std::endl;
    std::cout << "   📍 Posición física: (" << x << "," << y << ")" << std::endl;
    std::cout << "   📐 Tamaño físico: " << width << "x" << height << std::endl;
    
    // ===================================
    // PASO 1: CREAR PLATAFORMA BASE
    // ===================================
    PhysicalPlatform platform(x, y, width, height, color);
    
    // ===================================
    // PASO 2: CONFIGURAR SPRITE VISUAL MÁS GRUESO
    // ===================================
    if (m_floorTexture.getSize().x > 0) {
        // ===================================
        // HACER LA PARTE VISUAL MÁS GRUESA HACIA ABAJO
        // ===================================
        float visualThickness = 40.0f;  // Grosor visual fijo (puedes cambiar este valor)
        
        // Si la plataforma física ya es gruesa, usar su altura
        float finalVisualHeight = std::max(height, visualThickness);
        
        std::cout << "   🎨 Grosor físico: " << height << " px" << std::endl;
        std::cout << "   🎨 Grosor visual: " << finalVisualHeight << " px" << std::endl;
        
        // Configurar sprite con textura
        platform.floorSprite.setTexture(m_floorTexture);
        
        // ===================================
        // POSICIONAR: La parte SUPERIOR del visual coincide con la física
        // ===================================
        platform.floorSprite.setPosition(x, y);  // Misma posición superior
        
        // ===================================
        // ESCALAR: Ajustar a nuevo tamaño visual
        // ===================================
        sf::Vector2u textureSize = m_floorTexture.getSize(); // 336x112
        float scaleX = width / textureSize.x;                // Ancho igual
        float scaleY = finalVisualHeight / textureSize.y;    // Altura aumentada
        
        platform.floorSprite.setScale(scaleX, scaleY);
        platform.hasTexture = true;
        
        std::cout << "   📏 Escala aplicada: " << scaleX << "x" << scaleY << std::endl;
        std::cout << "   📍 Visual se extiende de Y=" << y << " hasta Y=" << (y + finalVisualHeight) << std::endl;
        
        // ===================================
        // ACTUALIZAR TAMBIÉN EL SHAPE DE RESPALDO
        // ===================================
        platform.shape.setPosition(x, y);
        platform.shape.setSize(sf::Vector2f(width, finalVisualHeight));
        platform.shape.setFillColor(color);
        platform.shape.setOutlineThickness(2.0f);
        platform.shape.setOutlineColor(sf::Color::White);
        
    } else {
        // Fallback: usar rectángulo de color también más grueso
        float visualThickness = 40.0f;
        float finalVisualHeight = std::max(height, visualThickness);
        
        platform.shape.setPosition(x, y);
        platform.shape.setSize(sf::Vector2f(width, finalVisualHeight));
        platform.shape.setFillColor(color);
        platform.shape.setOutlineThickness(2.0f);
        platform.shape.setOutlineColor(sf::Color::White);
        platform.hasTexture = false;
        
        std::cout << "   ⚠️ Usando color sólido grueso (textura no disponible)" << std::endl;
    }
    
    // ===================================
    // PASO 3: CREAR CUERPO FÍSICO (TAMAÑO ORIGINAL - NO CAMBIAR)
    // ===================================
    std::cout << "   🔧 Creando cuerpo físico con tamaño original..." << std::endl;
    platform.physicsBody = m_physics->createPlatform(x, y, width, height);  // Física original
    
    if (platform.physicsBody) {
        std::cout << "   ✅ Plataforma gruesa creada:" << std::endl;
        std::cout << "      Física: " << width << "x" << height << " (jugabilidad)" << std::endl;
        std::cout << "      Visual: " << width << "x" << std::max(height, 40.0f) << " (apariencia)" << std::endl;
        m_platforms.push_back(platform);
    } else {
        std::cerr << "    ERROR: No se pudo crear cuerpo físico" << std::endl;
    }
    
    std::cout << "🟩 PLATAFORMA GRUESA COMPLETADA\n" << std::endl;
}
// ===================================
// NUEVO: Limpiar plataformas físicas
// ===================================
void CLevel::clearPhysicalPlatforms() {
    std::cout << "🧹 Limpiando plataformas físicas..." << std::endl;
    
    // No necesitamos destruir los cuerpos manualmente aquí porque
    // CPhysics se encarga de eso cuando se destruye el mundo
    m_platforms.clear();
    
    std::cout << "✓ Plataformas físicas limpiadas" << std::endl;
}

// GESTIÓN DE OBSTÁCULOS (solo visuales, sin físicas)
void CLevel::addObstacle(float x, float y, float width, float height) {
    sf::RectangleShape obstacle;
    obstacle.setPosition(x, y);
    obstacle.setSize(sf::Vector2f(width, height));
    obstacle.setFillColor(sf::Color(64, 64, 64)); // Gris oscuro
    obstacle.setOutlineThickness(2.0f);
    obstacle.setOutlineColor(sf::Color::Black);
    
    m_obstacles.push_back(obstacle);
}

void CLevel::clearObstacles() {
    m_obstacles.clear();
}

bool CLevel::isPositionBlocked(const sf::Vector2f& position) const {
    for (const auto& obstacle : m_obstacles) {
        if (obstacle.getGlobalBounds().contains(position)) {
            return true;
        }
    }
    return false;
}

// VERIFICACIONES
bool CLevel::isPositionInBounds(const sf::Vector2f& position) const {
    return m_boundaries.contains(position);
}

void CLevel::checkLevelCompletion() {
    if (m_state != LevelState::ACTIVE) return;
    
    // Verificar si todos los enemigos han sido spawneados
    bool allSpawned = true;
    for (const auto& spawnPoint : m_spawnPoints) {
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

// MÉTODOS SFML
void CLevel::update(float deltaTime, const sf::Vector2f& playerPosition) {
    if (m_state != LevelState::ACTIVE) return;
    
    m_levelTime += deltaTime;
    
    // Spawn enemigos según tiempo
    spawnEnemiesFromPoints(deltaTime);
    
    // Actualizar enemigos existentes
    updateEnemies(deltaTime, playerPosition);
    
    // Remover enemigos muertos
    removeDeadEnemies();
    
    // Verificar completación del nivel
    checkLevelCompletion();
}

void CLevel::render(sf::RenderWindow& window) {
    if (!m_isLoaded) return;
    
    // Renderizar fondos
    if (m_texturesLoaded) {
        window.draw(m_layer1Sprite);
        window.draw(m_layer2Sprite);
    } else {
        window.draw(m_background);
    }
    
    // Renderizar plataformas con texturas
    renderPlatforms(window);
    
    // COMENTAR/ELIMINAR ESTA LÍNEA PARA QUITAR LOS CUADRADOS GRISES:
    // renderObstacles(window);  // ← COMENTA ESTA LÍNEA
    
    // Renderizar enemigos
    renderEnemies(window);
    
    // Renderizar borde del nivel
    window.draw(m_border);
}

// ===================================
// NUEVO: Renderizar plataformas físicas
// ===================================
void CLevel::renderPlatforms(sf::RenderWindow& window) {
    for (const auto& platform : m_platforms) {
        if (platform.hasTexture) {
            // Renderizar con textura de floor.png
            window.draw(platform.floorSprite);
        } else {
            // Fallback: renderizar rectángulo de color
            window.draw(platform.shape);
        }
    }
}

// DEBUG
void CLevel::printLevelInfo() const {
    std::cout << "=== Información del Nivel ===\n";
    std::cout << "Número: " << m_levelNumber << "\n";
    std::cout << "Nombre: " << m_levelName << "\n";
    std::cout << "Estado: " << levelStateToString(m_state) << "\n";
    std::cout << "Tiempo: " << m_levelTime << "s\n";
    std::cout << "Tamaño: " << m_levelSize.x << "x" << m_levelSize.y << "\n";
    std::cout << "Enemigos totales: " << m_totalEnemies << "\n";
    std::cout << "Enemigos eliminados: " << m_enemiesKilled << "\n";
    std::cout << "Enemigos vivos: " << getEnemiesAlive() << "\n";
    std::cout << "Progreso: " << getCompletionPercentage() << "%\n";
    std::cout << "===========================\n";
}


void CLevel::printEnemyCount() const {
    std::cout << "Enemigos en " << m_levelName << ": " 
              << getEnemiesAlive() << " vivos, " 
              << m_enemiesKilled << " eliminados\n";
}

// ===================================
// NUEVO: Debug de físicas del nivel
// ===================================
void CLevel::printPhysicsInfo() const {
    std::cout << "=== FÍSICAS DEL " << m_levelName << " ===" << std::endl;
    std::cout << "Sistema de físicas: " << (m_physics ? "ACTIVO" : "INACTIVO") << std::endl;
    std::cout << "Plataformas físicas: " << m_platforms.size() << std::endl;
    std::cout << "Límites del nivel: " << m_wallBodies.size() << std::endl;
    
    if (!m_platforms.empty()) {
        std::cout << "--- Detalle de plataformas ---" << std::endl;
        for (size_t i = 0; i < m_platforms.size(); i++) {
            const auto& platform = m_platforms[i];
            std::cout << "  " << (i+1) << ". Pos: (" << platform.position.x << "," << platform.position.y 
                      << ") Tamaño: " << platform.size.x << "x" << platform.size.y << std::endl;
        }
    }
    
    std::cout << "=============================" << std::endl;
}

// MÉTODOS PRIVADOS
void CLevel::setupLevelConfiguration() {
    switch (m_levelNumber) {
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
    // Si las texturas están cargadas, usar sprites; si no, usar rectángulos de color
    if (m_texturesLoaded) {
        // Configurar sprites de fondo
        m_layer1Sprite.setTexture(m_layer1Texture);
        m_layer2Sprite.setTexture(m_layer2Texture);
        
        // Escalar las imágenes para que cubran toda la pantalla
        sf::Vector2u layer1Size = m_layer1Texture.getSize();
        sf::Vector2u layer2Size = m_layer2Texture.getSize();
        
        float scaleX1 = m_levelSize.x / layer1Size.x;
        float scaleY1 = m_levelSize.y / layer1Size.y;
        m_layer1Sprite.setScale(scaleX1, scaleY1);
        
        float scaleX2 = m_levelSize.x / layer2Size.x;
        float scaleY2 = m_levelSize.y / layer2Size.y;
        m_layer2Sprite.setScale(scaleX2, scaleY2);
        
        std::cout << "🎨 Fondos configurados con texturas." << std::endl;
    } else {
        // Fallback: configurar fondo de color sólido MÁS VISIBLE
        m_background.setSize(m_levelSize);
        m_background.setPosition(0.0f, 0.0f);
        m_background.setFillColor(sf::Color(50, 50, 100)); // Azul oscuro más visible
        
        std::cout << "⚠️  Usando fondo de color azul (texturas no disponibles)." << std::endl;
    }
    
    // Configurar borde
    m_border.setSize(m_levelSize);
    m_border.setPosition(0.0f, 0.0f);
    m_border.setFillColor(sf::Color::Transparent);
    m_border.setOutlineThickness(4.0f);
    m_border.setOutlineColor(sf::Color::Yellow);
}

void CLevel::loadLevelTextures() {
    std::cout << "Cargando texturas del nivel..." << std::endl;
    
    m_texturesLoaded = true; // Assume success, set to false if any fails
    
    // ===================================
    // CARGAR TEXTURA DEL SUELO/PLATAFORMAS
    // ===================================
    std::cout << "🏗️ Cargando textura de plataformas..." << std::endl;
    if (!m_floorTexture.loadFromFile("assets/floor.png")) {
        std::cerr << "❌ Error: No se pudo cargar assets/floor.png" << std::endl;
        std::cerr << "   Las plataformas usarán colores sólidos" << std::endl;
    } else {
        sf::Vector2u floorSize = m_floorTexture.getSize();
        std::cout << "✅ floor.png cargado (" << floorSize.x << "x" << floorSize.y << ")" << std::endl;
    }
    
    // Cargar layer 1 (fondo lejano)
    std::cout << "Intentando cargar: assets/layer_1.png" << std::endl;
    if (!m_layer1Texture.loadFromFile("assets/layer_1.png")) {
        std::cerr << "❌ Error: No se pudo cargar assets/layer_1.png" << std::endl;
        m_texturesLoaded = false;
    } else {
        sf::Vector2u size1 = m_layer1Texture.getSize();
        std::cout << "✅ layer_1.png cargado (" << size1.x << "x" << size1.y << ")" << std::endl;
    }
    
    // Cargar layer 2 (fondo cercano)
    std::cout << "Intentando cargar: assets/layer_2.png" << std::endl;
    if (!m_layer2Texture.loadFromFile("assets/layer_2.png")) {
        std::cerr << "❌ Error: No se pudo cargar assets/layer_2.png" << std::endl;
        m_texturesLoaded = false;
    } else {
        sf::Vector2u size2 = m_layer2Texture.getSize();
        std::cout << "✅ layer_2.png cargado (" << size2.x << "x" << size2.y << ")" << std::endl;
    }
    
    if (m_texturesLoaded) {
        std::cout << "🎨 Todas las texturas cargadas exitosamente." << std::endl;
    } else {
        std::cout << "⚠️  Algunas texturas fallaron, usando gráficos por defecto." << std::endl;
    }
}

void CLevel::spawnEnemiesFromPoints(float deltaTime) {
    m_spawnTimer += deltaTime;
    
    for (auto& spawnPoint : m_spawnPoints) {
        if (!spawnPoint.hasSpawned && m_spawnTimer >= spawnPoint.spawnTime) {
            addEnemy(spawnPoint.enemyType, spawnPoint.position.x, spawnPoint.position.y);
            spawnPoint.hasSpawned = true;
        }
    }
}

void CLevel::updateEnemies(float deltaTime, const sf::Vector2f& playerPosition) {
    for (auto& enemy : m_enemies) {
        if (enemy && enemy->isAlive()) {
            enemy->updateAI(playerPosition, deltaTime);
            enemy->update(deltaTime);
            
            // *** NUEVO: Sincronizar posición del enemigo con físicas ***
            if (m_physics) {
                enemy->syncPositionFromPhysics();
            }
        }
    }
}

void CLevel::renderEnemies(sf::RenderWindow& window) {
    for (const auto& enemy : m_enemies) {
        if (enemy && enemy->isAlive()) {
            enemy->render(window);
        }
    }
}

void CLevel::renderObstacles(sf::RenderWindow& window) {
    for (const auto& obstacle : m_obstacles) {
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

// ===================================
// NUEVO: Configurar plataformas específicas por nivel
// ===================================
void CLevel::setupPhysicalPlatformsForLevel() {
    switch (m_levelNumber) {
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
            // Configuración básica para niveles generados
            addPhysicalPlatform(0.0f, 550.0f, 800.0f, 50.0f, sf::Color::Green);
            addPhysicalPlatform(300.0f, 400.0f, 200.0f, 20.0f, sf::Color::Yellow);
            break;
    }
}

// ===================================
// NUEVO: Destruir plataformas físicas
// ===================================
void CLevel::destroyPhysicalPlatforms() {
    std::cout << "🧹 LIMPIEZA COMPLETA de plataformas físicas..." << std::endl;
    
    // Limpiar plataformas visuales y físicas
    for (auto& platform : m_platforms) {
        if (platform.physicsBody && m_physics) {
            // Destruir cuerpo físico específicamente
            m_physics->destroyBody(platform.physicsBody);
            std::cout << "   🗑️ Cuerpo físico destruido" << std::endl;
        }
    }
    
    m_platforms.clear();
    std::cout << "✓ Todas las plataformas limpiadas completamente" << std::endl;
}

// ===================================
// NUEVO: Destruir límites del nivel
// ===================================
void CLevel::destroyLevelBoundaries() {
    // Los cuerpos se destruyen automáticamente con el mundo de Box2D
    m_wallBodies.clear();
}

// CONFIGURACIONES ESPECÍFICAS POR NIVEL
void CLevel::configureLevel1() {
    m_levelName = "Nivel 1 - Castillo";
    
    // Solo unos pocos enemigos para empezar
    addSpawnPoint(200.0f, 150.0f, EnemyType::MURCIELAGO, 2.0f);
    addSpawnPoint(600.0f, 200.0f, EnemyType::ESQUELETO, 5.0f);
    addSpawnPoint(400.0f, 350.0f, EnemyType::ZOMBIE, 8.0f);
    
    // Un obstáculo simple (visual, sin físicas)
    addObstacle(350.0f, 250.0f, 100.0f, 50.0f);
}

void CLevel::configureLevel2() {
    m_levelName = "Nivel 2 - Pasillo Principal";
    
    // Más enemigos y variedad
    addSpawnPoint(100.0f, 100.0f, EnemyType::MURCIELAGO, 1.0f);
    addSpawnPoint(700.0f, 100.0f, EnemyType::MURCIELAGO, 2.0f);
    addSpawnPoint(300.0f, 200.0f, EnemyType::ESQUELETO, 3.0f);
    addSpawnPoint(500.0f, 200.0f, EnemyType::ESQUELETO, 5.0f);
    addSpawnPoint(400.0f, 400.0f, EnemyType::ZOMBIE, 8.0f);
    
    // Más obstáculos visuales
    addObstacle(200.0f, 150.0f, 100.0f, 30.0f);
    addObstacle(500.0f, 150.0f, 100.0f, 30.0f);
    addObstacle(350.0f, 350.0f, 100.0f, 100.0f);
}

void CLevel::configureLevel3() {
    m_levelName = "Nivel 3 - Salón del Jefe";
    
    // Nivel difícil con muchos enemigos
    addSpawnPoint(100.0f, 100.0f, EnemyType::MURCIELAGO, 1.0f);
    addSpawnPoint(700.0f, 100.0f, EnemyType::MURCIELAGO, 1.5f);
    addSpawnPoint(100.0f, 500.0f, EnemyType::MURCIELAGO, 2.0f);
    addSpawnPoint(700.0f, 500.0f, EnemyType::MURCIELAGO, 2.5f);
    addSpawnPoint(200.0f, 200.0f, EnemyType::ESQUELETO, 3.0f);
    addSpawnPoint(600.0f, 200.0f, EnemyType::ESQUELETO, 4.0f);
    addSpawnPoint(200.0f, 400.0f, EnemyType::ZOMBIE, 5.0f);
    addSpawnPoint(600.0f, 400.0f, EnemyType::ZOMBIE, 6.0f);
    addSpawnPoint(400.0f, 300.0f, EnemyType::ZOMBIE, 10.0f); // Jefe final
    
    // Laberinto de obstáculos visuales
    addObstacle(150.0f, 150.0f, 80.0f, 20.0f);
    addObstacle(570.0f, 150.0f, 80.0f, 20.0f);
    addObstacle(300.0f, 100.0f, 20.0f, 100.0f);
    addObstacle(480.0f, 100.0f, 20.0f, 100.0f);
    addObstacle(300.0f, 400.0f, 200.0f, 20.0f);
}

void CLevel::configureDefaultLevel() {
    m_levelName = "Nivel " + std::to_string(m_levelNumber) + " - Generado";
    
    // Configuración escalable basada en el número de nivel
    int numEnemies = 2 + m_levelNumber;
    
    for (int i = 0; i < numEnemies; i++) {
        float x = 100.0f + std::fmod((i * 150.0f), 600.0f);
        float y = 100.0f + (i % 3) * 150.0f;
        EnemyType type = static_cast<EnemyType>(i % 3);
        float spawnTime = 1.0f + i * 2.0f;
        
        addSpawnPoint(x, y, type, spawnTime);
    }
    
    // Algunos obstáculos aleatorios
    for (int i = 0; i < m_levelNumber; i++) {
        float x = 200.0f + std::fmod((i * 200.0f), 400.0f);
        float y = 200.0f + std::fmod((i * 100.0f), 200.0f);
        addObstacle(x, y, 60.0f, 60.0f);
    }
}

// ===================================
// CORREGIDO: Configuración de plataformas para Nivel 1
// ===================================
void CLevel::configurePlatformsLevel1() {
    std::cout << "\n🟩 CONFIGURANDO PLATAFORMAS NIVEL 1 - CORREGIDO..." << std::endl;
    
    // SUELO NEGRO (mantener igual)
    float groundY = 450.0f;
    std::cout << "🔴 SUELO NEGRO en Y=" << groundY << std::endl;
    addPhysicalPlatform(0.0f, groundY, 800.0f, 150.0f, sf::Color::Black);
    
    // ===================================
    // PLATAFORMAS MÁS BAJAS Y ACCESIBLES
    // ===================================
    std::cout << "🟢 Creando plataformas MÁS BAJAS..." << std::endl;
    
    // Plataforma verde (izquierda) - MÁS BAJA
    addPhysicalPlatform(150.0f, 380.0f, 120.0f, 20.0f, sf::Color::Green);
    
    // Plataforma amarilla (centro) - MÁS BAJA  
    addPhysicalPlatform(350.0f, 320.0f, 120.0f, 20.0f, sf::Color::Yellow);
    
    // Plataforma roja (derecha) - MÁS BAJA
    addPhysicalPlatform(550.0f, 280.0f, 120.0f, 20.0f, sf::Color::Red);
    
    // Plataforma cyan (opcional, cerca del suelo)
    addPhysicalPlatform(250.0f, 400.0f, 100.0f, 20.0f, sf::Color::Cyan);
    
    std::cout << "✅ Plataformas reposicionadas MÁS BAJAS:" << std::endl;
    std::cout << "   🟢 Verde: Y=380 (antes muy alta)" << std::endl;
    std::cout << "   🟡 Amarilla: Y=320" << std::endl;
    std::cout << "   🔴 Roja: Y=280" << std::endl;
    std::cout << "   🔵 Cyan: Y=400" << std::endl;
    std::cout << "================================================\n" << std::endl;
}

// ===================================
// CORREGIDO: Configuración de plataformas para Nivel 2
// ===================================
void CLevel::configurePlatformsLevel2() {
    std::cout << "\n🟩 CONFIGURANDO PLATAFORMAS DEL NIVEL 2..." << std::endl;
    
    // Plataforma principal (suelo)
    addPhysicalPlatform(0.0f, 550.0f, 800.0f, 50.0f, sf::Color::Black);
    
    // Plataformas flotantes más complejas
    addPhysicalPlatform(100.0f, 450.0f, 120.0f, 20.0f, sf::Color::Yellow);
    addPhysicalPlatform(300.0f, 380.0f, 120.0f, 20.0f, sf::Color::Yellow);
    addPhysicalPlatform(500.0f, 320.0f, 120.0f, 20.0f, sf::Color::Red);
    addPhysicalPlatform(650.0f, 420.0f, 100.0f, 20.0f, sf::Color::Red);
    
    // Plataforma alta
    addPhysicalPlatform(350.0f, 200.0f, 100.0f, 20.0f, sf::Color::Red);
    
    std::cout << "✅ NIVEL 2: Plataformas intermedias creadas" << std::endl;
}

// ===================================
// CORREGIDO: Configuración de plataformas para Nivel 3
// ===================================
void CLevel::configurePlatformsLevel3() {
    std::cout << "\n🟩 CONFIGURANDO PLATAFORMAS DEL NIVEL 3..." << std::endl;
    
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
    
    std::cout << "✅ NIVEL 3: Plataformas avanzadas creadas" << std::endl;
}
void CLevel::adjustPlatformThickness(float deltaThickness) {
    std::cout << "\n🔧 AJUSTANDO GROSOR DE PLATAFORMAS: " << deltaThickness << " px" << std::endl;
    
    for (auto& platform : m_platforms) {
        if (platform.hasTexture && m_floorTexture.getSize().x > 0) {
            // Ajustar escala Y del sprite
            sf::Vector2f currentScale = platform.floorSprite.getScale();
            sf::Vector2u textureSize = m_floorTexture.getSize();
            
            // Calcular nueva altura visual
            float currentVisualHeight = currentScale.y * textureSize.y;
            float newVisualHeight = std::max(20.0f, currentVisualHeight + deltaThickness);
            float newScaleY = newVisualHeight / textureSize.y;
            
            platform.floorSprite.setScale(currentScale.x, newScaleY);
            
            // Ajustar también el shape de respaldo
            sf::Vector2f shapeSize = platform.shape.getSize();
            platform.shape.setSize(sf::Vector2f(shapeSize.x, newVisualHeight));
            
            std::cout << "   Plataforma: " << currentVisualHeight << " → " << newVisualHeight << " px" << std::endl;
        }
    }
    
    std::cout << "✅ Grosor ajustado. F7=Más grueso, F8=Más delgado" << std::endl;
}