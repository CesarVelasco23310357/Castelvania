#include "CPhysics.hpp"
#include <iostream>
#include <cmath>

// Constructor
CPhysics::CPhysics() {
    std::cout << "🔧 Inicializando sistema de físicas Box2D..." << std::endl;
    
    // Crear mundo con gravedad corregida
    b2Vec2 gravity(GRAVITY_X, GRAVITY_Y);
    m_world = std::make_unique<b2World>(gravity);
    
    std::cout << "✅ Mundo físico creado con gravedad: (" << GRAVITY_X << ", " << GRAVITY_Y << ")" << std::endl;
    std::cout << "📐 Escala de conversión CORREGIDA: " << SCALE << " píxeles = 1 metro" << std::endl;
    std::cout << "⚙️ Iteraciones mejoradas: V=" << VELOCITY_ITERATIONS << ", P=" << POSITION_ITERATIONS << std::endl;
}

// Destructor
CPhysics::~CPhysics() {
    cleanup();
    std::cout << "🧹 Sistema de físicas destruido." << std::endl;
}

// GESTIÓN DEL MUNDO FÍSICO
void CPhysics::update(float deltaTime) {
    if (!m_world) return;
    
    // Simular un paso del mundo físico con iteraciones mejoradas
    m_world->Step(deltaTime, VELOCITY_ITERATIONS, POSITION_ITERATIONS);
}

void CPhysics::setGravity(float x, float y) {
    if (m_world) {
        m_world->SetGravity(b2Vec2(x, y));
        std::cout << "🌍 Gravedad cambiada a: (" << x << ", " << y << ")" << std::endl;
    }
}

// CREACIÓN DE CUERPOS
b2Body* CPhysics::createPlayerBody(float x, float y, void* userData) {
    if (!m_world) return nullptr;
    
    std::cout << "👤 Creando cuerpo físico del jugador en (" << x << ", " << y << ")" << std::endl;
    
    // Definición del cuerpo
    b2BodyDef bodyDef = createBodyDef(x, y, b2_dynamicBody);
    b2Body* body = m_world->CreateBody(&bodyDef);
    
    // Forma del jugador (rectángulo)
    b2PolygonShape shape;
    float width = pixelsToMeters(32.0f);  // 32 píxeles de ancho
    float height = pixelsToMeters(32.0f); // 32 píxeles de alto
    shape.SetAsBox(width / 2.0f, height / 2.0f);
    
    // Propiedades físicas del jugador mejoradas
    b2FixtureDef fixtureDef = createFixtureDef(
        &shape,
        1.0f,   // Densidad
        0.4f,   // Fricción (ligeramente aumentada)
        0.0f,   // Restitución (rebote)
        CATEGORY_PLAYER,
        CATEGORY_PLATFORM | CATEGORY_WALL | CATEGORY_ENEMY
    );
    
    body->CreateFixture(&fixtureDef);
    
    // Evitar rotación del jugador
    body->SetFixedRotation(true);
    
    // Almacenar información del cuerpo
    m_bodies.emplace(userData, PhysicsBody(body, BodyType::PLAYER, userData));
    
    std::cout << "✅ Cuerpo del jugador creado (dinámico, sin rotación)" << std::endl;
    return body;
}

b2Body* CPhysics::createEnemyBody(float x, float y, void* userData) {
    if (!m_world) return nullptr;
    
    std::cout << "👹 Creando cuerpo físico del enemigo en (" << x << ", " << y << ")" << std::endl;
    
    // Definición del cuerpo
    b2BodyDef bodyDef = createBodyDef(x, y, b2_dynamicBody);
    b2Body* body = m_world->CreateBody(&bodyDef);
    
    // Forma del enemigo (rectángulo)
    b2PolygonShape shape;
    float width = pixelsToMeters(28.0f);  // 28 píxeles de ancho
    float height = pixelsToMeters(28.0f); // 28 píxeles de alto
    shape.SetAsBox(width / 2.0f, height / 2.0f);
    
    // Propiedades físicas del enemigo
    b2FixtureDef fixtureDef = createFixtureDef(
        &shape,
        0.8f,   // Densidad
        0.4f,   // Fricción
        0.0f,   // Restitución
        CATEGORY_ENEMY,
        CATEGORY_PLATFORM | CATEGORY_WALL | CATEGORY_PLAYER
    );
    
    body->CreateFixture(&fixtureDef);
    
    // Evitar rotación del enemigo
    body->SetFixedRotation(true);
    
    // Almacenar información del cuerpo
    m_bodies.emplace(userData, PhysicsBody(body, BodyType::ENEMY, userData));
    
    std::cout << "✅ Cuerpo del enemigo creado (dinámico, sin rotación)" << std::endl;
    return body;
}

