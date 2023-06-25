clc;
clear;
close all;

espPort = 4069;
espIP = "192.168.188.188";
udp = udpport;



%Esp connection message
write(udp, "Matlab waiting...", "String", espIP, espPort);
pause(1);
disp("Waiting on ESP..." + newline);


done = false;

%Standard packagevalues
packetsize = 100;
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
    if (availableBytes > packetsize)
        disp("Recieved  " + (bytecount) + " bytes");
        temp = read(udp, packetsize, "int16");
        data = [data, temp];
        packetsRecieved = packetsRecieved + 1;
    end    
end

disp("--> Finished! Plotting data")

figure(5)
plot(data)

save EKG_DATA.mat data
save("EKG_rec_sav.txt",'data','-ascii');

postProcessing

