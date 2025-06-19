# 🏰 CASTELVANIA
*Un juego de plataformas 2D inspirado en el clásico Castlevania*

![Pantalla de Título](assets/title_screen.png)

## 📖 Descripción

Castelvania es un juego de plataformas desarrollado en C++ que combina acción, aventura y combate en un castillo lleno de criaturas sobrenaturales. El jugador debe atravesar tres niveles cada vez más desafiantes, enfrentándose a murciélagos, esqueletos y zombies con un sistema de física realista y una banda sonora inmersiva.

## 🎮 Screenshots del Juego

### Menú Principal
![Menú Principal](assets/menu.png)

### Primer Nivel
![Primer Nivel](assets/1erNivel.png)

### Segundo Nivel
![Segundo Nivel](assets/2doNivel.png)

### Tercer Nivel
![Tercer Nivel](assets/3erNivel.png)

## ✨ Características Principales

### 🎯 **Sistema de Combate Dinámico**
- Combate cuerpo a cuerpo con animaciones fluidas
- Sistema de salud para jugador y enemigos
- Efectos visuales de daño y estados hurt

### 🎵 **Sistema de Audio Inmersivo**
- **Música adaptativa**: Diferentes temas para menú y gameplay
- **Transiciones suaves**: Fade in/out entre pistas musicales
- **Control de volumen**: Ajuste dinámico de volumen maestro y de música
- **Sistema de silencio**: Toggle rápido para silenciar/desilenciar
- **Audio debug**: Información en tiempo real del estado del audio

### 🤖 **IA para enemigos**
- **Murciélagos**: IA de vuelo que persigue al jugador en 3D
- **Esqueletos**: Saltan obstáculos y persiguen agresivamente  
- **Zombies**: Movimiento persistente y patrullaje inteligente
- Los enemigos detectan al jugador dentro de un rango y adaptan su comportamiento

### ⚙️ **Físicas Realistas**
- Motor de físicas **Box2D** integrado
- Gravedad, colisiones y momentum realistas
- Plataformas físicas con diferentes materiales
- Detección precisa de suelo para saltos

### 🎨 **Sistema de Animación**
- Sprites animados para todos los personajes
- Estados de animación: idle, correr, atacar, saltar, daño
- Transiciones fluidas entre animaciones
- Diferentes velocidades de animación por personaje

## 🎮 Controles

### Movimiento y Combate
| Tecla | Acción |
|-------|--------|
| **A** | Mover izquierda |
| **D** | Mover derecha |
| **W / Espacio** | Saltar |
| **Enter** | Atacar |
| **ESC** | Pausar/Menú |
| **R** | Reiniciar nivel |

### Controles de Audio 🎵
| Tecla | Acción |
|-------|--------|
| **M** | Silenciar/Desilenciar audio |
| **+** (Igual) | Subir volumen (+10%) |
| **-** (Guión) | Bajar volumen (-10%) |
| **F9** | Mostrar información de debug de música |

## 🏗️ Compilación

### Requisitos
- **C++17** o superior
- **SFML 2.5+** - Para gráficos y audio
- **Box2D** - Para físicas
- **CMake** (opcional)

### Archivos de Audio Necesarios
```
assets/
├── MenuFondo.ogg      # Música del menú principal
└── GameplaySound.ogg  # Música durante el gameplay
```

## 🚀 Ejecución

```bash
make run
```

**Nota**: Asegúrate de que la carpeta `assets/` con todos los recursos (gráficos y audio) esté en el mismo directorio que el ejecutable.

## 🎯 Mecánicas del Juego

### 💪 Sistema de Progresión
- **3 Niveles** con dificultad creciente
- Enemigos con diferentes comportamientos y estadísticas
- Sistema de puntuación basado en tiempo y eliminaciones

### 🏰 Niveles

| Nivel | Enemigos | Dificultad | Características |
|-------|----------|------------|-----------------|
| **1** | 3 enemigos básicos | ⭐⭐ | Plataformas introductorias |
| **2** | 5 enemigos variados | ⭐⭐⭐ | Plataformas complejas |
| **3** | 9 enemigos  | ⭐⭐⭐⭐⭐ | Laberinto de plataformas |

### 👾 Enemigos

#### 🦇 Murciélago
- **Salud**: 30 HP
- **Daño**: 10
- **Habilidad**: Vuelo libre, persigue en cualquier dirección
- **IA**: Detecta al jugador a 200px, vuela directamente hacia él

#### 💀 Esqueleto  
- **Salud**: 60 HP
- **Daño**: 20
- **Habilidad**: Salto para superar obstáculos
- **IA**: Persigue al jugador, salta cuando detecta desniveles

#### 🧟 Zombie
- **Salud**: 100 HP  
- **Daño**: 30
- **Habilidad**: Movimiento persistente, alta resistencia
- **IA**: Patrullaje constante, persigue tenazmente

## 🏗️ Arquitectura del Código

