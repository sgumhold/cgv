#pragma once
#include <cgv/math/mat.h>


namespace cgv{
	namespace math {



/// Gauss-Jordan elimination (A*X=B) with full pivoting:
/// The input matrix a is replaced by its inverse
/// and the right hand side matrix b is replaced by ist corresponding solution matrix x.
/// Returns false if the matrix is singular.
template <typename T>
bool gaussj(mat<T>& a, mat<T>& b)
{
	assert(a.nrows() == a.ncols());
	const unsigned N = a.nrows();
	const unsigned M = b.ncols();

	// Some important hints for understanding this implementation
	// - The input matrix is gradually replaced by the inverse matrix.
	//   Whenever a column reaches the identity form it is instantly
	//   replaced by the emerging inverse matrix.
	// - The same is true for the vectors b whose elements are gradually
	//   replaced by their solution vectors

	
	// the position of the chosen pivot element
	unsigned int pivotPos[2];
	const int COL = 0;
	const int ROW = 1;

	// these integer arrays are used for bookkeeping which swaps were done for the pivoting
	mat<unsigned int> swapedRows(N,2);
	//unsigned int swapedRows[N][2];
	const int REDUCED_COLUMN = 0;
	const int PIVOT_ROW = 1;

	bool *isPivotCol = new bool[N];
	for (unsigned int i = 0; i < N; i++) isPivotCol[i] = false;

	// main loop over all columns to be reduced to identity form
	for (unsigned int columnCnt = 0; columnCnt < N; columnCnt++)
	{
		// for maximum numerical stability we try to find the largest absolute value
		// in the matrix (excluding the columns which contained previous pivot elements
		// and now already contain parts of the inverse matrix!) as current pivot element
		T largestAbsVal = T(0);
		for (unsigned int j = 0; j < N; j++)
		{
			if (!isPivotCol[j])
				for (unsigned int k = 0; k < N; k++)
				{
					if (!isPivotCol[k])
						if (std::abs(a(j,k)) > largestAbsVal)
						{
							largestAbsVal = std::abs(a(j,k));
							pivotPos[ROW] = j;
							pivotPos[COL] = k;
						}
				}
		}
		isPivotCol[pivotPos[COL]] = true;
		
		// if the pivot element is not on the diagonal, we have to interchange rows
		if (pivotPos[ROW] != pivotPos[COL])
		{
			for (unsigned int l = 0; l < N; l++) std::swap(a(pivotPos[ROW],l),a(pivotPos[COL],l));
			for (unsigned int l = 0; l < M; l++) std::swap(b(pivotPos[ROW],l),b(pivotPos[COL],l));
		}
		swapedRows(columnCnt,PIVOT_ROW) = pivotPos[ROW];
		swapedRows(columnCnt,REDUCED_COLUMN) = pivotPos[COL];

		// return false if no pivot element > 0 could be found and thus the matrix is singular
		if (a(pivotPos[COL],pivotPos[COL]) == T(0))
		{
			delete [] isPivotCol;
			return false;
		}

		// divide the pivot row by the pivot element
		T pivotInv = T(1) / a(pivotPos[COL],pivotPos[COL]);
		// set this pivot element to one now to get the corresponding element of inverse matrix
		// instead of one AFTER the multiplication with the inverse of the pivot element
		a(pivotPos[COL],pivotPos[COL]) = T(1);
		for (unsigned int l = 0; l < N; l++) a(pivotPos[COL],l) *= pivotInv;
		for (unsigned int l = 0; l < M; l++) b(pivotPos[COL],l) *= pivotInv;
	
		// for all rows
		for (unsigned int ll = 0; ll < N; ll++)
			// excluding the row containing the current pivot element
			if (ll != pivotPos[COL])
			{
				// set the factor to get a zero in the pivot colum when subtracting the pivot row
				T factor = a(ll,pivotPos[COL]);
				// set this element in the pivot column to zero to get the corresponding element of inverse matrix
				// instead of a zero AFTER the subtraction
				a(ll,pivotPos[COL]) = T(0);
				// substract the pivot row multiplied with the factor
				for (unsigned int l = 0; l < N; l++) a(ll,l) -= a(pivotPos[COL],l) * factor;
			}
		

	}

	for (int undoSwapStep = N-1; undoSwapStep >= 0; undoSwapStep--)
	{
		if (swapedRows(undoSwapStep,PIVOT_ROW) != swapedRows(undoSwapStep,REDUCED_COLUMN))
		{
			for (unsigned int k = 0; k < N; k++)
				std::swap( a(k,swapedRows(undoSwapStep,PIVOT_ROW)), a(k,swapedRows(undoSwapStep,REDUCED_COLUMN)) );
			
		}
	}
	delete [] isPivotCol;
	return true;
}


/// inverts a matrix a using gauss jordan elimination
/// returns false if a is singular
/// a is replaced with its inverse
template <typename T>
bool gaussj(mat<T>& a)
{
	assert(a.nrows() == a.ncols());
	const unsigned N = a.nrows();

	// Some important hints for understanding this implementation
	// - The input matrix is gradually replaced by the inverse matrix.
	//   Whenever a column reaches the identity form it is instantly
	//   replaced by the emerging inverse matrix.

	
	// the position of the chosen pivot element
	unsigned int pivotPos[2];
	const int COL = 0;
	const int ROW = 1;

	// these integer arrays are used for bookkeeping which swaps were done for the pivoting
	mat<unsigned int> swapedRows(N,2);
	//unsigned int swapedRows[N][2];
	const int REDUCED_COLUMN = 0;
	const int PIVOT_ROW = 1;

	bool *isPivotCol = new bool[N];
	for (unsigned int i = 0; i < N; i++) isPivotCol[i] = false;

	// main loop over all columns to be reduced to identity form
	for (unsigned int columnCnt = 0; columnCnt < N; columnCnt++)
	{
		// for maximum numerical stability we try to find the largest absolute value
		// in the matrix (excluding the columns which contained previous pivot elements
		// and now already contain parts of the inverse matrix!) as current pivot element
		T largestAbsVal = T(0);
		for (unsigned int j = 0; j < N; j++)
		{
			if (!isPivotCol[j])
				for (unsigned int k = 0; k < N; k++)
				{
					if (!isPivotCol[k])
						if (std::abs(a(j,k)) > largestAbsVal)
						{
							largestAbsVal = std::abs(a(j,k));
							pivotPos[ROW] = j;
							pivotPos[COL] = k;
						}
				}
		}
		isPivotCol[pivotPos[COL]] = true;
		
		// if the pivot element is not on the diagonal, we have to interchange rows
		if (pivotPos[ROW] != pivotPos[COL])
		{
			for (unsigned int l = 0; l < N; l++) std::swap(a(pivotPos[ROW],l),a(pivotPos[COL],l));
		}
		swapedRows(columnCnt,PIVOT_ROW) = pivotPos[ROW];
		swapedRows(columnCnt,REDUCED_COLUMN) = pivotPos[COL];

		// return false if no pivot element > 0 could be found and thus the matrix is singular
		if (a(pivotPos[COL],pivotPos[COL]) == T(0))
		{
			delete [] isPivotCol;
			return false;
		}

		// divide the pivot row by the pivot element
		T pivotInv = T(1) / a(pivotPos[COL],pivotPos[COL]);
		// set this pivot element to one now to get the corresponding element of inverse matrix
		// instead of one AFTER the multiplication with the inverse of the pivot element
		a(pivotPos[COL],pivotPos[COL]) = T(1);
		for (unsigned int l = 0; l < N; l++) a(pivotPos[COL],l) *= pivotInv;
	
		// for all rows
		for (unsigned int ll = 0; ll < N; ll++)
			// excluding the row containing the current pivot element
			if (ll != pivotPos[COL])
			{
				// set the factor to get a zero in the pivot colum when subtracting the pivot row
				T factor = a(ll,pivotPos[COL]);
				// set this element in the pivot column to zero to get the corresponding element of inverse matrix
				// instead of a zero AFTER the subtraction
				a(ll,pivotPos[COL]) = T(0);
				// substract the pivot row multiplied with the factor
				for (unsigned int l = 0; l < N; l++) a(ll,l) -= a(pivotPos[COL],l) * factor;
			}
		

	}

	for (int undoSwapStep = N-1; undoSwapStep >= 0; undoSwapStep--)
	{
		if (swapedRows(undoSwapStep,PIVOT_ROW) != swapedRows(undoSwapStep,REDUCED_COLUMN))
		{
			for (unsigned int k = 0; k < N; k++)
				std::swap( a(k,swapedRows(undoSwapStep,PIVOT_ROW)), a(k,swapedRows(undoSwapStep,REDUCED_COLUMN)) );
			
		}
	}
	delete [] isPivotCol;
	return true;
}


}
}
