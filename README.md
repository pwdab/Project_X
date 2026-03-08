<a id="project_x"></a>
# [Project_X](#index) <img src="https://img.shields.io/badge/Solo-3c78d8?style=flat" align="absmiddle"/> <img src="https://img.shields.io/badge/Unreal%20Engine-0E1128?style=flat&logo=unrealengine&logoColor=white" align="absmiddle"/> <img src="https://img.shields.io/badge/C++-00599C?style=flat&logo=cplusplus&logoColor=white" align="absmiddle"/> <img src="https://img.shields.io/badge/GitHub-181717?style=flat&logo=github&logoColor=white" align="absmiddle"/>

<div align="center"><h3><strong><em>구현을 넘어 설계와 구조로, 나만의 TPS 멀티플레이 템플릿을 만들어 갑니다</em></strong></h3></div>

<p align="center">
	<img src="https://github.com/pwdab/Portfolio/blob/main/images/Project_X/Project_X.png" alt="Project_X" width="75%">
	<img src="https://github.com/pwdab/Portfolio/blob/main/images/Project_X/features1.gif" width="40%">
	<img src="https://github.com/pwdab/Portfolio/blob/main/images/Project_X/features2.gif" width="35%">
	<img src="https://github.com/pwdab/Portfolio/blob/main/images/Project_X/features3.gif" width="20%">
</p>


## 📌 프로젝트 소개
- **프로젝트 유형**: 개인 프로젝트 게임 개발 (프로그래밍 1명)
- **프로젝트 목적**: 단순 기능 구현보다 깔끔하고 가독성 좋은 코드, 설계와 구조화에 집중하는 것을 목표로 함
- **본인 역할**: 프로그래밍
- **게임 장르**: 3D 멀티플레이 TPS
- **게임 개요**: TPS 멀티플레이 게임의 공통 시스템을 설계·구현하며, C++ 기반 설계·코드 품질·모듈화·확장성에 집중
- **개발 기간**: 2025.04.11 ~ 진행 중
- **개발 상태**: 개발 중
- **개발 환경**: Unreal 5.4.4, Windows 10
- **주요 기술**: Unreal Engine 5, C++

