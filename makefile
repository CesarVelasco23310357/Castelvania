# Makefile para Castelvania con Box2D
# Estructura: Makefile en raíz, código fuente en src/

# Compilador y flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -g

# ===============================================
# NUEVO: Dependencias de Box2D y SFML
# ===============================================
SFML_FLAGS = -lsfml-graphics -lsfml-window -lsfml-system
BOX2D_FLAGS = -lbox2d

# Combinar todas las librerías
LIBS = $(SFML_FLAGS) $(BOX2D_FLAGS)

# Directorios
SRC_DIR = src
BUILD_DIR = build
TARGET_DIR = .

# Nombre del ejecutable
TARGET = castelvania

# ===============================================
# NUEVO: Archivos fuente (agregando CPhysics)
# ===============================================
# Archivos fuente (solo los .cpp) - NOTA: Castelvania.cpp es el main
SOURCES = Castelvania.cpp CGame.cpp CPlayer.cpp CEnemy.cpp CLevel.cpp CPhysics.cpp

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
	@echo "Enlazando ejecutable con SFML y Box2D..."
	$(CXX) $(OBJ_FILES) -o $(TARGET) $(LIBS)
	@echo "✓ Compilación exitosa: $(TARGET)"
	@echo "🎮 Librerías enlazadas: SFML + Box2D"

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

# ===============================================
# NUEVO: Verificaciones de dependencias
# ===============================================
# Verificar dependencias de SFML y Box2D
check-deps:
	@echo "=== Verificando dependencias ==="
	@echo "Verificando SFML..."
	@pkg-config --exists sfml-all && echo "✓ SFML encontrado" || echo "✗ SFML no encontrado"
	@echo "Verificando Box2D..."
	@ldconfig -p | grep -q box2d && echo "✓ Box2D encontrado" || echo "✗ Box2D no encontrado"
	@echo "==============================="

# Verificar solo SFML
check-sfml:
	@echo "Verificando instalación de SFML..."
	@pkg-config --exists sfml-all && echo "✓ SFML encontrado" || echo "✗ SFML no encontrado"

# Verificar solo Box2D
check-box2d:
	@echo "Verificando instalación de Box2D..."
	@ldconfig -p | grep -q box2d && echo "✓ Box2D encontrado" || echo "✗ Box2D no encontrado"

# ===============================================
# NUEVO: Instalación de dependencias mejorada
# ===============================================
# Instalar dependencias (Ubuntu/Debian)
install-deps:
	@echo "=== Instalando dependencias ==="
	@echo "Actualizando repositorios..."
	sudo apt-get update
	@echo "Instalando SFML..."
	sudo apt-get install -y libsfml-dev
	@echo "Instalando Box2D..."
	sudo apt-get install -y libbox2d-dev
	@echo "✓ Todas las dependencias instaladas"

# Instalar solo SFML
install-sfml:
	@echo "Instalando SFML..."
	sudo apt-get update
	sudo apt-get install -y libsfml-dev

# Instalar solo Box2D
install-box2d:
	@echo "Instalando Box2D..."
	sudo apt-get update
	sudo apt-get install -y libbox2d-dev

# Compilar con información de dependencias
compile-info: check-deps $(TARGET)

# Información del proyecto
info:
	@echo "=== Información del Proyecto ==="
	@echo "Nombre: Castelvania"
	@echo "Compilador: $(CXX)"
	@echo "Flags: $(CXXFLAGS)"
	@echo "Librerías: $(LIBS)"
	@echo "Directorio fuente: $(SRC_DIR)/"
	@echo "Directorio build: $(BUILD_DIR)/"
	@echo "Ejecutable: $(TARGET)"
	@echo "Archivos fuente:"
	@for file in $(SOURCES); do echo "  - $(SRC_DIR)/$$file"; done
	@echo "=============================="

# Ayuda actualizada
help:
	@echo "=== Comandos disponibles ==="
	@echo "== Compilación =="
	@echo "make         - Compilar el proyecto"
	@echo "make run     - Compilar y ejecutar"
	@echo "make clean   - Limpiar archivos compilados"
	@echo "make debug   - Compilar en modo debug"
	@echo "make release - Compilar optimizado"
	@echo ""
	@echo "== Verificaciones =="
	@echo "make check-files - Verificar archivos fuente"
	@echo "make check-deps  - Verificar SFML y Box2D"
	@echo "make check-sfml  - Verificar solo SFML"
	@echo "make check-box2d - Verificar solo Box2D"
	@echo ""
	@echo "== Instalación =="
	@echo "make install-deps  - Instalar SFML + Box2D"
	@echo "make install-sfml  - Instalar solo SFML"
	@echo "make install-box2d - Instalar solo Box2D"
	@echo ""
	@echo "== Información =="
	@echo "make info    - Mostrar información del proyecto"
	@echo "make help    - Mostrar esta ayuda"
	@echo "============================"

# Reglas que no son archivos
.PHONY: all clean run debug release check-sfml check-box2d check-deps install-deps install-sfml install-box2d info help check-files compile-info

# ===============================================
# NUEVO: Dependencias específicas actualizadas
# ===============================================
$(BUILD_DIR)/Castelvania.o: $(SRC_DIR)/Castelvania.cpp $(SRC_DIR)/CGame.hpp
$(BUILD_DIR)/CGame.o: $(SRC_DIR)/CGame.cpp $(SRC_DIR)/CGame.hpp $(SRC_DIR)/CPlayer.hpp $(SRC_DIR)/CLevel.hpp $(SRC_DIR)/CEnemy.hpp $(SRC_DIR)/CPhysics.hpp
$(BUILD_DIR)/CPlayer.o: $(SRC_DIR)/CPlayer.cpp $(SRC_DIR)/CPlayer.hpp $(SRC_DIR)/CPhysics.hpp
$(BUILD_DIR)/CEnemy.o: $(SRC_DIR)/CEnemy.cpp $(SRC_DIR)/CEnemy.hpp $(SRC_DIR)/CPhysics.hpp
$(BUILD_DIR)/CLevel.o: $(SRC_DIR)/CLevel.cpp $(SRC_DIR)/CLevel.hpp $(SRC_DIR)/CEnemy.hpp $(SRC_DIR)/CPhysics.hpp
$(BUILD_DIR)/CPhysics.o: $(SRC_DIR)/CPhysics.cpp $(SRC_DIR)/CPhysics.hpp