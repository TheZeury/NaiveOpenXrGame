@echo off 

set extlist=vert frag

for %%e in (%extlist%) do (
    echo Finding %%e shaders.
    for %%f in ("%~dp0*.%%e") do (
        if "%%~xf"==".%%e" (
            echo Compiling "%%~f" to "%%~f.spv".
            C:\VulkanSDK\1.3.236.0\Bin\glslc.exe "%%~f" -o "%%~f.spv"
        )
    )
)

pause