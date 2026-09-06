# Space Dart

C++과 DirectX 9을 사용하여 직접 구현한 3D 다트 아케이드 게임입니다.

게임의 기본적인 렌더링부터 입력 처리, Scene 관리, 충돌 및 점수 계산,
컴퓨터 플레이어의 행동 로직, 우주 배경 렌더링 등을 직접 구현했습니다.

![Space Dart Gameplay](docs/images/gameplay.PNG)

---

## Development Environment

- Language: C++
- Graphics API: DirectX 9
- Platform: Windows
- IDE: Visual Studio

---

## Main Features

### 1. Scene Management

Title, Play, Setting 등의 화면을 Scene 단위로 분리하고,
Manager를 통해 Scene의 생성, 전환 및 실행 흐름을 관리했습니다.

![Space Dart Title](docs/images/title.PNG)

주요 코드:

- `Space Dart/CGameManager.cpp`
- `Space Dart/CScene.h`

---

### 2. Input & UI

WinAPI 입력을 프레임 단위로 처리하고,
이전 프레임의 입력 상태와 현재 입력 상태를 비교하여
한 번의 클릭이 여러 번 처리되는 문제를 해결했습니다.

또한 DirectX 환경에서 사용할 수 있도록 `CButton` 클래스를 별도로 구현하여
Hover, Pressed, Clicked 상태와 UI 렌더링을 관리했습니다.

![Space Dart Settings](docs/images/settings.PNG)

주요 코드:

- `Space Dart/KeyProc.cpp`
- `Space Dart/CButton.cpp`

---

### 3. Computer Player

단순한 랜덤 행동이 아니라 현재 게임 상황을 기준으로
목표와 행동을 결정하는 규칙 기반 컴퓨터 플레이어를 구현했습니다.

주요 판단 요소:

- 난이도에 따른 명중 오차 조절
- 현재 필요한 점수에 따른 목표 위치 결정
- 아이템 보유 및 획득 여부 판단
- 아이템 효과에 따른 행동 변경
- 움직이는 다트판의 현재 위치와 이동 방향을 고려한 목표 보정
- 게임 진행 상황에 따른 행동 전략 변경

![Computer Player](docs/images/computer-player.PNG)

주요 코드:

- `Space Dart/CThink.cpp`
- `Space Dart/CThink_Target.cpp`
- `Space Dart/CThink_Item.cpp`
- `Space Dart/CThink_Difficulty.cpp`

---

### 4. Space Rendering

단순한 배경 텍스처만 사용하는 대신,
공간감을 표현하기 위해 별과 유성 등의 우주 배경 요소를 직접 구현했습니다.

별은 거리에 따라 크기와 밝기, 반짝임에 차이를 두었으며,
유성은 카메라와의 거리를 기준으로 속도, 크기, 꼬리 길이와 밝기가
달라지도록 구현했습니다.

![Space Rendering](docs/images/space-rendering.PNG)

주요 코드:

- `Space Dart/CSpaceBackground.cpp`
- `Space Dart/CSpaceRenderer.cpp`

---

### 5. Collision & Score

다트의 오브젝트 위치가 아닌 실제 다트의 핀이 다트판과 접촉하는 위치를 기준으로
충돌 영역과 점수를 계산하도록 구현했습니다.

개발 과정에서 다트가 다트판에 정상적으로 충돌했음에도
실제 접촉 위치와 다른 점수가 계산되는 문제가 있었습니다.

점수 계산 자체보다 충돌에 사용하는 좌표를 확인한 결과,
다트 오브젝트의 위치와 실제 핀의 접촉 위치가 일치하지 않는 것이 원인이었습니다.

이를 해결하기 위해 충돌 기준을 다트의 핀 위치로 변경하고,
여러 영역에 다트를 충돌시키며 실제 위치와 계산된 점수가 일치하는지 확인했습니다.

![Collision and Score](docs/images/collision-score.PNG)

주요 코드:

- `Space Dart/CCollision.cpp`
- `Space Dart/CDart.cpp`

---

### 6. Item System

게임 진행 중 사용할 수 있는 아이템을 구현하고,
아이템의 종류에 따라 게임 플레이에 서로 다른 효과가 적용되도록 구성했습니다.

플레이어뿐만 아니라 컴퓨터 플레이어도 현재 게임 상황과
아이템 보유 상태를 판단하여 아이템 사용 여부를 결정하도록 구현했습니다.

![Item Effect](docs/images/item-effect.PNG)

---

## Gameplay

Space Dart는 일반적인 다트 규칙을 기반으로 하면서
다트판의 이동과 아이템 등의 요소를 추가한 3D 아케이드 다트 게임입니다.

게임 상황에 따라 다트판과의 거리가 변화하거나,
아이템 효과가 적용되면서 목표 지점과 플레이 방식이 달라집니다.

![Space Dart Gameplay](docs/images/gameplay.PNG)

---

## What I Learned

Space Dart를 개발하면서 C++ 문법을 사용하는 것에 그치지 않고,
게임에 필요한 기능을 역할에 따라 나누고 서로 연결하는 과정을 경험했습니다.

Scene 관리, 입력 처리, UI, 충돌 판정, 컴퓨터 플레이어의 행동 로직,
공간 렌더링 등을 직접 구현하면서 하나의 기능을 만드는 것뿐만 아니라
문제가 발생했을 때 원인을 확인하고 수정한 뒤 실제 플레이를 통해
결과를 검증하는 과정을 반복했습니다.

특히 처음 접하는 기능을 구현할 때 완성된 결과를 한 번에 만들기보다,
필요한 요소를 작은 단위로 나누어 구현하고 결과를 확인하면서
점차 범위를 확장하는 방식으로 문제를 해결했습니다.