// ===============================================
// CORREGIDO: Método createPlatform mejorado
// ===============================================
b2Body* CPhysics::createPlatform(float x, float y, float width, float height) {
    if (!m_world) return nullptr;
    
    std::cout << "🟩 Creando plataforma: (" << x << "," << y << ") " << width << "x" << height << std::endl;
    
    // Definición del cuerpo estático
    b2BodyDef bodyDef = createBodyDef(x, y, b2_staticBody);
    b2Body* body = m_world->CreateBody(&bodyDef);
    
    // Forma de la plataforma
    b2PolygonShape shape;
    float w = pixelsToMeters(width);
    float h = pixelsToMeters(height);
    shape.SetAsBox(w / 2.0f, h / 2.0f);
    
    // Propiedades físicas SIMPLES
    b2FixtureDef fixtureDef = createFixtureDef(
        &shape,
        0.0f,   // Sin densidad (estático)
        1.0f,   // FRICCIÓN MÁXIMA
        0.0f,   // Sin rebote
        CATEGORY_PLATFORM,
        CATEGORY_PLAYER | CATEGORY_ENEMY
    );
    
    body->CreateFixture(&fixtureDef);
    m_bodies.emplace(body, PhysicsBody(body, BodyType::PLATFORM, nullptr));
    
    std::cout << "✅ Plataforma creada correctamente" << std::endl;
    return body;
}

b2Body* CPhysics::createWall(float x, float y, float width, float height) {
    if (!m_world) return nullptr;
    
    std::cout << "🧱 Creando muro en (" << x << ", " << y << ") tamaño: " << width << "x" << height << std::endl;
    
    // CORREGIDO: Aplicar la misma lógica que las plataformas
    float centerX = x + width/2.0f;
    float centerY = y + height/2.0f;
    
    // Definición del cuerpo estático
    b2BodyDef bodyDef;
    bodyDef.type = b2_staticBody;
    bodyDef.position.Set(pixelsToMeters(centerX), pixelsToMeters(centerY));
    
    b2Body* body = m_world->CreateBody(&bodyDef);
    
    // Forma del muro
    b2PolygonShape shape;
    float w = pixelsToMeters(width);
    float h = pixelsToMeters(height);
    shape.SetAsBox(w / 2.0f, h / 2.0f);
    
    // Propiedades físicas del muro
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shape;
    fixtureDef.density = 0.0f;        // Sin densidad (estático)
    fixtureDef.friction = 0.9f;       // ← CORREGIDO: Fricción muy alta para muros
    fixtureDef.restitution = 0.0f;    // Sin rebote
    fixtureDef.filter.categoryBits = CATEGORY_WALL;
    fixtureDef.filter.maskBits = CATEGORY_PLAYER | CATEGORY_ENEMY;
    
    body->CreateFixture(&fixtureDef);
    
    // Almacenar información del cuerpo
    m_bodies.emplace(body, PhysicsBody(body, BodyType::WALL, nullptr));
    
    std::cout << "✅ Muro creado (estático) - Centro: (" << centerX << "," << centerY << ")" << std::endl;
    return body;
}

// GESTIÓN DE CUERPOS
void CPhysics::destroyBody(void* userData) {
    auto it = m_bodies.find(userData);
    if (it != m_bodies.end() && m_world) {
        m_world->DestroyBody(it->second.body);
        m_bodies.erase(it);
        std::cout << "🗑️ Cuerpo físico destruido" << std::endl;
    }
}

b2Body* CPhysics::getBody(void* userData) {
    auto it = m_bodies.find(userData);
    return (it != m_bodies.end()) ? it->second.body : nullptr;
}

PhysicsBody* CPhysics::getPhysicsBody(void* userData) {
    auto it = m_bodies.find(userData);
    return (it != m_bodies.end()) ? &it->second : nullptr;
}

// UTILIDADES DE CONVERSIÓN
sf::Vector2f CPhysics::b2VecToSFML(const b2Vec2& vec) {
    return sf::Vector2f(metersToPixels(vec.x), metersToPixels(vec.y));
}

b2Vec2 CPhysics::sfmlVecToB2(const sf::Vector2f& vec) {
    return b2Vec2(pixelsToMeters(vec.x), pixelsToMeters(vec.y));
}

