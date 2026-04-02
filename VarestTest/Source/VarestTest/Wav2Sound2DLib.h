// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Wav2Sound2DLib.generated.h"


UCLASS()
class VARESTTEST_API UWav2Sound2DLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/**
	 * 将WAV格式的二进制数据转换为UE5可播放的USoundWave
	 * @param WavData API返回的二进制Byte数组
	 * @return 生成的USoundWave对象，转换失败则返回null
	*/
	UFUNCTION(BlueprintCallable, Category = "Audio|TTS")
	static USoundWave* ConvertWavBytesToSoundWave(const TArray<uint8>& WavData);
};
