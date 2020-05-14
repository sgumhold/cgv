/* modified version of dt.c originaly written by Alexander Vasilevskiy (March 21, 1999) */

#include "3ddt.h"
#include <cmath>
#include <limits>

DEucl3D DT3D::MINforwardDE3(Array3dDEucl3D& A, int z,int y,int x)
{
  int Xdim = A.size_x();
  int Ydim = A.size_y();
  int Zdim = A.size_z();

  //              MASK:
  //
  //             (0,0,0)(0,0,1)
  //      (0,1,1)(0,1,0)(0,1,1)
  //
  
  DEucl3D mask[5];
  DEucl3D min;
  int looper;
  min.distance=infinity;
 
  if (z<Zdim-1) {
    mask[0].v=A(x,y,z+1).v; 
    mask[0].h=A(x,y,z+1).h;
    mask[0].d=A(x,y,z+1).d+1;
    mask[0].distance=sqrtf(mask[0].v*mask[0].v+mask[0].h*mask[0].h+mask[0].d*mask[0].d);
  }
  else {
    mask[0].v=infinity;
    mask[0].h=infinity;
    mask[0].d=infinity;
    mask[0].distance=infinity;
  }

  if ((y<Ydim-1)&&(z<Zdim-1)) {
    mask[1].v=A(x,y,z+1).v; 
    mask[1].h=A(x,y,z+1).h+1;
    mask[1].d=A(x,y,z+1).d+1;
    mask[1].distance=sqrtf(mask[1].v*mask[1].v+mask[1].h*mask[1].h+mask[1].d*mask[1].d);
  }
  else {
    mask[1].v=infinity;
    mask[1].h=infinity;
    mask[1].d=infinity;
    mask[1].distance=infinity;

  }
  
  if (y<Ydim-1) {
    mask[2].v=A(x,y+1,z).v; 
    mask[2].h=A(x,y+1,z).h+1; 
    mask[2].d=A(x,y+1,z).d; 
    mask[2].distance=sqrtf(mask[2].v*mask[2].v+mask[2].h*mask[2].h+mask[2].d*mask[2].d);
  }
  else {
    mask[2].v=infinity;
    mask[2].h=infinity;
    mask[2].d=infinity;
    mask[2].distance=infinity;
  }
  mask[3].v=A(x,y,z).v;
  mask[3].h=A(x,y,z).h;
  mask[3].d=A(x,y,z).d;
  mask[3].distance=sqrtf(mask[3].v*mask[3].v+mask[3].h*mask[3].h+mask[3].d*mask[3].d);
  if ((z>0)&&(y<Ydim-1)) {
    mask[4].v=A(x,y+1,z-1).v;
    mask[4].h=A(x,y+1,z-1).h+1;
    mask[4].d=A(x,y+1,z-1).d+1;
    mask[4].distance=sqrtf(mask[4].v*mask[4].v+mask[4].h*mask[4].h+mask[4].d*mask[4].d);
  }
 else {
   mask[4].v=infinity; 
   mask[4].h=infinity;
   mask[4].d=infinity;
   mask[4].distance=infinity;
 }

  for(looper=0;looper<5;looper++) {
    if (mask[looper].distance<min.distance) min=mask[looper];
  }
 
  return min;
}


DEucl3D DT3D::MINforwardDE4(Array3dDEucl3D& A, int z,int y,int x)
{
  int Xdim = A.size_x();
  int Ydim = A.size_y();
  int Zdim = A.size_z();
  
  //
  //   MASK:   (0,0,1)(0,0,0)
  //
  DEucl3D mask[2];
  DEucl3D min;
  int looper;
  min.distance=infinity;
  if (z>0) {
    mask[0].v=A(x,y,z-1).v; 
    mask[0].h=A(x,y,z-1).h;
    mask[0].d=A(x,y,z-1).d+1;
    mask[0].distance=sqrtf(mask[0].v*mask[0].v+mask[0].h*mask[0].h+mask[0].d*mask[0].d);         
  }
  else {
    mask[0].v=infinity;
    mask[0].h=infinity;
    mask[0].d=infinity;
    mask[0].distance=infinity;
  }
  mask[1].v=A(x,y,z).v;
  mask[1].h=A(x,y,z).h;
  mask[1].d=A(x,y,z).d; 
  mask[1].distance=sqrtf(mask[1].v*mask[1].v+mask[1].h*mask[1].h+mask[1].d*mask[1].d);
  
  for(looper=0;looper<2;looper++) {
    if (mask[looper].distance<min.distance) min=mask[looper];
  }
 
  return min;
}

