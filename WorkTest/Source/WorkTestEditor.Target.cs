// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;


// CSS 파일을 자체적으로 빌드.
// 독립적인 모듈로 존재하며, 독립적으로 컴파일됨.
// 고유의 바이너리, 인터미디어를 가짐.

public class WorkTestEditorTarget : TargetRules
{
	public WorkTestEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
		ExtraModuleNames.Add("WorkTest");
	}
}
