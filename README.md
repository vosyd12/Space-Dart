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
`CGameManager`를 통해 Scene의 생성, 전환 및 실행 흐름을 관리했습니다.

게임 진행 중 Setting Scene으로 이동하는 경우에는
현재 게임 Scene을 보관하고 다시 복귀했을 때 기존 상태를 이어갈 수 있도록 구성했습니다.

![Space Dart Title](docs/images/title.PNG)

주요 코드:

- `Space Dart/CGameManager.cpp`
- `Space Dart/CScene.h`

---

### 2. Input & UI

WinAPI 입력을 프레임 단위로 처리하고,
현재 프레임의 입력 상태와 이전 프레임의 입력 상태를 비교하여
Pressed와 Triggered 입력을 구분했습니다.

이를 통해 키나 마우스를 누르고 있는 동안
동일한 입력이 여러 프레임에 걸쳐 반복 처리되는 문제를 방지했습니다.

DirectX 환경에서 사용할 수 있도록 `CButton` 클래스를 별도로 구현하여
Hover, Pressed, Clicked 상태를 관리하고 버튼 렌더링을 처리했습니다.

![Space Dart Settings](docs/images/settings.PNG)

주요 코드:

- `Space Dart/KeyProc.cpp`
- `Space Dart/CButton.cpp`

---

### 3. Computer Player

단순한 랜덤 행동이 아니라
현재 게임 상황을 기준으로 목표와 행동을 결정하는 규칙 기반 컴퓨터 플레이어를 구현했습니다.

컴퓨터 플레이어는 다음과 같은 상태 흐름으로 행동합니다.

`IDLE → THINK → AIM → THROW → WAIT`

목표를 선택할 때는 여러 후보를 평가하고,
각 후보에 대해 다음과 같은 요소를 고려하도록 구성했습니다.

- 현재 필요한 점수
- 마무리 가능성
- 안전성
- 위험도
- 최종 목표 가치
- 보유 아이템과 아이템 효과
- 필드에 존재하는 아이템 획득 여부
- 난이도와 게임 진행 상황

움직이는 다트판이나 목표를 상대할 때는
현재 위치만을 기준으로 하지 않고 이동 방향과 위치 변화를 바탕으로
목표가 이후 이동할 위치를 예측하도록 구현했습니다.

또한 현재 목표에 도달할 수 있는지를 실시간으로 다시 판단하고,
상황에 따라 목표를 변경할 수 있도록 구성했습니다.

난이도가 높아질수록 투척 오차 범위와 행동 기준을 다르게 적용하여
컴퓨터 플레이어의 정확도와 전략이 변화하도록 구현했습니다.

![Computer Player](docs/images/computer-player.PNG)

주요 코드:

- `Space Dart/CThink.cpp`
- `Space Dart/CThink_Target.cpp`
- `Space Dart/CThink_Risk.cpp`
- `Space Dart/CThink_Item.cpp`
- `Space Dart/CThink_Difficulty.cpp`
- `Space Dart/CThink_Cube.cpp`

---

### 4. Space Rendering

단순한 배경 텍스처만 사용하는 대신,
공간감을 표현하기 위해 별과 유성 등의 우주 배경 요소를 직접 구현했습니다.

별은 개별적으로 밝기와 반짝임 속도, 반짝임 강도를 관리하고,
거리와 배치에 따라 크기와 밝기가 달라지도록 구성했습니다.

유성은 카메라와의 거리를 비율값으로 변환하여
가까운 유성일수록 크기, 속도, 밝기와 꼬리 길이가 달라지도록 구현했습니다.

이를 통해 동일한 형태의 오브젝트를 단순 반복 배치하는 대신,
거리와 위치에 따라 서로 다른 깊이감이 느껴지도록 구성했습니다.

![Space Rendering](docs/images/space-rendering.PNG)

주요 코드:

- `Space Dart/CSpaceBackground.cpp`
- `Space Dart/CSpaceRenderer.cpp`

---

