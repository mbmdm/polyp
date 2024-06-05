# Vulkan C++ samples

This repository contains a collection of Vulkan samples, and maybe a simple rendering engine in the future, if I don't lose my motivation.

## Buil Windows

1. Clone repo with submodules
```
git clone --recurse-submodule https://github.com/mbmdm/polyp.git
```
2. Download and install [Vulkan SDK](https://vulkan.lunarg.com/) (e.g. verion 1.3.275)

3. Go to the repo folder

```
cd polyp
md _build2 & cd _build2
cmake ..
cmake --build .
```
