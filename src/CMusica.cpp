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
}

// Destructor
CMusica::~CMusica() {
    cleanup();
}

// INICIALIZACION
bool CMusica::initialize() {
    // Cargar archivos de musica
    if (!loadMusic()) {
        std::cerr << "CMusica: Error al cargar archivos de musica." << std::endl;
        return false;
    }
    
    // Configurar ajustes por defecto
    setLooping(true);
    applyVolumeSettings();
    
    std::cout << "Musica Lista" << std::endl;
    return true;
}

bool CMusica::loadMusic() {
    bool success = true;
    
    // Cargar musica del menu
    if (!m_menuMusic.openFromFile(m_menuMusicFile)) {
        std::cerr << "CMusica: Error al cargar " << m_menuMusicFile << std::endl;
        success = false;
    }
    
    // Cargar musica del gameplay
    if (!m_gameplayMusic.openFromFile(m_gameplayMusicFile)) {
        std::cerr << "CMusica: Error al cargar " << m_gameplayMusicFile << std::endl;
        success = false;
    }
    
    if (success) {
        m_musicLoaded = true;
    }
    
    return success;
}

void CMusica::cleanup() {
    stopMusic();
    m_currentMusic = nullptr;
    m_currentMusicType = MusicType::NONE;
    m_audioState = AudioState::STOPPED;
    m_musicLoaded = false;
}

// CONTROL PRINCIPAL DE MUSICA
void CMusica::playMenuMusic() {
    if (!m_musicLoaded) {
        std::cerr << "CMusica: No se puede reproducir musica del menu - archivos no cargados." << std::endl;
        return;
    }
    
    if (m_fadeEnabled && m_currentMusic && m_currentMusic->getStatus() == sf::Music::Playing) {
        fadeToMenuMusic();
    } else {
        switchToMusic(MusicType::MENU);
    }
}

void CMusica::playGameplayMusic() {
    if (!m_musicLoaded) {
        std::cerr << "CMusica: No se puede reproducir musica del gameplay - archivos no cargados." << std::endl;
        return;
    }
    
    if (m_fadeEnabled && m_currentMusic && m_currentMusic->getStatus() == sf::Music::Playing) {
        fadeToGameplayMusic();
    } else {
        switchToMusic(MusicType::GAMEPLAY);
    }
}

void CMusica::stopMusic() {
    if (m_currentMusic) {
        m_currentMusic->stop();
    }
    
    m_currentMusic = nullptr;
    m_currentMusicType = MusicType::NONE;
    m_audioState = AudioState::STOPPED;
    m_fadeTimer = 0.0f;
}

void CMusica::pauseMusic() {
    if (m_currentMusic && m_currentMusic->getStatus() == sf::Music::Playing) {
        m_currentMusic->pause();
        m_audioState = AudioState::PAUSED;
    }
}

void CMusica::resumeMusic() {
    if (m_currentMusic && m_currentMusic->getStatus() == sf::Music::Paused) {
        m_currentMusic->play();
        m_audioState = AudioState::PLAYING;
    }
}

// TRANSICIONES SUAVES
void CMusica::fadeToMenuMusic(float fadeTime) {
    if (!isMusicLoaded(MusicType::MENU)) return;
    
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
        m_transitionTarget = MusicType::NONE;
        startFade(m_currentMusic->getVolume(), 0.0f, fadeTime);
    }
}

void CMusica::setFadeEnabled(bool enabled) {
    m_fadeEnabled = enabled;
}

// CONTROL DE VOLUMEN
void CMusica::setMasterVolumen(float volume) {
    m_masterVolume = std::clamp(volume, MIN_VOLUME, MAX_VOLUME);
    applyVolumeSettings();
}

void CMusica::setMusicVolumen(float volume) {
    m_musicVolume = std::clamp(volume, MIN_VOLUME, MAX_VOLUME);
    applyVolumeSettings();
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
    }
}

