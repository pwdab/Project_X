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
	<img src="images/3.0/ActionSystem/before_structure.png" width="55%">
	<img src="images/3.0/ActionSystem/before_sequence.png" width="80%">
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
	<img src="images/3.0/ActionSystem/after_structure.png" width="75%">
	<img src="images/3.0/ActionSystem/after_sequence.png" width="100%">
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
	<img src="images/3.0/ActionSystem/action_context.png" width="25%">
	<img src="images/3.0/ActionSystem/action_pipeline.png" width="100%">
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
	<img src="images/3.0/InventorySystem/before_replication.png" width="25%">
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
	<img src="images/3.0/InventorySystem/after_replication.png" width="25%">
</p>

슬롯 변경 시 <strong>`MarkItemDirty()`</strong>을 호출해 변경된 슬롯만 Replication 되도록 구조를 개선했습니다.

- 각 인벤토리 슬롯을 <strong>`FastArraySerializerItem`</strong>으로 관리
- 슬롯 변경 시 <strong>`MarkItemDirty()`</strong> 호출

**3\. 슬롯 변경 이벤트 기반 동기화**

<p align="center">
	<img src="images/3.0/InventorySystem/after_sequence.png" width="25%">
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
