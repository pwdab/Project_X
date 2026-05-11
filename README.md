<a id="project_x"></a>
# [Project_X](#index) <img src="https://img.shields.io/badge/Solo-3c78d8?style=flat" align="absmiddle"/> <img src="https://img.shields.io/badge/Unreal%20Engine-0E1128?style=flat&logo=unrealengine&logoColor=white" align="absmiddle"/> <img src="https://img.shields.io/badge/C++-00599C?style=flat&logo=cplusplus&logoColor=white" align="absmiddle"/> <img src="https://img.shields.io/badge/GitHub-181717?style=flat&logo=github&logoColor=white" align="absmiddle"/>

<!--
<p align="center">
	<img src="https://github.com/pwdab/Portfolio/blob/ver-3.0/images/Project_X/features4.gif" width="32%">
	<img src="https://github.com/pwdab/Portfolio/blob/ver-3.0/images/Project_X/features5.gif" width="32%">
	<img src="https://github.com/pwdab/Portfolio/blob/ver-3.0/images/Project_X/features6.gif" width="32%">
</p>
-->

<p align="center">
  <img src="https://github.com/pwdab/Portfolio/blob/ver-3.0/images/Project_X/Project_X Combat.png?raw=true" width="75%">
</p>

<p align="center">
  <a href="https://youtu.be/fYw3tQospME">
    <img src="https://img.youtube.com/vi/fYw3tQospME/0.jpg" width="75%" alt="Project_X 전투 로직 데모 영상">
  </a>
</p>


## 📌 프로젝트 소개
- **프로젝트 유형**: 3D 멀티플레이 TPS 게임 개발 프로젝트 (개인 프로젝트)
- **본인 역할**: 게임 클라이언트 프로그래밍
- **개발 기간**: 2025.04.11 ~ 진행 중
- **개발 환경**: Unreal 5.4.4, Windows 10
- **주요 기술**:
	- Unreal Engine 5
	- Gameplay Ability System
	- Multiplayer Networking
	- Animation Blueprint
	- UMG, Custom UI Material
- **담당 업무**:
	- UE5 C++ 기반 **Dedicated Server** TPS **멀티플레이 전투 시스템** 구현
	- 무기 장착, 조준, 발사, 재장전, 데미지, 체력, 전투 UI를 연결한 **기본 전투 루프** 구현
	- **Client Prediction**과 서버 검증을 분리해 **네트워크 지연 환경**에서도 **즉각적인 전투 조작감** 구현
	- **GAS, Gameplay Tag, Gameplay Effect, Attribute Set** 기반으로 전투 액션과 상태 처리 구조화
	- **DataAsset**과 **ItemInstance**를 분리한 **Data-Driven** 무기/아이템 구조 설계
	- **Animation Blueprint, AnimMontage, AnimNotify**를 활용한 **전투 애니메이션** 연동
	- **UMG**와 **Custom UI Material**을 활용한 HP, 탄약, 쿨타임, 상태 변화 시각화

### 추가 정보