### 5. Collision & Score

다트의 충돌 위치와 다트판 중심 사이의 거리와 각도를 계산하여
Bull, Single, Double, Triple 영역을 판정하고 점수를 계산하도록 구현했습니다.

개발 과정에서는 다트가 시각적으로 특정 영역에 충돌했지만
의도한 점수와 다른 결과가 계산되는 문제가 발생했습니다.

관련 좌표와 계산 흐름을 확인한 결과,
다트 오브젝트의 기준 위치와 실제 다트판에 접촉하는 위치 사이에 차이가 있었습니다.

충돌 기준 좌표를 다시 조정하고,
여러 점수 영역에 직접 다트를 충돌시키며
실제 위치와 계산된 점수가 일치하는지 확인했습니다.

![Collision and Score](docs/images/collision-score.PNG)

주요 코드:

- `Space Dart/CCollision.cpp`
- `Space Dart/CDart.cpp`

---

### 6. Item System

게임 진행 중 사용할 수 있는 다양한 아이템을 구현하고,
아이템의 종류에 따라 서로 다른 효과가 적용되도록 구성했습니다.

플레이어는 상황에 따라 아이템을 선택하고 사용할 수 있으며,
컴퓨터 플레이어 역시 현재 게임 상황과 보유 아이템을 확인하여
아이템을 사용할지, 새로운 아이템을 획득할지를 판단하도록 구현했습니다.

화면에 배치된 컬러 오브젝트는 획득 가능한 아이템 박스이며,
게임 진행 중 다트의 이동 경로나 행동 선택에 영향을 줄 수 있습니다.

![Item Effect](docs/images/item-effect.PNG)

---

## Gameplay

Space Dart는 일반적인 다트 규칙을 기반으로 하면서
움직이는 다트판과 아이템 등의 요소를 추가한 3D 아케이드 다트 게임입니다.

게임 상황에 따라 다트판의 위치와 거리가 변화하고,
아이템 효과가 적용되면서 목표 지점과 플레이 방식이 달라질 수 있습니다.

플레이어는 현재 점수와 남은 기회, 아이템 상태 등을 고려하여
다트를 투척하고 목표 점수를 만들어야 합니다.

![Space Dart Gameplay](docs/images/gameplay.PNG)

---

## Source Code Guide

| System | Main Source |
| --- | --- |
| Scene Management | `Space Dart/CGameManager.cpp`, `Space Dart/CScene.h` |
| Input / UI | `Space Dart/KeyProc.cpp`, `Space Dart/CButton.cpp` |
| Computer Player | `Space Dart/CThink.cpp`, `Space Dart/CThink_Target.cpp`, `Space Dart/CThink_Risk.cpp`, `Space Dart/CThink_Item.cpp`, `Space Dart/CThink_Difficulty.cpp` |
| Space Rendering | `Space Dart/CSpaceBackground.cpp`, `Space Dart/CSpaceRenderer.cpp` |
| Collision / Score | `Space Dart/CCollision.cpp`, `Space Dart/CDart.cpp` |

---

## What I Learned

Space Dart를 개발하면서 C++ 문법을 사용하는 것에 그치지 않고,
게임에 필요한 기능을 역할에 따라 나누고 서로 연결하여
실제 플레이 가능한 시스템으로 구현하는 경험을 쌓았습니다.

Scene 관리, 입력 처리, UI, 충돌 및 점수 계산,
컴퓨터 플레이어의 행동 로직, 우주 배경 렌더링 등을 직접 구현하면서
기능을 만드는 것뿐 아니라 예상과 다른 결과가 발생했을 때
관련 값과 코드의 흐름을 확인하며 원인을 찾고 수정하는 과정을 반복했습니다.

또한 처음 접하는 기능을 구현할 때
완성된 결과를 한 번에 만들기보다
필요한 요소를 작은 단위로 나누고 하나씩 구현과 검증을 반복하면서
문제의 범위를 좁혀가는 방식으로 개발해왔습니다.
