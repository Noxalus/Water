#include "stdafx.h"
#include "d3d9.h"
#include "d3dx9.h"
#include <math.h>
#include "InputManager.h"

// Global Variables:
HINSTANCE hInst;			// current instance
HWND	    hWnd;				// windows handle used in DirectX initialization

// Forward declarations
bool				CreateWindows(HINSTANCE, int);
bool				CreateDevice();
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

// Input Manager
InputManager* _inputManager;

// Height map
unsigned short m_sizeX;
unsigned short m_sizeZ;
float m_maxY = 50;
float* m_height;

// Structure d'un vertex
struct Vertex
{
	D3DXVECTOR3 Position;
	D3DXVECTOR2 TextCoord;
	D3DXVECTOR3 Normal;
};


bool LoadRAW(const std::string& map)
{
	FILE  *file;
	fopen_s(&file, map.c_str(), "rb");
	if (!file)
		return false;
	fread(&m_sizeX, sizeof(unsigned short), 1, file);
	fread(&m_sizeZ, sizeof(unsigned short), 1, file);
	unsigned int size = m_sizeX * m_sizeZ;
	unsigned char *tmp = new unsigned char[size];
	m_height = new float[size];
	fread(tmp, sizeof(unsigned char), size, file);
	fclose(file);
	int i = 0;
	for (unsigned short z = 0; z < m_sizeZ; ++z)
	for (unsigned short x = 0; x < m_sizeX; ++x, ++i)
		m_height[i] = float((m_maxY * tmp[i]) / 255.0f);
	delete [] tmp;
	return true;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG oMsg;

	if (!CreateWindows(hInstance, nCmdShow))
	{
		MessageBox(NULL, L"Erreur lors de la création de la fenêtre", L"Error", 0);
		return false;
	}
	if (!CreateDevice())
	{
		MessageBox(NULL, L"Erreur lors de la création du device DirectX 9", L"Error", 0);
		return false;
	}

	// Input manager
	_inputManager = new InputManager();
	_inputManager->Create(hInstance, hWnd);

	D3DCOLOR backgroundColor = D3DCOLOR_RGBA(0, 0, 0, 0);
	DWORD fillMode = D3DFILL_SOLID;

	// Lights
	bool enableLights = true;

	// Ambient light
	D3DXVECTOR4 ambientColor = D3DXVECTOR4(0.5f, 0.5f, 0.5f, 1.0f);

	// Directional light
	D3DXVECTOR3 lightDirection = D3DXVECTOR3(-1, 0, 0);
	D3DXVECTOR4 diffuseColor = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);

	// Omni light
	D3DXVECTOR4 *omniLightPositions = new D3DXVECTOR4[1]{ D3DXVECTOR4(0, 15, 10, 0) };
	D3DXVECTOR4 *omniLightColors = new D3DXVECTOR4[1]{ D3DXVECTOR4(255, 0, 0, 0) };
	float omniLightDistance = 10;

	// World matrix
	D3DXMATRIX World;

	D3DXMATRIX Position;
	D3DXMATRIX Rotation;

	float Yaw = 0;
	float Pitch = 0;

	D3DXMatrixIdentity(&Rotation);
	D3DXMatrixIdentity(&Position);

	World = Position * Rotation;

	// View matrix
	D3DXMATRIX View;

	D3DXVECTOR3 CameraPosition(0.f, 0.f, -1.f);
	D3DXVECTOR3 CameraDirection(0.f, 0.f, 1.f);

	D3DXVECTOR3 At;
	D3DXVECTOR3 Up(0.f, 1.f, 0.f);

	// Projection matrix
	D3DXMATRIX Projection;

	float fovy = D3DX_PI / 4; // pi / 2
	D3DXMatrixPerspectiveFovLH(&Projection, fovy, 1.3f, 0.1f, 1000.0f);

	D3DXMATRIX ReflectionMatrix;

	// Création de l’interface DirectX 9
	LPDIRECT3D9 pD3D = Direct3DCreate9(D3D_SDK_VERSION);

	D3DDISPLAYMODE displayMode;
	D3DPRESENT_PARAMETERS pp;
	pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode);
	pp.Windowed = true; //Mode fenêtré ou pas
	pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	pp.BackBufferWidth = 800;// Taille en x du Back Buffer
	pp.BackBufferHeight = 600; // Taille en y du Back Buffer
	pp.BackBufferFormat = displayMode.Format; // Format du Back Buffer
	pp.BackBufferCount = 1; // Nombre de Back Buffer
	pp.MultiSampleType = D3DMULTISAMPLE_NONE; // Nombre de sample pour l’antialiasing
	pp.MultiSampleQuality = 0; // Qualité pour l’antialiasing
	pp.hDeviceWindow = hWnd; //Handle de la fenêtre
	pp.EnableAutoDepthStencil = true; // True si on veut un depth-stencil buffer
	pp.AutoDepthStencilFormat = D3DFMT_D24S8; // Le format du deth-stencil buffer
	pp.Flags = 0; // Voir le man
	pp.FullScreen_RefreshRateInHz = 0; //Voir le man
	pp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT; // Autrement dit 0, voir le man

	IDirect3DDevice9 *device;
	pD3D->CreateDevice(0, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &pp, &device);

	/*
	// Nvidia debug
	D3DDEVTYPE DeviceType = D3DDEVTYPE_HAL;
	int num_card = 0;
	// Look for 'NVIDIA PerfHUD' adapter
	// If it is present, override default settings
	for (unsigned int adapter = 0; adapter < pD3D->GetAdapterCount();
	adapter++)
	{
	D3DADAPTER_IDENTIFIER9 identifier;
	HRESULT Res;
	Res = pD3D->GetAdapterIdentifier(adapter, 0, &identifier);
	if (strstr(identifier.Description, "PerfHUD") != 0)
	{
	num_card = adapter;
	DeviceType = D3DDEVTYPE_REF;
	break;
	}
	}
	*/

	//Create and fill other DirectX Stuffs like Vertex/Index buffer, shaders  

	// Vertex declaration
	D3DVERTEXELEMENT9 dwDecl3 [] =
	{
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		{ 0, 20, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		D3DDECL_END()
	};

	IDirect3DVertexDeclaration9 *pDecl;
	device->CreateVertexDeclaration(dwDecl3, &pDecl);

	// Culling ?
	//device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	// Load texture
	LPCWSTR pTextureFile = L"../Resources/terraintexture.jpg";

	LPDIRECT3DTEXTURE9 pTexture;
	D3DXCreateTextureFromFile(device, pTextureFile, &pTexture);

	/** Vertex & Index buffers (Heightmap) **/
	// Load height map
	LoadRAW("../Resources/terrainheight.raw");

	// Vertex buffer
	IDirect3DVertexBuffer9* pMapVertexBuffer;
	device->CreateVertexBuffer((m_sizeX * m_sizeZ) * sizeof(Vertex), 0, 0, D3DPOOL_DEFAULT, &pMapVertexBuffer, NULL);

	Vertex* pMapVertexData;

	pMapVertexBuffer->Lock(0, 0, (void**) &pMapVertexData, 0);

	unsigned int i = 0;
	for (int z = 0; z < m_sizeX; z++)
	{
		for (int x = 0; x < m_sizeZ; x++)
		{
			i = z + (m_sizeX * x);
			float heightValue = m_height[z + (m_sizeX * x)];

			pMapVertexData[i].Position = D3DXVECTOR3(z, heightValue, x);
			pMapVertexData[i].TextCoord = D3DXVECTOR2(((float) z / (float) m_sizeX), 1 - ((float) x / (float) m_sizeZ));

			const D3DXVECTOR3& pos = pMapVertexData[i].Position;
			pMapVertexData[i].Normal.x = 0.0f;
			pMapVertexData[i].Normal.y = 0.0f;
			pMapVertexData[i].Normal.z = 0.0f;
			D3DXVECTOR3 normal;

			float diffHeight;
			if (z > 0)
			{
				if (z + 1 < m_sizeZ)
					diffHeight = m_height[i - m_sizeX] - m_height[i + m_sizeX];
				else
					diffHeight = m_height[i - m_sizeX] - m_height[i];
			}
			else
				diffHeight = m_height[i] - m_height[i + m_sizeX];

			D3DXVECTOR3 normalizedVector;
			D3DXVec3Normalize(&normalizedVector, &D3DXVECTOR3(0.0f, 1.0f, diffHeight));
			pMapVertexData[i].Normal += normalizedVector;
			if (x > 0)
			{
				if (x + 1 < m_sizeX)
					diffHeight = m_height[i - 1] - m_height[i + 1];
				else
					diffHeight = m_height[i - 1] - m_height[i];
			}
			else
				diffHeight = m_height[i] - m_height[i + 1];

			D3DXVec3Normalize(&normalizedVector, &D3DXVECTOR3(diffHeight, 1.0f, 0.0f));
			pMapVertexData[i].Normal += normalizedVector;
			D3DXVec3Normalize(&pMapVertexData[i].Normal, &pMapVertexData[i].Normal);
		}
	}

	pMapVertexBuffer->Unlock();

	// Index buffer
	IDirect3DIndexBuffer9* pMapIndexBuffer;
	device->CreateIndexBuffer(6 * (m_sizeX * m_sizeZ) * sizeof(int), 0, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &pMapIndexBuffer, NULL);

	int* pMapIndexData;
	pMapIndexBuffer->Lock(0, 0, (void**) &pMapIndexData, 0);


	int counter = 0;
	for (int y = 0; y < m_sizeZ - 1; y++)
	{
		for (int x = 0; x < m_sizeX - 1; x++)
		{
			// First triangle
			pMapIndexData[counter] = x + (m_sizeX * (y + 1));
			counter++;
			pMapIndexData[counter] = (x + 1) + (m_sizeX * y);
			counter++;
			pMapIndexData[counter] = x + (m_sizeX * y);
			counter++;

			// Second triangle
			pMapIndexData[counter] = (x + 1) + (m_sizeX * (y + 1));
			counter++;
			pMapIndexData[counter] = (x + 1) + (m_sizeX * y);
			counter++;
			pMapIndexData[counter] = x + (m_sizeX * (y + 1));
			counter++;
		}
	}

	pMapIndexBuffer->Unlock();

	/** Vertex & Index buffers (Water) **/
	// Load texture
	LPCWSTR pWaterTextureFile = L"../Resources/water.dds";

	LPDIRECT3DTEXTURE9 pWaterTexture;
	LPDIRECT3DTEXTURE9 pRefractionTexture;
	LPDIRECT3DTEXTURE9 pReflectionTexture;
	D3DXCreateTextureFromFile(device, pWaterTextureFile, &pWaterTexture);

	// Vertex buffer
	// Water height
	int waterHeight = 20;
	// Setup a clipping plane based on the height of the water to clip everything above it.
	D3DXVECTOR4 clipPlane = D3DXVECTOR4(0.0f, -1.0f, 0.0f, waterHeight + 0.1f);
	IDirect3DVertexBuffer9* pWaterVertexBuffer;
	device->CreateVertexBuffer((m_sizeX * m_sizeZ) * sizeof(Vertex), 0, 0, D3DPOOL_DEFAULT, &pWaterVertexBuffer, NULL);

	Vertex* pWaterVertexData;

	pWaterVertexBuffer->Lock(0, 0, (void**) &pWaterVertexData, 0);

	i = 0;
	for (int z = 0; z < m_sizeX; z++)
	{
		for (int x = 0; x < m_sizeZ; x++)
		{
			i = z + (m_sizeX * x);
			float heightValue = m_height[z + (m_sizeX * x)];

			pWaterVertexData[i].Position = D3DXVECTOR3(z, waterHeight, x);
			pWaterVertexData[i].TextCoord = D3DXVECTOR2(((float) z / (float) m_sizeX), 1 - ((float) x / (float) m_sizeZ));

			// Normals
			pWaterVertexData[i].Normal.x = 0.0f;
			pWaterVertexData[i].Normal.y = 0.0f;
			pWaterVertexData[i].Normal.z = 0.0f;
		}
	}

	pWaterVertexBuffer->Unlock();

	// Index buffer
	IDirect3DIndexBuffer9* pWaterIndexBuffer;
	device->CreateIndexBuffer(6 * (m_sizeX * m_sizeZ) * sizeof(int), 0, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &pWaterIndexBuffer, NULL);

	int* pWaterIndexData;
	pWaterIndexBuffer->Lock(0, 0, (void**) &pWaterIndexData, 0);


	counter = 0;
	for (int y = 0; y < m_sizeZ - 1; y++)
	{
		for (int x = 0; x < m_sizeX - 1; x++)
		{
			// First triangle
			pWaterIndexData[counter] = x + (m_sizeX * (y + 1));
			counter++;
			pWaterIndexData[counter] = (x + 1) + (m_sizeX * y);
			counter++;
			pWaterIndexData[counter] = x + (m_sizeX * y);
			counter++;

			// Second triangle
			pWaterIndexData[counter] = (x + 1) + (m_sizeX * (y + 1));
			counter++;
			pWaterIndexData[counter] = (x + 1) + (m_sizeX * y);
			counter++;
			pWaterIndexData[counter] = x + (m_sizeX * (y + 1));
			counter++;
		}
	}

	pWaterIndexBuffer->Unlock();

	// Classic shader
	LPCWSTR pFxFile = L"../Resources/Shaders/classic.fx";
	LPD3DXEFFECT pEffect;
	LPD3DXBUFFER CompilationErrors;

	if (D3D_OK != D3DXCreateEffectFromFile(device, pFxFile, NULL, NULL, 0, NULL, &pEffect, &CompilationErrors))
	{
		MessageBoxA(NULL, (char *)
			CompilationErrors->GetBufferPointer(), "Error", 0);
	}

	D3DXHANDLE hWorldMatrix = pEffect->GetParameterByName(NULL, "WorldMatrix");
	D3DXHANDLE hViewMatrix = pEffect->GetParameterByName(NULL, "ViewMatrix");
	D3DXHANDLE hProjectionMatrix = pEffect->GetParameterByName(NULL, "ProjectionMatrix");
	D3DXHANDLE hTexture = pEffect->GetParameterByName(NULL, "Texture");

	// Shader with lights
	LPCWSTR pFxFileLights = L"../Resources/Shaders/lights.fx";
	LPD3DXEFFECT pEffectLights;

	if (D3D_OK != D3DXCreateEffectFromFile(device, pFxFileLights, NULL, NULL, 0, NULL, &pEffectLights, &CompilationErrors))
	{
		MessageBoxA(NULL, (char *)
			CompilationErrors->GetBufferPointer(), "Error", 0);
	}

	D3DXHANDLE hWorldMatrixLights = pEffectLights->GetParameterByName(NULL, "WorldMatrix");
	D3DXHANDLE hViewMatrixLights = pEffectLights->GetParameterByName(NULL, "ViewMatrix");
	D3DXHANDLE hProjectionMatrixLights = pEffectLights->GetParameterByName(NULL, "ProjectionMatrix");
	D3DXHANDLE hShaderTextureLights = pEffectLights->GetParameterByName(NULL, "ShaderTexture");
	D3DXHANDLE hAmbientColorLights = pEffectLights->GetParameterByName(NULL, "AmbientColor");
	D3DXHANDLE hDiffuseColorLights = pEffectLights->GetParameterByName(NULL, "DiffuseColor");
	D3DXHANDLE hLightDirectionLights = pEffectLights->GetParameterByName(NULL, "LightDirection");
	D3DXHANDLE hCameraPositionLights = pEffectLights->GetParameterByName(NULL, "CameraPosition");

	// Shader for refraction
	LPCWSTR pFxFileRefraction = L"../Resources/Shaders/refraction.fx";
	LPD3DXEFFECT pEffectRefraction;

	if (D3D_OK != D3DXCreateEffectFromFile(device, pFxFileRefraction, NULL, NULL, 0, NULL, &pEffectRefraction, &CompilationErrors))
	{
		MessageBoxA(NULL, (char *)
			CompilationErrors->GetBufferPointer(), "Error", 0);
	}

	D3DXHANDLE hWorldMatrixRefraction = pEffectRefraction->GetParameterByName(NULL, "WorldMatrix");
	D3DXHANDLE hViewMatrixRefraction = pEffectRefraction->GetParameterByName(NULL, "ViewMatrix");
	D3DXHANDLE hProjectionMatrixRefraction = pEffectRefraction->GetParameterByName(NULL, "ProjectionMatrix");
	D3DXHANDLE hShaderTextureRefraction = pEffectRefraction->GetParameterByName(NULL, "ShaderTexture");
	D3DXHANDLE hAmbientColorRefraction = pEffectRefraction->GetParameterByName(NULL, "AmbientColor");
	D3DXHANDLE hDiffuseColorRefraction = pEffectRefraction->GetParameterByName(NULL, "DiffuseColor");
	D3DXHANDLE hLightDirectionRefraction = pEffectRefraction->GetParameterByName(NULL, "LightDirection");
	D3DXHANDLE hClipPlaneRefraction = pEffectRefraction->GetParameterByName(NULL, "ClipPlane");

	// Shader for water (reflection)
	LPCWSTR pFxFileWater = L"../Resources/Shaders/water.fx";
	LPD3DXEFFECT pEffectWater;

	if (D3D_OK != D3DXCreateEffectFromFile(device, pFxFileWater, NULL, NULL, 0, NULL, &pEffectWater, &CompilationErrors))
	{
		MessageBoxA(NULL, (char *)
			CompilationErrors->GetBufferPointer(), "Error", 0);
	}

	D3DXHANDLE hWorldMatrixWater = pEffectWater->GetParameterByName(NULL, "WorldMatrix");
	D3DXHANDLE hViewMatrixWater = pEffectWater->GetParameterByName(NULL, "ViewMatrix");
	D3DXHANDLE hProjectionMatrixWater = pEffectWater->GetParameterByName(NULL, "ProjectionMatrix");
	D3DXHANDLE hWorldViewProjMatrixWater = pEffectWater->GetParameterByName(NULL, "WorldViewProjMatrix");
	D3DXHANDLE hReflectionMatrixWater = pEffectWater->GetParameterByName(NULL, "ReflectionMatrix");
	D3DXHANDLE hTextureNormalWater = pEffectWater->GetParameterByName(NULL, "NormalTexture");
	D3DXHANDLE hTextureReflectedWater = pEffectWater->GetParameterByName(NULL, "ReflectedTexture");

	PeekMessage(&oMsg, NULL, 0, 0, PM_NOREMOVE);
	while (oMsg.message != WM_QUIT)
	{
		if (PeekMessage(&oMsg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&oMsg);
			DispatchMessage(&oMsg);
		}
		else
		{
			// Input management
			_inputManager->Manage();

			// Wireframe ?
			if (_inputManager->IsKeyPressed(DIK_F1))
			{
				if (fillMode == D3DFILL_SOLID)
					fillMode = D3DFILL_WIREFRAME;
				else
					fillMode = D3DFILL_SOLID;

				device->SetRenderState(D3DRS_FILLMODE, fillMode);
			}

			// Lights ?
			if (_inputManager->IsKeyPressed(DIK_F2))
			{
				enableLights = !enableLights;
			}

			// Move camera direction

			// Up
			if (_inputManager->IsKeyDone(DIK_UP))
			{
				if (Pitch < D3DX_PI / 2 - 0.1f)
					Pitch += 0.05f;
			}
			// Left
			if (_inputManager->IsKeyDone(DIK_LEFT))
			{
				Yaw -= 0.05f;
				if (Yaw < 0)
					Yaw = 2 * D3DX_PI;
			}
			// Down
			if (_inputManager->IsKeyDone(DIK_DOWN))
			{
				if (Pitch > -(D3DX_PI / 2) + 0.1f)
					Pitch -= 0.05f;
			}
			// Right
			if (_inputManager->IsKeyDone(DIK_RIGHT))
			{
				Yaw = fmod((Yaw + 0.05f), 2 * D3DX_PI);
			}

			CameraDirection.x = sin(Yaw) * cos(Pitch);
			CameraDirection.y = sin(Pitch);
			CameraDirection.z = cos(Yaw) * cos(Pitch);

			// Move camera position
			// Up
			if (_inputManager->IsKeyDone(DIK_W))
			{
				CameraPosition += CameraDirection * 1;
			}
			// Left
			if (_inputManager->IsKeyDone(DIK_A))
			{
				D3DXVECTOR3 orthogonaleDirection;
				D3DXVec3Cross(&orthogonaleDirection, &CameraDirection, &Up);
				CameraPosition += orthogonaleDirection * 1;
			}
			// Down
			if (_inputManager->IsKeyDone(DIK_S))
			{
				CameraPosition -= CameraDirection * 1;
			}
			// Right
			if (_inputManager->IsKeyDone(DIK_D))
			{
				D3DXVECTOR3 orthogonaleDirection;
				D3DXVec3Cross(&orthogonaleDirection, &CameraDirection, &Up);
				CameraPosition -= orthogonaleDirection * 1;
			}
			// Jump
			if (_inputManager->IsKeyDone(DIK_SPACE))
			{
				CameraPosition.y += 1;
			}
			// Crouch
			if (_inputManager->IsKeyDone(DIK_LCONTROL))
			{
				CameraPosition.y -= 1;
			}

			// Move the sun
			if (_inputManager->IsKeyDone(DIK_NUMPAD4))
			{
				lightDirection.x -= 0.1f;
			}
			if (_inputManager->IsKeyDone(DIK_NUMPAD6))
			{
				lightDirection.x += 0.1f;
			}
			if (_inputManager->IsKeyDone(DIK_NUMPAD5))
			{
				lightDirection.y -= 0.1f;
			}
			if (_inputManager->IsKeyDone(DIK_NUMPAD8))
			{
				lightDirection.y += 0.1f;
			}
			if (_inputManager->IsKeyDone(DIK_NUMPAD7))
			{
				lightDirection.z -= 0.1f;
			}
			if (_inputManager->IsKeyDone(DIK_NUMPAD9))
			{
				lightDirection.z += 0.1f;
			}

			// World computation
			D3DXVec3Normalize(&lightDirection, &lightDirection);

			At = CameraPosition + CameraDirection;

			D3DXMatrixLookAtLH(&View, &CameraPosition, &At, &Up);
			D3DXMatrixIdentity(&World);

			// Reflection matrix
			D3DXVECTOR3 CameraPositionReflected = CameraPosition;
			D3DXVECTOR3 CameraDirectionReflected = CameraDirection;
			CameraPositionReflected.y = -CameraPositionReflected.y + (waterHeight * 2);
			CameraDirectionReflected.y = -CameraDirectionReflected.y;
			D3DXVECTOR3 AtReflected = CameraPositionReflected + CameraDirectionReflected;
			D3DXVECTOR3 UpReflected(0, 1, 0);
			D3DXMatrixLookAtLH(&ReflectionMatrix, &CameraPositionReflected, &AtReflected, &Up);

			// Draw calls
			device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, backgroundColor, 1.0f, 0);
			

			// Set device vertex declaration
			device->SetVertexDeclaration(pDecl);

			unsigned int cPasses, iPass;

			/*
			LPDIRECT3DSURFACE9 savedRenderTarget;
			LPDIRECT3DSURFACE9 reflectionRenderTarget;

			// Create reflected texture
			device->CreateTexture(1024, 1024, 0, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pReflectionTexture, NULL);
			pReflectionTexture->GetSurfaceLevel(0, &reflectionRenderTarget);
			device->GetRenderTarget(0, &savedRenderTarget);

			device->SetRenderTarget(0, reflectionRenderTarget);
			//device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, backgroundColor, 1.0f, 0);

			// "Send" WorldViewProj matrix to shader
			pEffect->SetMatrix(hWorldViewProj, &WorldViewProjReflected);

			// Set texture
			pEffect->SetTexture(hTexture, pTexture);
			pEffect->SetVector(hCameraDirection, new D3DXVECTOR4(CameraDirectionReflected, 0));

			// Draw map
			device->SetStreamSource(0, pMapVertexBuffer, 0, sizeof(Vertex));
			device->SetIndices(pMapIndexBuffer);

			cPasses = 0, iPass = 0;
			pEffect->Begin(&cPasses, 0);
			for (iPass = 0; iPass < cPasses; ++iPass)
			{
			pEffect->BeginPass(iPass);
			pEffect->CommitChanges();

			device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 2 * (m_sizeX * m_sizeZ), 0, 2 * ((m_sizeX * m_sizeZ) - 1));

			pEffect->EndPass();
			}

			pEffect->End();

			device->SetRenderTarget(0, savedRenderTarget);


			//void *pContainer;
			//HRESULT hr = reflectionRenderTarget->GetContainer(IID_IDirect3DTexture9, &pContainer);
			//if (SUCCEEDED(hr) && pContainer)
			//{
			//	pReflectionTexture = (IDirect3DTexture9 *) pContainer;
			//}


			// Draw Water
			pEffectWater->SetMatrix(hWorldMatrixWater, &World);
			pEffectWater->SetMatrix(hViewMatrixWater, &View);
			pEffectWater->SetMatrix(hProjectionMatrixWater, &Projection);
			pEffectWater->SetMatrix(hReflectionMatrixWater, &ReflectionMatrix);
			pEffectWater->SetTexture(hTexture, pWaterTexture);
			pEffectWater->SetTexture(hTextureReflectedWater, pReflectionTexture);

			device->SetStreamSource(0, pWaterVertexBuffer, 0, sizeof(Vertex));
			device->SetIndices(pWaterIndexBuffer);

			cPasses = 0, iPass = 0;
			pEffectWater->Begin(&cPasses, 0);
			for (iPass = 0; iPass < cPasses; ++iPass)
			{
			pEffectWater->BeginPass(iPass);
			pEffectWater->CommitChanges();

			device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 2 * (m_sizeX * m_sizeZ), 0, 2 * ((m_sizeX * m_sizeZ) - 1));

			pEffectWater->EndPass();
			}

			pEffectWater->End();
			*/
			
			LPDIRECT3DSURFACE9 savedRenderTarget = NULL;
			LPDIRECT3DSURFACE9 refractionRenderTarget = NULL;
			
			// Create refraction texture
			/*
			device->CreateTexture(256, 256, 1, D3DUSAGE_RENDERTARGET, 
				D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &pRefractionTexture, NULL);

			pRefractionTexture->GetSurfaceLevel(0, &refractionRenderTarget);
			device->GetRenderTarget(0, &savedRenderTarget);

			device->SetRenderTarget(0, refractionRenderTarget);
			refractionRenderTarget->Release();
			*/
			device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, backgroundColor, 1.0f, 0);
			device->BeginScene();


			/*
			// Set shader parameters

			// World, View and Projection matrix
			pEffect->SetMatrix(hWorldMatrixLights, &World);
			pEffect->SetMatrix(hViewMatrixLights, &View);
			pEffect->SetMatrix(hProjectionMatrixLights, &Projection);

			// Set texture
			pEffect->SetTexture(hTexture, pTexture);

			// Draw map
			device->SetStreamSource(0, pMapVertexBuffer, 0, sizeof(Vertex));
			device->SetIndices(pMapIndexBuffer);


			cPasses = 0, iPass = 0;
			pEffect->Begin(&cPasses, 0);
			for (iPass = 0; iPass < cPasses; ++iPass)
			{
				pEffect->BeginPass(iPass);
				pEffect->CommitChanges();

				device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 2 * (m_sizeX * m_sizeZ), 0, 2 * ((m_sizeX * m_sizeZ) - 1));

				pEffect->EndPass();
			}
			pEffect->End();

			device->EndScene();

			device->SetRenderTarget(0, savedRenderTarget);
			savedRenderTarget->Release();

			device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, backgroundColor, 1.0f, 0);
			device->BeginScene();

			// Draw water with refraction
			pEffectRefraction->SetMatrix(hWorldMatrixRefraction, &World);
			pEffectRefraction->SetMatrix(hViewMatrixRefraction, &View);
			pEffectRefraction->SetMatrix(hProjectionMatrixRefraction, &Projection);
			pEffectRefraction->SetTexture(hShaderTextureRefraction, pRefractionTexture);
			pEffectRefraction->SetVector(hAmbientColorRefraction, &ambientColor);
			pEffectRefraction->SetVector(hDiffuseColorRefraction, &diffuseColor);
			pEffectRefraction->SetFloatArray(hLightDirectionRefraction, (float*) &lightDirection, 3);
			pEffectRefraction->SetVector(hClipPlaneRefraction, &clipPlane);

			device->SetStreamSource(0, pWaterVertexBuffer, 0, sizeof(Vertex));
			device->SetIndices(pWaterIndexBuffer);

			cPasses = 0, iPass = 0;
			pEffectRefraction->Begin(&cPasses, 0);
			for (iPass = 0; iPass < cPasses; ++iPass)
			{
				pEffectRefraction->BeginPass(iPass);
				pEffectRefraction->CommitChanges();

				device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 2 * (m_sizeX * m_sizeZ), 0, 2 * ((m_sizeX * m_sizeZ) - 1));

				pEffectRefraction->EndPass();
			}

			pEffectRefraction->End();
			*/




			// Draw terrain without lights
			if (!enableLights)
			{
				// Set shader parameters

				// World, View and Projection matrix
				pEffect->SetMatrix(hWorldMatrix, &World);
				pEffect->SetMatrix(hViewMatrix, &View);
				pEffect->SetMatrix(hProjectionMatrix, &Projection);

				// Set texture
				pEffect->SetTexture(hTexture, pTexture);

				// Draw map
				device->SetStreamSource(0, pMapVertexBuffer, 0, sizeof(Vertex));
				device->SetIndices(pMapIndexBuffer);

				cPasses = 0, iPass = 0;
				pEffect->Begin(&cPasses, 0);
				for (iPass = 0; iPass < cPasses; ++iPass)
				{
					pEffect->BeginPass(iPass);
					pEffect->CommitChanges();

					device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 2 * (m_sizeX * m_sizeZ), 0, 2 * ((m_sizeX * m_sizeZ) - 1));

					pEffect->EndPass();
				}

				pEffect->End();
			}
			// Draw with lights
			else
			{
				// Set shader parameters

				// World, View and Projection matrix
				pEffectLights->SetMatrix(hWorldMatrixLights, &World);
				pEffectLights->SetMatrix(hViewMatrixLights, &View);
				pEffectLights->SetMatrix(hProjectionMatrixLights, &Projection);

				pEffectLights->SetTexture(hShaderTextureLights, pTexture);
				pEffectLights->SetVector(hAmbientColorLights, &ambientColor);
				pEffectLights->SetVector(hDiffuseColorLights, &diffuseColor);
				pEffectLights->SetFloatArray(hLightDirectionLights, (float*) &lightDirection, 3);
				
				pEffectLights->SetFloatArray(hCameraPositionLights, (float*) &CameraPosition, 3);

				// Draw map
				device->SetStreamSource(0, pMapVertexBuffer, 0, sizeof(Vertex));
				device->SetIndices(pMapIndexBuffer);

				cPasses = 0, iPass = 0;
				pEffectLights->Begin(&cPasses, 0);
				for (iPass = 0; iPass < cPasses; ++iPass)
				{
					pEffectLights->BeginPass(iPass);
					pEffectLights->CommitChanges();

					device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 2 * (m_sizeX * m_sizeZ), 0, 2 * ((m_sizeX * m_sizeZ) - 1));

					pEffectLights->EndPass();
				}

				pEffectLights->End();
			}

			device->EndScene();
			device->Present(NULL, NULL, NULL, NULL);
		}
	}

	//Release D3D objectssss
	pD3D->Release();
	device->Release();
	pDecl->Release();
	pMapVertexBuffer->Release();
	pMapIndexBuffer->Release();
	pTexture->Release();
	pEffect->Release();
	_inputManager->Destroy();

	return (int) oMsg.wParam;
}


bool CreateWindows(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	//
	WNDCLASSEX wcex;
	memset(&wcex, 0, sizeof(WNDCLASSEX));
	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wcex.lpszClassName = L"3DTPClassName";

	if (RegisterClassEx(&wcex) == 0)
		return false;

	hWnd = CreateWindow(L"3DTPClassName", L"This course is awesome", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
		return false;

	SetWindowPos(hWnd, NULL, 0, 0, 800, 600, 0);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return true;
}

bool CreateDevice()
{
	return true;
}

//
//  PURPOSE:  Processes messages for the main window.
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		break;
	case WM_KEYUP:
	{
					 switch (wParam)
					 {
					 case VK_ESCAPE:
					 {
									   PostQuitMessage(0);
									   break;
					 }
					 }
					 break;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}