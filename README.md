# Project_X <img src="https://img.shields.io/badge/Solo-3c78d8?style=flat" align="absmiddle"/> <img src="https://img.shields.io/badge/Unreal%20Engine-0E1128?style=flat&logo=unrealengine&logoColor=white" align="absmiddle"/> <img src="https://img.shields.io/badge/C++-00599C?style=flat&logo=cplusplus&logoColor=white" align="absmiddle"/> <img src="https://img.shields.io/badge/GitHub-181717?style=flat&logo=github&logoColor=white" align="absmiddle"/>

<table>
  <tr>
    <td>
      <b><a id="index"></a> 목차</b><br>
      &nbsp;&nbsp;<a href="#introduction"> 1. Introduction </a><br>
      &nbsp;&nbsp;<a href="#technical-stacks"> 2. Technical Stacks </a><br>
      &nbsp;&nbsp;<a href="#projects"> 3. Projects </a><br>
      &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href="#project_hal"> 3-1. Project_HAL </a><br>
      &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href="#the-first-foreigner"> 3-2. The First Foreigner </a><br>
      &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href="#project_x"> 3-3. Project_X </a><br>
    </td>
  </tr>
</table>

# 1. 무기 시스템 구현
## 1-1. 기능 요약
- 무기 시스템을 컴포넌트로 설계해 캐릭터와 분리함으로써 캐릭터의 책임을 축소하고 무기 시스템과의 결합도를 낮췄습니다.   
- 무기 정의는 타입 객체 패턴을 적용해 클래스가 아닌 데이터로 관리했으며, 입력 처리 로직은 전략 패턴 기반으로 구성해 무기 종류에 의존하지 않는 실행 경로를 확보했습니다.   
- 이를 통해 분기 없는 입력 처리 구조를 구현하고, 무기 확장 시 코드 수정 범위를 최소화했습니다.   

## 1-2. 문제 상황
- 무기의 종류를 PX_Weapon을 상속 받아 새로운 클래스로 작성. 무기의 종류가 늘어날 수록 클래스의 수가 비례해 늘어났다. 또한 무기의 로직이 중복됨.
  
## 1-3. 설계 의도   
### 무기 시스템 구조   
  (코드 링크)   
- 무기 관련 로직을 컴포넌트로 분리하여 캐릭터는 무기를 사용한다는 의도로 설계
### 무기 정의 방식   
  (코드 링크)   
- 무기 종류를 클래스 상속이 아닌, 타입 객체 패턴을 적용해 데이터 기반으로 정의
### 입력 처리 방식   
  (코드 링크)   
- 입력은 명령 단위로 처리하고, 실제 동작은 무기에 구현을 위임
  
## 1-4. 대안 비교
## 1-4-1.무기 시스템 구조 
### 1. 캐릭터 내부에 직접 구현   
- 장점 : 빠르게 구현 가능   
- 단점 : 캐릭터 클래스가 비대해짐(God Object), 무기 로직 변경이 캐릭터 전반에 영향을 미침, 다른 액터(AI, 터렛 등)에 재사용 어려움   
### 2. 상속   
- 장점 : 무기 유무를 타입으로 구분 가능   
- 단점 : 기능 조합이 늘어날수록 상속 구조가 복잡해짐, 런타임 확장 및 조합에 취약   
### 3. 컴포넌트 기반 분리 (선택)   
- 장점 : 캐릭터 책임 축소 및 역할 분리, 다른 액터에도 동일한 무기 시스템 재사용 가능   
- 단점 : 구조 이해 비용 증가, 단순한 프로젝트에서는 과설계가 될 수 있음   
## 1-4-2. 무기 정의 방식   
### 1. 상속   
- 장점 : 무기 로직이 명확하게 분리됨   
- 단점 : 무기 종류 증가 시 클래스 폭증, 단순 수치 변경에도 컴파일 필요   
### 2. 타입 객체 패턴 기반 데이터 정의   
- 장점 : 신규 무기 추가가 데이터 추가 중심으로 이루어짐, 밸런스 조정 및 실험이 용이, 코드 변경 범위 최소화   
- 단점 : 데이터와 로직의 관계를 명확히 설계하지 않으면 구조가 흐려질 수 있음   
## 1-4-3. 입력 처리 방식   
### 1. 무기 타입 분기 기반 처리   
- 장점 : 구현이 직관적   
- 단점 : 무기 종류 증가 시 조건 분기 증가, 입력 처리 로직이 무기 구현에 강하게 결합됨   
### 2. 전략 패턴 기반 처리 (선택)   
- 장점 : 입력은 명령 단위로 유지되고, 행동은 무기별로 교체 가능, 무기 추가 시 입력 로직 수정 불필요, 분기 없는 실행 경로 확보   
- 단점 : 초기 설계 비용 증가, 구조를 이해하지 못하면 추적이 어려울 수 있음   
       
