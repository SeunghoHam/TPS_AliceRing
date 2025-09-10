// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class WorkTest : ModuleRules
{
	public WorkTest(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput","UMG","Niagara",
			"OnlineSubsystem", // 온라인 서브시스템 기능 사용
			"OnlineSubsystemSteam",  // 스팀과 연결하기용
			"CableComponent",
			"Slate", // PingWidget의 FSlateBrush
			"SlateCore" // PingWidget의 FSlateBrush
		});

        PrivateIncludePaths.Add("WorkTest");
        PrivateIncludePaths.Add("WorkTest/Skills");
        PrivateIncludePaths.Add("WorkTest/Anby");
        PrivateIncludePaths.Add("WorkTest/Animation");
        PrivateIncludePaths.Add("WorkTest/Enemy");
        PrivateIncludePaths.Add("WorkTest/Component");
        PrivateIncludePaths.Add("WorkTest/MultiSystem");
        PrivateIncludePaths.Add("WorkTest/Widget");
        
        PrivateIncludePaths.Add("WorkTest/Alice");
        PrivateIncludePaths.Add("WorkTest/Alice/Widget");
        PrivateIncludePaths.Add("WorkTest/Alice/System");
        PrivateIncludePaths.Add("WorkTest/Alice/Animation");
    }
}
