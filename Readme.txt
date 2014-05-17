To use this plugin:

1. First create a new folder under your unreal project folder called "Plugins"
2. Create an appropriate subfolder. I chose "EyeXEyetracking"
3. Clone / copy this repo to that subfolder
4. Download the EyeX SDK and put the libs/includes in the ThirdParty/EyeX/ folder. More directions for that in that folder
5. Regenerate your solution using the UE4 shell extension
6. Build the project
7. Start the editor and open the plugin viewer (can be found under the windows->Plugins menu)
8. Program against the API using the provided singleton  FEyeXEyetracking::GetHost().METHODYOUWANTTOCALL();
9. HAVE FUN!!!




/Temaran