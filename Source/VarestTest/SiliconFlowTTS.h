#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Interfaces/IHttpRequest.h"
#include "Sound/SoundWaveProcedural.h"
#include "SiliconFlowTTS.generated.h"

// 定义委托，用于蓝图回调
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnAudioLoaded, USoundWave*, SoundWave);

UCLASS()
class VARESTTEST_API USiliconFlowTTS : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// 暴露给蓝图的静态函数
	UFUNCTION(BlueprintCallable, Category = "SiliconFlow")
	static void RequestTTS(FString Text, FString ApiKey, FOnAudioLoaded OnComplete);

private:
	// 处理 HTTP 回调
	static void OnRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess, FOnAudioLoaded OnComplete);
};