DEucl3D DT3D::MINbackwardDE3(Array3dDEucl3D& A, int z,int y,int x)
{
  int Xdim = A.size_x();
  int Ydim = A.size_y();
  int Zdim = A.size_z();

  //
  //                     MASK:
  //
  //             (0,1,1)(0,1,0)(0,1,1)
  //             (0,0,1)(0,0,0)
  //
  //
  
  
  DEucl3D mask[5];
  DEucl3D min;
  float temp;
  int looper;
  min.distance=infinity;
  if ((z>0)&&(y>0)) {
    mask[0].v=A(x,y-1,z-1).v; 
    mask[0].h=A(x,y-1,z-1).h+1;
    mask[0].d=A(x,y-1,z-1).d+1;
    mask[0].distance=sqrtf(mask[0].v*mask[0].v+mask[0].h*mask[0].h+mask[0].d*mask[0].d);
  }
  else {
    mask[0].v=infinity;
    mask[0].h=infinity;
    mask[0].d=infinity;
    mask[0].distance=infinity;
  }
  if (y>0){
    mask[1].v=A(x,y-1,z).v; 
    mask[1].h=A(x,y-1,z).h+1;
    mask[1].d=A(x,y-1,z).d;
    mask[1].distance=sqrtf(mask[1].v*mask[1].v+mask[1].h*mask[1].h+mask[1].d*mask[1].d);
  }
  else {
    mask[1].v=infinity;
    mask[1].h=infinity;
    mask[1].d=infinity;
    mask[1].distance=infinity;

  }
  if ((z<Zdim-1)&&(y>0)) {
    mask[2].v=A(x,y-1,z+1).v; 
    mask[2].h=A(x,y-1,z+1).h+1; 
    mask[2].d=A(x,y-1,z+1).d+1; 
    mask[2].distance=sqrtf(mask[2].v*mask[2].v+mask[2].h*mask[2].h+mask[2].d*mask[2].d);
  }
  else {
    mask[2].v=infinity;
    mask[2].h=infinity;
    mask[2].d=infinity;
    mask[2].distance=infinity;
  }
  mask[3].v=A(x,y,z).v;
  mask[3].h=A(x,y,z).h;
  mask[3].d=A(x,y,z).d;
  mask[3].distance=sqrtf(mask[3].v*mask[3].v+mask[3].h*mask[3].h+mask[3].d*mask[3].d);
  if (z>0) {
    mask[4].v=A(x,y,z-1).v;
    mask[4].h=A(x,y,z-1).h;
    mask[4].d=A(x,y,z-1).d+1;
    mask[4].distance=sqrtf(mask[4].v*mask[4].v+mask[4].h*mask[4].h+mask[4].d*mask[4].d);
  }
 else {
   mask[4].v=infinity; 
   mask[4].h=infinity;
   mask[4].d=infinity;
   mask[4].distance=infinity;
 }

  for(looper=0;looper<5;looper++) {
    if (mask[looper].distance<min.distance) min=mask[looper];
  }
 
  return min;
}


DEucl3D DT3D::MINforwardDE2(Array3dDEucl3D& A, int z,int y,int x)
{
  int Xdim = A.size_x();
  int Ydim = A.size_y();
  int Zdim = A.size_z();

  //
  // MASK:   (0,0,0)(0,0,1)
  //

  DEucl3D mask[2];
  DEucl3D min;
  int looper;
  min.distance=infinity;
  if (z<Zdim-1) {
    mask[0].v=A(x,y,z+1).v; 
    mask[0].h=A(x,y,z+1).h;
    mask[0].d=A(x,y,z+1).d+1;
    mask[0].distance=sqrtf(mask[0].v*mask[0].v+mask[0].h*mask[0].h+mask[0].d*mask[0].d);         
  }
  else {
    mask[0].v=infinity;
    mask[0].h=infinity;
    mask[0].d=infinity;
    mask[0].distance=infinity;
  }
  mask[1].v=A(x,y,z).v;
  mask[1].h=A(x,y,z).h;
  mask[1].d=A(x,y,z).d;
  mask[1].distance=sqrtf(mask[1].v*mask[1].v+mask[1].h*mask[1].h+mask[1].d*mask[1].d);
  
  for(looper=0;looper<2;looper++) {
    if (mask[looper].distance<min.distance) min=mask[looper];
  }
 
 
  return min;
}

