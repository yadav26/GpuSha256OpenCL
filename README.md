# GpuSha256OpenCL
Source that offloads cpu and calculate sha on Radeon Gpu as fast as 100 times then the CPU
To make code work you need following libraries to install

1. Visual Studio 2015 - IDE to develop / compile your application
if not VS2015, you can port the project to older / other installed version of Visual Studio

2. App SDK - Library to provide interface to offload work items on GPU
https://www.dropbox.com/s/gq8vqhelq0m6gj4/AMD-APP-SDKInstaller-v3.0.130.135-GA-windows-F-x64.exe?dl=0

3. Radeon Catalyst driver - Driver that access GPU MMIO and facilitate low level gpu register access
https://devhub.amd.com/wp-content/uploads/Software/17.20Beta19-170510a-314723E-AES.zip

HARDWARE - 
1. Radeon Graphics card - Radeon R9 390X 8G for best performance
else
2. AMD APU A12 - Carrizo family 0x15 with Gpu compute cores

Sanity Test :
Run ClInfo.exe comes as a bundled application with SDK. It's success matters to run these sources.


