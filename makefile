# Makefile para Castelvania
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -g

# Librerías
LIBS = -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lbox2d

# Directorios
SRC_DIR = src
BUILD_DIR = build
TARGET = castelvania

# Archivos fuente
SOURCES = Castelvania.cpp CGame.cpp CPlayer.cpp CEnemy.cpp CLevel.cpp CPhysics.cpp CMusica.cpp
OBJ_FILES = $(addprefix $(BUILD_DIR)/,$(SOURCES:.cpp=.o))

# Regla por defecto
all: $(TARGET)

# Crear directorio build
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Crear ejecutable
$(TARGET): $(BUILD_DIR) $(OBJ_FILES)
	$(CXX) $(OBJ_FILES) -o $(TARGET) $(LIBS)

# Compilar archivos .cpp a .o
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Limpiar
clean:
	@rm -rf $(BUILD_DIR)
	@rm -f $(TARGET)

# Ejecutar
run: $(TARGET)
	./$(TARGET)

# Debug
debug: CXXFLAGS += -DDEBUG -g3 -O0
debug: clean $(TARGET)

# Dependencias
$(BUILD_DIR)/Castelvania.o: $(SRC_DIR)/Castelvania.cpp $(SRC_DIR)/CGame.hpp
$(BUILD_DIR)/CGame.o: $(SRC_DIR)/CGame.cpp $(SRC_DIR)/CGame.hpp $(SRC_DIR)/CPlayer.hpp $(SRC_DIR)/CLevel.hpp $(SRC_DIR)/CEnemy.hpp $(SRC_DIR)/CPhysics.hpp $(SRC_DIR)/CMusica.hpp
$(BUILD_DIR)/CPlayer.o: $(SRC_DIR)/CPlayer.cpp $(SRC_DIR)/CPlayer.hpp $(SRC_DIR)/CPhysics.hpp
$(BUILD_DIR)/CEnemy.o: $(SRC_DIR)/CEnemy.cpp $(SRC_DIR)/CEnemy.hpp $(SRC_DIR)/CPhysics.hpp
$(BUILD_DIR)/CLevel.o: $(SRC_DIR)/CLevel.cpp $(SRC_DIR)/CLevel.hpp $(SRC_DIR)/CEnemy.hpp $(SRC_DIR)/CPhysics.hpp
$(BUILD_DIR)/CPhysics.o: $(SRC_DIR)/CPhysics.cpp $(SRC_DIR)/CPhysics.hpp
$(BUILD_DIR)/CMusica.o: $(SRC_DIR)/CMusica.cpp $(SRC_DIR)/CMusica.hpp

.PHONY: all clean run debug