## 프로젝트 구조
```plaintext
Source/
└── Project_X/
    ├── AbilitySystem/                                      # Gameplay Ability System
    │   ├── Abilities/                                      # Gameplay Ability 정의
    │   │   ├── PX_GameplayAbilityBase.h/.cpp               # Ability 공통 베이스
    │   │   ├── PX_GA_Locomotion.h/.cpp                     # 이동 계열 Ability
    │   │   ├── PX_GA_Aim.h/.cpp                            # 조준 Ability
    │   │   ├── PX_GameplayAbility_GunFire.h/.cpp           # 총기 공격 Ability
    │   │   ├── PX_GameplayAbility_GunReload.h/.cpp         # 재장전 Ability
    │   │   ├── PX_GameplayAbility_Equip.h/.cpp             # 장착/해제 Ability
    │   │   ├── PX_GA_SwitchAttackMode.h/.cpp               # 공격 모드 전환 Ability
    │   │   ├── PX_GA_WeaponStatusImbue.h/.cpp              # 무기 상태 부여 Ability
    │   │   └── PX_AbilitiesDataAsset.h/.cpp                # Ability 부여 데이터
    │   ├── Attributes/                                     # AttributeSet
    │   │   ├── PX_AttributeSetBase.h/.cpp                  # AttributeSet 공통 베이스
    │   │   ├── PX_ResourceAttributeSet.h/.cpp              # Health, Shield, Stamina, Hunger, Water
    │   │   ├── PX_CombatAttributeSet.h/.cpp                # 전투 관련 Attribute
    │   │   └── PX_MovementAttributeSet.h/.cpp              # 이동 관련 Attribute
    │   ├── Component/
    │   │   └── PX_AbilitySystemComponent.h/.cpp            # 입력 태그 기반 Ability 처리
    │   ├── Effects/
    │   │   ├── PX_GE_Damage.h/.cpp                         # 데미지 GameplayEffect
    │   │   ├── PX_GE_Regen.h/.cpp                          # 자원 재생 GameplayEffect
    │   │   └── PX_GE_StatusEffects.h/.cpp                  # 상태 이상 GameplayEffect
    │   └── Tags/
    │       └── PX_GamePlayTags.h/.cpp                      # GameplayTag 정의
    │
    ├── Component/                                          # Actor Component 모듈
    │   ├── Demo/                                           # 비교 촬영/데모용 자동화
    │   │   ├── PX_ComparisonInputDriverComponent.h/.cpp    # 입력 시퀀스 재생 컴포넌트
    │   │   └── PX_DemoBotComponent.h/.cpp                  # 데모 봇 행동 컴포넌트
    │   ├── Inventory/
    │   │   ├── Client/
    │   │   │   ├── PX_InventoryComponentClient.cpp
    │   │   │   ├── PX_ItemInstanceClient.cpp
    │   │   │   └── PX_WeaponItemInstanceClient.cpp
    │   │   ├── Server/
    │   │   │   ├── PX_InventoryComponentServer.cpp
    │   │   │   ├── PX_ItemInstanceServer.cpp
    │   │   │   ├── PX_EquippableItemInstanceServer.cpp
    │   │   │   └── PX_WeaponItemInstanceServer.cpp
    │   │   ├── PX_InventoryComponent.h/.cpp                # 인벤토리 컴포넌트
    │   │   ├── PX_ItemDataAsset.h/.cpp                     # 아이템 데이터 에셋
    │   │   ├── PX_ItemInstance.h/.cpp                      # 아이템 인스턴스
    │   │   ├── PX_EquippableItemDataAsset.h                # 장착 가능 아이템 데이터
    │   │   ├── PX_EquippableItemInstance.h/.cpp            # 장착 가능 아이템 인스턴스
    │   │   └── PX_WeaponItemInstance.h/.cpp                # 무기 아이템 인스턴스
    │   ├── UI/
    │   │   └── PX_TargetStatusComponent.h/.cpp             # 대상 상태 UI 연동 컴포넌트
    │   ├── Weapon/
    │   │   ├── Client/
    │   │   │   ├── Driver/                                 # 무기별 애니메이션 재생
    │   │   │   │   ├── PX_WeaponAnimDriver.h/.cpp
    │   │   │   │   └── PX_BowAnimDriver.h/.cpp
    │   │   │   ├── Handler/                                # 무기 액션 컨텍스트 처리
    │   │   │   │   ├── PX_WeaponActionHandler.h/.cpp
    │   │   │   │   ├── PX_GunActionHandler.h/.cpp
    │   │   │   │   └── PX_BowActionHandler.h/.cpp
    │   │   │   ├── Notify/                                 # 무기 애님 노티파이
    │   │   │   │   ├── PX_AnimNotifyWeaponCommit.h/.cpp
    │   │   │   │   ├── PX_AnimNotifyWeaponEquipCancel.h/.cpp
    │   │   │   │   ├── PX_AnimNotifyWeaponEquipEnd.h/.cpp
    │   │   │   │   └── PX_AnimNotifyWeaponReloadEnd.h/.cpp
    │   │   │   └── PX_WeaponSystemComponentClient.cpp
    │   │   ├── Server/
    │   │   │   └── PX_WeaponSystemComponentServer.cpp
    │   │   ├── PX_WeaponDataAsset.h                        # 무기 데이터 에셋
    │   │   ├── PX_WeaponSystemComponent.h/.cpp             # 무기 시스템 컴포넌트
    │   │   └── PX_WeaponTypes.h                            # 무기 관련 타입 정의
    │   ├── PX_InteractableComponent.h/.cpp                 # 상호작용 컴포넌트
    │   └── PX_WeaponComponent.h/.cpp                       # 기본 무기 컴포넌트
    │
    ├── Entity/                                             # 월드 액터
    │   ├── Client/
    │   │   ├── PX_CharacterClient.cpp                      # 캐릭터 클라이언트 로직
    │   │   ├── PX_CharacterAnimInstance.h/.cpp             # 캐릭터 애님 인스턴스
    │   │   ├── PX_CharacterLayerAnimInstance.h/.cpp        # 캐릭터 레이어 애님 인스턴스
    │   │   └── PX_BowAnimInstance.h/.cpp                   # 활 전용 애님 인스턴스
    │   ├── Server/
    │   │   └── PX_CharacterServer.cpp                      # 캐릭터 서버 로직
    │   ├── PX_Actor.h/.cpp                                 # 공통 Actor 베이스
    │   ├── PX_Pawn.h/.cpp                                  # 공통 Pawn 베이스
    │   ├── PX_Character.h/.cpp                             # 플레이어/봇 캐릭터
    │   ├── PX_Item.h/.cpp                                  # 월드 아이템
    │   ├── PX_Weapon.h/.cpp                                # 월드 무기
    │   └── PX_Projectile.h/.cpp                            # 투사체
    │
    ├── Framework/                                          # 게임 프레임워크
    │   ├── Controller/
    │   │   └── PX_PlayerController.h/.cpp                  # 플레이어 컨트롤러
    │   ├── GameMode/
    │   │   └── PX_MainGameMode.h/.cpp                      # 메인 게임 모드
    │   ├── HUD/
    │   │   └── PX_HUD.h/.cpp                               # 메인 HUD
    │   └── PlayerState/
    │       └── PX_PlayerState.h/.cpp                       # ASC/Attribute 보유 PlayerState
    │
    ├── Input/
    │   ├── Component/
    │   │   └── PX_EnhancedInputComponent.h/.cpp            # Enhanced Input 확장 컴포넌트
    │   └── Config/
    │       └── PX_InputConfigDataAsset.h/.cpp              # 입력 액션-태그 매핑 데이터
    │
    ├── Interface/
    │   └── PX_InteractableInterface.h                      # 상호작용 인터페이스
    │
    ├── Settings/
    │   └── PX_GameplayPredictionSettings.h/.cpp            # 프로젝트 단위 예측 설정
    │
    ├── Subsystem/
    │   └── PX_WeaponCommitSubsystem.h/.cpp                 # 무기 액션 커밋 처리 서브시스템
    │
    └── UI/
        ├── HUD/
        │   ├── PX_HUDWidget.h/.cpp                         # 전체 HUD
        │   ├── PX_CurrentWeaponWidget.h/.cpp               # 현재 무기 HUD
        │   ├── PX_WeaponListsWidget.h/.cpp                 # 보유 무기 리스트
        │   ├── PX_WeaponListWidget.h/.cpp                  # 개별 무기 슬롯
        │   ├── PX_WeaponIconWidget.h/.cpp                  # 무기 아이콘
        │   ├── PX_KeyIconWidget.h/.cpp                     # 키 입력 아이콘
        │   ├── PX_SkillIconWidget.h/.cpp                   # 스킬 아이콘
        │   ├── PX_StaminaGaugeWidget.h/.cpp                # 게이지 위젯
        │   └── PX_TargetHealthWidget.h/.cpp                # 대상 체력 UI
        └── Inventory/
            ├── PX_InventoryWidget.h/.cpp                   # 전체 인벤토리
            ├── PX_InventoryItemSlotsWidget.h/.cpp          # 아이템 슬롯 목록
            ├── PX_InventoryWeaponSlotsWidget.h/.cpp        # 무기 슬롯 목록
            └── PX_InventorySlotWidget.h/.cpp               # 개별 인벤토리 슬롯
```

<br>

---

<br>
