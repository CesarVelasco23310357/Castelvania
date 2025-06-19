#include "CMusica.hpp"
#include <iostream>
#include <algorithm>

// Constructor
CMusica::CMusica() 
    : currentMusicType(MusicType::NONE), audioState(AudioState::STOPPED),
      currentMusic(nullptr), masterVolume(DEFAULT_MASTER_VOLUME), 
      musicVolume(DEFAULT_MUSIC_VOLUME), muted(false),
      fadeEnabled(true), fadeTimer(0.0f), fadeDuration(DEFAULT_FADE_DURATION),
      fadeStartVolume(0.0f), fadeTargetVolume(0.0f), transitionTarget(MusicType::NONE),
      menuMusicFile("assets/MenuFondo.ogg"), gameplayMusicFile("assets/GameplaySound.ogg"),
      musicLoaded(false) {
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
    if (!menuMusic.openFromFile(menuMusicFile)) {
        std::cerr << "CMusica: Error al cargar " << menuMusicFile << std::endl;
        success = false;
    }
    
    // Cargar musica del gameplay
    if (!gameplayMusic.openFromFile(gameplayMusicFile)) {
        std::cerr << "CMusica: Error al cargar " << gameplayMusicFile << std::endl;
        success = false;
    }
    
    if (success) {
        musicLoaded = true;
    }
    
    return success;
}

void CMusica::cleanup() {
    stopMusic();
    currentMusic = nullptr;
    currentMusicType = MusicType::NONE;
    audioState = AudioState::STOPPED;
    musicLoaded = false;
}

// CONTROL PRINCIPAL DE MUSICA
void CMusica::playMenuMusic() {
    if (!musicLoaded) {
        std::cerr << "CMusica: No se puede reproducir musica del menu - archivos no cargados." << std::endl;
        return;
    }
    
    if (fadeEnabled && currentMusic && currentMusic->getStatus() == sf::Music::Playing) {
        fadeToMenuMusic();
    } else {
        switchToMusic(MusicType::MENU);
    }
}

void CMusica::playGameplayMusic() {
    if (!musicLoaded) {
        std::cerr << "CMusica: No se puede reproducir musica del gameplay - archivos no cargados." << std::endl;
        return;
    }
    
    if (fadeEnabled && currentMusic && currentMusic->getStatus() == sf::Music::Playing) {
        fadeToGameplayMusic();
    } else {
        switchToMusic(MusicType::GAMEPLAY);
    }
}

void CMusica::stopMusic() {
    if (currentMusic) {
        currentMusic->stop();
    }
    
    currentMusic = nullptr;
    currentMusicType = MusicType::NONE;
    audioState = AudioState::STOPPED;
    fadeTimer = 0.0f;
}

void CMusica::pauseMusic() {
    if (currentMusic && currentMusic->getStatus() == sf::Music::Playing) {
        currentMusic->pause();
        audioState = AudioState::PAUSED;
    }
}

void CMusica::resumeMusic() {
    if (currentMusic && currentMusic->getStatus() == sf::Music::Paused) {
        currentMusic->play();
        audioState = AudioState::PLAYING;
    }
}

// TRANSICIONES SUAVES
void CMusica::fadeToMenuMusic(float fadeTime) {
    if (!isMusicLoaded(MusicType::MENU)) return;
    
    transitionTarget = MusicType::MENU;
    audioState = AudioState::TRANSITIONING;
    
    if (currentMusic && currentMusic->getStatus() == sf::Music::Playing) {
        startFade(currentMusic->getVolume(), 0.0f, fadeTime);
    } else {
        switchToMusic(MusicType::MENU);
        startFade(0.0f, calculateEffectiveVolume(), fadeTime);
    }
}

