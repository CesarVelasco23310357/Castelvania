#include "CPhysics.hpp"
#include <iostream>
#include <cmath>

// Constructor
CPhysics::CPhysics() {
    
    // Crear mundo con gravedad corregida
    b2Vec2 gravity(GRAVITY_X, GRAVITY_Y);
    world = std::make_unique<b2World>(gravity);
    
    // Configurar listener de contactos
    contactListener = std::make_unique<PhysicsContactListener>();
    world->SetContactListener(contactListener.get());
}

// Destructor
CPhysics::~CPhysics() {
    cleanup();
}

// GESTION DEL MUNDO FISICO
void CPhysics::update(float deltaTime) {
    if (!world) return;
    
    // Simular un paso del mundo fisico
    world->Step(deltaTime, VELOCITY_ITERATIONS, POSITION_ITERATIONS);
    
    // Actualizar informacion de contactos
    if (contactListener) {
        contactListener->updateGroundContacts();
    }
}

void CPhysics::setGravity(float x, float y) {
    if (world) {
        world->SetGravity(b2Vec2(x, y));
    }
}

// CREACION DE CUERPOS
b2Body* CPhysics::createPlayerBody(float x, float y, void* userData) {
    if (!world) return nullptr;
    
    // Definicion del cuerpo
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position.Set(pixelsToMeters(x), pixelsToMeters(y));
    bodyDef.fixedRotation = true; // Evitar rotacion
    
    b2Body* body = world->CreateBody(&bodyDef);
    
    // Forma del jugador (rectangulo)
    b2PolygonShape shape;
    float width = pixelsToMeters(32.0f);  
    float height = pixelsToMeters(32.0f); 
    shape.SetAsBox(width / 2.0f, height / 2.0f);
    
    // Propiedades fisicas del jugador
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shape;
    fixtureDef.density = 1.0f;
    fixtureDef.friction = 0.3f;    
    fixtureDef.restitution = 0.0f; 
    fixtureDef.filter.categoryBits = CATEGORY_PLAYER;
    fixtureDef.filter.maskBits = CATEGORY_PLATFORM | CATEGORY_WALL | CATEGORY_ENEMY;
    fixtureDef.userData.pointer = reinterpret_cast<uintptr_t>(userData);
    
    body->CreateFixture(&fixtureDef);
    
    // Almacenar informacion del cuerpo
    bodies.emplace(userData, PhysicsBody(body, BodyType::PLAYER, userData));
    
    return body;
}

b2Body* CPhysics::createEnemyBody(float x, float y, void* userData) {
    if (!world) return nullptr;
    
    // Definicion del cuerpo
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position.Set(pixelsToMeters(x), pixelsToMeters(y));
    bodyDef.fixedRotation = true;
    
    b2Body* body = world->CreateBody(&bodyDef);
    
    // Forma del enemigo
    b2PolygonShape shape;
    float width = pixelsToMeters(28.0f);  
    float height = pixelsToMeters(28.0f); 
    shape.SetAsBox(width / 2.0f, height / 2.0f);
    
    // Propiedades fisicas del enemigo
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shape;
    fixtureDef.density = 0.8f;
    fixtureDef.friction = 0.4f;
    fixtureDef.restitution = 0.0f;
    fixtureDef.filter.categoryBits = CATEGORY_ENEMY;
    fixtureDef.filter.maskBits = CATEGORY_PLATFORM | CATEGORY_WALL | CATEGORY_PLAYER;
    fixtureDef.userData.pointer = reinterpret_cast<uintptr_t>(userData);
    
    body->CreateFixture(&fixtureDef);
    
    // Almacenar informacion del cuerpo
    bodies.emplace(userData, PhysicsBody(body, BodyType::ENEMY, userData));
    
    return body;
}

