#include "SiliconFlowTTS.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"

void USiliconFlowTTS::RequestTTS(FString Text, FString ApiKey, FOnAudioLoaded OnComplete)
{
    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(TEXT("https://api.siliconflow.cn/v1/audio/speech"));
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("Authorization"), TEXT("Bearer ") + ApiKey);

    // 构建 JSON
    TSharedPtr<FJsonObject> JsonObj = MakeShareable(new FJsonObject);
    JsonObj->SetStringField(TEXT("model"), TEXT("fnlp/MOSS-TTSD-v0.5"));
    JsonObj->SetStringField(TEXT("input"), Text);
    JsonObj->SetStringField(TEXT("voice"), TEXT("fnlp/MOSS-TTSD-v0.5:alex"));
    JsonObj->SetStringField(TEXT("response_format"), TEXT("wav")); // 确保是 wav
    JsonObj->SetBoolField(TEXT("stream"), false); // 硅基流动建议小写 stream
    JsonObj->SetNumberField(TEXT("sample_rate"), 24000);
    
    FString JsonStr;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonStr);
    FJsonSerializer::Serialize(JsonObj.ToSharedRef(), Writer);
    Request->SetContentAsString(JsonStr);

    // 绑定回调
    Request->OnProcessRequestComplete().BindStatic(&USiliconFlowTTS::OnRequestComplete, OnComplete);
    Request->ProcessRequest();
}

void USiliconFlowTTS::OnRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess, FOnAudioLoaded OnComplete)
{
    if (!bSuccess || !Response.IsValid() || Response->GetResponseCode() != 200)
    {
        UE_LOG(LogTemp, Error, TEXT("TTS 请求失败！HTTP Code: %d"), Response.IsValid() ? Response->GetResponseCode() : -1);
        OnComplete.ExecuteIfBound(nullptr);
        return;
    }

    const TArray<uint8>& RawData = Response->GetContent();
    
    // 硅基流动返回的 WAV 标准头部通常为 44 字节
    // 格式固定为：采样率 24000, 单声道 (1 Channel), 16-bit PCM
    const int32 HeaderSize = 44;
    if (RawData.Num() <= HeaderSize)
    {
        UE_LOG(LogTemp, Error, TEXT("音频数据长度不足，解析失败"));
        OnComplete.ExecuteIfBound(nullptr);
        return;
    }

    int32 SampleRate = 24000;
    int32 Channels = 1;
    int32 DataSize = RawData.Num() - HeaderSize;

    // 创建 Procedural SoundWave (UE5 推荐方式)
    USoundWaveProcedural* SoundWave = NewObject<USoundWaveProcedural>();
    if (!SoundWave)
    {
        OnComplete.ExecuteIfBound(nullptr);
        return;
    }

    // 设置基本参数
    SoundWave->SetSampleRate(SampleRate);
    SoundWave->NumChannels = Channels;
    SoundWave->Duration = (float)DataSize / (SampleRate * Channels * 2.0f);
    SoundWave->bLooping = false;

    // 将除去 44 字节头部的 PCM 原始数据推送进音频队列
    SoundWave->QueueAudio(RawData.GetData() + HeaderSize, DataSize);

    // 重要：为了防止蓝图还没来得及播放就被 GC 回收，这里可以暂时加到 Root
    // 但注意：这需要你在蓝图播放完后手动处理，或者在蓝图里定义一个变量接收它
    // SoundWave->AddToRoot(); 

    UE_LOG(LogTemp, Log, TEXT("✅ TTS 成功生成！时长: %.2f 秒"), SoundWave->Duration);
    OnComplete.ExecuteIfBound(SoundWave);
}