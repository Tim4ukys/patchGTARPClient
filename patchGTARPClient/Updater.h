/****************************************************
 *                                                   *
 *    Developer: Tim4ukys                            *
 *                                                   *
 *    email: tim4ukys.dev@yandex.ru                  *
 *    vk: vk.com/tim4ukys                            *
 *                                                   *
 *    License: GNU GPLv3                             *
 *                                                   *
 ****************************************************/
#pragma once

#ifndef CURRENT_VERSION
#define CURRENT_VERSION "10.2.0"
#endif

class Updater {
public:
    #undef DECLARATION_VERSION
    #define DECLARATION_VERSION(maj, min, patch) \
        static constexpr std::string_view VERSION = #maj "." #min "." #patch; \
        using MAJ = std::integral_constant<int, maj>; \
        using MIN = std::integral_constant<int, min>; \
        using PATCH = std::integral_constant<int, patch>;

    DECLARATION_VERSION(10, 2, 0)
    #undef DECLARATION_VERSION

    static inline bool check(int l_maj, int l_min, int l_patch,
                             int r_maj, int r_min, int r_patch) {
        if (r_maj > l_maj ||
            (r_maj == l_maj && r_min > l_min) ||
            (r_maj == l_maj && r_min == l_min && r_patch > l_patch)) {
            return true;
        }
        return false;
    }

    static inline bool check(int maj, int min, int patch) {
        if (maj > MAJ::value ||
            (maj == MAJ::value && min > MIN::value) ||
            (maj == MAJ::value && min == MIN::value && patch > PATCH::value)) {
            return true;
        }
        return false;
    }

private:
    struct stVersion {
        int maj, min, patch;
    } m_sync{};

    bool m_bWasSync{};
    void sync();

public:

    inline bool isHaveUpdate() {
        sync();
        return check(m_sync.maj, m_sync.min, m_sync.patch);
    }

};
