#ifndef CMUSICA_HPP
#define CMUSICA_HPP

#include <SFML/Audio.hpp>
#include <string>
#include <memory>

enum class MusicType {
    NONE,
    MENU,
    GAMEPLAY
};

enum class AudioState {
    STOPPED,
    PLAYING,
    PAUSED,
    FADING_IN,
    FADING_OUT,
    TRANSITIONING
};

class CMusica {
private:
    // Objetos de música SFML
    sf::Music menuMusic;
    sf::Music gameplayMusic;
    
    // Estado del sistema de audio
    MusicType currentMusicType;
    AudioState audioState;
    sf::Music* currentMusic;
    
    // Configuración de volumen
    float masterVolume;
    float musicVolume;
    bool muted;
    
    // Sistema de transiciones suaves
    bool fadeEnabled;
    float fadeTimer;
    float fadeDuration;
    float fadeStartVolume;
    float fadeTargetVolume;
    MusicType transitionTarget;
    
    // Archivos de música
    std::string menuMusicFile;
    std::string gameplayMusicFile;
    
    // Estado de carga
    bool musicLoaded;
    
public:
    // Constructor y Destructor
    CMusica();
    ~CMusica();
    
    // Inicialización
    bool initialize();
    bool loadMusic();
    void cleanup();
    
    // Control principal de música
    void playMenuMusic();
    void playGameplayMusic();
    void stopMusic();
    void pauseMusic();
    void resumeMusic();
    
    // Transiciones suaves
    void fadeToMenuMusic(float fadeTime = 1.0f);
    void fadeToGameplayMusic(float fadeTime = 1.0f);
    void fadeOutCurrentMusic(float fadeTime = 1.0f);
    void setFadeEnabled(bool enabled);
    
    // Control de volumen
    void setMasterVolumen(float volume);        // 0.0f - 100.0f
    void setMusicVolumen(float volume);         // 0.0f - 100.0f
    float getMasterVolumen() const;
    float getMusicVolumen() const;
    
    // Silenciar/Desilenciar
    void silenciar();
    void desilenciar();
    void toggleSilencio();
    bool isSilenciado() const;
    
    // Estado del sistema
    MusicType getCurrentMusicType() const;
    AudioState getAudioState() const;
    bool isPlaying() const;
    bool isPaused() const;
    bool isTransitioning() const;
    
    // Configuración
    void setLooping(bool loop);
    bool isLooping() const;
    
    // Update - para transiciones y efectos
    void update(float deltaTime);
    
    // Debug
    void printAudioStatus() const;
    void printVolumeInfo() const;
    
private:
    // Métodos privados de control
    void switchToMusic(MusicType type, bool immediate = true);
    void applyVolumeSettings();
    float calculateEffectiveVolume() const;
    
    // Sistema de fade
    void startFade(float startVolume, float targetVolume, float duration);
    void updateFade(float deltaTime);
    void completeFade();
    
    // Utilidades
    sf::Music* getMusicByType(MusicType type);
    std::string musicTypeToString(MusicType type) const;
    std::string audioStateToString(AudioState state) const;
    bool isMusicLoaded(MusicType type) const;
    
    // Configuración por defecto
    static constexpr float DEFAULT_MASTER_VOLUME = 70.0f;
    static constexpr float DEFAULT_MUSIC_VOLUME = 80.0f;
    static constexpr float DEFAULT_FADE_DURATION = 1.5f;
    static constexpr float MIN_VOLUME = 0.0f;
    static constexpr float MAX_VOLUME = 100.0f;
};

#endif // CMUSICA_HPP