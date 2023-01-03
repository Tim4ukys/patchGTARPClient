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
    ~D3D9Hook() = default;

    FSignal<void(IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA*)> onPresentEvent;
    FSignal<void(LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS*)>                         onLostDevice;
    FSignal<void(LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS*)>                         onResetDevice;
    FSignal<void(LPDIRECT3DDEVICE9)>                                                 onInitDevice;

private:

    static NOINLINE HRESULT __stdcall HookedPresent(IDirect3DDevice9* pDevice, CONST RECT* pSrcRect, CONST RECT* pDestRect, HWND hDestWindow, CONST RGNDATA* pDirtyRegion);
    static NOINLINE HRESULT __stdcall HookedReset(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentParams);

	struct stHookData {
        uint64_t m_uiJump;
        PLH::x86Detour* m_pDetour;

	public:
        inline stHookData()
                : m_uiJump(NULL),
                  m_pDetour(nullptr) {
        }
        inline ~stHookData() {
            if (m_pDetour != nullptr)
                delete m_pDetour;
        }
	};

	stHookData present;
    stHookData reset;

    DWORD getDeviceAddress(int VTableIndex);
};
