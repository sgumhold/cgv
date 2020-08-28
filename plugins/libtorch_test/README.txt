This project shows how to use libtorch with the CGV-framework. The example is based on the MNIST database (http://yann.lecun.com/exdb/mnist/) and
includes a pretrained exported network in "res\exported_net.zip". This network tries to identify handwritten digits. Some test images are also
located in the res folder. Images must have a black background with a white digit. The network works on images 28x28 pixels in size. You can also
supply larger images, which will get resized before inference. The network also is not perfect. While it recognizes most of the example images it
fails to correctly identify the image of the 9.
_____________________________________________________________________________________________________________________

Running this example currently only works in the develop branch of the CGV-framework.
To switch to the develop branch use the following commnd in git bash:
> git checkout develop

The project uses libtorch, the C++ library of pytorch to load and run neural networks.
Before running, the binaries must be downloaded and extracted to their respective folder.

This can be done automatically or manually if the automatic mode fails somehow.

_____________________________________________________________________________________________________________________

==AUTOMATIC==
Go to the root folder of the CGV Framework and run the "configure_libtorch.bat" script.
This will download and extract the libtorch files as well as copy the necessary dlls.

==MANUAL==
To enable running unsigned PowerShell scripts:

Start PowerShell as Administrator:
> Set-ExecutionPolicy RemoteSigned -Force
Check with:
> Get-ExecutionPolicy -List | ft -AutoSize

Use PowerShell to run the "get_prebuilt.ps1" script located in your CGV Framework installation folder under "<CGV_DIR>\3rd\libtorch\".
The script will download and extract the libtorch library into a "dist" folder.

Copy dlls "<CGV_DIR>\3rd\libtorch\dist\libtorch\lib" from into the build folder "<CGV_BUILD>\bin":
asmjit.dll
c10.dll
fbgemm.dll
libiomp5md.dll
torch_cpu.dll
_____________________________________________________________________________________________________________________

After configuring:

Add the "libtorch" dependency in your project .pj file unter "addProjectDeps".
Include the libtorch library in your source file using:
#include "libtorch.h"
