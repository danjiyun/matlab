% Copyright (C) 2017 Maxim Integrated Products, Inc., All Rights Reserved.
%
% Permission is hereby granted, free of charge, to any person obtaining a
% copy of this software and associated documentation files (the "Software"),
% to deal in the Software without restriction, including without limitation
% the rights to use, copy, modify, merge, publish, distribute, sublicense,
% and/or sell copies of the Software, and to permit persons to whom the
% Software is furnished to do so, subject to the following conditions:
%
% The above copyright notice and this permission notice shall be included
% in all copies or substantial portions of the Software.
%
% THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
% OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
% MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
% IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
% OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
% ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
% OTHER DEALINGS IN THE SOFTWARE.
%
% Except as contained in this notice, the name of Maxim Integrated
% Products, Inc. shall not be used except as stated in the Maxim Integrated
% Products, Inc. Branding Policy.
%
% The mere transfer of this software does not imply any licenses
% of trade secrets, proprietary technology, copyrights, patents,
% trademarks, maskwork rights, or any other form of intellectual
% property whatsoever. Maxim Integrated Products, Inc. retains all
% ownership rights.
 
function plot_flow(samples)

    lc = 0.062;     % colinear length (meters) of the audiowell ultrasonic transducer 
                    % included in the MAX35103EVKIT2
    lp = 0.01032;   % perpendicular length (meters) of the audiowell ultrasonic 
                    % transducer 
   
    smoothing = 40; % averaging period applied to the time samples.
    oscillation_freq = 1E6; % oscillation period of the audiowell transducer (set by hardware)
    first_hit_wave = 8;     % oscillation period count of the first hit wave 
                    
    % smoothed up-direction time samples
    up = movmean(svflow_prop(samples.up.average,first_hit_wave,oscillation_freq),smoothing);
    % smoothed down-direction time samples
    down = movmean(svflow_prop(samples.down.average,first_hit_wave,oscillation_freq),smoothing);

    % calculate speed of the medium
    flow_ms = svflow_som(up,down,lc,lp);
    
    plot( samples.timestamp, flow_ms );
    title('Water flow');
    ylabel('m/s');
    xlabel('seconds');
end                    