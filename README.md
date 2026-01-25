# Project_X <img src="https://img.shields.io/badge/Solo-3c78d8?style=flat" align="absmiddle"/> <img src="https://img.shields.io/badge/Unreal%20Engine-0E1128?style=flat&logo=unrealengine&logoColor=white" align="absmiddle"/> <img src="https://img.shields.io/badge/C++-00599C?style=flat&logo=cplusplus&logoColor=white" align="absmiddle"/> <img src="https://img.shields.io/badge/GitHub-181717?style=flat&logo=github&logoColor=white" align="absmiddle"/>


## 🎮 게임 플레이
- **직접 플레이**   
  
- **플레이 영상**   
  

## 📌 프로젝트 소개
- **프로젝트 개요**   
- **개발 기간**   
- **개발 상태**   
- **개발 환경**   
- **멤버 구성**   

## 🎯 담당 업무

## ⚙️ 기능 구현
### 1. 
- **기능 설명**:   
- **주요 기술**:
- **구현 방법**:

## 🛠 이슈 및 해결 과정
### 1. 
- **문제**   
- **원인**   
- **해결**   
- **결과**   

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