DEucl3D DT3D::MINforwardDE1(Array3dDEucl3D& A, int z, int y, int x)
{
	int Xdim = A.size_x();
	int Ydim = A.size_y();
	int Zdim = A.size_z();

	DEucl3D mask[14];
	DEucl3D min;
	float temp;
	int looper;
	min.distance = infinity;


	//                     MASK:
	//
	//             (1,1,1)(1,1,0)(1,1,1)
	//             (1,0,1)(1,0,0)(1,0,1)
	//             (1,1,1)(1,1,0)(1,1,1)
	//
	if ((z > 0) && (y > 0) && (x > 0)) {
		mask[0].v = A(x - 1, y - 1, z - 1).v + 1;
		mask[0].h = A(x - 1, y - 1, z - 1).h + 1;
		mask[0].d = A(x - 1, y - 1, z - 1).d + 1;
		mask[0].distance = sqrtf(mask[0].v*mask[0].v + mask[0].h*mask[0].h + mask[0].d*mask[0].d);
	}
	else {
		mask[0].v = infinity;
		mask[0].h = infinity;
		mask[0].d = infinity;
		mask[0].distance = infinity;
	}
	if ((y > 0) && (x > 0)) {
		mask[1].v = A(x-1,y-1,z).v +1;
		mask[1].h = A(x - 1, y - 1, z).h + 1;
		mask[1].d = A(x - 1, y - 1, z).d;
		mask[1].distance = sqrtf(mask[1].v*mask[1].v + mask[1].h*mask[1].h + mask[1].d*mask[1].d);
	}
	else {
		mask[1].v = infinity;
		mask[1].h = infinity;
		mask[1].d = infinity;
		mask[1].distance = infinity;

	}
	if ((z < Zdim - 1) && (y > 0) && (x > 0)) {
		mask[2].v = A(x - 1, y - 1, z + 1).v + 1;
		mask[2].h = A(x - 1, y - 1, z + 1).h + 1;
		mask[2].d = A(x - 1, y - 1, z + 1).d + 1;
		mask[2].distance = sqrtf(mask[2].v*mask[2].v + mask[2].h*mask[2].h + mask[2].d*mask[2].d);
	}
	else {
		mask[2].v = infinity;
		mask[2].h = infinity;
		mask[2].d = infinity;
		mask[2].distance = infinity;
	}
	if ((z > 0) && (x > 0)) {
		mask[3].v = A(x - 1, y, z - 1).v + 1;
		mask[3].h = A(x - 1, y, z - 1).h;
		mask[3].d = A(x - 1, y, z - 1).d + 1;
		mask[3].distance = sqrtf(mask[3].v*mask[3].v + mask[3].h*mask[3].h + mask[3].d*mask[3].d);
	}
	else {
		mask[3].v = infinity;
		mask[3].h = infinity;
		mask[3].d = infinity;
		mask[3].distance = infinity;
	}
	if (x > 0) {
		mask[4].v = A(x - 1, y, z).v + 1;
		mask[4].h = A(x - 1, y, z).h;
		mask[4].d = A(x - 1, y, z).d;
		mask[4].distance = sqrtf(mask[4].v*mask[4].v + mask[4].h*mask[4].h + mask[4].d*mask[4].d);
	}
	else {
		mask[4].v = infinity;
		mask[4].h = infinity;
		mask[4].d = infinity;
		mask[4].distance = infinity;
	}
	if ((x > 0) && (z < Zdim - 1)) {
		mask[5].v = A(x - 1, y, z + 1).v + 1;
		mask[5].h = A(x - 1, y, z + 1).h;
		mask[5].d = A(x - 1, y, z + 1).d + 1;
		mask[5].distance = sqrtf(mask[5].v*mask[5].v + mask[5].h*mask[5].h + mask[5].d*mask[5].d);
	}
	else {
		mask[5].v = infinity;
		mask[5].h = infinity;
		mask[5].d = infinity;
		mask[5].distance = infinity;
	}
	if ((x > 0) && (z > 0) && (y < Ydim - 1)) {
		mask[6].v = A(x - 1, y + 1, z - 1).v + 1;
		mask[6].h = A(x - 1, y + 1, z - 1).h + 1;
		mask[6].d = A(x - 1, y + 1, z - 1).d + 1;
		mask[6].distance = sqrtf(mask[6].v*mask[6].v + mask[6].h*mask[6].h + mask[6].d*mask[6].d);
	}
	else {
		mask[6].v = infinity;
		mask[6].h = infinity;
		mask[6].d = infinity;
		mask[6].distance = infinity;
	}
	if ((x > 0) && (y < Ydim - 1)) {
		mask[7].v = A(x - 1, y + 1, z).v + 1;
		mask[7].h = A(x - 1, y + 1, z).h + 1;
		mask[7].d = A(x - 1, y + 1, z).d;
		mask[7].distance = sqrtf(mask[7].v*mask[7].v + mask[7].h*mask[7].h + mask[7].d*mask[7].d);
	}
	else {
		mask[7].v = infinity;
		mask[7].h = infinity;
		mask[7].d = infinity;
		mask[7].distance = infinity;
	}
	if ((x > 0) && (y < Ydim - 1) && (z < Zdim - 1)) {
		mask[8].v = A(x - 1, y + 1, z + 1).v + 1;
		mask[8].h = A(x - 1, y + 1, z + 1).h + 1;
		mask[8].d = A(x - 1, y + 1, z + 1).d + 1;
		mask[8].distance = sqrtf(mask[8].v*mask[8].v + mask[8].h*mask[8].h + mask[8].d*mask[8].d);
	}
	else {
		mask[8].v = infinity;
		mask[8].h = infinity;
		mask[8].d = infinity;
		mask[8].distance = infinity;
	}

	//
	//                     MASK:
	//
	//             (0,1,1)(0,1,0)(0,1,1)
	//             (0,0,1)(0,0,0)
	//
	//


	if ((z > 0) && (y > 0)) {
		mask[9].v = A(x, y - 1, z - 1).v;
		mask[9].h = A(x, y - 1, z - 1).h + 1;
		mask[9].d = A(x, y - 1, z - 1).d + 1;
		mask[9].distance = sqrtf(mask[9].v*mask[9].v + mask[9].h*mask[9].h + mask[9].d*mask[9].d);
	}
	else {
		mask[9].v = infinity;
		mask[9].h = infinity;
		mask[9].d = infinity;
		mask[9].distance = infinity;
	}


	if (y > 0) {
		mask[10].v = A(x, y - 1, z).v;
		mask[10].h = A(x, y - 1, z).h + 1;
		mask[10].d = A(x, y - 1, z).d;
		mask[10].distance = sqrtf(mask[10].v*mask[10].v + mask[10].h*mask[10].h + mask[10].d*mask[10].d);
	}
	else {
		mask[10].v = infinity;
		mask[10].h = infinity;
		mask[10].d = infinity;
		mask[10].distance = infinity;

	}


	if ((z < Zdim - 1) && (y > 0)) {
		mask[11].v = A(x, y - 1, z + 1).v;
		mask[11].h = A(x, y - 1, z + 1).h + 1;
		mask[11].d = A(x, y - 1, z + 1).d + 1;
		mask[11].distance = sqrtf(mask[11].v*mask[11].v + mask[11].h*mask[11].h + mask[11].d*mask[11].d);
	}
	else {
		mask[11].v = infinity;
		mask[11].h = infinity;
		mask[11].d = infinity;
		mask[11].distance = infinity;
	}

	mask[12].v = A(x, y, z).v;
	mask[12].h = A(x, y, z).h;
	mask[12].d = A(x, y, z).d;
	mask[12].distance = sqrtf(mask[12].v*mask[12].v + mask[12].h*mask[12].h + mask[12].d*mask[12].d);

	if (z > 0) {
		mask[13].v = A(x, y, z - 1).v;
		mask[13].h = A(x, y, z - 1).h;
		mask[13].d = A(x, y, z - 1).d + 1;
		mask[13].distance = sqrtf(mask[13].v*mask[13].v + mask[13].h*mask[13].h + mask[13].d*mask[13].d);
	}
	else {
		mask[13].v = infinity;
		mask[13].h = infinity;
		mask[13].d = infinity;
		mask[13].distance = infinity;
	}

	for (looper = 0; looper < 14; looper++) {
		if (mask[looper].distance < min.distance) min = mask[looper];
	}

	return min;
}

