%% Init 
n = 7500;
FS = 250;

%% R-Zacken erkennung
b=bandstop(data,[49.5 50.5],FS);  % Y = signal, FS = Sample rate, [] = stopband frequnzyrate 

R_data_diff = diff(b);
R_data_diff = power(R_data_diff,2);
for m=1:length(R_data_diff)                
    if (R_data_diff(m)<20000)
        R_data_diff(m)= 0;
    end
end



minInterval = 91;
indices = [];
maxima=[];
index=1;

for i=1:length(R_data_diff)
    if (R_data_diff(i) > R_data_diff(index) && R_data_diff(i)>0)
        index = i-1;   
    end

    if (mod(i, minInterval) == 0 && index > 1)
        maxima = [maxima,R_data_diff(index)];
        indices = [indices, index];
        index = 1;
    end
end

figure(1)
plot(b,'b')
title('Detektieren der R-Zacken')
grid on
hold on
plot(indices,b(indices),'r*')


%Herzrate berechnen

%timestep 4000µs or 4ms

hrt_rate_avg = 0;

for i=1:length(indices)-1

    r_diff(i) = ((indices(i+1)-indices(i)))*0.004; %time diff [s]

    hrt_rate_avg = hrt_rate_avg + r_diff(i);

end

r_diff = (1./r_diff) .* 60;

figure(2)
plot(r_diff,'b o')
title('Herzratenvariabilität')
grid on

hrt_rate_avg_sec = hrt_rate_avg/(length(indices)-1)
hrt_rate_avg_hz = 1/hrt_rate_avg_sec           %herzrate in [1/s]
hrt_rate_avg_bpm = hrt_rate_avg_hz * 60     %herzrate in [1/min]

 
%% DFT 
n_fft=linspace(0,FS,n);
X=zeros(1,n);
Xb=zeros(1,n);

for k=1:n
    for o=1:n
        X(k)=X(k)+data(o)*exp(-2*pi*1i*k*o/n);
    end
end

for k=1:n
    for o=1:n
        Xb(k)=Xb(k)+b(o)*exp(-2*pi*1i*k*o/n);
    end
end


figure(3)
subplot(2,1,1)
semilogy(n_fft,abs(X))
ylim([0 10^5]);
grid on
title('Frequenzspektrum EKG-Signal, ohne Bandsperre')
subplot(2,1,2)
semilogy(n_fft,abs(Xb))
ylim([0 10^5]);
grid on
title('Frequenzspektrum EKG-Signal, mit Bandsperre')

