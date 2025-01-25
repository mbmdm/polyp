# Vulkan C++ samples

This repository contains a collection of Vulkan samples, and maybe a simple rendering engine in the future, if I don't lose my motivation :smile:.

## Build Windows

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

## License

See [license](https://github.com/mbmdm/polyp/blob/master/LICENSE)

This project has several [third-party dependencies](https://github.com/mbmdm/polyp/tree/master/3rdparty)

This project uses assets from (each one has its own license):
- [vulkan-samples-assets](https://github.com/KhronosGroup/Vulkan-Samples-Assets)
- [NVIDIA DesignWorks Samples](https://github.com/nvpro-samples)
