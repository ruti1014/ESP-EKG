clc;
clear;


espPort = 4069;
espIP = "192.168.0.27";
udp = udpport;



%Esp connection message
write(udp, "new Phone who dis", "String", espIP, espPort);
pause(1);



connected = true;
%Standard packagevalues
packetsize = 3750;
packageAmount = 2;


connected = false;
%wait for packagesize and amount
while (not(connected))
    if (udp.NumBytesAvailable>0)
        data = read(udp,2,'uint16');
        connected = true;
        pause(.1); 
    end
end

packageAmount = data(1)
packetsize = data(2)





%recieve data
%esp sends 8 bit recieving 16 bit
packageRecieved = 0;

data = [];
if (packetsize > 0)
    while (true && connected)
        if (udp.NumBytesAvailable>0)
            temp = read(udp,packetsize,'uint16');
            data = cat(2, data, temp);
            packageRecieved = packageRecieved + 1
            pause(.01); 
            flush(udp)
        end

        if (packageRecieved >= packageAmount)
            connected = false;
        end
    end
end

figure(1)
plot(data)