### 📁 Estructura de Archivos
```
CASTELVANIA/
├── src/
│   ├── Castelvania.cpp    # Punto de entrada principal
│   ├── CGame.*           # Lógica principal del juego
│   ├── CPlayer.*         # Clase del jugador
│   ├── CEnemy.*          # Sistema de enemigos con IA
│   ├── CLevel.*          # Gestión de niveles
│   ├── CPhysics.*        # Motor de físicas Box2D
│   └── CMusica.*         # Sistema de audio y música
├── assets/               # Recursos gráficos y audio
│   ├── Character.png     # Sprites del jugador
│   ├── murcielago.png    # Sprites de murciélago
│   ├── skeleton.png      # Sprites de esqueleto
│   ├── zombie.png        # Sprites de zombie
│   ├── floor.png         # Textura de plataformas
│   ├── layer_1.png       # Fondo lejano
│   ├── layer_2.png       # Fondo cercano
│   ├── title_screen.png  # Pantalla de título
│   ├── MenuFondo.ogg     # Música del menú
│   └── GameplaySound.ogg # Música del gameplay
└── README.md
```

### 🔧 Clases Principales

#### `CGame`
- **Responsabilidad**: Loop principal, estados del juego, gestión de niveles
- **Características**: Manejo de eventos, renderizado, física global, integración de audio
- **Estados**: Menu, Playing, Paused, Game Over, Victory

#### `CPlayer`  
- **Responsabilidad**: Lógica del jugador, animaciones, físicas
- **Características**: 6 estados de animación, integración con Box2D
- **Sistema**: Salud (100 HP), velocidad configurable, salto físico

#### `CEnemy`
- **Responsabilidad**: IA de enemigos, comportamientos únicos
- **Características**: 3 tipos diferentes, detección de jugador, física individual
- **IA**: Algoritmos específicos por tipo de enemigo

#### `CLevel`
- **Responsabilidad**: Gestión de niveles, spawn de enemigos, plataformas
- **Características**: Plataformas físicas, fondos multicapa, spawn points

#### `CPhysics`
- **Responsabilidad**: Integración con Box2D, colisiones, físicas realistas
- **Características**: Gravedad configurable, categorías de colisión, contactos

#### `CMusica` 🎵
- **Responsabilidad**: Gestión completa del sistema de audio
- **Características**: 
  - Reproducción de música de fondo
  - Transiciones suaves entre pistas
  - Control de volumen maestro y de música
  - Sistema de fade in/out
  - Estados de audio: Playing, Stopped, Paused, Transitioning
  - Debug de información de audio en tiempo real
- **Formatos soportados**: OGG Vorbis (recomendado), WAV
- **Integración**: Cambios automáticos de música según el estado del juego

## 🎨 Sistema de Sprites

### Jugador (Character.png)
```
Estados disponibles:
- IDLE: Frame estático de reposo
- RUN: 6 frames de animación de carrera  
- ATTACK: 4 frames de animación de ataque
- HURT: Frame de daño recibido
- JUMP/FALL: Frames de salto y caída
```

### Enemigos
- **murcielago.png**: 5 frames de vuelo
- **skeleton.png**: Idle + 5 frames de movimiento  
- **zombie.png**: Idle + 4 frames de movimiento

## 🎵 Sistema de Audio

### Arquitectura de Audio
- **Motor**: SFML Audio
- **Formatos**: OGG Vorbis, WAV
- **Canales**: Música de fondo (estéreo)
- **Streaming**: Carga bajo demanda para archivos grandes

### Características del Audio
- **Música adaptativa**: Cambia automáticamente según el contexto del juego
- **Volumen configurable**: Control independiente de volumen maestro y música
- **Transiciones suaves**: Fade in/out entre diferentes pistas
- **Memoria eficiente**: Streaming de archivos de audio grandes
- **Sistema de bucle**: Reproducción continua de música de fondo

### Estados de Música
| Estado | Descripción |
|--------|-------------|
| **Menu** | Música ambiental para el menú principal |
| **Gameplay** | Música de acción durante los niveles |
| **Transition** | Transiciones suaves entre estados |
| **Paused** | Audio pausado durante pausa del juego |

## ⚡ Optimizaciones Implementadas

### 🔧 Rendimiento
- **Pooling de objetos**: Reutilización de enemigos
- **Frustum culling**: Solo renderiza objetos visibles
- **Física optimizada**: 60 FPS estables con múltiples objetos
- **Audio streaming**: Carga eficiente de archivos de audio grandes

### 🧠 IA
- **Detección por distancia**: Los enemigos solo calculan IA cuando el jugador está cerca
- **Estados de comportamiento**: Cada enemigo tiene estados idle/chase/attack
- **Pathfinding básico**: Los enemigos evitan obstáculos simples

### 💾 Gestión de Memoria
- **Smart pointers**: Gestión automática de memoria
- **RAII**: Recursos liberados automáticamente
- **Minimal copying**: Referencias y movimientos para eficiencia
- **Audio streaming**: Los archivos de audio se cargan bajo demanda

### Problemas de Audio Comunes

#### "No se escucha música"
1. Verificar que existan los archivos:
   - `assets/MenuFondo.ogg`
   - `assets/GameplaySound.ogg`
2. Presionar **F9** para ver el estado del audio
3. Usar **M** para verificar que no esté silenciado
4. Usar **+** para subir el volumen


## 📄 Licencia

Este proyecto está bajo la Licencia MIT 

## 🙏 Reconocimientos

- **SFML** - Framework de multimedia y audio
- **Box2D** - Motor de físicas 2D  
- **Castlevania** - Inspiración original de Konami
- Sprites y assets creados para el proyecto
- Música y efectos de sonido bajo licencias Creative Commons

---

**¡Disfruta explorando el castillo y enfrentando a las criaturas de la noche con una experiencia audiovisual completa!** 🌙🏰🎵