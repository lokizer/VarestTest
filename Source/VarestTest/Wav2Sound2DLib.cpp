#include "Wav2Sound2DLib.h"
#include "Audio.h" // 包含 FWaveModInfo 需要的头文件

USoundWave* UWav2Sound2DLib::ConvertWavBytesToSoundWave(const TArray<uint8>& WavData)
{
	// 检查数据是否为空
	if (WavData.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("WavData is empty!"));
		return nullptr;
	}

	// 实例化 FWaveModInfo 来解析 WAV 头文件信息
	FWaveModInfo WaveInfo;
	if (!WaveInfo.ReadWaveInfo(WavData.GetData(), WavData.Num()))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to parse WAV data header."));
		return nullptr;
	}

	// 在临时包(Transient Package)中动态创建一个 USoundWave 对象
	USoundWave* SoundWave = NewObject<USoundWave>(GetTransientPackage(), TEXT("DynamicTTSSoundWave"));
	if (!SoundWave)
	{
		return nullptr;
	}

	// 根据解析出来的WAV信息设置 USoundWave 的属性
	int32 SampleRate = *WaveInfo.pSamplesPerSec;
	SoundWave->SetSampleRate(SampleRate);
	SoundWave->NumChannels = *WaveInfo.pChannels;
	SoundWave->SoundGroup = SOUNDGROUP_Default;
	SoundWave->bLooping = false;
    
	// 计算音频时长 (数据总大小 / 每秒平均字节数)
	if (*WaveInfo.pAvgBytesPerSec > 0)
	{
		SoundWave->Duration = (float)WaveInfo.SampleDataSize / *WaveInfo.pAvgBytesPerSec;
	}
	else
	{
		// 备用计算方式: 假设是 16-bit 深度
		SoundWave->Duration = (float)WaveInfo.SampleDataSize / (SampleRate * SoundWave->NumChannels * sizeof(int16));
	}

	// 分配内存并拷贝原始PCM音频数据
	// 注意：USoundWave 在销毁时会自动使用 FMemory::Free 释放 RawPCMData，所以这里必须使用 FMemory::Malloc 分配
	SoundWave->RawPCMDataSize = WaveInfo.SampleDataSize;
	SoundWave->RawPCMData = (uint8*)FMemory::Malloc(SoundWave->RawPCMDataSize);
	FMemory::Memcpy(SoundWave->RawPCMData, WaveInfo.SampleDataStart, SoundWave->RawPCMDataSize);

	return SoundWave;
}
