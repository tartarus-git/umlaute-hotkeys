# umlaute-hotkeys
Windowless program that runs in the background on startup and registers hotkeys for inputting German Umlaute. The keybindings are variable. Only for Windows.

# Building
Visual Studio solution. Open it and build it however you want. Recommendation: Release|x86

# Usage
Drop the program into some sort of startup folder and your done. Restart your computer and everything should be active.
Before you do that, you have to put a settings file (called "umlaute_bindings.config") into your AppData/Roaming directory with the binding specifications.
An example file is in the repo (called "temp_settings.config" or something like that), so you can just rename that and drop it into the Roaming folder.
The syntax is pretty self-explanatory, but in case it's not clear:
  - first is the identifier, so that the program knows which umlaut your talking about (A, O, U, a, o, u, SS)
  - then is the modifier key configuration (shfit+control+alt for example)
  - then is the actual keyboard key (A, O, U, S for example)
  
  Those things are put together with the following syntax: ```A:shift+control+alt,A```

# Errors
If something goes wrong, the program simply terminates and doesn't say anything. If this happenes often, something systematic may be wrong with either the program
or your system. To debug the issue, open the Visual Studio solution and start running the program in Debug mode. This will enable error outputting through the
debug console, which you can use to diagnose the issue and/or post and issue on this GitHub repo.
