#include "CMusica.hpp"
#include <iostream>
#include <algorithm>

// Constructor
CMusica::CMusica() 
    : m_currentMusicType(MusicType::NONE), m_audioState(AudioState::STOPPED),
      m_currentMusic(nullptr), m_masterVolume(DEFAULT_MASTER_VOLUME), 
      m_musicVolume(DEFAULT_MUSIC_VOLUME), m_isMuted(false),
      m_fadeEnabled(true), m_fadeTimer(0.0f), m_fadeDuration(DEFAULT_FADE_DURATION),
      m_fadeStartVolume(0.0f), m_fadeTargetVolume(0.0f), m_transitionTarget(MusicType::NONE),
      m_menuMusicFile("assets/MenuFondo.ogg"), m_gameplayMusicFile("assets/GameplaySound.ogg"),
      m_musicLoaded(false) {
    
    std::cout << "CMusica: Sistema de audio inicializado." << std::endl;
}

// Destructor
CMusica::~CMusica() {
    cleanup();
    std::cout << "CMusica: Sistema de audio destruido." << std::endl;
}

// INICIALIZACIÓN
bool CMusica::initialize() {
    std::cout << "CMusica: Inicializando sistema de música..." << std::endl;
    
    // Cargar archivos de música
    if (!loadMusic()) {
        std::cerr << "CMusica: Error al cargar archivos de música." << std::endl;
        return false;
    }
    
    // Configurar ajustes por defecto
    setLooping(true);
    applyVolumeSettings();
    
    std::cout << "CMusica: Sistema inicializado exitosamente." << std::endl;
    return true;
}

bool CMusica::loadMusic() {
    std::cout << "CMusica: Cargando archivos de música..." << std::endl;
    
    bool success = true;
    
    // Cargar música del menú
    if (!m_menuMusic.openFromFile(m_menuMusicFile)) {
        std::cerr << "CMusica: Error al cargar " << m_menuMusicFile << std::endl;
        success = false;
    } else {
        std::cout << "CMusica: ✓ " << m_menuMusicFile << " cargado exitosamente." << std::endl;
    }
    
    // Cargar música del gameplay
    if (!m_gameplayMusic.openFromFile(m_gameplayMusicFile)) {
        std::cerr << "CMusica: Error al cargar " << m_gameplayMusicFile << std::endl;
        success = false;
    } else {
        std::cout << "CMusica: ✓ " << m_gameplayMusicFile << " cargado exitosamente." << std::endl;
    }
    
    if (success) {
        m_musicLoaded = true;
        std::cout << "CMusica: Todos los archivos de música cargados correctamente." << std::endl;
    } else {
        std::cout << "CMusica: Algunos archivos de música no se pudieron cargar." << std::endl;
    }
    
    return success;
}

void CMusica::cleanup() {
    std::cout << "CMusica: Limpiando sistema de audio..." << std::endl;
    
    stopMusic();
    m_currentMusic = nullptr;
    m_currentMusicType = MusicType::NONE;
    m_audioState = AudioState::STOPPED;
    m_musicLoaded = false;
}

// CONTROL PRINCIPAL DE MÚSICA
void CMusica::playMenuMusic() {
    if (!m_musicLoaded) {
        std::cerr << "CMusica: No se puede reproducir música del menú - archivos no cargados." << std::endl;
        return;
    }
    
    std::cout << "CMusica: Reproduciendo música del menú..." << std::endl;
    
    if (m_fadeEnabled && m_currentMusic && m_currentMusic->getStatus() == sf::Music::Playing) {
        fadeToMenuMusic();
    } else {
        switchToMusic(MusicType::MENU);
    }
}

void CMusica::playGameplayMusic() {
    if (!m_musicLoaded) {
        std::cerr << "CMusica: No se puede reproducir música del gameplay - archivos no cargados." << std::endl;
        return;
    }
    
    std::cout << "CMusica: Reproduciendo música del gameplay..." << std::endl;
    
    if (m_fadeEnabled && m_currentMusic && m_currentMusic->getStatus() == sf::Music::Playing) {
        fadeToGameplayMusic();
    } else {
        switchToMusic(MusicType::GAMEPLAY);
    }
}

