%
% Copyright 2014 Samuele Maci (macisamuele@gmail.com)
%
% Licensed under the Apache License, Version 2.0 (the "License");
% you may not use this file except in compliance with the License.
% You may obtain a copy of the License at
%
% http://www.apache.org/licenses/LICENSE-2.0
%
% Unless required by applicable law or agreed to in writing, software
% distributed under the License is distributed on an "AS IS" BASIS,
% WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
% See the License for the specific language governing permissions and
% limitations under the License.
%

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