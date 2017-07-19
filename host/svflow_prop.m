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

function propagation_time = svflow_som(tp, periods, frequency)

    % this function calculates the arrival time of the incident
    % wave given the hit values returned by the MAX3510x on the
    % MAX35103EVKIT2
    
    % tp is the average of 6 hit time value returned by the MAX3510x
    % periods is then number of oscillation periods before the 
    % first hit wave
    % frequency is the frequency of oscillation
    
    c1 = (5/2 + periods)/frequency;
    propagation_time = tp - c1;

end

