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
class D3D9Hook {
public:

	D3D9Hook();
    ~D3D9Hook();

    template<typename T>
    struct stSignal : public std::vector<std::function<T>> {
        stSignal& operator+=(std::function<T> func) {
            this->push_back(func);
            return *this;
        }
    };

    stSignal<void(IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA*)> onPresentEvent;
    stSignal<void(LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS*)>  onLostDevice;
    stSignal<void(LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS*)> onResetDevice;
    stSignal<void(LPDIRECT3DDEVICE9)> onInitDevice;

private:

    static NOINLINE HRESULT __stdcall HookedPresent(IDirect3DDevice9* pDevice, CONST RECT* pSrcRect, CONST RECT* pDestRect, HWND hDestWindow, CONST RGNDATA* pDirtyRegion);
    static NOINLINE HRESULT __stdcall HookedReset(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentParams);

    PLH::CapstoneDisassembler dis = {PLH::Mode::x86};

    patch::callHook* m_pInitDevice;
    static void               initDeviceHook();
    void        initHooks();

	struct stHookData {
        uint64_t m_uiJump;
        PLH::x86Detour* m_pDetour;

	public:
        inline stHookData()
                : m_uiJump(NULL),
                  m_pDetour(nullptr) {
        }
	};

	stHookData present;
    stHookData reset;

    //

    DWORD findDevice(DWORD len);
    DWORD getDeviceAddress(int VTableIndex);

};
