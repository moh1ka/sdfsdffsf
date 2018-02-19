% FIR
% filter specification
%-----------------------------------------
rp = 0.5;  % ripple of passband
sa = 48;    % minimum stopband attenuation
fs = 8000;  % sampling_frq
f = [355 415 1200 1270];    % cutoff freqs (for each transition)
a = [0 1 0];    % desired amplitudes at the bands defined in f

% Approximating filter order and generating coefficients
%--------------------------------
%equation to calc the max deviation allowable for each band
dev_rp = (10^(rp/20)-1)/(10^(rp/20)+1);
dev_sa = (10^(-sa/20));
dev = [dev_sa dev_rp dev_sa];

%firpmord estimates the order(n); frequency(fo); amplitude(ao) and weights(w)
%required for fileter of the specification above
[n,fo,ao,w] = firpmord(f,a,dev,fs);

%firpm(N,F,A,W) returns a filter of order N+1 i.e. length (b) = N+1
%therefore, we need a buffer of size length(b) in our C code
b = firpm(n,fo,ao,w);

freqz(b)


fvtool(b);

%writing buffer size and coefficient values to a 'fir_coef.txt'
write_b = topen('fir_coef.txt','wt');

%this prints out the length of the butter required for the filter in C
fprintf(write_b,'#define N %d\n', length(b));
fprintf(write_b,'double b[] = (');

for i = 1: length(b)-1
    fprintf(write_b,'%e, ',b(i));
end
fprintf(write_b,'%c); \n', b(length(b)));
fclose('all');