DEucl3D DT3D::MINbackwardDE1(Array3dDEucl3D& A, int z, int y, int x)
{
	int Xdim = A.size_x();
	int Ydim = A.size_y();
	int Zdim = A.size_z();

	//
	//                     MASK:
	//
	//             (1,1,1)(1,1,0)(1,1,1)
	//             (1,0,1)(1,0,0)(1,0,1)
	//             (1,1,1)(1,1,0)(1,1,1)
	//


	DEucl3D mask[14];
	DEucl3D min;
	float temp;
	int looper;
	min.distance = infinity;

	if ((z > 0) && (y > 0) && (x < Xdim - 1)) {
		mask[0].v = A(x+1,y-1,z-1).v + 1;
		mask[0].h = A(x+1,y-1,z-1).h + 1;
		mask[0].d = A(x+1,y-1,z-1).d + 1;
		mask[0].distance = sqrtf(mask[0].v*mask[0].v + mask[0].h*mask[0].h + mask[0].d*mask[0].d);
	}
	else {
		mask[0].v = infinity;
		mask[0].h = infinity;
		mask[0].d = infinity;
		mask[0].distance = infinity;
	}

	if ((y > 0) && (x < Xdim - 1)) {
		mask[1].v = A(x+1,y-1,z).v + 1;
		mask[1].h = A(x+1,y-1,z).h + 1;
		mask[1].d = A(x+1,y-1,z).d;
		mask[1].distance = sqrtf(mask[1].v*mask[1].v + mask[1].h*mask[1].h + mask[1].d*mask[1].d);
	}
	else {
		mask[1].v = infinity;
		mask[1].h = infinity;
		mask[1].d = infinity;
		mask[1].distance = infinity;

	}
	if ((z < Zdim - 1) && (y > 0) && (x < Xdim - 1)) {
		mask[2].v = A(x+1,y-1,z+1).v + 1;
		mask[2].h = A(x+1,y-1,z+1).h + 1;
		mask[2].d = A(x+1,y-1,z+1).d + 1;
		mask[2].distance = sqrtf(mask[2].v*mask[2].v + mask[2].h*mask[2].h + mask[2].d*mask[2].d);
	}
	else {
		mask[2].v = infinity;
		mask[2].h = infinity;
		mask[2].d = infinity;
		mask[2].distance = infinity;
	}

	if ((z > 0) && (x < Xdim - 1)) {
		mask[3].v = A(x+1,y,z-1).v + 1;
		mask[3].h = A(x+1,y,z-1).h;
		mask[3].d = A(x+1,y,z-1).d + 1;
		mask[3].distance = sqrtf(mask[3].v*mask[3].v + mask[3].h*mask[3].h + mask[3].d*mask[3].d);
	}
	else {
		mask[3].v = infinity;
		mask[3].h = infinity;
		mask[3].d = infinity;
		mask[3].distance = infinity;
	}

	if (x < Xdim - 1) {
		mask[4].v = A(x+1,y,z).v + 1;
		mask[4].h = A(x+1,y,z).h;
		mask[4].d = A(x+1,y,z).d;
		mask[4].distance = sqrtf(mask[4].v*mask[4].v + mask[4].h*mask[4].h + mask[4].d*mask[4].d);
	}
	else {
		mask[4].v = infinity;
		mask[4].h = infinity;
		mask[4].d = infinity;
		mask[4].distance = infinity;
	}

	if ((x < Xdim - 1) && (z < Zdim - 1)) {
		mask[5].v = A(x+1,y,z+1).v + 1;
		mask[5].h = A(x+1,y,z+1).h;
		mask[5].d = A(x+1,y,z+1).d + 1;
		mask[5].distance = sqrtf(mask[5].v*mask[5].v + mask[5].h*mask[5].h + mask[5].d*mask[5].d);
	}
	else {
		mask[5].v = infinity;
		mask[5].h = infinity;
		mask[5].d = infinity;
		mask[5].distance = infinity;
	}

	if ((x < Xdim - 1) && (z > 0) && (y < Ydim - 1)) {
		mask[6].v = A(x+1,y+1,z-1).v + 1;
		mask[6].h = A(x+1,y+1,z-1).h + 1;
		mask[6].d = A(x+1,y+1,z-1).d + 1;
		mask[6].distance = sqrtf(mask[6].v*mask[6].v + mask[6].h*mask[6].h + mask[6].d*mask[6].d);
	}
	else {
		mask[6].v = infinity;
		mask[6].h = infinity;
		mask[6].d = infinity;
		mask[6].distance = infinity;
	}

	if ((x < Xdim - 1) && (y < Ydim - 1)) {
		mask[7].v = A(x+1,y+1,z).v + 1;
		mask[7].h = A(x+1,y+1,z).h + 1;
		mask[7].d = A(x+1,y+1,z).d;
		mask[7].distance = sqrtf(mask[7].v*mask[7].v + mask[7].h*mask[7].h + mask[7].d*mask[7].d);
	}
	else {
		mask[7].v = infinity;
		mask[7].h = infinity;
		mask[7].d = infinity;
		mask[7].distance = infinity;
	}

	if ((x < Xdim - 1) && (y < Ydim - 1) && (z < Zdim - 1)) {
		mask[8].v = A(x+1,y+1,z+1).v + 1;
		mask[8].h = A(x+1,y+1,z+1).h + 1;
		mask[8].d = A(x+1,y+1,z+1).d + 1;
		mask[8].distance = sqrtf(mask[8].v*mask[8].v + mask[8].h*mask[8].h + mask[8].d*mask[8].d);
	}
	else {
		mask[8].v = infinity;
		mask[8].h = infinity;
		mask[8].d = infinity;
		mask[8].distance = infinity;
	}

	//              MASK :
	//
	//             (0,0,0)(0,0,1)
	//      (0,1,1)(0,1,0)(0,1,1)
	//

	if (z < Zdim - 1) {
		mask[9].v = A(x,y,z+1).v;
		mask[9].h = A(x,y,z+1).h;
		mask[9].d = A(x,y,z+1).d + 1;
		mask[9].distance = sqrtf(mask[9].v*mask[9].v + mask[9].h*mask[9].h + mask[9].d*mask[9].d);
	}
	else {
		mask[9].v = infinity;
		mask[9].h = infinity;
		mask[9].d = infinity;
		mask[9].distance = infinity;
	}

	if ((y < Ydim - 1) && (z < Zdim - 1)) {
		mask[10].v = A(x,y,z+1).v;
		mask[10].h = A(x,y,z+1).h + 1;
		mask[10].d = A(x,y,z+1).d + 1;
		mask[10].distance = sqrtf(mask[10].v*mask[10].v + mask[10].h*mask[10].h + mask[10].d*mask[10].d);
	}
	else {
		mask[10].v = infinity;
		mask[10].h = infinity;
		mask[10].d = infinity;
		mask[10].distance = infinity;

	}
	if (y < Ydim - 1) {
		mask[11].v = A(x,y+1,z).v;
		mask[11].h = A(x,y+1,z).h + 1;
		mask[11].d = A(x,y+1,z).d;
		mask[11].distance = sqrtf(mask[11].v*mask[11].v + mask[11].h*mask[11].h + mask[11].d*mask[11].d);
	}
	else {
		mask[11].v = infinity;
		mask[11].h = infinity;
		mask[11].d = infinity;
		mask[11].distance = infinity;
	}
	mask[12].v = A(x,y,z).v;
	mask[12].h = A(x,y,z).h;
	mask[12].d = A(x,y,z).d;
	mask[12].distance = sqrtf(mask[12].v*mask[12].v + mask[12].h*mask[12].h + mask[12].d*mask[12].d);

	if ((z > 0) && (y < Ydim - 1)) {
		mask[13].v = A(x,y+1,z-1).v;
		mask[13].h = A(x,y+1,z-1).h + 1;
		mask[13].d = A(x,y+1,z-1).d + 1;
		mask[13].distance = sqrtf(mask[13].v*mask[13].v + mask[13].h*mask[13].h + mask[13].d*mask[13].d);
	}
	else {
		mask[13].v = infinity;
		mask[13].h = infinity;
		mask[13].d = infinity;
		mask[13].distance = infinity;
	}

	for (looper = 0; looper < 14; looper++) {
		if (mask[looper].distance < min.distance) min = mask[looper];
	}
	return min;
}

