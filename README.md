# matlab
Example application for the MAX35103EVKIT2 and MAX35104EVKIT2.

Requires IAR ARM with support for the MAX32620.

Host software components (DLL) requries Microsoft Visual Studio.

Matlab is required to run the host-side examples.

This example shows how to extract ultrasonic time-of-flight data from the MAX3510x and transmit it to a host PC running Matlab for analysis.

Please note that this respository uses git submodules. The proper way to clone is:

git clone --recursive -j8 https://github.com/maxim-ic-flow/matlab.git
