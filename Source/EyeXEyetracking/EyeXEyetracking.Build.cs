using System.IO;

namespace UnrealBuildTool.Rules
{
	public class EyeXEyetracking: ModuleRules
	{
        private string ModulePath
        {
            get { return Path.GetDirectoryName(RulesCompiler.GetModuleFilename(this.GetType().Name)); }
        }

        private string ThirdPartyPath
        {
            get { return Path.GetFullPath(Path.Combine(ModulePath, "../../ThirdParty/")); }
        }

		public EyeXEyetracking(TargetInfo Target)
		{
			PublicIncludePaths.AddRange(
				new string[] {
					// ... add public include paths required here ...
				}
				);

			PrivateIncludePaths.AddRange(
				new string[] {
					"EyeXEyetracking/Private",
					// ... add other private include paths required here ...
				}
				);

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					// ... add other public dependencies that you statically link with here ...
				}
				);

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
					// ... add private dependencies that you statically link with here ...
				}
				);

			DynamicallyLoadedModuleNames.AddRange(
				new string[]
				{
					// ... add any modules that your module loads dynamically here ...
				}
				);

            LoadEyeXDependencies(Target);
		}

        public bool LoadEyeXDependencies(TargetInfo target)
        {
            bool isLibrarySupported = false;

            if ((target.Platform == UnrealTargetPlatform.Win64) || (target.Platform == UnrealTargetPlatform.Win32))
            {
                isLibrarySupported = true;

                string platformString = (target.Platform == UnrealTargetPlatform.Win64) ? "x64" : "x86";
                string librariesPath = Path.Combine(ThirdPartyPath, "EyeX", "lib");

                PublicAdditionalLibraries.Add(Path.Combine(librariesPath, platformString, "Tobii.EyeX.Client.lib"));
            }

            if (isLibrarySupported)
            {
                // Include path
                PublicIncludePaths.Add(Path.Combine(ThirdPartyPath, "EyeX", "include"));
            }

            Definitions.Add(string.Format("USING_EYEX={0}", isLibrarySupported ? 1 : 0));

            return isLibrarySupported;
        }
	}
}