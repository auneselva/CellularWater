# Cellular Water

*** 
ABOUT


Personal project. Water simulation implementation using cellular automata in Unreal Engine. The water is meant to be spawned by user and act accordingly; fall down when there is no block cell or world border below, and spill otherwise. Also, levels in connected vessels should endeavor to be the same.
The concept of how to simulate pressure is taken from the following article: https://w-shadow.com/blog/2009/09/01/simple-fluid-simulation/, where the amount of water a cell can contain is specified by the number of water cells stacked on it.
For now, it is done in the project, however there are some glitches when a large water area reaches border level - I would like to spend some time in the future repairing those. 


See demo video here:
https://www.youtube.com/watch?v=x6FPRfo4Svg

Download playable build from:
https://aune-connie.itch.io/cellular-water


The whole physics code is written in C++. To check out the code for physics simulation proccess, I recommend beginning with the Tick() method in the
/Source/UnrealCppManual/Private/WorldController.cpp file.
UI, HUD, buttons' responses, save data and scene transitions were made using blueprints.

***
PROJECT SETUP


If you decide to clone repo on your local machine make sure to run "git lfs pull" command after that as the project uses Git Large File Storage (https://git-lfs.com/)
The project was made in Unreal Engine 5.0.3 - see https://docs.unrealengine.com/5.0/en-US/installing-unreal-engine/ if you don't already have it on your computer.

