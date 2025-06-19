#ifndef CPHYSICS_HPP
#define CPHYSICS_HPP

#include <box2d/box2d.h>
#include <SFML/Graphics.hpp>
#include <memory>
#include <unordered_map>

// Categorías de colisión
enum CollisionCategory {
    CATEGORY_PLAYER = 0x0001,
    CATEGORY_ENEMY = 0x0002,
    CATEGORY_PLATFORM = 0x0004,
    CATEGORY_WALL = 0x0008
};

// Tipos de cuerpo físico
enum class BodyType {
    PLAYER,
    ENEMY,
    PLATFORM,
    WALL
};

// Información del cuerpo físico
struct PhysicsBody {
    b2Body* body;
    BodyType type;
    void* userData;  // Puntero al objeto del juego (CPlayer*, CEnemy*, etc.)
    
    PhysicsBody(b2Body* b, BodyType t, void* data = nullptr) 
        : body(b), type(t), userData(data) {}
};

// ===============================================
// NUEVO: ContactListener para detectar colisiones
// ===============================================
class PhysicsContactListener : public b2ContactListener {
private:
    std::unordered_map<void*, int> m_groundContacts; // Conteo de contactos con el suelo por jugador
    
public:
    void BeginContact(b2Contact* contact) override;
    void EndContact(b2Contact* contact) override;
    
    bool isPlayerOnGround(void* playerData);
    void updateGroundContacts();
    
private:
    bool isPlayerPlatformContact(b2Fixture* fixtureA, b2Fixture* fixtureB);
};

class CPhysics {
private:
    std::unique_ptr<b2World> m_world;
    std::unordered_map<void*, PhysicsBody> m_bodies;
    
    // ===============================================
    // NUEVO: ContactListener para detectar colisiones
    // ===============================================
    std::unique_ptr<PhysicsContactListener> m_contactListener;
    
    // ===============================================
    // CORREGIDO: Configuración del mundo mejorada
    // ===============================================
    static constexpr float GRAVITY_X = 0.0f;
    static constexpr float GRAVITY_Y = 20.0f;        // ← CORREGIDO: De 9.8f a 20.0f para mejor adherencia
    static constexpr float SCALE = 30.0f;           // ← CORREGIDO: De 30.0f a 30.0f para mejor precisión
    
    // ===============================================
    // CORREGIDO: Configuración de simulación mejorada
    // ===============================================
    static constexpr int32 VELOCITY_ITERATIONS = 8;  // ← CORREGIDO: De 6 a 8 para más precisión
    static constexpr int32 POSITION_ITERATIONS = 3;  // ← CORREGIDO: De 2 a 3 para mejor estabilidad
    
public:
    // Constructor y destructor
    CPhysics();
    ~CPhysics();
    
    // Gestión del mundo físico
    void update(float deltaTime);
    void setGravity(float x, float y);
    
    // Creación de cuerpos
    b2Body* createPlayerBody(float x, float y, void* userData = nullptr);
    b2Body* createEnemyBody(float x, float y, void* userData = nullptr);
    b2Body* createPlatform(float x, float y, float width, float height);
    b2Body* createWall(float x, float y, float width, float height);
    
    // Gestión de cuerpos
    void destroyBody(void* userData);
    void destroyBody(b2Body* body); 
    b2Body* getBody(void* userData);
    void destroyAllPlatforms(); 
    PhysicsBody* getPhysicsBody(void* userData);
    
    // Utilidades de conversión
    static sf::Vector2f b2VecToSFML(const b2Vec2& vec);
    static b2Vec2 sfmlVecToB2(const sf::Vector2f& vec);
    static float pixelsToMeters(float pixels);
    static float metersToPixels(float meters);
    static sf::Vector2f pixelsToMeters(const sf::Vector2f& pixels);
    static sf::Vector2f metersToPixels(const b2Vec2& meters);
    
    // Control de movimiento
    void setBodyVelocity(void* userData, float x, float y);
    void applyForce(void* userData, float x, float y);
    void applyImpulse(void* userData, float x, float y);
    
    // Verificaciones mejoradas
    bool isBodyOnGround(void* userData);
    bool canJump(void* userData);
    
    // ===============================================
    // NUEVO: Acceso al ContactListener
    // ===============================================
    PhysicsContactListener* getContactListener() const;
    
    // Debug
    void debugPrint() const;
    int getBodyCount() const;
    
private:
    // Métodos auxiliares
    b2BodyDef createBodyDef(float x, float y, b2BodyType type);
    b2FixtureDef createFixtureDef(b2Shape* shape, float density, float friction, float restitution, uint16 category, uint16 mask);
    
    // Cleanup
    void cleanup();
};

#endif // CPHYSICS_HPP