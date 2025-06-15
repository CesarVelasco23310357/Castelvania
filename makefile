# Makefile para Castelvania
# Estructura: Makefile en raíz, código fuente en src/

# Compilador y flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -g
SFML_FLAGS = -lsfml-graphics -lsfml-window -lsfml-system

# Directorios
SRC_DIR = src
BUILD_DIR = build
TARGET_DIR = .

# Nombre del ejecutable
TARGET = castelvania

# Archivos fuente (solo los .cpp) - NOTA: Castelvania.cpp es el main
SOURCES = Castelvania.cpp CGame.cpp CPlayer.cpp CEnemy.cpp CLevel.cpp

# Crear rutas completas para archivos fuente y objeto
SRC_FILES = $(addprefix $(SRC_DIR)/,$(SOURCES))
OBJ_FILES = $(addprefix $(BUILD_DIR)/,$(SOURCES:.cpp=.o))

# Regla por defecto
all: $(TARGET)

# Crear directorio build si no existe
$(BUILD_DIR):
	@echo "Creando directorio build..."
	@mkdir -p $(BUILD_DIR)

# Regla para crear el ejecutable
$(TARGET): $(BUILD_DIR) $(OBJ_FILES)
	@echo "Enlazando ejecutable..."
	$(CXX) $(OBJ_FILES) -o $(TARGET) $(SFML_FLAGS)
	@echo "✓ Compilación exitosa: $(TARGET)"

# Regla para compilar archivos .cpp a .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "Compilando $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Limpiar archivos compilados
clean:
	@echo "Limpiando archivos compilados..."
	@rm -rf $(BUILD_DIR)
	@rm -f $(TARGET)
	@echo "✓ Limpieza completada"

# Ejecutar el juego
run: $(TARGET)
	@echo "Ejecutando $(TARGET)..."
	@./$(TARGET)

# Verificar que los archivos existen
check-files:
	@echo "Verificando archivos fuente..."
	@for file in $(SOURCES); do \
		if [ -f "$(SRC_DIR)/$$file" ]; then \
			echo "✓ $(SRC_DIR)/$$file encontrado"; \
		else \
			echo "✗ $(SRC_DIR)/$$file NO encontrado"; \
		fi; \
	done

# Compilar en modo debug (con más información de depuración)
debug: CXXFLAGS += -DDEBUG -g3 -O0
debug: $(TARGET)
	@echo "✓ Compilación en modo debug completada"

# Compilar en modo release (optimizado)
release: CXXFLAGS += -DNDEBUG -O3
release: clean $(TARGET)
	@echo "✓ Compilación en modo release completada"

# Verificar dependencias de SFML
check-sfml:
	@echo "Verificando instalación de SFML..."
	@pkg-config --exists sfml-all && echo "✓ SFML encontrado" || echo "✗ SFML no encontrado"

# Instalar dependencias (Ubuntu/Debian)
install-deps:
	@echo "Instalando dependencias de SFML..."
	sudo apt-get update
	sudo apt-get install libsfml-dev

# Información del proyecto
info:
	@echo "=== Información del Proyecto ==="
	@echo "Nombre: Castelvania"
	@echo "Compilador: $(CXX)"
	@echo "Flags: $(CXXFLAGS)"
	@echo "SFML Flags: $(SFML_FLAGS)"
	@echo "Directorio fuente: $(SRC_DIR)/"
	@echo "Directorio build: $(BUILD_DIR)/"
	@echo "Ejecutable: $(TARGET)"
	@echo "Archivos fuente:"
	@for file in $(SOURCES); do echo "  - $(SRC_DIR)/$$file"; done
	@echo "=============================="

# Ayuda
help:
	@echo "=== Comandos disponibles ==="
	@echo "make         - Compilar el proyecto"
	@echo "make run     - Compilar y ejecutar"
	@echo "make clean   - Limpiar archivos compilados"
	@echo "make debug   - Compilar en modo debug"
	@echo "make release - Compilar optimizado"
	@echo "make check-files - Verificar archivos fuente"
	@echo "make check-sfml - Verificar SFML"
	@echo "make install-deps - Instalar dependencias"
	@echo "make info    - Mostrar información del proyecto"
	@echo "make help    - Mostrar esta ayuda"
	@echo "============================"

# Reglas que no son archivos
.PHONY: all clean run debug release check-sfml install-deps info help check-files

# Dependencias específicas (para recompilación automática)
$(BUILD_DIR)/Castelvania.o: $(SRC_DIR)/Castelvania.cpp $(SRC_DIR)/CGame.hpp
$(BUILD_DIR)/CGame.o: $(SRC_DIR)/CGame.cpp $(SRC_DIR)/CGame.hpp $(SRC_DIR)/CPlayer.hpp $(SRC_DIR)/CLevel.hpp $(SRC_DIR)/CEnemy.hpp
$(BUILD_DIR)/CPlayer.o: $(SRC_DIR)/CPlayer.cpp $(SRC_DIR)/CPlayer.hpp
$(BUILD_DIR)/CEnemy.o: $(SRC_DIR)/CEnemy.cpp $(SRC_DIR)/CEnemy.hpp
$(BUILD_DIR)/CLevel.o: $(SRC_DIR)/CLevel.cpp $(SRC_DIR)/CLevel.hpp $(SRC_DIR)/CEnemy.hpp