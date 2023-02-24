# NaiveOpenXrGame

My first [OpenXR](https://github.com/KhronosGroup/OpenXR-SDK/) application.

Renderer implemented with Vulkan.

Physics Engine implemented upon NVIDIA PhysX.

To be precise, the APIs I'm using are [OpenXR-Hpp](https://github.com/KhronosGroup/OpenXR-Hpp) and [Vulkan-Hpp](https://github.com/KhronosGroup/Vulkan-Hpp) which has already been part of the LunarG Vulkan SDK.

OpenXR version : 1.0.26

Vulkan version : 1.3.236.0

NVIDIA PhysX version : 5.1.2.32190885

**OpenXR SDK (including openxr_loader), Vulkan SDK and PhysX SDK are not uploaded with this repo.** To properly build this repo, one should download them separately from their official sites/repos and do the *including* and *linking* works.

## Free libraries (other than OpenXR SDK and Vulkan SDK):

1. [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader)

2. [stb_image](https://github.com/nothings/stb)

## Other free resources:

### Pictures

1. *Photo* by Robert Lukeman. 
[repo](NaiveOpenXrGame/textures/robert-lukeman-PH0HYjsf2n8-unsplash.jpg) |
[source](https://unsplash.com/photos/PH0HYjsf2n8)

### Models

1. *Steampowered Steed* by Bohdan Lvov. 
[repo](NaiveOpenXrGame/models/steed) |
[source](https://sketchfab.com/3d-models/steampowered-steed-95537929b1c04dc5a3d0b8fdc5310ee1)

2. *Revolver Navy Colt 1851 Silver* by Johan Pindeville. 
[repo](NaiveOpenXrGame/models/revolver) | 
[source](https://sketchfab.com/3d-models/revolver-navy-colt-1851-silver-c254bb8ee01a4d9db9e6bbdc652d6c11)

## Compression

Some large files are compressed to be uploaded, and they should be decompressed in your local repo first to compile or run this application correctly.

1. steed.obj -> [steed.zip](NaiveOpenXrGame/models/steed/steed.zip)