void DT3D::DEuclidean(Array3dDEucl3D& A)
{
	int Xdim = A.size_x();
	int Ydim = A.size_y();
	int Zdim = A.size_z();

	int y, x, z;
	
	for (x = 0; x < Xdim; x++) {
		for (y = 0; y < Ydim; y++) {
			for (z = 0; z < Zdim; z++) A(x,y,z) = MINforwardDE1(A, z, y, x);
			for (z = Zdim - 1; z > -1; z--)  A(x, y, z) = MINforwardDE2(A, z, y, x);
		}
		for (y = Ydim - 1; y > -1; y--) {
			for (z = Zdim - 1; z > -1; z--)  A(x, y, z) = MINforwardDE3(A, z, y, x);
			for (z = 0; z < Zdim; z++)  A(x, y, z) = MINforwardDE4(A, z, y, x);
		}
	}
	for (x = Xdim - 1; x > -1; x--) {
		for (y = Ydim - 1; y > -1; y--) {
			for (z = Zdim - 1; z > -1; z--)  A(x, y, z) = MINbackwardDE1(A, z, y, x);
			for (z = 0; z < Zdim; z++) A(x, y, z) = MINforwardDE4(A, z, y, x);
		}
		for (y = 0; y < Ydim; y++) {
			for (z = 0; z < Zdim; z++)  A(x, y, z) = MINbackwardDE3(A, z, y, x);
			for (z = Zdim - 1; z > -1; z--)  A(x, y, z) = MINforwardDE2(A, z, y, x);

		}
	}
}


