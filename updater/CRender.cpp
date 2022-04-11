#include "CRender.h"
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "d3d9.lib")

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) if (p) { delete p; p=nullptr; }
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) if (p) { p->Release(); p=nullptr; }
#endif


CRender::CRender()
{
	m_pPresentParams = new D3DPRESENT_PARAMETERS;
	ZeroMemory(m_pPresentParams, sizeof(D3DPRESENT_PARAMETERS));
}
CRender::~CRender()
{
	SAFE_DELETE(m_pPresentParams);

	SAFE_RELEASE(m_pDevice);
	SAFE_RELEASE(m_pIDirect3D9);
}

bool CRender::initD3D(HWND hWnd)
{
	if (m_bIsInitDirect3D) { return true; }

	if (m_pIDirect3D9 = Direct3DCreate9(D3D_SDK_VERSION); !m_pIDirect3D9) { return false; }

	//RECT r;
	//GetClientRect(hWnd, &r);
#pragma warning(push)
#pragma warning(disable : 26812)
	m_pPresentParams->hDeviceWindow = hWnd;
	m_pPresentParams->Windowed = TRUE;
	//m_pPresentParams->BackBufferWidth = r.right - r.left;
	//m_pPresentParams->BackBufferHeight = r.bottom - r.top;
	m_pPresentParams->BackBufferCount = 1;
	m_pPresentParams->BackBufferFormat = D3DFMT_UNKNOWN;
	m_pPresentParams->SwapEffect = D3DSWAPEFFECT_DISCARD; // ?
	//m_pPresentParams->MultiSampleType = D3DMULTISAMPLE_16_SAMPLES; // sglazhivanue
	//m_pPresentParams->MultiSampleQuality // ?
	//m_pPresentParams->EnableAutoDepthStencil = TRUE; // z byffer
	//m_pPresentParams->AutoDepthStencilFormat = D3DFMT_D24S8; // format z byffera
#pragma warning(pop)

	if (FAILED(m_pIDirect3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, m_pPresentParams, &m_pDevice)))
	{
		ZeroMemory(m_pPresentParams, sizeof(D3DPRESENT_PARAMETERS));

		SAFE_RELEASE(m_pIDirect3D9);
		return false;
	}

	for (auto& fnc : m_eventInitD3D) {
		fnc(m_pDevice);
	}

	return m_bIsInitDirect3D = true;
}

void CRender::onResize(UINT nWidth, UINT nHeight)
{
	m_pPresentParams->BackBufferWidth = nWidth;
	m_pPresentParams->BackBufferHeight = nHeight;
	
	if (m_pDevice) {
		resetDevice();
	}
}

void CRender::draw()
{
	if (m_bIsInitDirect3D)
	{
		m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(125, 125, 125), 1.0f, 0);

		if (SUCCEEDED(m_pDevice->BeginScene()))
		{
			for (auto& fnc : m_eventDraw) {
				fnc(m_pDevice, m_pPresentParams);
			}

			m_pDevice->EndScene();
		}

		antiRTSHooks();
		auto result = m_pDevice->Present(NULL, NULL, NULL, NULL);
		
		if (result == D3DERR_DEVICELOST && m_pDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		{
			resetDevice();
		}
	}
}

void CRender::resetDevice() {
	for (auto& fnc : m_eventOnLostDevice) {
		fnc(m_pDevice);
	}

	auto result = m_pDevice->Reset(m_pPresentParams);

	if (result == D3D_OK) {
		for (auto& fnc : m_eventOnResetDevice)
		{
			fnc(m_pDevice, m_pPresentParams);
		}
	}
}

void CRender::antiRTSHooks()
{
	struct stDirectDeviceUnk {
		struct {
			PVOID QueryInterface;
			PVOID AddRef;
			PVOID Release;
		} *vtbl;
	};
#ifdef _M_IX86
	BYTE origStartPresent[]{ 0x8b, 0xff, 0x55, 0x8b, 0xec };
	PVOID pPresentFNC = decltype(pPresentFNC)(reinterpret_cast<stDirectDeviceUnk*>(m_pDevice)->vtbl[5].Release);
	if (memcmp(pPresentFNC, origStartPresent, sizeof(origStartPresent)) != NULL) {
		DWORD oldProtect;
		VirtualProtect(pPresentFNC, sizeof(origStartPresent), PAGE_READWRITE, &oldProtect);
		memcpy(pPresentFNC, (PVOID)origStartPresent, sizeof(origStartPresent));
		VirtualProtect(pPresentFNC, sizeof(origStartPresent), oldProtect, &oldProtect);
	}
#else
#error "Poshol naxyu"
#endif
}