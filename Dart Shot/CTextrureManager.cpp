#include "CTextureManager.h"

CTextureManager::CTextureManager()
{
	m_device = nullptr;
}

CTextureManager::~CTextureManager()
{
	Release();
}

void CTextureManager::Init(LPDIRECT3DDEVICE9 device)
{
	m_device = device;
}

// 지원하는 파일 확장자인지 검사
// .ico도 등록은 하지만 Direct3D 텍스처로 로드하지 않고 경로 조회용으로 사용한다.
bool CTextureManager::IsSupportedFile(const wstring& fileName) const
{
	wstring lowerName = fileName;

	for (auto& ch : lowerName) { ch = towlower(ch); }

	if (lowerName.find(L".png") != wstring::npos) { return true; }
	if (lowerName.find(L".jpg") != wstring::npos) { return true; }
	if (lowerName.find(L".dds") != wstring::npos) { return true; }
	if (lowerName.find(L".bmp") != wstring::npos) { return true; }
	if (lowerName.find(L".tga") != wstring::npos) { return true; }

	// 실행파일/창 아이콘용
	if (lowerName.find(L".ico") != wstring::npos) { return true; }

	return false;
}

//폴더탐색
void CTextureManager::ScanFolder(const wstring& folderPath)
{
	wstring searchPath = folderPath + L"\\*";

	WIN32_FIND_DATAW findData;
	HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);

	if (hFind == INVALID_HANDLE_VALUE)
	{
		wstring msg = L"폴더 탐색 실패: " + searchPath;
		MessageBoxW(NULL, msg.c_str(), L"Texture Scan Error", MB_OK);
		return;
	}

	do
	{
		wstring name = findData.cFileName;

		if (name == L"." || name == L"..")
		{
			continue;
		}

		wstring fullPath = folderPath + L"\\" + name;

		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// 하위 폴더도 계속 탐색
			ScanFolder(fullPath);
		}

		else
		{
			// 지원 확장자만 등록
			if (IsSupportedFile(name))
			{
				TextureData textureData;

				// 확장자를 제거한 이름을 key로 사용
				// 예: title.ico -> title
				textureData.fileName = RemoveExtension(name);
				textureData.fullPath = fullPath;
				textureData.texture = nullptr;

				m_textures.push_back(textureData);
			}
		}

	} while (FindNextFileW(hFind, &findData));

	FindClose(hFind);
}

// 파일명 추출
wstring CTextureManager::ExtractFileName(const wstring& path)
{
	size_t pos = path.find_last_of(L"/\\");

	if (pos == wstring::npos)
	{
		return path;
	}

	return path.substr(pos + 1);
}

// 확장자 제거
wstring CTextureManager::RemoveExtension(const wstring& file)
{
	size_t pos = file.find_last_of(L'.');

	if (pos == wstring::npos)
	{
		return file;
	}

	return file.substr(0, pos);
}

// 텍스처 데이터베이스 구축
void CTextureManager::BuildTextureDatabase(const wstring& rootFolder)
{
	m_textures.clear();

	ScanFolder(rootFolder);

	// 디버깅용
	//wchar_t buf[128];
	//wsprintfW(buf, L"로드된 텍스처 수: %d", (int)m_textures.size());
	//MessageBoxW(NULL, buf, L"Texture DB", MB_OK);
}

// 텍스처 찾기
TextureData* CTextureManager::FindTexture(const wstring& fileName)
{
	wstring key = RemoveExtension(fileName);

	for (auto& texture : m_textures)
	{
		// 대소문자 무시 비교
		if (_wcsicmp(texture.fileName.c_str(), key.c_str()) == 0)
		{
			return &texture;
		}
	}

	return nullptr;
}

// 존재 여부
bool CTextureManager::HasTexture(const wstring& fileName)
{
	return FindTexture(fileName) != nullptr;
}

// 전체 경로 얻기
wstring CTextureManager::GetFullPath(const wstring& fileName)
{
	TextureData* textureData = FindTexture(fileName);

	if (!textureData)
	{
		return L"";
	}

	return textureData->fullPath;
}

