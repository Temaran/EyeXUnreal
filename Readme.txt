Initial info:
Currently, for this plugin to work, you need to do some changes to the core source, this will not change until I can get Epic to pull these changes :)
For more info on the changes see:
https://forums.unrealengine.com/showthread.php?7812-Transforming-O-S-mouse-coordinates-to-viewport-pixels

In a nutshell, you need to do these changes:

To UnrealClient.h in the FViewport class add:

virtual bool OperatingSystemPixelToViewportPixel(FIntPoint const * const OperatingSystemPoint, FIntPoint& ViewportPoint) const { return false; }
virtual bool ViewportPixelToOperatingSystemPixel(FIntPoint const * const ViewportPoint, FIntPoint& OperatingSystemPoint) const { return false; }

override them in FSceneViewport.h and then create these implementations in SceneViewport.cpp:

bool FSceneViewport::OperatingSystemPixelToViewportPixel(FIntPoint const * const OperatingSystemPoint, FIntPoint& ViewportPoint) const
{
	auto TransformedPoint = CachedGeometry.AbsoluteToLocal(FVector2D(OperatingSystemPoint->X, OperatingSystemPoint->Y));
	ViewportPoint.X = TransformedPoint .X;
	ViewportPoint.Y = TransformedPoint .Y;

	return true;
}

bool FSceneViewport::ViewportPixelToOperatingSystemPixel(FIntPoint const * const ViewportPoint, FIntPoint& OperatingSystemPoint) const
{
	auto TransformedPoint = CachedGeometry.LocalToAbsolute(FVector2D(ViewportPoint->X, ViewportPoint->Y));
	OperatingSystemPoint.X = TransformedPoint .X;
	OperatingSystemPoint.Y = TransformedPoint .Y;

	return true;
}




Then, to use this plugin:

1. First create a new folder under your unreal project folder called "Plugins"
2. Create an appropriate subfolder. I chose "EyeXEyetracking"
3. Clone / copy this repo to that subfolder
4. Download the EyeX SDK and put the libs/includes in the ThirdParty/EyeX/ folder. More directions for that in that folder
5. Put the relevant client dll into the Binaries/PLATFORM/ subfolder. if x64, use the x64 version of Tobii.EyeX.Client.dll and conversely for x86. You can find the dll in the ThirdParty/EyeX/lib/PLATFORM/ folders
6. Regenerate your solution using the UE4 shell extension
7. Build the project
8. Start the editor and open the plugin viewer (can be found under the windows->Plugins menu)
9. Add a reference to the module in your build script, the relevant row might look something like (Your buildfile is probably called something like MYPROJECT.Build.cs: 
    PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EyeXEyetracking" });

10. Program against the API using the provided singleton  FEyeXEyetracking::GetHost().METHODYOUWANTTOCALL();
11. HAVE FUN!!!




/Temaran