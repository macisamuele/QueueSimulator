function [size, perc_of_avgX, Imin, Imax] = IntervalOfConfidence(X, CL)
% X: samples
% CL: confidence level
    if(~isvector(X))
        error('X must be a vector')
        Imin=0
        Imax=0
    end
    if(CL<0 || CL>100)
        error('CL must be a percentage, so in the interval 0 100')
        Imin=0
        Imax=0
    end
    n = length(X);
    s = std(X, 1);
    alpha = 1-CL/100
    % X ~ N(0, 1)   P(X<y) = 1/2*[1+erf(y/sqrt(2))] = 1- erfc(y/sqrt(2))/2
    % P(X>y) = erfc(y/sqrt(2))/2 = alpha/2 => alpha=erfc(y/sqrt(2)) => 
    % y/sqrt(2) = erfcinv(alpha) => y = sqrt(2)*erfcinv(alpha)
    ERFC=sqrt(2)*erfcinv(alpha);
    size = ERFC*s/sqrt(n);
    perc_of_avgX = 100*ERFC*s/sqrt(n)/mean(X);
    Imin = mean(X)-ERFC*s/sqrt(n);
    Imax = mean(X)+ERFC*s/sqrt(n);
end