LPDIRECT3DTEXTURE9 CTextureManager::LoadTexture(const wstring& fileName)
{
	TextureData* textureData = FindTexture(fileName);

	if (!textureData) { return nullptr; }
	if (textureData->texture) { return textureData->texture; }

	// 좌상단 픽셀 색상을 컬러키로 사용한다.
	// D3DXCreateTextureFromFileExW의 colorKey는
	// 이 색상과 완전히 같은 픽셀만 투명 처리한다.
	D3DCOLOR colorKey = GetTopLeftColor(textureData->fullPath);

	HRESULT hr = D3DXCreateTextureFromFileExW(
		m_device,
		textureData->fullPath.c_str(),
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		0,
		D3DFMT_A8R8G8B8,
		D3DPOOL_MANAGED,
		D3DX_FILTER_LINEAR,
		D3DX_FILTER_LINEAR,
		colorKey,
		nullptr,
		nullptr,
		&textureData->texture);

	if (FAILED(hr))
	{
		textureData->texture = nullptr;
		return nullptr;
	}

	return textureData->texture;
}

/*
// 텍스처 로드(좌상단의 컬러키를 기준으로 오차범위까지 같이 없애는 기능)
LPDIRECT3DTEXTURE9 CTextureManager::LoadTexture(const wstring& fileName)
{
	TextureData* textureData = FindTexture(fileName);

	if (!textureData) { return nullptr; }
	if (textureData->texture) { return textureData->texture; }

	D3DCOLOR colorKey = GetTopLeftColor(textureData->fullPath);

	HRESULT hr = D3DXCreateTextureFromFileExW(
		m_device,
		textureData->fullPath.c_str(),
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		0,
		D3DFMT_A8R8G8B8,
		D3DPOOL_MANAGED,
		D3DX_FILTER_LINEAR,
		D3DX_FILTER_LINEAR,
		colorKey,
		nullptr,
		nullptr,
		&textureData->texture);

	if (FAILED(hr))
	{
		textureData->texture = nullptr;
		return nullptr;
	}

		// 컬러키 오차 보정
		// D3DXCreateTextureFromFileExW의 colorKey는
		// "완전히 같은 RGB"만 제거한다.

		// 예:
		// 컬러키가 RGB(0,0,0)일 때
		// RGB(0,0,0)   -> 제거됨
		// RGB(1,0,0)   -> 제거 안 됨
		// RGB(5,3,2)   -> 제거 안 됨

		// 하지만 이미지에는 압축, 안티앨리어싱, 글로우, 필터링 때문에
		// 배경처럼 보이는 부분도 RGB(1,1,1), RGB(3,0,0)처럼
		// 아주 미세하게 다른 색으로 남는 경우가 많다.

		// 그래서 텍스처 생성 후 픽셀을 직접 검사해서
		// 컬러키와 "가까운 색"도 알파를 줄인다.

	if (textureData->texture)
	{
		D3DSURFACE_DESC desc;
		ZeroMemory(&desc, sizeof(desc));

		if (SUCCEEDED(textureData->texture->GetLevelDesc(0, &desc)))
		{
			D3DLOCKED_RECT locked;

			if (SUCCEEDED(textureData->texture->LockRect(0, &locked, nullptr, 0)))
			{
				// 컬러키 RGB 분리.
				// D3DCOLOR_XRGB는 내부적으로 0xFFRRGGBB 형태다.
				BYTE keyR = (BYTE)((colorKey >> 16) & 0xFF);
				BYTE keyG = (BYTE)((colorKey >> 8) & 0xFF);
				BYTE keyB = (BYTE)((colorKey >> 0) & 0xFF);

				// hardTolerance:
				// 컬러키와 이 정도로 가까우면 완전 투명 처리한다.

				// 검정 배경 기준 예:
				// RGB(0,0,0), RGB(5,3,4), RGB(10,8,6) 같은 색 제거.

				// 너무 크게 잡으면 검붉은 글로우까지 사라질 수 있으므로
				// 12~20 사이부터 조절하는 것을 추천한다.
				const int hardTolerance = 16;

				// softTolerance:
				// hardTolerance보다 조금 더 먼 색은 부드럽게 반투명 처리한다.

				// 예:
				// hardTolerance = 16
				// softTolerance = 36

				// diff가 16 이하     -> 알파 0
				// diff가 16~36 사이  -> 알파가 0에서 원래 알파까지 서서히 증가
				// diff가 36 초과     -> 원래 알파 유지
				const int softTolerance = 36;

				for (UINT y = 0; y < desc.Height; y++)
				{
					// Pitch:
					// 한 줄의 실제 바이트 크기.
					// 이미지 width * 4와 항상 같지 않을 수 있으므로 반드시 Pitch를 사용해야 한다.
					DWORD* row = (DWORD*)((BYTE*)locked.pBits + (y * locked.Pitch));

					for (UINT x = 0; x < desc.Width; x++)
					{
						DWORD pixel = row[x];

						BYTE b = (BYTE)((pixel >> 0) & 0xFF);
						BYTE g = (BYTE)((pixel >> 8) & 0xFF);
						BYTE r = (BYTE)((pixel >> 16) & 0xFF);
						BYTE a = (BYTE)((pixel >> 24) & 0xFF);

						// RGB 각각의 차이를 구한다.
						int diffR = abs((int)r - (int)keyR);
						int diffG = abs((int)g - (int)keyG);
						int diffB = abs((int)b - (int)keyB);

						// 가장 단순한 오차 판정:
						// R/G/B 각각이 기준색과 일정 범위 안에 있으면
						// 배경색 계열로 본다.

						// 예:
						// key = RGB(0,0,0), hardTolerance = 16
						// RGB(10,12,8)는 제거 대상.
						bool hardKey = (diffR <= hardTolerance && diffG <= hardTolerance && diffB <= hardTolerance);

						bool softKey = (diffR <= softTolerance && diffG <= softTolerance && diffB <= softTolerance);

						if (hardKey)
						{
							// 컬러키와 거의 같은 색.
							// 완전 투명 처리.
							a = 0;
						}

						else if (softKey)
						{
							// 컬러키와 조금 비슷한 색.
							// 완전히 지우면 이펙트 가장자리가 깨질 수 있으므로
							// 거리에 따라 알파를 서서히 복구한다.

							// maxDiff:
							// R/G/B 중 가장 큰 차이값.
							// 가장 많이 다른 채널을 기준으로
							// 얼마나 컬러키에서 멀어졌는지 판단한다.
							int maxDiff = diffR;
							if (diffG > maxDiff) { maxDiff = diffG; }
							if (diffB > maxDiff) { maxDiff = diffB; }

							float t = (float)(maxDiff - hardTolerance) / (float)(softTolerance - hardTolerance);

							if (t < 0.0f) { t = 0.0f; }
							if (t > 1.0f) { t = 1.0f; }

							// t = 0이면 거의 배경색이므로 알파 0.
							// t = 1이면 배경색과 충분히 멀어졌으므로 원래 알파 유지.
							int newAlpha = (int)((float)a * t);

							if (newAlpha < 0) { newAlpha = 0; }
							if (newAlpha > 255) { newAlpha = 255; }

							a = (BYTE)newAlpha;
						}

						// 수정한 알파를 다시 픽셀에 반영한다.
						// 색상 RGB는 손대지 않고 A만 변경한다.
						row[x] = ((DWORD)a << 24) | ((DWORD)r << 16) | ((DWORD)g << 8) | ((DWORD)b);
					}
				}

				textureData->texture->UnlockRect(0);
			}
		}
	}

	return textureData->texture;
}
*/

