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

function som = svflow_som(tpu, tpd, L, Lp)

    % this function calculates the speed of the medium
    % through the audiowell ultrasonic transducer included
    % in the MAX35103EVKIT2 evaluation kit
    
    % up propagation time (tpu), down propagation time (tpd)
    % the total colinear acoustic path length (L)
    % the total perpendicular acoustic path length (Lp)

    prod = tpu .* tpd;
    sum = tpu + tpd;
    diff = tpu - tpd;
    tpu2 = tpu .^ 2;
    tpd2 = tpd .^ 2;
    sos = tpd2 + tpu2;
    
    c1 = -6*L - 8*Lp;
    c2 = L + 4*Lp;
    c3 =  2*L^2 - 16*L*Lp - 32*Lp^2;
    c4 = L^2 + 8*L*Lp + 16*Lp^2;
    
    s = sqrt( c3 * prod + c4 * sos );
    n = (c1 * prod + c2 * sos + sum .* s );
    d = 4 * prod .* diff;
    som = n ./ d;
    
end