void CMusica::fadeToGameplayMusic(float fadeTime) {
    if (!isMusicLoaded(MusicType::GAMEPLAY)) return;
    
    transitionTarget = MusicType::GAMEPLAY;
    audioState = AudioState::TRANSITIONING;
    
    if (currentMusic && currentMusic->getStatus() == sf::Music::Playing) {
        startFade(currentMusic->getVolume(), 0.0f, fadeTime);
    } else {
        switchToMusic(MusicType::GAMEPLAY);
        startFade(0.0f, calculateEffectiveVolume(), fadeTime);
    }
}

void CMusica::fadeOutCurrentMusic(float fadeTime) {
    if (currentMusic && currentMusic->getStatus() == sf::Music::Playing) {
        transitionTarget = MusicType::NONE;
        startFade(currentMusic->getVolume(), 0.0f, fadeTime);
    }
}

void CMusica::setFadeEnabled(bool enabled) {
    fadeEnabled = enabled;
}

// CONTROL DE VOLUMEN
void CMusica::setMasterVolumen(float volume) {
    masterVolume = std::clamp(volume, MIN_VOLUME, MAX_VOLUME);
    applyVolumeSettings();
}

void CMusica::setMusicVolumen(float volume) {
    musicVolume = std::clamp(volume, MIN_VOLUME, MAX_VOLUME);
    applyVolumeSettings();
}

float CMusica::getMasterVolumen() const {
    return masterVolume;
}

float CMusica::getMusicVolumen() const {
    return musicVolume;
}

// SILENCIAR/DESILENCIAR
void CMusica::silenciar() {
    if (!muted) {
        muted = true;
        applyVolumeSettings();
    }
}

void CMusica::desilenciar() {
    if (muted) {
        muted = false;
        applyVolumeSettings();
    }
}

void CMusica::toggleSilencio() {
    if (muted) {
        desilenciar();
    } else {
        silenciar();
    }
}

bool CMusica::isSilenciado() const {
    return muted;
}

// ESTADO DEL SISTEMA
MusicType CMusica::getCurrentMusicType() const {
    return currentMusicType;
}

AudioState CMusica::getAudioState() const {
    return audioState;
}

bool CMusica::isPlaying() const {
    return currentMusic && currentMusic->getStatus() == sf::Music::Playing;
}

bool CMusica::isPaused() const {
    return currentMusic && currentMusic->getStatus() == sf::Music::Paused;
}

bool CMusica::isTransitioning() const {
    return audioState == AudioState::TRANSITIONING;
}

// CONFIGURACION
void CMusica::setLooping(bool loop) {
    menuMusic.setLoop(loop);
    gameplayMusic.setLoop(loop);
}

bool CMusica::isLooping() const {
    return menuMusic.getLoop(); // Ambas musicas tienen el mismo setting
}

// UPDATE
void CMusica::update(float deltaTime) {
    // Actualizar sistema de fade
    if (audioState == AudioState::FADING_IN || audioState == AudioState::FADING_OUT || 
        audioState == AudioState::TRANSITIONING) {
        updateFade(deltaTime);
    }
    
    // Verificar si la musica sigue reproduciendo
    if (currentMusic && audioState == AudioState::PLAYING) {
        if (currentMusic->getStatus() != sf::Music::Playing) {
            if (!isLooping()) {
                audioState = AudioState::STOPPED;
            }
        }
    }
}

// DEBUG
void CMusica::printAudioStatus() const {
    std::cout << "=== ESTADO DEL SISTEMA DE MUSICA ===" << std::endl;
    std::cout << "Musica actual: " << musicTypeToString(currentMusicType) << std::endl;
    std::cout << "Estado: " << audioStateToString(audioState) << std::endl;
    std::cout << "Reproduciendo: " << (isPlaying() ? "Si" : "No") << std::endl;
    std::cout << "Pausado: " << (isPaused() ? "Si" : "No") << std::endl;
    std::cout << "En transicion: " << (isTransitioning() ? "Si" : "No") << std::endl;
    std::cout << "Archivos cargados: " << (musicLoaded ? "Si" : "No") << std::endl;
    std::cout << "Reproduccion en bucle: " << (isLooping() ? "Si" : "No") << std::endl;
    std::cout << "===================================" << std::endl;
}

