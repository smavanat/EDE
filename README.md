# About  
EDE is a 2D game engine with a focus on pixel-based destruction. The initial scope for this project is to re-write the engine I cobbled together for this project: https://github.com/smavanat/URSS-2025
with a few extra features.

## Building  
This project can be built and run in the following ways:  
### Windows  
```
cd ${DIR-WITH-CODE}  
mkdir build
cd build
cmake -G "${YOUR-CMAKE-BUILD-CONFIG}" ..
cmake --build .
.\ede.exe
```
### Mac and Linux  
```
cd ${DIR-WITH-CODE}  
mkdir build
cd build
cmake ..
make
./ede
```

## Features
The following is a list of features that I plan to add/have added to this project
### Implemented  
Basic ECS  
Rendering System including a debug renderer to draw primitives (points, lines, quads and circles)  
Shader Loading  
Synchronous Events (Observer Pattern and Publisher-Subscriber Pattern)  
### Planned  
Pathfinding System  
Grid System  
Destruction System  
World Chunking  
GUI for adding/removing/modifying/placing entities  
World data loading and saving  
Optimised ECS with entity chunking and proper archetypes  
Asynchronous Events  
