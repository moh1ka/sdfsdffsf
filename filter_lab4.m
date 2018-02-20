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
%required for filter of the specification above
[n,fo,ao,w] = firpmord(f,a,dev,fs);

%firpm(N,F,A,W) returns a filter of order N+1 
%therefore, we need a buffer of size length N+1 in our C code
B = firpm(n,fo,ao,w);

freqz(B);

%writing buffer size and coefficient values to a 'fir_coef.txt'
write_B = fopen('fir_coef.txt','wt');
 
%this prints out the length of the buffer required for the filter in C
fprintf(write_B,'#define N %d\n', length(B));
%this intializes the buffer 
fprintf(write_B,'double b[] = {');
%------write coeffs into array-------------------------
for i = 1: length(B)-1
    fprintf(write_B,'%e, ',B(i));
end
fprintf(write_B,'%c}; \n', B(length(B)));
fclose('all');