float CPhysics::pixelsToMeters(float pixels) {
    return pixels / SCALE;
}

float CPhysics::metersToPixels(float meters) {
    return meters * SCALE;
}

sf::Vector2f CPhysics::pixelsToMeters(const sf::Vector2f& pixels) {
    return sf::Vector2f(pixels.x / SCALE, pixels.y / SCALE);
}

sf::Vector2f CPhysics::metersToPixels(const b2Vec2& meters) {
    return sf::Vector2f(meters.x * SCALE, meters.y * SCALE);
}

// CONTROL DE MOVIMIENTO
void CPhysics::setBodyVelocity(void* userData, float x, float y) {
    b2Body* body = getBody(userData);
    if (body) {
        body->SetLinearVelocity(b2Vec2(x, y));
    }
}

void CPhysics::applyForce(void* userData, float x, float y) {
    b2Body* body = getBody(userData);
    if (body) {
        body->ApplyForceToCenter(b2Vec2(x, y), true);
    }
}

void CPhysics::applyImpulse(void* userData, float x, float y) {
    b2Body* body = getBody(userData);
    if (body) {
        body->ApplyLinearImpulseToCenter(b2Vec2(x, y), true);
    }
}

// VERIFICACIONES
bool CPhysics::isBodyOnGround(void* userData) {
    b2Body* body = getBody(userData);
    if (!body) return false;
    
    // CORREGIDO: Verificación más precisa para estar en el suelo
    b2Vec2 velocity = body->GetLinearVelocity();
    
    // Está en el suelo si:
    // 1. La velocidad vertical es muy pequeña (cerca de 0)
    // 2. Y está cayendo o estático (no subiendo)
    return (std::abs(velocity.y) < 0.3f) && (velocity.y >= -0.1f);
}

bool CPhysics::canJump(void* userData) {
    return isBodyOnGround(userData);
}

// DEBUG
void CPhysics::debugPrint() const {
    std::cout << "=== DEBUG SISTEMA DE FÍSICAS CORREGIDO ===" << std::endl;
    std::cout << "Cuerpos totales: " << m_bodies.size() << std::endl;
    
    int players = 0, enemies = 0, platforms = 0, walls = 0;
    
    for (const auto& pair : m_bodies) {
        switch (pair.second.type) {
            case BodyType::PLAYER: players++; break;
            case BodyType::ENEMY: enemies++; break;
            case BodyType::PLATFORM: platforms++; break;
            case BodyType::WALL: walls++; break;
        }
    }
    
    std::cout << "  👤 Jugadores: " << players << std::endl;
    std::cout << "  👹 Enemigos: " << enemies << std::endl;
    std::cout << "  🟩 Plataformas: " << platforms << std::endl;
    std::cout << "  🧱 Muros: " << walls << std::endl;
    
    if (m_world) {
        b2Vec2 gravity = m_world->GetGravity();
        std::cout << "  🌍 Gravedad CORREGIDA: (" << gravity.x << ", " << gravity.y << ")" << std::endl;
        std::cout << "  📐 Escala CORREGIDA: " << SCALE << " píxeles = 1 metro" << std::endl;
        std::cout << "  ⚙️ Iteraciones: V=" << VELOCITY_ITERATIONS << ", P=" << POSITION_ITERATIONS << std::endl;
    }
    
    std::cout << "=======================================" << std::endl;
}

int CPhysics::getBodyCount() const {
    return static_cast<int>(m_bodies.size());
}

// MÉTODOS AUXILIARES PRIVADOS
b2BodyDef CPhysics::createBodyDef(float x, float y, b2BodyType type) {
    b2BodyDef bodyDef;
    bodyDef.type = type;
    bodyDef.position.Set(pixelsToMeters(x), pixelsToMeters(y));
    return bodyDef;
}

b2FixtureDef CPhysics::createFixtureDef(b2Shape* shape, float density, float friction, float restitution, uint16 category, uint16 mask) {
    b2FixtureDef fixtureDef;
    fixtureDef.shape = shape;
    fixtureDef.density = density;
    fixtureDef.friction = friction;
    fixtureDef.restitution = restitution;
    fixtureDef.filter.categoryBits = category;
    fixtureDef.filter.maskBits = mask;
    return fixtureDef;
}

void CPhysics::cleanup() {
    if (m_world) {
        // Box2D limpia automáticamente todos los cuerpos cuando se destruye el mundo
        m_bodies.clear();
        m_world.reset();
        std::cout << "Mundo físico CORREGIDO limpiado" << std::endl;
    }
}