void CMusica::stopMusic() {
    if (m_currentMusic) {
        std::cout << "CMusica: Deteniendo música actual..." << std::endl;
        m_currentMusic->stop();
    }
    
    m_currentMusic = nullptr;
    m_currentMusicType = MusicType::NONE;
    m_audioState = AudioState::STOPPED;
    m_fadeTimer = 0.0f;
}

void CMusica::pauseMusic() {
    if (m_currentMusic && m_currentMusic->getStatus() == sf::Music::Playing) {
        std::cout << "CMusica: Pausando música..." << std::endl;
        m_currentMusic->pause();
        m_audioState = AudioState::PAUSED;
    }
}

void CMusica::resumeMusic() {
    if (m_currentMusic && m_currentMusic->getStatus() == sf::Music::Paused) {
        std::cout << "CMusica: Reanudando música..." << std::endl;
        m_currentMusic->play();
        m_audioState = AudioState::PLAYING;
    }
}

// TRANSICIONES SUAVES
void CMusica::fadeToMenuMusic(float fadeTime) {
    if (!isMusicLoaded(MusicType::MENU)) return;
    
    std::cout << "CMusica: Transición suave a música del menú (" << fadeTime << "s)" << std::endl;
    
    m_transitionTarget = MusicType::MENU;
    m_audioState = AudioState::TRANSITIONING;
    
    if (m_currentMusic && m_currentMusic->getStatus() == sf::Music::Playing) {
        startFade(m_currentMusic->getVolume(), 0.0f, fadeTime);
    } else {
        switchToMusic(MusicType::MENU);
        startFade(0.0f, calculateEffectiveVolume(), fadeTime);
    }
}

void CMusica::fadeToGameplayMusic(float fadeTime) {
    if (!isMusicLoaded(MusicType::GAMEPLAY)) return;
    
    std::cout << "CMusica: Transición suave a música del gameplay (" << fadeTime << "s)" << std::endl;
    
    m_transitionTarget = MusicType::GAMEPLAY;
    m_audioState = AudioState::TRANSITIONING;
    
    if (m_currentMusic && m_currentMusic->getStatus() == sf::Music::Playing) {
        startFade(m_currentMusic->getVolume(), 0.0f, fadeTime);
    } else {
        switchToMusic(MusicType::GAMEPLAY);
        startFade(0.0f, calculateEffectiveVolume(), fadeTime);
    }
}

void CMusica::fadeOutCurrentMusic(float fadeTime) {
    if (m_currentMusic && m_currentMusic->getStatus() == sf::Music::Playing) {
        std::cout << "CMusica: Fade out de música actual (" << fadeTime << "s)" << std::endl;
        m_transitionTarget = MusicType::NONE;
        startFade(m_currentMusic->getVolume(), 0.0f, fadeTime);
    }
}

void CMusica::setFadeEnabled(bool enabled) {
    m_fadeEnabled = enabled;
    std::cout << "CMusica: Transiciones suaves " << (enabled ? "activadas" : "desactivadas") << std::endl;
}

// CONTROL DE VOLUMEN
void CMusica::setMasterVolumen(float volume) {
    m_masterVolume = std::clamp(volume, MIN_VOLUME, MAX_VOLUME);
    applyVolumeSettings();
    std::cout << "CMusica: Volumen maestro ajustado a " << m_masterVolume << "%" << std::endl;
}

void CMusica::setMusicVolumen(float volume) {
    m_musicVolume = std::clamp(volume, MIN_VOLUME, MAX_VOLUME);
    applyVolumeSettings();
    std::cout << "CMusica: Volumen de música ajustado a " << m_musicVolume << "%" << std::endl;
}

float CMusica::getMasterVolumen() const {
    return m_masterVolume;
}

float CMusica::getMusicVolumen() const {
    return m_musicVolume;
}

