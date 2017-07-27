# matlab
Example application for the MAX35103EVKIT2 and MAX35104EVKIT2.

# Overview

This application example uses the MAX35103EVKIT2 to demonstrate transfering data from an embedded system into Matlab running on a Windows PC via UART.  The protcol library is designed to be portable.

Please see matlab/matlab.pdf for details.

## Repository

Please note that this project uses git submodules.  The proper way to clone this repository is as follows:

```
git clone --recursive https://github.com/maxim-ic-flow/volumetric.git
```
To switch between branches:

```
git checkout <branch>
git submodule update --recursive --remote
```
## Tools

<b>IAR Embedded Workbench for ARM 7.70+</b>
<p>https://www.iar.com/iar-embedded-workbench/tools-for-arm/arm-cortex-m-edition/

<b>Microsoft Visual Studio (Community 2015)</b>
<p>https://www.visualstudio.com/

<b>Matlab 2016a+ (other versions may work)</b>
<p>https://www.mathworks.com/products/matlab.html
  
