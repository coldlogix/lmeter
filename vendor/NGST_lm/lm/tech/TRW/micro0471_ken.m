% M-file to calculate the inductance per unit length of a microstrip line
% for various widths using Chang's formula. (2D, long line)
% 
% units are pH and um
%
	mu0 = 1.256637;
%
% layer definitions
% lambda is estimated penetration depth of the conductors
% resitor has effective penetration depth of 1/3 film thickness
%        1      2      3      4      5      6      7      8      9
layer= {'GND'  'SIOG' 'TRIW' 'SIOA' 'RESI' 'SIOR' 'WIRA' 'SIOB' 'WIRB'};
thick= [0.500  0.15  0.200  0.160  0.080  0.100  0.500  0.620  0.950];
overe= [0.000  0.015  0.000  0.010  0.000  0.020  0.000  0.050  0.000];
lambda=[0.400  0      0.48  0      0.027  0      0.350  0      0.400];
%
% layer pairs
% fudge is a factor for each layer to shift result to match empirical data
pair =[ 3,   1;    7,   1;    7,   3;     9,   1;    9,   3;     9,   7;     5,   1;    5,   3;     7,   5];
%pair=('TRIW/GND' 'WIRA/GND' 'WIRA/TRIW' 'WIRB/GND' 'WIRB/TRIW' 'WIRB/WIRA' 'RESI/GND' 'RESI/TRIW' 'WIRA/RESI')
fudge=[ 1.00       1.00       1           1.00       1           1           1          1           1];
%
%wire widths
width=[1.5  1.75  2.0  2.25  2.5  2.75  3.0  3.5  4.0  4.5  5.0  5.5  6  7  8  9  10  11  12  14  16  18  20  22  24];
%
outfile='trash';
%
%end user-specified data

L = zeros(length(width),length(pair),3);
for i = 1:1:length(pair);
  t1= thick(pair(i,1));
  t2= thick(pair(i,2));
  lambda1=lambda(pair(i,1));
  lambda2=lambda(pair(i,2));
  % add up thickness of all the insulator layers in between electrodes
  % these have even index
  h = 0;
  flag = 0;
  for j = pair(i,1) - 1: -2: pair(i,2) + 1;
    h = h + thick(j);
    if (flag == 1)
      h = h - overe(j);
    end
    flag = 1;
  end;
  for j = 1: 1: length(width);
    W = width(j);
    %refer to Chang's paper for eqn numbers
    % (13)
    Beta=1 +t1/h;
    % (12)
    p=2*Beta^2 -1 +sqrt((2*Beta^2 -1)^2 -1);
    % (11)
    n=sqrt(p)*(pi*W/(2*h) +(p+1)/(2*sqrt(p))*(1 +log(4/(p-1))) -2*atanh(1/sqrt(p)));
    % (10)
    if (p > n)
      Deta=p;
    else
      Deta=n;
    end
    % (9)
    rbo=n +(p+1)/2*log(Deta);
    % (7)
    if ((W/h) >= 5)
      rb=rbo;
    % (8)
    elseif ((5 > (W/h)) & ((W/h) >= 1))
      rb=rbo -sqrt((rbo-1)*(rbo-p)) +(p+1)*atanh(sqrt((rbo-p)/(rbo-1))) -2*sqrt(p)*atanh(sqrt((rbo-p)/(p*(rbo-1)))) +pi*W/(2*h)*sqrt(p);
    else
      printf('Error. W/h is less than unity for indexes i=%d  j=%d\n',i,j);
    end
    % (6)
    ra=exp(-1 -pi*W/(2*h) -(p+1)/sqrt(p)*atanh(1/sqrt(p)) -log((p-1)/(4*p)));
    % (15)
    K=h*2/(W*pi)*log(2*rb/ra);
    % (26a)
    % he is effective layer separation
    he = h;
    he = he + lambda1*coth(t1/lambda1);
    he = he + lambda2*coth(t2/lambda2);
    EffH(j,i,1) = he;
    he1 = he + lambda1*2*p^0.5/rb*csch(t1/lambda1);
    EffH(j,i,2) = he1;
    % (26)
    % inductance per square  [pH/square]
    L(j, i, 1) = mu0/K*he1*fudge(i);
    L1(j, i, 1) = mu0/K*he*fudge(i);
    % inductance per unit length  [pH/um]
    L(j, i, 2) = L(j, i, 1)/W;
    % diagnostic: this value should be greater than unity
    L(j,i,3) = W/he;
  end
end

%print the table
fprintf(outfile,'Microstrip Inductance\nUnits are pH and um\n');
fprintf(outfile,'Layer, thickness, penetration depth, over-etch:\n');
for i = 1:1:length(layer);
  fprintf(outfile,'%4s  %5.3f  %5.3f  %5.3f\n',layer{i},thick(i),lambda(i),overe(i));
end
fprintf(outfile,'\nFudge factor:\n\t');
for i = 1:1:length(pair);
  fprintf(outfile,'%5.3f\t',fudge(i));
end

fprintf(outfile,'\n\n*** pH/Square ***');
fprintf(outfile,'\nWidth\t');
for i = 1:1:length(pair);
  fprintf(outfile,'%s/\t',layer{pair(i,1)});
end
fprintf(outfile,'\n\t');
for i = 1:1:length(pair);
  fprintf(outfile,'%s\t',layer{pair(i,2)});
end
for i = 1:1:length(width);
  fprintf(outfile,'\n%5.2f\t',width(i));
  %print the result for 10 um wide lines
  if (width(i) == 10)
    L(i,1:8,1)
  end
  for j = 1:1:length(pair);
    fprintf(outfile,'%5.3f\t',L(i,j,1));
  end
