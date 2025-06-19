#ifndef CGAME_HPP
#define CGAME_HPP

#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include <string>
#include "CPlayer.hpp"
#include "CLevel.hpp"
#include "CEnemy.hpp"
#include "CPhysics.hpp"  // ← Sistema de físicas Box2D

enum class GameState {
    MENU,
    PLAYING,
    PAUSED,
    LEVEL_COMPLETED,
    GAME_OVER,
    VICTORY
};

class CGame {
private:
    // SFML Core
    sf::RenderWindow m_window;
    sf::Clock m_clock;
    sf::Font m_font;
    
    // Game State
    GameState m_gameState;
    bool m_isRunning;
    bool m_fontLoaded;
    
    // Game Objects
    std::unique_ptr<CPlayer> m_player;
    std::vector<std::unique_ptr<CLevel>> m_levels;
    int m_currentLevelIndex;
    
    // ===================================
    // Sistema de físicas Box2D
    // ===================================
    std::unique_ptr<CPhysics> m_physics;
    
    // Input handling
    bool m_keyPressed[sf::Keyboard::KeyCount];
    float m_inputCooldown;
    
    // Game settings
    float m_playerSpeed;
    float m_jumpForce;        // Fuerza de salto del jugador
    float m_attackRange;
    int m_attackDamage;
    
    // UI Elements
    sf::Text m_titleText;
    sf::Text m_statusText;
    sf::Text m_instructionsText;
    sf::Text m_levelText;
    sf::Text m_healthText;
    sf::Text m_scoreText;
    sf::Texture m_titleScreenTexture;           // ← NUEVA: Textura de la pantalla de título
    sf::Sprite m_titleScreenSprite;             // ← NUEVA: Sprite de la pantalla de título

    // Game statistics
    int m_totalScore;
    int m_levelsCompleted;
    float m_totalPlayTime;
    
    // Visual effects
    sf::RectangleShape m_healthBar;
    sf::RectangleShape m_healthBarBackground;
    
public:
    // Constructor y Destructor
    CGame();
    ~CGame();
    
    // Main game loop
    void run();
    
    // Game state management
    void initialize();
    void cleanup();
    bool isRunning() const;
    void debugShowPlatformPositions();         // ← NUEVA
    void adjustPlatformOffset(float x, float y); // ← NUEVA  
    void resetPlatformOffsets();               // ← NUEVA
    // Getters
    GameState getGameState() const;
    int getCurrentLevel() const;          // Devuelve el número del nivel (1, 2, 3...)
    int getTotalScore() const;
    float getTotalPlayTime() const;
    CPhysics* getPhysics() const;         // Acceso al sistema de físicas

    // Game control
    void startNewGame();
    void pauseGame();
    void resumeGame();
    void restartLevel();
    void nextLevel();
    void endGame();
    
private:
    // Core game loop methods
    void handleEvents();
    void handleInput(float deltaTime);
    void update(float deltaTime);
    void render();
    
    // Input processing
    void processMenuInput();
    void processGameInput(float deltaTime);
    void processPauseInput();
    bool isKeyJustPressed(sf::Keyboard::Key key);
    
    // Game logic
    void updateGameplay(float deltaTime);
    void updatePhysics(float deltaTime);          // Actualizar sistema de físicas
    void checkCollisions();
    void checkPlayerEnemyCollisions();
    void checkAttackCollisions();
    void updateGameState();
    
    // Level management
    void loadLevel(int levelIndex);
    void createLevels();
    CLevel* getActiveLevel();                     // Devuelve puntero al nivel activo
    const CLevel* getActiveLevel() const;         // Versión const del método anterior
    
    // Player management
    void createPlayer();
    void handlePlayerMovement(float deltaTime);
    void handlePlayerJump();                      // Manejar salto del jugador
    void handlePlayerAttack();
    void updatePlayerBounds();
    void syncPlayerWithPhysics();                 // Sincronizar posición física con visual
    void debugPlatformInfo();
    // Physics management
    void initializePhysics();
    void createPhysicsWorld();
    void createLevelPlatforms();
    void addPlayerToPhysics();
    void addEnemyToPhysics(CEnemy* enemy);
    void removeEnemyFromPhysics(CEnemy* enemy);
    void debugPositions(); 
    // UI and rendering
    void setupUI();
    void updateUI();
    void renderMenu();
    void renderGame();
    void renderPauseScreen();
    void renderGameOver();
    void renderVictory();
    void renderLevelCompleted();
    void renderUI();
    void renderHUD();
    void renderPhysicsDebug();                    // Debug visual de físicas
    void debugMovement();
    void debugPlatformSync(); 
    // Utility methods
    void centerText(sf::Text& text, float y);
    void updateHealthBar();
    std::string gameStateToString(GameState state) const;
    sf::Color getHealthBarColor(float healthPercentage) const;
    void debugFullPhysicsState();
    
    // Game setup
    void initializeWindow();
    void loadResources();
    void setupGameSettings();
    
    // Debug methods
    void printGameState() const;
    void printPlayerPosition() const;
    void printPhysicsInfo() const;                // Información de físicas para debug
    
    // ===============================================
    // NUEVO: Funciones de debug adicionales
    // ===============================================
    void debugPlatformInfo() const;               // Debug específico de plataformas
    void forcePlayerRepositioning();              // Forzar reposicionamiento del jugador
};

#endif // CGAME_HPP