void CMusica::desilenciar() {
    if (m_isMuted) {
        m_isMuted = false;
        applyVolumeSettings();
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

// CONFIGURACION
void CMusica::setLooping(bool loop) {
    m_menuMusic.setLoop(loop);
    m_gameplayMusic.setLoop(loop);
}

bool CMusica::isLooping() const {
    return m_menuMusic.getLoop(); // Ambas musicas tienen el mismo setting
}

// UPDATE
void CMusica::update(float deltaTime) {
    // Actualizar sistema de fade
    if (m_audioState == AudioState::FADING_IN || m_audioState == AudioState::FADING_OUT || 
        m_audioState == AudioState::TRANSITIONING) {
        updateFade(deltaTime);
    }
    
    // Verificar si la musica sigue reproduciendo
    if (m_currentMusic && m_audioState == AudioState::PLAYING) {
        if (m_currentMusic->getStatus() != sf::Music::Playing) {
            if (!isLooping()) {
                m_audioState = AudioState::STOPPED;
            }
        }
    }
}

// DEBUG
void CMusica::printAudioStatus() const {
    std::cout << "=== ESTADO DEL SISTEMA DE MUSICA ===" << std::endl;
    std::cout << "Musica actual: " << musicTypeToString(m_currentMusicType) << std::endl;
    std::cout << "Estado: " << audioStateToString(m_audioState) << std::endl;
    std::cout << "Reproduciendo: " << (isPlaying() ? "Si" : "No") << std::endl;
    std::cout << "Pausado: " << (isPaused() ? "Si" : "No") << std::endl;
    std::cout << "En transicion: " << (isTransitioning() ? "Si" : "No") << std::endl;
    std::cout << "Archivos cargados: " << (m_musicLoaded ? "Si" : "No") << std::endl;
    std::cout << "Reproduccion en bucle: " << (isLooping() ? "Si" : "No") << std::endl;
    std::cout << "===================================" << std::endl;
}

void CMusica::printVolumeInfo() const {
    std::cout << "=== INFORMACION DE VOLUMEN ===" << std::endl;
    std::cout << "Volumen maestro: " << m_masterVolume << "%" << std::endl;
    std::cout << "Volumen de musica: " << m_musicVolume << "%" << std::endl;
    std::cout << "Volumen efectivo: " << calculateEffectiveVolume() << "%" << std::endl;
    std::cout << "Silenciado: " << (m_isMuted ? "Si" : "No") << std::endl;
    std::cout << "Transiciones suaves: " << (m_fadeEnabled ? "Si" : "No") << std::endl;
    std::cout << "==============================" << std::endl;
}

// METODOS PRIVADOS
void CMusica::switchToMusic(MusicType type, bool immediate) {
    // Detener musica actual
    if (m_currentMusic) {
        m_currentMusic->stop();
    }
    
    // Cambiar a nueva musica
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
    
    // Interpolacion lineal del volumen
    float progress = m_fadeTimer / m_fadeDuration;
    float currentVolume = m_fadeStartVolume + (m_fadeTargetVolume - m_fadeStartVolume) * progress;
    
    if (m_currentMusic) {
        m_currentMusic->setVolume(currentVolume);
    }
}

void CMusica::completeFade() {
    if (m_audioState == AudioState::FADING_OUT) {
        if (m_transitionTarget != MusicType::NONE) {
            // Cambiar a nueva musica despues del fade out
            switchToMusic(m_transitionTarget, false);
            startFade(0.0f, calculateEffectiveVolume(), m_fadeDuration / 2.0f);
        } else {
            // Solo fade out, detener musica
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
        case MusicType::MENU: return "Menu";
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
        case AudioState::TRANSITIONING: return "En Transicion";
        default: return "Desconocido";
    }
}

bool CMusica::isMusicLoaded(MusicType type) const {
    if (!m_musicLoaded) return false;
    
    sf::Music* music = const_cast<CMusica*>(this)->getMusicByType(type);
    return music != nullptr;
}