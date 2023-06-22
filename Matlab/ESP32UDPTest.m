clc;
clear;
close all;

espPort = 4069;
espIP = "192.168.0.27";
udp = udpport;



%Esp connection message
write(udp, "Matlab waiting...", "String", espIP, espPort);
pause(1);
disp("Waiting on ESP..." + newline);


done = false;

%Standard packagevalues
packetsize = 1500;
packetAmount = 2;


%recieve data
%esp sends 8 bit, matlab reading 16 bit
packetsRecieved = 0;
bytecount = 0;

data = [];
while (size(data) < 7500)
    availableBytes = udp.NumBytesAvailable;
    bytecount = bytecount + availableBytes;

    %recieve data packet
    if (availableBytes>0)
        disp("Recieved  " + (bytecount) + " bytes");
        temp = read(udp, availableBytes, "int16");
        data = [data, temp];
        packetsRecieved = packetsRecieved + 1;
    end
    
    
end

disp("--> Finished! Plotting data")

figure(1)
plot(data)


