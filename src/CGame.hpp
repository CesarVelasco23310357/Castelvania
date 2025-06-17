#ifndef CGAME_HPP
#define CGAME_HPP

#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include <string>
#include "CPlayer.hpp"
#include "CLevel.hpp"
#include "CEnemy.hpp"
#include "CPhysics.hpp"  // ← NUEVO: Sistema de físicas

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
    // NUEVO: Sistema de físicas
    // ===================================
    std::unique_ptr<CPhysics> m_physics;
    
    // Input handling
    bool m_keyPressed[sf::Keyboard::KeyCount];
    float m_inputCooldown;
    
    // Game settings
    float m_playerSpeed;
    float m_jumpForce;        // ← NUEVO: Fuerza de salto
    float m_attackRange;
    int m_attackDamage;
    
    // UI Elements
    sf::Text m_titleText;
    sf::Text m_statusText;
    sf::Text m_instructionsText;
    sf::Text m_levelText;
    sf::Text m_healthText;
    sf::Text m_scoreText;
    
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
    
    // Getters
    GameState getGameState() const;
    int getCurrentLevel() const;  // Devuelve el número del nivel (1, 2, 3...)
    int getTotalScore() const;
    float getTotalPlayTime() const;
    CPhysics* getPhysics() const;  // ← NUEVO: Acceso al sistema de físicas
    
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
    void updatePhysics(float deltaTime);    // ← NUEVO: Actualizar físicas
    void checkCollisions();
    void checkPlayerEnemyCollisions();
    void checkAttackCollisions();
    void updateGameState();
    
    // Level management
    void loadLevel(int levelIndex);
    void createLevels();
    CLevel* getActiveLevel();  // Devuelve puntero al nivel activo
    const CLevel* getActiveLevel() const;  // ← NUEVO: Versión const
    
    // Player management
    void createPlayer();
    void handlePlayerMovement(float deltaTime);
    void handlePlayerJump();               // ← NUEVO: Manejar salto
    void handlePlayerAttack();
    void updatePlayerBounds();
    void syncPlayerWithPhysics();          // ← NUEVO: Sincronizar posición física con visual
    
    // Physics management (NUEVO)
    void initializePhysics();
    void createPhysicsWorld();
    void createLevelPlatforms();
    void addPlayerToPhysics();
    void addEnemyToPhysics(CEnemy* enemy);
    void removeEnemyFromPhysics(CEnemy* enemy);
    
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
    void renderPhysicsDebug();             // ← NUEVO: Debug visual de físicas
    
    // Utility methods
    void centerText(sf::Text& text, float y);
    void updateHealthBar();
    std::string gameStateToString(GameState state) const;
    sf::Color getHealthBarColor(float healthPercentage) const;
    
    // Game setup
    void initializeWindow();
    void loadResources();
    void setupGameSettings();
    
    // Debug
    void printGameState() const;
    void printPlayerPosition() const;
    void printPhysicsInfo() const;         // ← NUEVO: Info de físicas
};

#endif // CGAME_HPP