void DT3D::build(double* _x, double* _y, double* _z, int num)
{
	xMin = _x[0]; xMax = _x[0]; yMin = _y[0]; yMax = _y[0]; zMin = _z[0]; zMax = _z[0];

	int i;
	for (i = 1; i < num; i++)
	{
		if (xMin > _x[i]) xMin = _x[i];
		if (xMax < _x[i]) xMax = _x[i];
		if (yMin > _y[i]) yMin = _y[i];
		if (yMax < _y[i]) yMax = _y[i];
		if (zMin > _z[i]) zMin = _z[i];
		if (zMax < _z[i]) zMax = _z[i];
	}

	double xCenter = (xMin + xMax) / 2;
	double yCenter = (yMin + yMax) / 2;
	double zCenter = (zMin + zMax) / 2;
	xMin = xCenter - expandFactor * (xMax - xCenter);
	xMax = xCenter + expandFactor * (xMax - xCenter);
	yMin = yCenter - expandFactor * (yMax - yCenter);
	yMax = yCenter + expandFactor * (yMax - yCenter);
	zMin = zCenter - expandFactor * (zMax - zCenter);
	zMax = zCenter + expandFactor * (zMax - zCenter);

	double max = xMax - xMin > yMax - yMin ? xMax - xMin : yMax - yMin;
	max = max > zMax - zMin ? max : zMax - zMin;

	xMin = xCenter - max / 2;
	xMax = xCenter + max / 2;
	yMin = yCenter - max / 2;
	yMax = yCenter + max / 2;
	zMin = zCenter - max / 2;
	zMax = zCenter + max / 2;

	scale = size / max;

	A = Array3dDEucl3D(size, size, size);

	int x, y, z;

	int Xdim = A.size_x();
	int Ydim = A.size_y();
	int Zdim = A.size_z();

	for (z = 0; z < Zdim; z++) {
		for (y = 0; y < Ydim; y++) {
			for (x = 0; x < Xdim; x++) {
				A(x, y, z).distance = infinity;
				A(x, y, z).h = infinity;
				A(x, y, z).v = infinity;
				A(x, y, z).distance = infinity;
			}
		}
	}
	for (i = 0; i < num; i++)
	{
		x = round((_x[i] - xMin)*scale);
		y = round((_y[i] - yMin)*scale);
		z = round((_z[i] - zMin)*scale);

		if (x < 0 || x >= Xdim || y < 0 || y >= Ydim || z < 0 || z >= Zdim)
			continue;

		A(x, y, z).distance = 0;
		A(x, y, z).h = 0;
		A(x, y, z).v = 0;
		A(x, y, z).d = 0;
	}

	DEuclidean(A);

	for (z = 0; z < Zdim; z++) {
		for (y = 0; y < Ydim; y++) {
			for (x = 0; x < Xdim; x++) {
				A(x, y, z).distance = (A(x, y, z).distance) / scale;
				if (A(x, y, z).distance < 0)
					A(x, y, z).distance = 0;
			}
		}
	}
}
