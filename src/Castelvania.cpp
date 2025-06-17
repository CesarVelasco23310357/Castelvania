#include <iostream>
#include <exception>
#include "CGame.hpp"

int main() {
    try {
        std::cout << "=== Inicializando Castelvania ===" << std::endl;
        std::cout << "🎮 Versión con físicas Box2D" << std::endl;
        std::cout << "⚡ Controles: A/D = Mover, W/Espacio = Saltar, Enter = Atacar" << std::endl;
        std::cout << "=================================" << std::endl;
        
        // Crear el objeto del juego
        CGame game;
        
        // Ejecutar el juego principal
        game.run();
        
        std::cout << "=== Castelvania cerrado ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error critico: " << e.what() << std::endl;
        std::cerr << "El juego se cerrara." << std::endl;
        return -1;
        
    } catch (...) {
        std::cerr << "Error desconocido ocurrió." << std::endl;
        std::cerr << "El juego se cerrará." << std::endl;
        return -1;
    }
    
    return 0;
}