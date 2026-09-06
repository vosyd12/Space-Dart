#ifndef CTEXTUREMANAGER_H
#define CTEXTUREMANAGER_H

#include <windows.h>
#include <vector>
#include <string>

#include <d3d9.h>
#include <d3dx9.h>

using namespace std;

struct TextureData
{
	wstring fileName;                // 파일명 key
	wstring fullPath;                // 전체 경로
	LPDIRECT3DTEXTURE9 texture;      // 실제 DX Texture
};

class CTextureManager
{
	private:
		LPDIRECT3DDEVICE9 m_device;
		// 모든 텍스처 저장
		vector<TextureData> m_textures;
		// 폴더 재귀 탐색
		void ScanFolder(const wstring& folderPath);
		// 파일명 추출
		wstring ExtractFileName(const wstring& path);
		// 확장자 제거
		wstring RemoveExtension(const wstring& file);
		// 파일 확장자가 지원 대상인지 검사
		bool IsSupportedFile(const wstring& fileName) const;
		// 좌측 상단 컬러 읽어오기
		D3DCOLOR GetTopLeftColor(const wstring& fullPath); // 좌측 상단 컬러 읽어오기

	public:
		CTextureManager();
		~CTextureManager();

		// 초기화
		void Init(LPDIRECT3DDEVICE9 device);

		// 전체 폴더 검색
		void BuildTextureDatabase(const wstring& rootFolder);

		// 파일명 기반 로드
		//
		// 예:
		// Load("star");
		// Load("star.png");
		LPDIRECT3DTEXTURE9 LoadTexture(const wstring& fileName);

		// 텍스처 존재 여부
		bool HasTexture(const wstring& fileName);

		// 텍스처 찾기
		TextureData* FindTexture(const wstring& fileName);

		// 파일명 key로 실제 전체 경로 얻기
		// title.ico 같은 아이콘 파일을 LoadImageW로 로드할 때 사용
		wstring GetFullPath(const wstring& fileName);

		// 메모리 해제
		void Release();
};

#endif