// SILENCIAR/DESILENCIAR
void CMusica::silenciar() {
    if (!m_isMuted) {
        m_isMuted = true;
        applyVolumeSettings();
        std::cout << "CMusica: Audio silenciado." << std::endl;
    }
}

void CMusica::desilenciar() {
    if (m_isMuted) {
        m_isMuted = false;
        applyVolumeSettings();
        std::cout << "CMusica: Audio des-silenciado." << std::endl;
    }
}

void CMusica::toggleSilencio() {
    if (m_isMuted) {
        desilenciar();
    } else {
        silenciar();
    }
}

bool CMusica::isSilenciado() const {
    return m_isMuted;
}

// ESTADO DEL SISTEMA
MusicType CMusica::getCurrentMusicType() const {
    return m_currentMusicType;
}

AudioState CMusica::getAudioState() const {
    return m_audioState;
}

bool CMusica::isPlaying() const {
    return m_currentMusic && m_currentMusic->getStatus() == sf::Music::Playing;
}

bool CMusica::isPaused() const {
    return m_currentMusic && m_currentMusic->getStatus() == sf::Music::Paused;
}

bool CMusica::isTransitioning() const {
    return m_audioState == AudioState::TRANSITIONING;
}

// CONFIGURACIÓN
void CMusica::setLooping(bool loop) {
    m_menuMusic.setLoop(loop);
    m_gameplayMusic.setLoop(loop);
    std::cout << "CMusica: Reproducción en bucle " << (loop ? "activada" : "desactivada") << std::endl;
}

bool CMusica::isLooping() const {
    return m_menuMusic.getLoop(); // Ambas músicas tienen el mismo setting
}

// UPDATE
void CMusica::update(float deltaTime) {
    // Actualizar sistema de fade
    if (m_audioState == AudioState::FADING_IN || m_audioState == AudioState::FADING_OUT || 
        m_audioState == AudioState::TRANSITIONING) {
        updateFade(deltaTime);
    }
    
    // Verificar si la música sigue reproduciendo
    if (m_currentMusic && m_audioState == AudioState::PLAYING) {
        if (m_currentMusic->getStatus() != sf::Music::Playing) {
            if (!isLooping()) {
                m_audioState = AudioState::STOPPED;
                std::cout << "CMusica: Música terminó de reproducirse." << std::endl;
            }
        }
    }
}

// DEBUG
void CMusica::printAudioStatus() const {
    std::cout << "=== ESTADO DEL SISTEMA DE MÚSICA ===" << std::endl;
    std::cout << "Música actual: " << musicTypeToString(m_currentMusicType) << std::endl;
    std::cout << "Estado: " << audioStateToString(m_audioState) << std::endl;
    std::cout << "Reproduciendo: " << (isPlaying() ? "Sí" : "No") << std::endl;
    std::cout << "Pausado: " << (isPaused() ? "Sí" : "No") << std::endl;
    std::cout << "En transición: " << (isTransitioning() ? "Sí" : "No") << std::endl;
    std::cout << "Archivos cargados: " << (m_musicLoaded ? "Sí" : "No") << std::endl;
    std::cout << "Reproducción en bucle: " << (isLooping() ? "Sí" : "No") << std::endl;
    std::cout << "===================================" << std::endl;
}

void CMusica::printVolumeInfo() const {
    std::cout << "=== INFORMACIÓN DE VOLUMEN ===" << std::endl;
    std::cout << "Volumen maestro: " << m_masterVolume << "%" << std::endl;
    std::cout << "Volumen de música: " << m_musicVolume << "%" << std::endl;
    std::cout << "Volumen efectivo: " << calculateEffectiveVolume() << "%" << std::endl;
    std::cout << "Silenciado: " << (m_isMuted ? "Sí" : "No") << std::endl;
    std::cout << "Transiciones suaves: " << (m_fadeEnabled ? "Sí" : "No") << std::endl;
    std::cout << "==============================" << std::endl;
}

