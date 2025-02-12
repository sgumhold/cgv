Windows
- use most recent installer from https://github.com/microsoft/azure-kinect-sensor-sdk/blob/develop/docs/usage.md
- check that system variable AZURE_KINECT_SDK is set to the azure kinect sdk subdirectory of install directory
  (for version 1.4.2 this defaults to C:\Program Files\Azure Kinect SDK v1.4.2\sdk)
- add %AZURE_KINECT_SDK%\windows-desktop\amd64\release\bin to PATH

Linux
# install libsoundio version 1:
wget mirrors.kernel.org/ubuntu/pool/universe/libs/libsoundio/libsoundio1_1.1.0-1_amd64.deb
sudo dpkg -i libsoundio1_1.1.0-1_amd64.deb
# wget the 3 debian packages into an empty current folder
wget https://packages.microsoft.com/ubuntu/18.04/prod/pool/main/libk/libk4a1.4/libk4a1.4_1.4.2_amd64.deb
wget https://packages.microsoft.com/ubuntu/18.04/prod/pool/main/libk/libk4a1.4-dev/libk4a1.4-dev_1.4.2_amd64.deb
wget https://packages.microsoft.com/ubuntu/18.04/prod/pool/main/k/k4a-tools/k4a-tools_1.4.2_amd64.deb
# install the 3 packages
sudo apt install ./*.deb
# copy rules file so no root / sudo is needed to access the kinect device
wget https://raw.githubusercontent.com/microsoft/Azure-Kinect-Sensor-SDK/refs/heads/develop/scripts/99-k4a.rules
sudo cp 99-k4a.rules /etc/udev/rules.d/
# increase usb memory for camera devices which is 16MB only by default
# check current value in MB:
more /sys/module/usbcore/parameters/usbfs_memory_mb
# to increase to 128MB edit 
nano /etc/default/grub
# and change line
#	GRUB_CMDLINE_LINUX_DEFAULT="quiet splash ..."
# to
#	GRUB_CMDLINE_LINUX_DEFAULT="quiet splash usbcore.usbfs_memory_mb=128 ..."
# update grub
sudo update-grub
# reboot system and check whether value has been increased
more /sys/module/usbcore/parameters/usbfs_memory_mb
