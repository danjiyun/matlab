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

function analyze_samples( samples )

    lc = 0.062;     % colinear length (meters) of the audiowell ultrasonic transducer 
                    % included in the MAX35103EVKIT2
    lp = 0.01032;   % perpendicular length (meters) of the audiowell ultrasonic 
                    % transducer 

    smoothing = 40; % averaging period applied to the time samples.
    oscillation_freq = 1E6; % oscillation period of the audiowell transducer (set by hardware)
    first_hit_wave = 8;     % oscillation period count of the first hit wave 
                            % (set by matlab example firmware running on the
                            % MAX35103EVKIT2)

    % smoothed up-direction time samples
    up = movmean(svflow_prop(samples.up.average,first_hit_wave,oscillation_freq),smoothing);
    % smoothed down-direction time samples
    down = movmean(svflow_prop(samples.down.average,first_hit_wave,oscillation_freq),smoothing);

    % calculate the speed of sound based on time samples
    sos = svflow_sos(up,down,lc,lp);

    % calculate the temperature data received from the firmware (an RTD1000
    % must be attached to the MAX35103EVKIT2 (not inculded))
    temp = movmean( svflow_rtd(1000,samples.temp_ref,samples.temp_sense), smoothing );

    % calculate the speed of sound based on the temperature of the medium
    sos_temp = temp_comp(temp);

    % calculate speed of the medium using the 'absolute' method
    % this method does not requrie the speed-of-sound and therefore does not
    % require temperature compensation
    absolute_tof = svflow_som(up,down,lc,lp);

    % calculate the speed of the medium using the 'delta' method
    % this method requires temperature compensated speed-of-sound 
    delta_tof_compensated = an6105_som1( lc, sos_temp, down, up );

    % calculate the speed of the medium using the 'delta' method
    % wihtout compensating for temperature.  The first temperature
    % sample is used to obtain the speed of sound for all measurements
    delta_tof_uncompensated = an6105_som1( lc, sos(1), down, up );

    % first plot compares flow for both methods
    figure
    subplot(3,1,1)
    plot( samples.timestamp, delta_tof_compensated, 'r' );
    title('Flow Rate Absolute vs. Delta')
    ylabel('m/s')
    hold on
    plot(samples.timestamp, absolute_tof, 'm' )
    plot( samples.timestamp, delta_tof_uncompensated, 'b' );
    yyaxis right
    
    pd = 100*(absolute_tof-delta_tof_compensated)./delta_tof_compensated;
    
    plot(samples.timestamp, pd, 'k' );
    x = gca;
    x.YAxis(2).Color = 'k';
    ylabel('% difference')
    legend('Delta', 'Absolute', 'Uncompensated', '% diff');

    hold off 

    % second plot compares speed of sound for both methods
    subplot(3,1,2)
    plot( samples.timestamp, sos_temp );
    title('Speed-of-Sound Absolute vs. Delta')
    hold on
    plot( samples.timestamp, sos );
    ylabel('m/s');
    yyaxis right
    ylabel('% difference');
    plot( samples.timestamp, 100*(sos-sos_temp)./sos_temp, 'k' );
    x = gca;
    x.YAxis(2).Color = 'k';
    legend('Delta', 'Absolute', '% diff');
    hold off

    % third plot is temperature of medium
    subplot(3,1,3)
    plot(samples.timestamp, temp );
    title('Temperature of Medium')
    s = sprintf('%cC', char(176));
    ylabel(s);

end

