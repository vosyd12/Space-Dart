# Space Dart

C++과 DirectX 9을 사용하여 직접 구현한 3D 다트 아케이드 게임입니다.

게임의 기본적인 렌더링부터 입력 처리, Scene 관리, 충돌 및 점수 계산,
컴퓨터 플레이어의 행동 로직, 우주 배경 렌더링 등을 직접 구현했습니다.

## Development Environment

- Language: C++
- Graphics API: DirectX 9
- Platform: Windows
- IDE: Visual Studio

## Main Features

### Scene Management
Title, Play, Setting 등의 화면을 Scene 단위로 분리하고,
Manager를 통해 Scene의 생성, 전환 및 실행 흐름을 관리했습니다.

### Input & UI
WinAPI 입력을 프레임 단위로 처리하고,
이전 프레임의 입력 상태와 비교하여 한 번의 클릭이 여러 번 처리되는 문제를 해결했습니다.

DirectX 환경에서 사용할 수 있도록 CButton 클래스를 별도로 구현하여
Hover, Pressed, Clicked 상태와 UI 렌더링을 관리했습니다.

### Computer Player
단순한 랜덤 행동이 아니라 현재 게임 상황을 기준으로 행동을 결정하도록 구현했습니다.

- 난이도에 따른 명중 오차 조절
- 현재 필요한 점수에 따른 목표 위치 결정
- 아이템 보유 및 획득 여부 판단
- 아이템 효과에 따른 행동 변경
- 움직이는 다트판의 현재 위치와 이동 방향을 고려한 목표 보정
- 게임 진행 상황에 따른 행동 전략 변경

### Space Rendering
단순한 배경 텍스처 대신 공간감을 표현하기 위해 별과 유성을 직접 구현했습니다.

별은 거리에 따라 크기와 밝기, 반짝임을 다르게 설정했으며,
유성은 카메라와의 거리를 기준으로 속도, 크기, 꼬리 길이와 밝기가
달라지도록 구현했습니다.

### Collision & Score
다트의 실제 충돌 위치를 기준으로 다트판의 영역과 점수를 계산하도록 구현했습니다.
