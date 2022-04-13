#pragma once

#include <Windows.h>
#include <d3d9.h>
#include <d3dx9.h>

#include <vector>
#include <functional>

class CRender {
public:

	CRender();
	~CRender();

	bool initD3D(HWND hWnd);
	void draw();
	void onResize(UINT nWidth, UINT nHeight);

protected:

	D3DPRESENT_PARAMETERS* m_pPresentParams = nullptr;
	IDirect3D9* m_pIDirect3D9 = nullptr;
	IDirect3DDevice9* m_pDevice = nullptr;

public:

	template<typename T>
	struct on_event : public std::vector<std::function<T>> {
		on_event& operator+=(std::function<T> func) {
			this->push_back(func);
			return *this;
		}
	};

	on_event<void(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*)> m_eventDraw{};
	on_event<void(IDirect3DDevice9*)> m_eventInitD3D{};
	on_event<void(IDirect3DDevice9*)> m_eventOnLostDevice{};
	on_event<void(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*)> m_eventOnResetDevice{};


private:

	bool m_bIsInitDirect3D = false;

	void resetDevice();

	void antiRTSHooks();
};

