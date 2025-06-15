#include <iostream>
#include <exception>
#include "CGame.hpp"

int main() {
    try {
        std::cout << "=== Inicializando Castelvania ===" << std::endl;
        
        // Crear el objeto del juego
        CGame game;
        
        // Ejecutar el juego principal
        game.run();
        
        std::cout << "=== Castelvania cerrado ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error crítico: " << e.what() << std::endl;
        std::cerr << "El juego se cerrará." << std::endl;
        return -1;
        
    } catch (...) {
        std::cerr << "Error desconocido ocurrió." << std::endl;
        std::cerr << "El juego se cerrará." << std::endl;
        return -1;
    }
    
    return 0;
}