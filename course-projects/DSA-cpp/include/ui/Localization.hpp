#pragma once
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>

namespace ui {

enum class Language {
    UA = 0,
    EN = 1
};

class Localization {
public:
    static Localization& instance() {
        static Localization s_instance;
        return s_instance;
    }

    Language getLanguage() const noexcept { return currentLang_; }
    void setLanguage(Language lang) noexcept {
        currentLang_ = lang;
        savePreference();
    }

    void toggleLanguage() noexcept {
        currentLang_ = (currentLang_ == Language::UA) ? Language::EN : Language::UA;
        savePreference();
        triggerGlitch(0.38f);
    }

    void savePreference(const std::string& path = "session_state.json") const {
        try {
            nlohmann::json j;
            {
                std::ifstream in(path);
                if (in.is_open()) {
                    try {
                        in >> j;
                    } catch (...) {}
                }
            }
            if (!j.is_object()) {
                j = nlohmann::json::object();
            }
            if (j.contains("gui") && j["gui"].is_object()) {
                j["gui"]["language"] = (currentLang_ == Language::EN ? "EN" : "UA");
            } else {
                j["gui"] = {{"language", (currentLang_ == Language::EN ? "EN" : "UA")}};
            }
            std::ofstream out(path);
            if (out.is_open()) {
                out << j.dump(4) << "\n";
            }
        } catch (...) {}
    }

    void loadPreference(const std::string& path = "session_state.json") {
        try {
            std::ifstream in(path);
            if (in.is_open()) {
                nlohmann::json j;
                in >> j;
                std::string code;
                if (j.contains("gui") && j["gui"].is_object() && j["gui"].contains("language")) {
                    code = j["gui"]["language"].get<std::string>();
                } else if (j.contains("language")) {
                    code = j["language"].get<std::string>();
                }
                if (code == "EN") currentLang_ = Language::EN;
                else if (code == "UA") currentLang_ = Language::UA;
            }
        } catch (...) {}
    }

    void triggerGlitch(float duration = 0.38f) noexcept {
        glitchDuration_ = duration;
        glitchTimer_ = duration;
    }

    void update(float dt) noexcept {
        if (glitchTimer_ > 0.0f) {
            glitchTimer_ = std::max(0.0f, glitchTimer_ - dt);
        }
    }

    bool isGlitching() const noexcept { return glitchTimer_ > 0.0f; }
    float getGlitchProgress() const noexcept {
        return (glitchDuration_ > 0.001f) ? (glitchTimer_ / glitchDuration_) : 0.0f;
    }
    float getGlitchTimer() const noexcept { return glitchTimer_; }

    bool isEnglish() const noexcept { return currentLang_ == Language::EN; }
    bool isUkrainian() const noexcept { return currentLang_ == Language::UA; }

    const char* getLangCode() const noexcept {
        return (currentLang_ == Language::UA) ? "UA" : "EN";
    }

    const char* getOtherLangCode() const noexcept {
        return (currentLang_ == Language::UA) ? "EN" : "UA";
    }

    const char* getLangName() const noexcept {
        return (currentLang_ == Language::UA) ? "Українська" : "English";
    }

    // Return English if active, otherwise Ukrainian
    const char* tr(const char* ua, const char* en) const noexcept {
        return (currentLang_ == Language::EN) ? en : ua;
    }

    std::string tr(const std::string& ua, const std::string& en) const {
        return (currentLang_ == Language::EN) ? en : ua;
    }

private:
    Localization() : currentLang_(Language::UA), glitchTimer_(0.0f), glitchDuration_(0.38f) {
        loadPreference();
    }
    Language currentLang_{Language::UA};
    float glitchTimer_{0.0f};
    float glitchDuration_{0.38f};
};

// Global shorthand helpers
inline const char* LOC(const char* ua, const char* en) {
    return Localization::instance().tr(ua, en);
}

inline std::string LOC(const std::string& ua, const std::string& en) {
    return Localization::instance().tr(ua, en);
}

} // namespace ui