## 1-5. 정량 평가   

## 1-6. 결론   
책임의 분리, 분기 없는 입력 처리, 무기 확장 시 코드 수정 범위를 최소화. 다양한 무기에도 유연히 대응할 수 있는 시스템 마련.


# 2. 무기와 상황에 맞는 애니메이션 제어   
## 2-1. 기능 요약   
- 캐릭터의 상체와 하체 애니메이션을 분리하고, 상황에 맞는 애니메이션을 재생해야 한다.   
- 무기 종류가 늘어나도 캐릭터가 “해당 무기에 맞는” 애니메이션을 올바르게 재생해야 한다.   

## 2-2. 문제 상황   
- 무기의 종류와 상황이 늘어남에 따라 적절한 애니메이션을 하나의 State Machine에서 관리하기 어려워짐.   
  
## 2-3. 설계 의도   
### State 제어     
- (코드 링크)   
- 메인 AnimInstance에서 매 프레임마다 변수를 계산하고 이를 Layer AnimInstance에 전달한다.   
- 무기를 교체할 때마다 런타임에 Layer BP를 연결하는 방식 채택   
  
## 2-4. 대안 비교   
## 2-4-1. State 제어     
### 1. 단일 State Machine   
- 장점 : 하나의 State Machine만 관리하면 된다.   
- 단점 : 무기가 늘어날수록 상태/전이 규칙이 늘어나 그래프가 거대해지고 유지보수 난이도 상승   
### 2. 복수의 AnimBP   
- 장점 : 무기별 그래프를 완전히 분리 가능   
- 단점 : 공통 로직(이동, 에임오프셋 등)이 BP에 중복된다, 런타임 교체 시 “변수 초기화/상태 끊김” 이슈가 생기기 쉬움, 공통 기능 수정이 모든 무기 AnimBP 수정으로 번짐   
### 3. Layered Anim Instance   
- 장점 : “공통 계산은 한 곳”, “무기별 State Machine은 분리”라는 목표에 가장 잘 맞음   
- 단점 :    
       
## 2-5. 정량 평가   

## 2-6. 결론   


# 3. 애니메이션 진행도 제어
## 3-1. 기능 요약
- 무기 시스템을 컴포넌트로 설계해 캐릭터와 분리함으로써 캐릭터의 책임을 축소하고 무기 시스템과의 결합도를 낮췄습니다.
- 무기 정의는 타입 객체 패턴을 적용해 클래스가 아닌 데이터로 관리했으며, 입력 처리 로직은 전략 패턴 기반으로 구성해 무기 종류에 의존하지 않는 실행 경로를 확보했습니다.
- 이를 통해 분기 없는 입력 처리 구조를 구현하고, 무기 확장 시 코드 수정 범위를 최소화했습니다.

## 3-2. 요구사항
- DrawProgress : 
입력을 홀드하는 시간에 비례하여 0~1로 증가하는 진행률이다.
진행률에 따라 발사 속도와 공격력이 달라진다.
- AimProgress :
조준 진입 및 해제 시 캐릭터 자세를 부드럽게 표현하기 위한 진행률이다.
게임플레이에는 직접적인 영향이 없고, 시각적 연출(UX) 이 핵심이다.
  
## 3-3. 설계 의도   
### DrawProgress   
- (코드 링크)   
- 입력을 Ongoing으로 받아 입력이 유지되는 동안 Elapsed Time을 기반으로 서버에 주기적으로 업데이트하도록 설계.
- 이때 주기와 임계값을 기준으로 스로틀링을 적용해 네트워크 부하를 줄인다.
- 원격 클라이언트는 OnRep으로 상태 변화를 감지하고 DrawProgress를 로컬에 업데이트한다.
### AimProgress   
- (코드 링크)   
- 조준 진입 및 해제 시점에 변경된 상태를 한 번만 동기화하도록 설계.
- 원격 클라이언트는 OnRep으로 상태 변화를 감지하고 로컬의 Timeline을 재생한다.
  