// MÉTODOS PRIVADOS
void CMusica::switchToMusic(MusicType type, bool immediate) {
    // Detener música actual
    if (m_currentMusic) {
        m_currentMusic->stop();
    }
    
    // Cambiar a nueva música
    m_currentMusic = getMusicByType(type);
    m_currentMusicType = type;
    
    if (m_currentMusic) {
        if (immediate) {
            m_currentMusic->setVolume(calculateEffectiveVolume());
        } else {
            m_currentMusic->setVolume(0.0f);
        }
        
        m_currentMusic->play();
        m_audioState = immediate ? AudioState::PLAYING : AudioState::FADING_IN;
        
        std::cout << "CMusica: Cambiado a " << musicTypeToString(type) << std::endl;
    }
}

void CMusica::applyVolumeSettings() {
    float effectiveVolume = calculateEffectiveVolume();
    
    m_menuMusic.setVolume(effectiveVolume);
    m_gameplayMusic.setVolume(effectiveVolume);
    
    if (m_currentMusic) {
        m_currentMusic->setVolume(effectiveVolume);
    }
}

float CMusica::calculateEffectiveVolume() const {
    if (m_isMuted) {
        return 0.0f;
    }
    
    return (m_masterVolume / 100.0f) * (m_musicVolume / 100.0f) * 100.0f;
}

// SISTEMA DE FADE
void CMusica::startFade(float startVolume, float targetVolume, float duration) {
    m_fadeStartVolume = startVolume;
    m_fadeTargetVolume = targetVolume;
    m_fadeDuration = duration;
    m_fadeTimer = 0.0f;
    
    if (targetVolume > startVolume) {
        m_audioState = AudioState::FADING_IN;
    } else {
        m_audioState = AudioState::FADING_OUT;
    }
}

void CMusica::updateFade(float deltaTime) {
    m_fadeTimer += deltaTime;
    
    if (m_fadeTimer >= m_fadeDuration) {
        completeFade();
        return;
    }
    
    // Interpolación lineal del volumen
    float progress = m_fadeTimer / m_fadeDuration;
    float currentVolume = m_fadeStartVolume + (m_fadeTargetVolume - m_fadeStartVolume) * progress;
    
    if (m_currentMusic) {
        m_currentMusic->setVolume(currentVolume);
    }
}

void CMusica::completeFade() {
    if (m_audioState == AudioState::FADING_OUT) {
        if (m_transitionTarget != MusicType::NONE) {
            // Cambiar a nueva música después del fade out
            switchToMusic(m_transitionTarget, false);
            startFade(0.0f, calculateEffectiveVolume(), m_fadeDuration / 2.0f);
        } else {
            // Solo fade out, detener música
            stopMusic();
        }
    } else {
        // Fade in completado
        if (m_currentMusic) {
            m_currentMusic->setVolume(calculateEffectiveVolume());
        }
        m_audioState = AudioState::PLAYING;
    }
    
    m_fadeTimer = 0.0f;
}

// UTILIDADES
sf::Music* CMusica::getMusicByType(MusicType type) {
    switch (type) {
        case MusicType::MENU:
            return &m_menuMusic;
        case MusicType::GAMEPLAY:
            return &m_gameplayMusic;
        default:
            return nullptr;
    }
}

std::string CMusica::musicTypeToString(MusicType type) const {
    switch (type) {
        case MusicType::NONE: return "Ninguna";
        case MusicType::MENU: return "Menú";
        case MusicType::GAMEPLAY: return "Gameplay";
        default: return "Desconocido";
    }
}

std::string CMusica::audioStateToString(AudioState state) const {
    switch (state) {
        case AudioState::STOPPED: return "Detenido";
        case AudioState::PLAYING: return "Reproduciendo";
        case AudioState::PAUSED: return "Pausado";
        case AudioState::FADING_IN: return "Fade In";
        case AudioState::FADING_OUT: return "Fade Out";
        case AudioState::TRANSITIONING: return "En Transición";
        default: return "Desconocido";
    }
}

bool CMusica::isMusicLoaded(MusicType type) const {
    if (!m_musicLoaded) return false;
    
    sf::Music* music = const_cast<CMusica*>(this)->getMusicByType(type);
    return music != nullptr;
}