clear all;clc;clf

p = 12;
nv = 1;
np = 2 * p * (p + 1);
ntotal = nv * np;

xname    = './Surfaces.out';
gridname = './EvaluationPoints.out';
velname  = '../test/VelocityField.out';

%- Reading the configuration
xv = ReadAscii(xname);
PlotShape(reshape(xv,[],nv),p);

%- Generating the grid points
gridsize = {50,6,6};
gridlim  = [-1.5 1.5;-1 1;-1 1];
gridSpec = {linspace(gridlim(1,1),gridlim(1,2),gridsize{1}),
            linspace(gridlim(2,1),gridlim(2,2),gridsize{2}),
            linspace(gridlim(3,1),gridlim(3,2),gridsize{3})};           

[x y z] = meshgrid(gridSpec{:});
XX = [x(:);y(:);z(:)];
save(gridname,'XX','-ascii'); 
gridsize = mat2cell(size(x),1,[1 1 1]);

%- Reading streamline data
if ( exist(velname) )
  XX = ReadAscii(gridname);
  UU = ReadAscii(velname);

  xv = reshape(xv,3*np,nv);
  XX = reshape(XX,[],3);
  UU = reshape(UU,[],3);

  xx = reshape(XX(:,1), gridsize{:});
  yy = reshape(XX(:,2), gridsize{:});
  zz = reshape(XX(:,3), gridsize{:});

  uu = reshape(UU(:,1), gridsize{:});
  vv = reshape(UU(:,2), gridsize{:});
  ww = reshape(UU(:,3), gridsize{:});

%   hold on;
%   quiver3(xx,yy,zz,uu,vv,ww,.5);
%   axis(reshape(gridlim',1,[]));
%   axis on;
%   hold off;
  xs = [squeeze(xx(:,end,end/2+1:end)) squeeze(xx(:,1,1:end/2))];
  ys = squeeze(yy(:,1,:));
  zs = squeeze(zz(:,1,:));

  hold on;
  vertices = stream3(xx,yy,zz,uu,vv,ww, xs, ys, zs);
  streamtube(vertices,.05);
  view(3);
  axis tight;
  shading interp;
  camlight; lighting gouraud
  hold off;
end
