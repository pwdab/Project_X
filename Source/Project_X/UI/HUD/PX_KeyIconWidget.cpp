// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HUD/PX_KeyIconWidget.h"

#include "Components/TextBlock.h"

void UPX_KeyIconWidget::UpdateWidget(int32 InSlotIndex)
{
	BP_UpdateWidget(InSlotIndex);
}

void UPX_KeyIconWidget::UpdateText(const FText& InText)
{
	if ( UTextBlock* KeyTextBlock = ResolveKeyTextBlock() )
	{
		KeyTextBlock->SetText(InText);
	}
}

void UPX_KeyIconWidget::ClearWidget()
{
	BP_ClearWidget();
	UpdateText(FText::GetEmpty());
}

void UPX_KeyIconWidget::HighlightWidget(bool bHighlighted)
{
	SetRenderOpacity(bHighlighted ? 1.0f : 0.35f);
}

UTextBlock* UPX_KeyIconWidget::ResolveKeyTextBlock() const
{
	static const FName CandidateNames[] = {
		TEXT("TextBlock_KeyIcon"),
		TEXT("TextBlock_Key"),
		TEXT("Text_KeyIcon"),
		TEXT("Text_Key"),
		TEXT("TextBlock")
	};

	for ( const FName& CandidateName : CandidateNames )
	{
		if ( UTextBlock* TextBlock = Cast<UTextBlock>(GetWidgetFromName(CandidateName)) )
		{
			return TextBlock;
		}
	}

	return nullptr;
}