## 프로젝트 구조
```plaintext
Source/
├── Project_X/
│   │   ├── Component/												# 컴포넌트
│   │   │   ├── Inventory/
│   │   │   │   ├── Client/
│   │   │   │   │   ├── PX_InventroyComponentClient.cpp
│   │   │   │   │   ├── PX_ItemInstanceClient.cpp
│   │   │   │   │   ├── PX_WeaponItemInstanceClient.cpp
│   │   │   │   ├── Server/
│   │   │   │   │   ├── PX_InventoryComponentServer.cpp
│   │   │   │   │   ├── PX_ItemInstanceServer.cpp
│   │   │   │   │   ├── PX_WeaponItemInstanceServer.cpp
│   │   │   │   ├── PX_InventoryComponent.h							# 인벤토리 컴포넌트
│   │   │   │   ├── PX_InventoryComponent.cpp
│   │   │   │   ├── PX_ItemDataAsset.h								# 아이템 사용자 정의 데이터
│   │   │   │   ├── PX_ItemDataAsset.cpp
│   │   │   │   ├── PX_ItemInstance.h								# 아이템 인스턴스
│   │   │   │   ├── PX_ItemInstance.cpp
│   │   │   │   ├── PX_WeaponItemInstance.h							# 아이템 인스턴스를 부모 클래스로 하는 무기 인스턴스
│   │   │   │   ├── PX_WeaponItemInstanec.cpp
│   │   │   ├── Weapon/												
│   │   │   │   ├── Client/
│   │   │   │   │   ├── Driver/										# 무기에 따른 애니메이션 재생
│   │   │   │   │   │   ├── PX_WeaponAnimDriver.h
│   │   │   │   │   │   ├── PX_WeaponAnimDriver.cpp
│   │   │   │   │   ├── Handler/									# 무기에 따른 애니메이션 컨텍스트 생성
│   │   │   │   │   │   ├── PX_WeaponActionHandler.h
│   │   │   │   │   │   ├── PX_WeaponActionHandler.cpp
│   │   │   │   │   ├── Notify/										# 무기의 애님 노티파이
│   │   │   │   │   │   ├── PX_AnimNotifyWeaponCommit.h
│   │   │   │   │   │   ├── PX_AnimNotifyWeaponCommit.cpp
│   │   │   │   │   │   ├── PX_AnimNotifyWeaponEquipEnd.h
│   │   │   │   │   │   ├── PX_AnimNotifyWeaponEquipEnd.cpp
│   │   │   │   │   │   ├── PX_AnimNotifyWeaponReloadEnd.h
│   │   │   │   │   │   ├── PX_AnimNotifyWeaponReloadEnd.cpp
│   │   │   │   │   ├── PX_WeaponSystemComponentClient.cpp
│   │   │   │   ├── Server/
│   │   │   │   │   ├── PX_WeaponSystemComponentServer.cpp
│   │   │   │   ├── PX_WeaponDataAsset.h							# 무기 데이터 에셋 사용자 정의 데이터
│   │   │   │   ├── PX_WeaponSystemComponent.h						# 무기 컴포넌트
│   │   │   │   ├── PX_WeaponSystemComponent.cpp
│   │   │   │   ├── PX_WeaponTypes.h								# 무기 사용자 정의 데이터
│   │   ├── Entity/													# 액터
│   │   │   │   ├── Client/
│   │   │   │   │   ├── PX_CharacterAnimInstance.h					# 캐릭터 애님 인스턴스
│   │   │   │   │   ├── PX_CharacterAnimInstance.cpp
│   │   │   │   │   ├── PX_CharacterClient.cpp
│   │   │   │   │   ├── PX_CharacterLayerAnimInstance.h				# 캐릭터 레이어 애님 인스턴스
│   │   │   │   │   ├── PX_CharacterLayerAnimInstance.h
│   │   │   │   ├── Server/
│   │   │   │   │   ├── PX_CharacterServer.cpp
│   │   │   │   ├── PX_Character.h									# 월드에 스폰하는 캐릭터 액터
│   │   │   │   ├── PX_Character.cpp
│   │   │   │   ├── PX_Item.h										# 월드에 스폰하는 아이템 액터
│   │   │   │   ├── PX_Item.cpp
│   │   │   │   ├── PX_Projectile.h									# 월드에 스폰하는 투사체 액터
│   │   │   │   ├── PX_Projectile.cpp
│   │   │   │   ├── PX_Weapon.h										# 월드에 스폰하는 무기 액터
│   │   │   │   ├── PX_Weapon.cpp
│   │   ├── Framework/												# 프레임워크
│   │   │   │   ├── Controller/
│   │   │   │   │   ├── PX_PlayerController.h						# 플레이어 컨트롤러
│   │   │   │   │   ├── PX_PlayerController.cpp
│   │   │   │   ├── GameMode/
│   │   │   │   │   ├── PX_MainGameMode.h							# 메인 게임 모드
│   │   │   │   │   ├── PX_MainGameMode.cpp
│   │   │   │   ├── HUD/
│   │   │   │   │   ├── PX_HUD.h									# 메인 HUD
│   │   │   │   │   ├── PX_HUD.cpp
│   │   ├── Subsystem/												# 서브 시스템
│   │   │   │   ├── PX_WeaponCommitSubsystem.h						# 무기 컴포넌트의 액션 커밋을 60Hz로 실행
│   │   │   │   ├── PX_WeaponCommitSubsystem.cpp
│   │   ├── UI/														# UI
│   │   │   │   ├── HUD/											# HUD
│   │   │   │   │   ├── PX_CurrentWeaponWidget.h					# 현재 무기 HUD
│   │   │   │   │   ├── PX_CurrentWeaponWidget.cpp
│   │   │   │   │   ├── PX_HUDWidget.h								# 전체 HUD
│   │   │   │   │   ├── PX_HUDWidget.cpp
│   │   │   │   │   ├── PX_KeyIconWidget.h							# 키보드 키를 나타내는 아이콘
│   │   │   │   │   ├── PX_KeyIconWidget.cpp
│   │   │   │   │   ├── PX_WeaponIconWidget.h						# 무기를 나타내는 아이콘
│   │   │   │   │   ├── PX_WeaponIconWidget.cpp
│   │   │   │   │   ├── PX_WeaponListsWidget.h						# 소유하고 있는 무기 리스트
│   │   │   │   │   ├── PX_WeaponListsWidget.cpp
│   │   │   │   │   ├── PX_WeaponListWidget.h						# 소유하고 있는 무기
│   │   │   │   │   ├── PX_WeaponListWidget.cpp
│   │   │   │   ├── Inventory/										# 인벤토리 UI
│   │   │   │   │   ├── PX_InventoryItemSlotsWidget.h				# 아이템 인벤토리
│   │   │   │   │   ├── PX_InventoryItemSlotsWidget.cpp
│   │   │   │   │   ├── PX_InventorySlotWidget.h					# 개별 슬롯
│   │   │   │   │   ├── PX_InventorySlotWidget.cpp
│   │   │   │   │   ├── PX_InventoryWeaponSlotsWidget.h				# 무기 인벤토리
│   │   │   │   │   ├── PX_InventoryWeaponSlotsWidget.cpp
│   │   │   │   │   ├── PX_InventoryWidget.h						# 전체 인벤토리
└── └── └── └── └── └── PX_InventoryWidget.cpp

<br>

---

<br>

<a id="action-system"></a>
## [1. Action 기반 멀티플레이 행동 처리 시스템](#index) 

<a id="action-system-overview"></a>
### [요약](#index)

**핵심 문제**   
- 새로운 행동 추가 시 Character 코드 수정이 필요해 시스템 확장이 어려웠고, 네트워크 지연 환경에서는 행동 실행 순서 불일치 문제가 발생했습니다.

**해결 전략**   
- 행동을 `ActionContext` 기반 데이터 구조로 통합하고 하나의 파이프라인으로 처리하도록 시스템을 재설계했습니다.   
- `Sequence ID` 기반 처리 구조를 도입하여 멀티플레이 환경에서도 행동 실행 순서를 안정적으로 관리하도록 개선했습니다.
<a id="action-system-problem"></a>
### [문제](#index)

기존 행동 처리 시스템에서는 다음과 같은 문제가 발생했습니다.   

- 새로운 행동을 추가할 때마다 <strong>`Character`</strong> 코드 수정이 필요   
- 네트워크 지연 환경에서 행동 실행 순서가 보장되지 않음

<a id="action-system-analysis"></a>
### [원인](#index)

문제의 원인은 기존 **행동 처리 구조**에 있었습니다.

<p align="center">
	<img src="https://github.com/pwdab/Portfolio/blob/ver-3.0/images/3.0/ActionSystem/before_structure.png" width="65%">
	<img src="https://github.com/pwdab/Portfolio/blob/ver-3.0/images/3.0/ActionSystem/before_sequence.png" width="80%">
</p>

- Character의 과도한 책임   
  - 입력 이후의 행동 실행 로직을 <strong>`Character`</strong>가 직접 호출   
  - 입력 처리, 상태 변경, 서버 RPC 호출, 공격 시작까지 <strong>`Character`</strong>가 담당   

- 행동 처리의 표준화 부재   
  - 행동을 공통 개념으로 관리하는 구조가 존재하지 않음
  - 각 행동이 개별 함수와 RPC 흐름으로 구현되어 서로 다른 처리 경로를 가짐

- RPC 기반 즉시 실행 구조   
  - 입력 이벤트 발생 시 서버 RPC를 즉시 호출
  - 네트워크 지연 환경에서 RPC 도착 순서가 달라질 경우 행동 순서가 보장되지 않음

<a id="action-system-solution"></a>
### [해결](#index)

**1\. Character 책임 분리**

<p align="center">
	<img src="https://github.com/pwdab/Portfolio/blob/ver-3.0/images/3.0/ActionSystem/after_structure.png" width="75%">
	<img src="https://github.com/pwdab/Portfolio/blob/ver-3.0/images/3.0/ActionSystem/after_sequence.png" width="80%">
</p>

<strong>`Character`</strong> 중심 구조를 다음과 같이 역할 기반 구조로 분리했습니다.

- **`Character`**
  - 입력 처리
- **`WeaponSystemComponent`**
  - Action(Aim On, Fire Once, Reload, Equip, ..) 제출
  - 서버 승인
  - 상태 업데이트
- **`ActionHandler`**
  - Action을 Animation Context로 변환
- **`AnimDriver`**
  - 애니메이션 재생

**2\. 행동 처리 구조 통합**

<p align="center">
	<img src="https://github.com/pwdab/Portfolio/blob/ver-3.0/images/3.0/ActionSystem/action_context.png" width="25%">
	<img src="https://github.com/pwdab/Portfolio/blob/ver-3.0/images/3.0/ActionSystem/action_pipeline.png" width="100%">
</p>

행동을 공통 데이터 구조인 <strong>`ActionContext`</strong>로 표현하고, 모든 행동이 동일한 처리 파이프라인을 거치도록 설계했습니다.

- 입력을 <strong>`ActionContext`</strong>로 변환하여 제출   
- 행동 종류를 <strong>`ActionType`</strong>으로 구분   
- 서버와 클라이언트에서 동일한 처리 구조 유지   

**3\. Sequence 기반 순서 제어**

RPC 도착 순서가 입력 순서와 달라질 수 있으므로 행동 처리 순서를 안정적으로 유지하기 위해 <strong>`Sequence ID`</strong>를 도입했습니다.

- Client에서 입력 발생 시 <strong>`ClientSequence`</strong> 증가   
- ActionContext에 <strong>`Sequence ID`</strong> 포함하여 서버 RPC 전송   
- Server는 <strong>`ServerSequence`</strong> 기준으로 Commit   
- Client는 Commit된 <strong>`Sequence ID`</strong> 기준으로 상태 동기화   

<a id="action-system-result"></a>
### [결과](#index)

Action 기반 구조를 도입한 결과 다음과 같은 효과를 얻었습니다.   

**1\. 시스템 확장성 개선**

- 행동을 공통 `Action` 구조로 관리
- 새로운 행동 추가 시 `ActionType` 추가와 처리 로직 확장만 필요

**→** **기존 코드 수정 범위를 최소화하여 행동 확장 가능**

**2\. 분산된 행동 로직 통합**

- 공격, 장착 등 행동마다 개별적으로 나뉘어 있던 실행 흐름을 `ActionContext` 기반 구조로 통합했습니다.
- 모든 행동을 **Action 파이프라인**에서 처리하도록 설계

**→** 기능별로 흩어져 있던 로직을 정리하여 **구조 복잡도를 낮추고 코드 이해 및 수정 비용을 절감**

**3\. 멀티플레이 정합성 확보**

- 입력 발생 시점마다 `Sequence ID`를 부여하고, 서버 커밋 순서를 기준으로 행동을 처리
- 네트워크 지연 환경에서도 행동 처리 순서를 안정적으로 유지

**→** **Action 실행 순서 불일치 문제를 해결**하고 멀티플레이 환경에서 **행동 처리 정합성을 확보**

<br>

<br>

<a id="fastarray-inventory"></a>
## [2. Fast Array 기반 인벤토리 시스템](#index) 

<a id="fastarray-inventory-overview"></a>
### [요약](#index)

**핵심 문제**   
- 기존 시스템에서 TArray 기반 인벤토리를 사용하면서 일부 변경 시에도 전체 배열이 Replicate 되어 불필요한 네트워크 트래픽이 발생했습니다.

**해결 전략**   
- Fast Array Replication 기반 인벤토리 구조를 도입하여 슬롯 단위 Replication과 이벤트 기반 동기화를 통해 네트워크 트래픽을 줄였습니다.

<a id="fastarray-inventory-problem"></a>
### [문제](#index)

기존 인벤토리 시스템에서는 다음과 같은 문제가 발생했습니다.  

- **인벤토리 일부가 변경되어도 항상 전체 배열이 Replicate 되어 불필요한 네트워크 트래픽이 발생**

<a id="fastarray-inventory-analysis"></a>
### [원인](#index)

문제의 원인은 기존 **인벤토리 Replication 구조**에 있었습니다.

<p align="center">
	<img src="https://github.com/pwdab/Portfolio/blob/ver-3.0/images/3.0/InventorySystem/before_replication.png" width="50%">
</p>

**TArray 기반 Replication**   
- 인벤토리를 배열 전체를 기준으로 Replicate하는 TArray로 관리
- 아이템 추가 / 제거 / 수량 변경과 같은 작은 변화에도 항상 배열 전체를 Replicate
- 플레이어 수와 인벤토리 변경 빈도가 증가할수록 네트워크 트래픽이 급격히 증가

<a id="fastarray-inventory-solution"></a>
### [해결](#index)

**1\. Fast Array Replication 기반 인벤토리 구조 설계**

```C++
USTRUCT(BlueprintType)
struct FPXInventorySlot : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY()
	int32 SlotIndex = INDEX_NONE;
	UPROPERTY()
	TObjectPtr<UPX_ItemInstance> ItemInstance = nullptr;
	UPROPERTY()
	FGuid ItemInstanceId = FGuid();
};
```

```C++
USTRUCT()
struct FPXInventorySlotArray : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FPXInventorySlot> Slots;
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FPXInventorySlot, FPXInventorySlotArray>(Slots, DeltaParams, *this); }
};
```

<!--
<p align="center">
	<img src="images/3.0/InventorySystem/after_structure.png" width="100%">
</p>
-->

Unreal Engine의 **Fast Array Replication**을 활용하여 인벤토리 동기화 구조를 개선했습니다.   

- **`FPXInventorySlot`**
  - `FFastArraySerializerItem` 기반 개별 슬롯 구조
  - 슬롯 인덱스와 아이템 인스턴스를 관리
- **`FPXInventorySlotArray`**
  - `FFastArraySerializer` 기반 슬롯 배열 관리 구조
- **Fast Array Replication**을 통해 배열 전체가 아닌 변경된 슬롯만 네트워크로 전송되도록 개선

**2\. 슬롯 단위 Replication 구조 도입**

<p align="center">
	<img src="https://github.com/pwdab/Portfolio/blob/ver-3.0/images/3.0/InventorySystem/after_replication.png" width="65%">
</p>

슬롯 변경 시 <strong>`MarkItemDirty()`</strong>을 호출해 변경된 슬롯만 Replication 되도록 구조를 개선했습니다.

- 각 인벤토리 슬롯을 <strong>`FastArraySerializerItem`</strong>으로 관리
- 슬롯 변경 시 <strong>`MarkItemDirty()`</strong> 호출

**3\. 슬롯 변경 이벤트 기반 동기화**

<p align="center">
	<img src="https://github.com/pwdab/Portfolio/blob/ver-3.0/images/3.0/InventorySystem/after_sequence.png" width="70%">
</p>

```C++
void PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize);
void PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize);
void PostReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize);
```

<strong>`Fast Array`</strong>의 Replication 콜백을 활용하여 슬롯 변경을 이벤트 기반으로 처리하도록 개선했습니다.

- 아이템 추가, 제거, 수량 변경 등의 변화를 **슬롯 단위 콜백으로 통지**
- 클라이언트에서 해당 이벤트를 기반으로 인벤토리 상태를 갱신

<a id="fastarray-inventory-result"></a>
### [결과](#index)

Fast Array Replication 기반 인벤토리 구조를 도입한 결과 다음과 같은 효과를 얻었습니다.

**1\. 변경된 슬롯만 네트워크 동기화**

- 인벤토리 변경 시 전체 배열이 Replicate 되던 구조에서 **Delta Replication을 통해 변경된 슬롯만 네트워크로 전송**되도록 개선

**→** 불필요한 데이터 전송을 줄여 **네트워크 트래픽 감소**

**2\. 인벤토리 동기화 효율 개선**

- 아이템 추가, 제거, 수량 변경 등 다양한 인벤토리 변화를 이벤트로 처리

**→** 멀티플레이 환경에서도 효율적으로 동작할 수 있는 **네트워크 친화적인 인벤토리 동기화 구조 확보**