D3DCOLOR CTextureManager::GetTopLeftColor(const wstring& fullPath)
{
	D3DXIMAGE_INFO info;
	ZeroMemory(&info, sizeof(info));

	HRESULT hr = D3DXGetImageInfoFromFileW(fullPath.c_str(), &info);

	if (FAILED(hr)) { return D3DCOLOR_XRGB(255, 255, 255); }

	LPDIRECT3DSURFACE9 surface = nullptr;

	hr = m_device->CreateOffscreenPlainSurface(
		info.Width,
		info.Height,
		D3DFMT_A8R8G8B8,
		D3DPOOL_SCRATCH,
		&surface,
		nullptr);

	if (FAILED(hr)) { return D3DCOLOR_XRGB(255, 255, 255); }

	hr = D3DXLoadSurfaceFromFileW(
		surface,
		nullptr,
		nullptr,
		fullPath.c_str(),
		nullptr,
		D3DX_FILTER_NONE,
		0,
		nullptr);

	if (FAILED(hr))
	{
		surface->Release();
		return D3DCOLOR_XRGB(255, 255, 255);
	}

	D3DLOCKED_RECT locked;
	hr = surface->LockRect(&locked, nullptr, D3DLOCK_READONLY);

	if (FAILED(hr))
	{
		surface->Release();
		return D3DCOLOR_XRGB(255, 255, 255);
	}

	DWORD* pixels = (DWORD*)locked.pBits;
	DWORD topLeft = pixels[0];

	surface->UnlockRect();
	surface->Release();

	BYTE b = (BYTE)((topLeft >> 0) & 0xFF);
	BYTE g = (BYTE)((topLeft >> 8) & 0xFF);
	BYTE r = (BYTE)((topLeft >> 16) & 0xFF);

	return D3DCOLOR_XRGB(r, g, b);
}

// 메모리 헤제
void CTextureManager::Release()
{
	for (auto& texture : m_textures)
	{
		if (texture.texture)
		{
			texture.texture->Release();
			texture.texture = nullptr;
		}
	}

	m_textures.clear();
}