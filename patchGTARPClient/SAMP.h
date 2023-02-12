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

#ifndef _D3D9_H_
typedef DWORD D3DCOLOR;
#endif

typedef void(__cdecl* CMDPROC)(const char*);

enum eChatMessageType {
    CHAT_TYPE_NONE = 0,
    CHAT_TYPE_CHAT = 2,
    CHAT_TYPE_INFO = 4,
    CHAT_TYPE_DEBUG = 8
};

enum CursorMode {
    CURSOR_NONE,
    CURSOR_LOCKKEYS_NOCURSOR,
    CURSOR_LOCKCAMANDCONTROL,
    CURSOR_LOCKCAM,
    CURSOR_LOCKCAM_NOCURSOR
};

enum Gamestate : int {
    GAMESTATE_NONE = 0,
    GAMESTATE_WAIT_CONNECT = 1,
    GAMESTATE_DISCONNECTED = 2,
    GAMESTATE_AWAIT_JOIN = 6,
    GAMESTATE_CONNECTED = 5,
    GAMESTATE_RESTARTING = 11,
};

class SAMP {
public:

#pragma pack(push, 1)
    struct ServerPresets {
        bool         useCJWalk;
        unsigned int deadDropsMoney;
        float        worldBoundaries[4];
        bool         allowWeapons;
        float        gravity;
        bool         disableInteriorEnterExits;
        BOOL         vehicleFriendlyFire;
        bool         holdTime;
        bool         instagib;
        bool         zoneNames;
        bool         friendlyFire;
        BOOL         classesAvailable;
        float        nameTagsDrawDistance;
        bool         manualVehicleEngineAndLight;
        uint8_t      worldTimeHour;
        uint8_t      worldTimeMinute;
        uint8_t      weatherId;
        bool         nametagsBehindWalls;
        int          playerMarkersMode;
        float        globalChatRadiusLimit;
        bool         showNameTags;
        bool         ltdChatRadius;
    };

    struct NetGame {
        char                  unk[44];
        PVOID                 rakClientInterface;
        char                  serverIp[257];
        char                  serverName[257];
        bool                  disableCollision;
        bool                  updateCameraTarget;
        bool                  nametagStatus;
        uint32_t              serverPort;
        BOOL                  lanMode;
        uint32_t              mapIcons[100];
        Gamestate             gameState;
        uint32_t              lastConnectAttempt;
        struct ServerPresets* serverPresets;
        char                  controlLocked;
        int                   unk2;
        struct SAMPPools*     pools;
    };

    struct FontRenderer {
        ID3DXFont*        chatFont;
        ID3DXFont*        littleFont;
        ID3DXFont*        chatShadowFont;
        ID3DXFont*        littleShadowFont;
        ID3DXFont*        carNumberFont;
        ID3DXSprite*      tempSprite;
        IDirect3DDevice9* d3dDevice;
        char*             textBuffer;
        int               charHeight;
        int               littleCharHeight;
    };

    struct ChatInfo {
        int      pagesize;
        void*    lastMessage;
        int      mode;
        uint8_t  timestamps;
        uint32_t logFileIsExist;
        char     logFilePath[MAX_PATH + 1];
        void*    gameUI;         // CDXUTDialog
        void*    editBackground; // CDXUTEditBox
        void*    dxutScrollBar;  // CDXUTScrollBar
        D3DCOLOR textColor;
        D3DCOLOR infoColor;
        D3DCOLOR debugColor;
        uint32_t chatWindowBottom;
        struct ChatEntry {
            uint32_t systemTime;
            char     prefix[28];
            char     text[144];
            uint8_t  unk[64];
            int      type; // 2 - text + prefix, 4 - text (server msg), 8 - text (debug)
            D3DCOLOR textColor;
            D3DCOLOR prefixColor; // or textOnly colour
        } chatEntry[100];
        FontRenderer*         fontRenderer;
        ID3DXSprite*          chatTextSprite;
        ID3DXSprite*          sprite;
        IDirect3DDevice9*     d3dDevice;
        int                   renderMode; // 0 - Direct Mode (slow), 1 - Normal mode
        ID3DXRenderToSurface* d3dRenderToSurface;
        IDirect3DTexture9*    texture;
        IDirect3DSurface9*    surface;
        D3DDISPLAYMODE*       d3dDisplayMode;
        int                   unk[2];
        int                   redraw;
        int                   scrollBarPosition;
        int                   fontSizeY;
        int                   timestampWidth;
    };
#pragma pack(pop)
    
    SAMP();
    //~SAMP();

    bool isSAMPInit();

    void addChatMessage(D3DCOLOR color, const char* fmt, ...);
    void cmdRect(const char* szCmdName, CMDPROC pCmdProc);

    void setCursorMode(int nMode, BOOL bImmediatelyHideCursor);
    //void redraw();

    //void setHour(unsigned char hour);

private:

    void initPointerList();

    ChatInfo* g_pChat{};
    NetGame*  g_pNetGame{};
    PVOID     g_pInput{};
    PVOID     g_pGame{};

    void(__thiscall* g_fncAddEntry)(PVOID pChat, char eType, const char* szString, const char* szNick, D3DCOLOR dwTextColor, D3DCOLOR dwNickColor) = nullptr;
    void(__thiscall* g_fncAddCmdProc)(PVOID pInput, const char* szCmdName, CMDPROC cmdHandler) = nullptr;
    void(__thiscall* g_fncSetCursorMode)(PVOID pGame, int nMode, BOOL bImmediatelyHideCursor) = nullptr;

public:

    inline ChatInfo* getChat() noexcept {
        return g_pChat;
    }
    inline NetGame* getNetGame() noexcept {
        return g_pNetGame;
    }
};