// Metodo createPlatform corregido
b2Body* CPhysics::createPlatform(float x, float y, float width, float height) {
    if (!world) return nullptr;

    // PASO 1: CALCULAR CENTRO EXACTO DE LA PLATAFORMA VISUAL
    float centerX = x + (width / 2.0f);
    float centerY = y + (height / 2.0f);
    
    // PASO 2: CONVERTIR A COORDENADAS DE BOX2D (METROS)
    float centerX_meters = pixelsToMeters(centerX);
    float centerY_meters = pixelsToMeters(centerY);
    float width_meters = pixelsToMeters(width);
    float height_meters = pixelsToMeters(height);
    
    // PASO 3: CREAR CUERPO FISICO EN LA POSICION EXACTA
    b2BodyDef bodyDef;
    bodyDef.type = b2_staticBody;
    bodyDef.position.Set(centerX_meters, centerY_meters);
    
    b2Body* body = world->CreateBody(&bodyDef);
    
    // PASO 4: CREAR FORMA CON TAMANO EXACTO
    b2PolygonShape shape;
    shape.SetAsBox(width_meters / 2.0f, height_meters / 2.0f);
    
    // PASO 5: PROPIEDADES FISICAS OPTIMIZADAS
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shape;
    fixtureDef.density = 0.0f;        
    fixtureDef.friction = 0.7f;       
    fixtureDef.restitution = 0.0f;   
    fixtureDef.filter.categoryBits = CATEGORY_PLATFORM;
    fixtureDef.filter.maskBits = CATEGORY_PLAYER | CATEGORY_ENEMY;
    
    body->CreateFixture(&fixtureDef);

    // Almacenar informacion del cuerpo
    bodies.emplace(body, PhysicsBody(body, BodyType::PLATFORM, nullptr));
    
    return body;
}

b2Body* CPhysics::createWall(float x, float y, float width, float height) {
    if (!world) return nullptr;
        
    float centerX = x + width/2.0f;
    float centerY = y + height/2.0f;
    
    // Definicion del cuerpo estatico
    b2BodyDef bodyDef;
    bodyDef.type = b2_staticBody;
    bodyDef.position.Set(pixelsToMeters(centerX), pixelsToMeters(centerY));
    
    b2Body* body = world->CreateBody(&bodyDef);
    
    // Forma del muro
    b2PolygonShape shape;
    float w = pixelsToMeters(width);
    float h = pixelsToMeters(height);
    shape.SetAsBox(w / 2.0f, h / 2.0f);
    
    // Propiedades fisicas del muro
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &shape;
    fixtureDef.density = 0.0f;        
    fixtureDef.friction = 0.9f;       
    fixtureDef.restitution = 0.0f;    
    fixtureDef.filter.categoryBits = CATEGORY_WALL;
    fixtureDef.filter.maskBits = CATEGORY_PLAYER | CATEGORY_ENEMY;
    
    body->CreateFixture(&fixtureDef);
    
    // Almacenar informacion del cuerpo
    bodies.emplace(body, PhysicsBody(body, BodyType::WALL, nullptr));
    
    return body;
}

// GESTION DE CUERPOS
void CPhysics::destroyBody(void* userData) {
    auto it = bodies.find(userData);
    if (it != bodies.end() && world) {
        world->DestroyBody(it->second.body);
        bodies.erase(it);
    }
}

b2Body* CPhysics::getBody(void* userData) {
    auto it = bodies.find(userData);
    return (it != bodies.end()) ? it->second.body : nullptr;
}

void CPhysics::destroyBody(b2Body* body) {
    if (!body || !world) return;
    
    // Buscar en el mapa y eliminar
    for (auto it = bodies.begin(); it != bodies.end(); ++it) {
        if (it->second.body == body) {
            bodies.erase(it);
            break;
        }
    }
    
    // Destruir el cuerpo
    world->DestroyBody(body);
}

PhysicsBody* CPhysics::getPhysicsBody(void* userData) {
    auto it = bodies.find(userData);
    return (it != bodies.end()) ? &it->second : nullptr;
}

// UTILIDADES DE CONVERSION
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

// Verificacion mejorada de estar en el suelo
bool CPhysics::isBodyOnGround(void* userData) {
    if (!contactListener) return false;
    
    // Usar el sistema de contactos para verificar si esta tocando el suelo
    return contactListener->isPlayerOnGround(userData);
}

bool CPhysics::canJump(void* userData) {
    return isBodyOnGround(userData);
}

// Sistema de contactos
PhysicsContactListener* CPhysics::getContactListener() const {
    return contactListener.get();
}