## 3-4. 대안 비교
## 3-4-1. DrawProgress 계산 방식   
### 1. Started + Tick()
- 장점 : 입력과 게임 로직 책임이 깔끔하게 분리된다, 캐릭터가 아닌 Ai, 봇과 같이 입력 주체와 무관하게 재사용 가능
- 단점 : 불필요한 연산이 매 프레임 발생한다
### 2. Ongoing
- 장점 : 입력 유지 시간이 곧 DrawProgress이므로 가장 직관적이다, 입력 시스템이 Elapsed을 직접 관리하므로 Progress 누적을 신경 쓰지 않아도 된다.
- 단점 : 게임 로직이 입력 로직과 커플링된다.
## 3-4-2. DrawProgress 업데이트 방식   
### 1. Tick()
- 장점 : 불필요한 변수가 줄어들어 메모리 오버헤드가 적다, 프레임당 연산이 단순하고 호출 스택이 얇아 프로파일링이 쉽다.
- 단점 : 전환 로직이 많아질수록 코드가 복잡해지고 상태 관리가 어려워진다, 연출의 감각을 조정하기 어렵다, 불필요한 분기 체크가 매 프레임 발생한다.
### 2. TimelineComponent
- 장점 : 트래픽 감소, 실전에서 체감 손실 적음
- 단점 : 구현/튜닝 포인트 증가(주기, 변화량 기준)
## 3-4-3. AimProgress 계산 방식   
### 1. 상속
- 장점 : 무기 로직이 명확하게 분리됨
- 단점 : 무기 종류 증가 시 클래스 폭증, 단순 수치 변경에도 컴파일 필요
### 2. 타입 객체 패턴 기반 데이터 정의
- 장점 : 코드의 의도가 명확하다, 전환 로직을 관리하기 편리하다, 연출의 감각을 조정하기 편리하다, 불필요한 연산을 하지 않는다.
- 단점 : 추가 변수로 인한 오버헤드 발생, 호출 스택이 두꺼워 동작이 느려진다.
       
## 3-5. 정량 평가   

## 3-6. 결론   


## 프로젝트 구조
```plaintext
Source/
├── Project_X/
│   ├── Component/
│   │   ├── PX_InteractableComponent.h              # 상호작용 기능을 구현한 컴포넌트
│   │   ├── PX_InteractableComponent.cpp
│   │   ├── PX_WeaponComponent.h                    # 무기의 공격 기능을 구현한 컴포넌트
│   └─└── PX_WeaponComponent.cpp
│   ├── Entity/
│   │   ├── Client/
│   │   │   ├── PX_BowAnimInstance.h                # 활의 애님 인스턴스
│   │   │   ├── PX_BowAnimInstance.cpp
│   │   │   ├── PX_CharacterAnimInstance.h          # 캐릭터의 애님 인스턴스
│   │   │   ├── PX_CharacterAnimInstance.cpp
│   │   └─└── PX_CharacterClient.cpp                # 캐릭터의 클라이언트 사이드 코드
│   │   ├── Server/
│   │   └─└── PX_CharacterServer.cpp                # 캐릭터의 서버 사이드 코드
│   │   ├── PX_Character.h                          # 캐릭터의 공용 코드
│   │   ├── PX_Character.cpp
│   │   ├── PX_Projectile.h                         # 발사체의 공용 코드
│   │   ├── PX_Projectile.cpp
│   │   ├── PX_Weapon.h                             # 무기의 공용 코드
│   └─└── PX_Weapon.cpp
│   ├── GameMode/
│   │   ├── PX_MainGameMode.h                       # 게임의 규칙을 선언하고 게임의 흐름을 제어
│   └─└── PX_MainGameMode.cpp
│   ├── Interface/
│   └─└── PX_InteractableInterface.h                # 상호작용 기능을 구현한 인터페이스
│   ├── Project_X.Build.cs                          # 빌드 설정 파일
│   ├── Project_X.h
└─└── Project_X.cpp

```