end

fprintf(outfile,'\n\n*** pH/um ***');
fprintf(outfile,'\nWidth\t');
for i = 1:1:length(pair);
  fprintf(outfile,'%s/\t',layer{pair(i,1)});
end
fprintf(outfile,'\n\t');
for i = 1:1:length(pair);
  fprintf(outfile,'%s\t',layer{pair(i,2)});
end
for i = 1:1:length(width);
  fprintf(outfile,'\n%5.2f\t',width(i));
  for j = 1:1:length(pair);
    fprintf(outfile,'%6.4f\t',L(i,j,2));
  end
end
fprintf(outfile,'\n');

fprintf(outfile,'\n\n*** ratio W/he should be greater than unity ***');
fprintf(outfile,'\nWidth\t');
for i = 1:1:length(pair);
  fprintf(outfile,'%s/\t',layer{pair(i,1)});
end
fprintf(outfile,'\n\t');
for i = 1:1:length(pair);
  fprintf(outfile,'%s\t',layer{pair(i,2)});
end
for i = 1:1:length(width);
  fprintf(outfile,'\n%5.2f\t',width(i));
  for j = 1:1:length(pair);
    fprintf(outfile,'%6.4f\t',L(i,j,3));
  end
end
fprintf(outfile,'\n');

%plot the table
%plot(width,L(1:length(width),1,1),'-',width,L(1:length(width),2,1),'-',width,L(1:length(width),3,1),'-',width,L(1:length(width),4,1),'-',width,L(1:length(width),5,1),'-',width,L(1:length(width),6,1),'-',width,L(1:length(width),7,1),'--',width,L(1:length(width),8,1),'--',width,L(1:length(width),9,1),'--')
%xlabel('line width (\mum)');
%ylabel('1/Inductance (sqr/pH)');
%legend('TRIW/GND', 'WIRA/GND', 'WIRA/TRIW', 'WIRB/GND', 'WIRB/TRIW', 'WIRB/WIRA', 'RESI/GND', 'RESI/TRIW', 'WIRA/RESI')

%plot data and table

%SEA-0471-1
width1=[2 10 3 10 3.5 7 10 20];
error1=[0.44 0.44 -0.267 -0.267 -0.288 -0.288 -0.288 -0.288];
Lsq1=[1.79 2.26 1.04 1.27 1.22 1.53 1.67 1.89];
%SEA-0471-2
width2=[2 10 3 10 3.5 7 10 20];
error2=[0 0  0 0  0   0 0  0];
Lsq2=[1.76 2.26 1.07 1.33 1.20 1.57 1.69 1.94];
%SEA-0471-3
width3=[2 10 3 10 3.5 7 10 20];
error3=[0 0  0 0  0   0 0  0];
Lsq3=[1.71 2.16 1.04 1.32 1.22 1.61 1.73 1.96];
%SEA-0471-4
width4=[2 10 3 10 3.5 7 10 20];
error4=[0 0  0 0  0   0 0  0];
Lsq4=[1.85 2.33 1.06 1.33 1.07 1.45 1.60 1.83];
%SEA-0422-1
width4221=[2 10 3 10];
error4221=[0 0  0 0];
Lsq4221=[1.87 2.20 1.20 1.40];
%SEA-0292-2
width2922=[3 10];
error2922=[0 0];
Lsq2922=[1.19 1.39];
%SEA-0292-3
width2923=[2 10 3 10 3.5 7 10 20];
error2923=[0 0  0 0  0   0 0  0];
Lsq2923=[1.50 1.83 1.23 1.46 1.09 1.45 1.58 1.82];
%SEA-0292-4
width2924=[3 10];
error2924=[0 0];
Lsq2924=[1.19 1.42];

%Invert y axis
%L=1./L;
%Lsq1=1./Lsq1;
%Lsq2=1./Lsq2;
%Lsq3=1./Lsq3;
%Lsq4=1./Lsq4;

%plot(width,L(1:length(width),1,1),'-',width,L(1:length(width),2,1),'-',width,L(1:length(width),4,1),'-',width1+error1,Lsq1,'o',width2+error2,Lsq2,'o',width3+error3,Lsq3,'o',width4+error4,Lsq4,'o',width4221+error4221,Lsq4221,'o',width2922+error2922,Lsq2922,'o',width2923+error2923,Lsq2923,'o',width2924+error2924,Lsq2924,'o',0,0);

plot(width,L(1:length(width),1,1),'-',width,L(1:length(width),2,1),'-',width,L(1:length(width),4,1),'-',width,L1(1:length(width),1,1),'-',width,L1(1:length(width),2,1),'-',width,L1(1:length(width),4,1),'-',width1+error1,Lsq1,'o',width2+error2,Lsq2,'o',width3+error3,Lsq3,'o',width4+error4,Lsq4,'o',0,0);

%plot(width,EffH(1:length(width),1,1),'r-',width,EffH(1:length(width),2,1),'b-',width,EffH(1:length(width),4,1),'g-',width,EffH(1:length(width),1,2),'r-',width,EffH(1:length(width),2,2),'b-',width,EffH(1:length(width),4,2),'g-');

h = gca;
set(h,'FontSize',10);
title('Seahawk Inductance Data Fit to Chang''s Formula');
xlabel('line width (\mum)');
ylabel('Inductance (pH/sqr)');
%ylabel('1/Inductance (sqr/pH)');
%%%legend('TRIW/GND: d=145nm', 'WIRA/GND: d=395nm', 'WIRB/GND: d=975nm', 'SEA-0471-1', 'SEA-0471-2', 'SEA-0471-3', 'SEA-0471-4', 4);

orient landscape;
