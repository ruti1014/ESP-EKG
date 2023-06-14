clc;
clear;


espPort = 4069;
espIP = "192.168.0.27";
udp = udpport;



%Esp connection message
write(udp, "new Phone who dis", "String", espIP, espPort);
pause(1);
disp("Waiting on ESP..." + newline);


done = false;

%Standard packagevalues
packetsize = 3750;
packetAmount = 2;


%recieve data
%esp sends 8 bit, matlab reading 16 bit
packetsRecieved = 0;

data = [];
while (true && not(done))
    availableBytes = udp.NumBytesAvailable;
    

    %recieve data packet
    if (availableBytes>0)
        disp("Recieved packet " + (1 + packetsRecieved) + "/" + packetAmount);
        disp("Bytes: " + availableBytes);
        temp = read(udp, availableBytes, "int16");
        packetAmount = temp(1);
        data = cat(2, data, temp(2:(availableBytes/2)));
        packetsRecieved = packetsRecieved + 1;
    end
    
    if (packetsRecieved >= packetAmount)
        done = true;
    end
    
    pause(.1)
end

disp("--> Finished, plotting data")

figure(1)
plot(data)


