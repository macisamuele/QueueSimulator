clear all
close all
clc
n_decimals = 4;
n_points = 10^n_decimals-1;
CL = linspace(0, 1, n_points+2);
CL(1) = [];
CL(end) = [];
fp = fopen('table.txt', 'w');
for cl=CL
    fprintf('CL = %.*f | alpha = %.*f | z_{1-alpha/2} = %.16f\n', ...
        n_decimals, cl, n_decimals, 1-cl, sqrt(2)*erfcinv(1-cl));
    fprintf(fp, 'CL = %.*f | alpha = %.*f | z_{1-alpha/2} = %.16f\n', ...
        n_decimals, cl, n_decimals, 1-cl, sqrt(2)*erfcinv(1-cl));
end
fclose(fp);