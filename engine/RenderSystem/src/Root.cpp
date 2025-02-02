#include "Root.h"

static CRoot* g_pRoot = NULL;

CRoot& GetRoot()
{
	//assert(g_pRenderSystem);
	return *g_pRoot;
}

CRoot::CRoot():
m_pRenderWindow(NULL),
m_pRenderSystem(NULL)
{
}

CRoot::~CRoot()
{
}

LRESULT CALLBACK CRoot::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	g_pRoot->m_hWnd = hWnd;
	LRESULT nResult = 0;

	if(g_pRoot->MsgProc(hWnd, uMsg, wParam, lParam))
		return nResult;
	
	if (g_pRoot->GetRenderWindow())
	{
		if(g_pRoot->GetRenderWindow()->MsgProc(hWnd, uMsg, wParam, lParam))
			return nResult;
	}
	return DefWindowProcW(hWnd, uMsg, wParam, lParam);;
}

typedef CRenderSystem* (WINAPI * PFN_RENDER_SYSTEM_PLUG_CREATE_SYSTEM)();
bool CRoot::LoadRenderSystemLibrary(const char* szFilename)
{
	HMODULE hIns = LoadLibrary(szFilename);

	if (hIns)
	{
		PFN_RENDER_SYSTEM_PLUG_CREATE_SYSTEM pFunc = (PFN_RENDER_SYSTEM_PLUG_CREATE_SYSTEM)GetProcAddress(hIns, "RENDER_SYSTEM_PLUG_CREATE_SYSTEM");
		if (NULL==pFunc)
		{
			MessageBox(NULL,"Create render system failed!","Error",0);
			return false;
		}
		m_pRenderSystem = pFunc();
		CRenderSystem::setSingleton(m_pRenderSystem);
	}
	else
	{
		MessageBox(NULL,"Load render system library failed!","Error",0);
		return false;
	}
	return true;
}

bool CRoot::CreateRenderWindow(const wchar_t* wcsTitle, int32 nWidth, int32 nHeight, bool bFullScreen)
{
	g_pRoot = this;
	m_pRenderWindow = m_pRenderSystem->CreateRenderWindow(CRoot::WndProc, wcsTitle, nWidth, nHeight, bFullScreen);
	return m_pRenderWindow!=NULL;
}

void CRoot::OnFrameMove(double fTime, float fElapsedTime)
{
	// 声音
	//GetSound().FrameMove(fElapsedTime);
	// 字
	//GetTextRender().OnFrameMove();
	// 纹理更新 加载等操作
	m_pRenderSystem->GetTextureMgr().Update();	
}

void CRoot::OnFrameRender(double fTime, float fElapsedTime)
{
	if(m_pRenderSystem->BeginFrame())
	{
		m_pRenderSystem->EndFrame();
	}
}

// Rejects any devices that aren't acceptable by returning false

//bool CRoot::IsDeviceAcceptable(D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, 
//								 D3DFORMAT BackBufferFormat, bool bWindowed)
//{
//	// Typically want to skip backbuffer formats that don't support alpha blending
//	IDirect3D9* pD3D = DXUTGetD3DObject(); 
//	if(FAILED(pD3D->CheckDeviceFormat(pCaps->AdapterOrdinal, pCaps->DeviceType,
//		AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, 
//		D3DRTYPE_TEXTURE, BackBufferFormat)))
//		return false;
//
//	return true;
//}
//
//
//
//// Before a device is created, modify the device settings as needed
//
//bool CRoot::ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps)
//{
//	return true;
//}
static double dfFreq;
static LONGLONG s_fLastTime;
void CRoot::Run()
{
	LARGE_INTEGER litmp; 
	QueryPerformanceFrequency(&litmp);
	dfFreq = (double)litmp.QuadPart;// 获得计数器的时钟频率
	QueryPerformanceCounter(&litmp);
	s_fLastTime = litmp.QuadPart;
	while (MainUpdate())
	{
	}
}

bool CRoot::MainUpdate()
{
	MSG  msg;
	while( PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE) )
	{

		if (WM_QUIT == msg.message)
		{
			return false;
		}
		else
		// Translate and dispatch the message
		//if(hAccel == NULL || hWnd == NULL || 
		//	0 == TranslateAccelerator(hWnd, hAccel, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	// 锁帧
	//{
	//	double fSleepTime = 1.0f/60.0f - fElapsedTime;
	//	if (fSleepTime > 0.0f)
	//	{
	//		Sleep(fSleepTime*1000);
	//		fElapsedTime += fSleepTime;
	//		fTime += fSleepTime;
	//		fAbsTime += fSleepTime;
	//	}
	//	else
	//	{
	//		fSleepTime = 0.0f;
	//	}
	//	GetDXUTState().SetSleepTime(fSleepTime);
	//}
	LARGE_INTEGER litmp; 
	QueryPerformanceCounter(&litmp);
	double fTime = (double)(litmp.QuadPart) / dfFreq;
	static float fElapsedTime =  (double)(litmp.QuadPart - s_fLastTime) / dfFreq;
	s_fLastTime = litmp.QuadPart;
	OnFrameMove(fTime, fElapsedTime);
	if (m_pRenderWindow->FrameBegin())
	{
		OnFrameRender(fTime, fElapsedTime);
		m_pRenderWindow->FrameEnd();
	}
	QueryPerformanceCounter(&litmp);
	fElapsedTime =  (double)(litmp.QuadPart - s_fLastTime) / dfFreq;
	return true;
}