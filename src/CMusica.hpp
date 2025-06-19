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
    sf::Music m_menuMusic;
    sf::Music m_gameplayMusic;
    
    // Estado del sistema de audio
    MusicType m_currentMusicType;
    AudioState m_audioState;
    sf::Music* m_currentMusic;
    
    // Configuración de volumen
    float m_masterVolume;
    float m_musicVolume;
    bool m_isMuted;
    
    // Sistema de transiciones suaves
    bool m_fadeEnabled;
    float m_fadeTimer;
    float m_fadeDuration;
    float m_fadeStartVolume;
    float m_fadeTargetVolume;
    MusicType m_transitionTarget;
    
    // Archivos de música
    std::string m_menuMusicFile;
    std::string m_gameplayMusicFile;
    
    // Estado de carga
    bool m_musicLoaded;
    
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