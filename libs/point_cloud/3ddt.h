#pragma once
#include <memory>
#include <cassert>

#include "lib_begin.h"

struct DEucl3D {
 short v,h,d;
 float distance;
};


template <typename T>
	class array3d_t {
		T* arr_data;
		int dim_x, dim_y, dim_z;

		void destroy_data() {
			assert(arr_data != nullptr);
			for (int i = 0; i < dim_x*dim_y*dim_z; ++i) {
				arr_data[i].~T();
			}
		}
	public:
		//default constructor
		array3d_t() : arr_data(nullptr), dim_x(0), dim_y(0), dim_z(0) {}

		//copy constructor
		array3d_t(const array3d_t &source) {
			dim_x = source.dim_x;
			dim_y = source.dim_y;
			dim_z = source.dim_z;
			arr_data = reinterpret_cast<T*>(malloc(dim_x*dim_y*dim_z*sizeof(T)));

			for (int i = 0; i < dim_x*dim_y*dim_z; ++i) {
				new (arr_data + i) T(source.arr_data[i]);
			}
		}

		//move constructor
		array3d_t(array3d_t &&source) {
			dim_x = source.dim_x;
			dim_y = source.dim_y;
			dim_z = source.dim_z;
			arr_data = source.arr_data;
			source.arr_data = nullptr;
		}

		array3d_t(const int x, const int y, const int z) : dim_x(x),dim_y(y),dim_z(z){
			arr_data = reinterpret_cast<T*>(malloc(z*y*x * sizeof(T)));
			for (size_t i = 0; i < x*y*z; ++i) {
				new (arr_data + i) T();
			}
		}

		array3d_t(const int x, const int y,const int z, const T init) : dim_x(x), dim_y(y), dim_z(z) {
			arr_data = reinterpret_cast<T*>(malloc(z*y*x * sizeof(T)));
			for (size_t i = 0; i < x*y*z; ++i) {
				new (arr_data + i) T();
				arr_data[i] = init;
			}
		}

		~array3d_t() {
			if (arr_data != nullptr) {
				destroy_data();
				free(arr_data);
			}
		}

		array3d_t& operator=(const array3d_t& source) {
			if (&source != this) {
				if (arr_data != nullptr) {
					destroy_data();
				}
				int x = source.dim_x;
				int y = source.dim_y;
				int z = source.dim_z;
				arr_data = reinterpret_cast<T*>(realloc(arr_data, z*y*x * sizeof(T)));

				for (int i = 0; i < dim_x*dim_y*dim_z; ++i) {
					new (arr_data + i) T(source.arr_data[i]);
				}
			}
			return *this;
		}

		//move assignment
		array3d_t& operator=(array3d_t&& source) {
			dim_x = source.dim_x;
			dim_y = source.dim_y;
			dim_z = source.dim_z;
			arr_data = source.arr_data;
			source.arr_data = nullptr;
			return *this;
		}


		const T& operator()(int x, int y, int z) const {
			assert(x < dim_x && x >= 0);
			assert(y < dim_y && y >= 0);
			assert(z < dim_z && z >= 0);
			return arr_data[z*(dim_y*dim_x) + y * dim_x + x];
		}

		T& operator()(int x, int y, int z) {
			assert(x < dim_x && x >= 0);
			assert(y < dim_y && y >= 0);
			assert(z < dim_z && z >= 0);
			return arr_data[z*(dim_x*dim_y) + y * dim_x + x];
		}

		T* begin() {
			return arr_data;
		}

		T* end() {
			return arr_data + dim_x * dim_y*dim_z;
		}

		const T* begin() const {
			return arr_data;
		}

		const T* end() const {
			return arr_data + dim_x * dim_y*dim_z;
		}

		//setter getter
		inline const int size_x() const {
			return dim_x;
		}

		inline const int size_y() const {
			return dim_y;
		}

		inline const int size_z() const {
			return dim_z;
		}

		inline const T* data() const {
			return arr_data;
		}
	};


typedef array3d_t<DEucl3D> Array3dDEucl3D;
typedef array3d_t<float> Array3dfloat;

class DT3D{
public:
	static const short infinity = std::numeric_limits<short>::max();
	int size;
	double scale;
	double expandFactor;
	double xMin, xMax, yMin, yMax, zMin, zMax;
	void build(double* x, double* y, double* z, int num);
	float distance(double x, double y, double z);
protected:
	static DEucl3D MINforwardDE3(Array3dDEucl3D& A, int z, int y, int x);
	static DEucl3D MINforwardDE4(Array3dDEucl3D& A, int z, int y, int x);
	static DEucl3D MINbackwardDE3(Array3dDEucl3D& A, int z, int y, int x);
	static DEucl3D MINforwardDE2(Array3dDEucl3D& A, int z, int y, int x);
	static DEucl3D MINforwardDE1(Array3dDEucl3D& A, int z, int y, int x);
	static DEucl3D MINbackwardDE1(Array3dDEucl3D& A, int z, int y, int x);
	static void DEuclidean(Array3dDEucl3D& A);
private:
	Array3dDEucl3D A;
};

#include <cgv/config/lib_end.h>