// DEBUG
void CPhysics::debugPrint() const {
    std::cout << "Cuerpos totales: " << bodies.size() << std::endl;
    
    int players = 0, enemies = 0, platforms = 0, walls = 0;
    
    for (const auto& pair : bodies) {
        switch (pair.second.type) {
            case BodyType::PLAYER: players++; break;
            case BodyType::ENEMY: enemies++; break;
            case BodyType::PLATFORM: platforms++; break;
            case BodyType::WALL: walls++; break;
        }
    }
    
    std::cout << "  Jugadores: " << players << std::endl;
    std::cout << "  Enemigos: " << enemies << std::endl;
    std::cout << "  Plataformas: " << platforms << std::endl;
    std::cout << "  Muros: " << walls << std::endl;
    
    if (world) {
        b2Vec2 gravity = world->GetGravity();
        std::cout << "  Gravedad: (" << gravity.x << ", " << gravity.y << ")" << std::endl;
    }
}

int CPhysics::getBodyCount() const {
    return static_cast<int>(bodies.size());
}

// METODOS AUXILIARES PRIVADOS
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
    if (world) {
        // Box2D limpia automaticamente todos los cuerpos cuando se destruye el mundo
        bodies.clear();
        world.reset();
        contactListener.reset();
    }
}

// Implementacion del ContactListener
void PhysicsContactListener::BeginContact(b2Contact* contact) {
    b2Fixture* fixtureA = contact->GetFixtureA();
    b2Fixture* fixtureB = contact->GetFixtureB();
    
    void* userDataA = reinterpret_cast<void*>(fixtureA->GetUserData().pointer);
    void* userDataB = reinterpret_cast<void*>(fixtureB->GetUserData().pointer);
    
    // Verificar si es un contacto jugador-plataforma
    if (isPlayerPlatformContact(fixtureA, fixtureB)) {
        void* playerData = (fixtureA->GetFilterData().categoryBits & CATEGORY_PLAYER) ? userDataA : userDataB;
        if (playerData) {
            groundContacts[playerData]++;
        }
    }
}

void PhysicsContactListener::EndContact(b2Contact* contact) {
    b2Fixture* fixtureA = contact->GetFixtureA();
    b2Fixture* fixtureB = contact->GetFixtureB();
    
    void* userDataA = reinterpret_cast<void*>(fixtureA->GetUserData().pointer);
    void* userDataB = reinterpret_cast<void*>(fixtureB->GetUserData().pointer);
    
    // Verificar si es un contacto jugador-plataforma
    if (isPlayerPlatformContact(fixtureA, fixtureB)) {
        void* playerData = (fixtureA->GetFilterData().categoryBits & CATEGORY_PLAYER) ? userDataA : userDataB;
        if (playerData && groundContacts[playerData] > 0) {
            groundContacts[playerData]--;
        }
    }
}

bool PhysicsContactListener::isPlayerPlatformContact(b2Fixture* fixtureA, b2Fixture* fixtureB) {
    uint16 categoryA = fixtureA->GetFilterData().categoryBits;
    uint16 categoryB = fixtureB->GetFilterData().categoryBits;
    
    return ((categoryA & CATEGORY_PLAYER) && (categoryB & (CATEGORY_PLATFORM | CATEGORY_WALL))) ||
           ((categoryB & CATEGORY_PLAYER) && (categoryA & (CATEGORY_PLATFORM | CATEGORY_WALL)));
}

bool PhysicsContactListener::isPlayerOnGround(void* playerData) {
    auto it = groundContacts.find(playerData);
    return (it != groundContacts.end()) && (it->second > 0);
}

void PhysicsContactListener::updateGroundContacts() {
    // Limpiar contactos obsoletos
    for (auto it = groundContacts.begin(); it != groundContacts.end();) {
        if (it->second <= 0) {
            it = groundContacts.erase(it);
        } else {
            ++it;
        }
    }
}

void CPhysics::destroyAllPlatforms() {    
    std::vector<b2Body*> platformsToDestroy;
    
    // Recopilar todas las plataformas
    for (auto& pair : bodies) {
        if (pair.second.type == BodyType::PLATFORM) {
            platformsToDestroy.push_back(pair.second.body);
        }
    }
    
    // Destruir cada plataforma
    for (b2Body* platform : platformsToDestroy) {
        destroyBody(platform);
    }
}