void CMusica::printVolumeInfo() const {
    std::cout << "=== INFORMACION DE VOLUMEN ===" << std::endl;
    std::cout << "Volumen maestro: " << masterVolume << "%" << std::endl;
    std::cout << "Volumen de musica: " << musicVolume << "%" << std::endl;
    std::cout << "Volumen efectivo: " << calculateEffectiveVolume() << "%" << std::endl;
    std::cout << "Silenciado: " << (muted ? "Si" : "No") << std::endl;
    std::cout << "Transiciones suaves: " << (fadeEnabled ? "Si" : "No") << std::endl;
    std::cout << "==============================" << std::endl;
}

// METODOS PRIVADOS
void CMusica::switchToMusic(MusicType type, bool immediate) {
    // Detener musica actual
    if (currentMusic) {
        currentMusic->stop();
    }
    
    // Cambiar a nueva musica
    currentMusic = getMusicByType(type);
    currentMusicType = type;
    
    if (currentMusic) {
        if (immediate) {
            currentMusic->setVolume(calculateEffectiveVolume());
        } else {
            currentMusic->setVolume(0.0f);
        }
        
        currentMusic->play();
        audioState = immediate ? AudioState::PLAYING : AudioState::FADING_IN;
    }
}

void CMusica::applyVolumeSettings() {
    float effectiveVolume = calculateEffectiveVolume();
    
    menuMusic.setVolume(effectiveVolume);
    gameplayMusic.setVolume(effectiveVolume);
    
    if (currentMusic) {
        currentMusic->setVolume(effectiveVolume);
    }
}

float CMusica::calculateEffectiveVolume() const {
    if (muted) {
        return 0.0f;
    }
    
    return (masterVolume / 100.0f) * (musicVolume / 100.0f) * 100.0f;
}

// SISTEMA DE FADE
void CMusica::startFade(float startVolume, float targetVolume, float duration) {
    fadeStartVolume = startVolume;
    fadeTargetVolume = targetVolume;
    fadeDuration = duration;
    fadeTimer = 0.0f;
    
    if (targetVolume > startVolume) {
        audioState = AudioState::FADING_IN;
    } else {
        audioState = AudioState::FADING_OUT;
    }
}

void CMusica::updateFade(float deltaTime) {
    fadeTimer += deltaTime;
    
    if (fadeTimer >= fadeDuration) {
        completeFade();
        return;
    }
    
    // Interpolacion lineal del volumen
    float progress = fadeTimer / fadeDuration;
    float currentVolume = fadeStartVolume + (fadeTargetVolume - fadeStartVolume) * progress;
    
    if (currentMusic) {
        currentMusic->setVolume(currentVolume);
    }
}

void CMusica::completeFade() {
    if (audioState == AudioState::FADING_OUT) {
        if (transitionTarget != MusicType::NONE) {
            // Cambiar a nueva musica despues del fade out
            switchToMusic(transitionTarget, false);
            startFade(0.0f, calculateEffectiveVolume(), fadeDuration / 2.0f);
        } else {
            // Solo fade out, detener musica
            stopMusic();
        }
    } else {
        // Fade in completado
        if (currentMusic) {
            currentMusic->setVolume(calculateEffectiveVolume());
        }
        audioState = AudioState::PLAYING;
    }
    
    fadeTimer = 0.0f;
}

// UTILIDADES
sf::Music* CMusica::getMusicByType(MusicType type) {
    switch (type) {
        case MusicType::MENU:
            return &menuMusic;
        case MusicType::GAMEPLAY:
            return &gameplayMusic;
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
    if (!musicLoaded) return false;
    
    sf::Music* music = const_cast<CMusica*>(this)->getMusicByType(type);
    return music != nullptr;
}