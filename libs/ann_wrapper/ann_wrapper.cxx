#include "ann.h"

namespace cgv{
	namespace math {

ann::ann()
{
	kdTree=NULL;
	dataPts=NULL;
	eps=0;
}


ann::ann(mat<double>& points, double eps)
{
	kdTree=NULL;
	dataPts=NULL;
	setup(points,eps);
}

	
void ann::setup(mat<double>& points, double eps)
{
	if(kdTree)
		delete kdTree;
	if(dataPts)
		delete [] dataPts;

	this->eps = eps;
	maxPts = points.ncols();
	dataPts = new ANNpoint[maxPts];
				
	for(unsigned i = 0; i< maxPts;i++) 
	{
		dataPts[i]= &(points(0,i));	
	}

		
	kdTree = new ANNkd_tree( // build search structure
		dataPts, // the data points
		maxPts, // number of points
		3); // dimension of space
}

void ann::setup(ANNpointArray data_points,unsigned n,unsigned dim, double eps)
{
	this->eps = eps;
	kdTree = new ANNkd_tree( // build search structure
		data_points, // the data points
		n, // number of points
		dim); // dimension of space
}

	
bool ann::is_setup()
{
	return kdTree != NULL;
}

ann::~ann()
{
	if(kdTree)
		delete kdTree;
	if(dataPts)
		delete [] dataPts;
	//annClose(); // done with ANN
}

void ann::query_knn(double x, double y, double z, int k, int *indices,
	double *sqrdists)
{
	bool nodist=false;

	if(sqrdists == NULL)
	{
		nodist = true;
		sqrdists = new double[k];
	}

	ANNpoint queryPt  = annAllocPt(3); // query point
	queryPt[0] = x;
	queryPt[1] = y;
	queryPt[2] = z;
	kdTree->annkSearch( // search
		queryPt, // query point
		k, // number of near neighbors
		indices, // nearest neighbors (returned)
		sqrdists, // distance (returned)
		eps); // error bound
	annDeallocPt(queryPt);
	if(nodist)
		delete[] sqrdists;

}

///query the number of points in the sphere with center (x,y,z) and
///radius "radius"
int ann::query_num_points_in_FR(double x, double y, double z,double radius)
{
	ANNpoint queryPt  = annAllocPt(3); // query point
	queryPt[0] = x;
	queryPt[1] = y;
	queryPt[2] = z;
		
	int num = kdTree->annkFRSearch(
		queryPt, // query point
		radius*radius,			// squared radius of query ball
		0,				// number of neighbors to return
			NULL,	// nearest neighbor array (modified)
			NULL,		// dist to near neighbors (modified)
		eps);		// error bound
	annDeallocPt(queryPt);
	return num;
}

int ann::query_num_points_in_FR(const math::vec<double> &query_point, double radius)
{
	ANNpoint queryPt  = const_cast<double*>(query_point.begin());

	int num = kdTree->annkFRSearch(
		queryPt, // query point
		radius*radius,			// squared radius of query ball
		0,				// number of neighbors to return
			NULL,	// nearest neighbor array (modified)
			NULL,		// dist to near neighbors (modified)
		eps);		// error bound
		
	return num;
}

void ann::query_fixed_radius(double x, double y, double z,double radius, int k, int *indices,double *sqrdists)
{
	bool nodist=false;

	if(sqrdists == NULL)
	{
		nodist = true;
		sqrdists = new double[k];
	}
		
	ANNpoint queryPt  = annAllocPt(3); // query point
	queryPt[0] = x;
	queryPt[1] = y;
	queryPt[2] = z;
	

	kdTree->annkFRSearch(
		queryPt,		// query point
		radius*radius,	// squared radius of query ball
		k,				// number of neighbors to return
			indices,		// nearest neighbor array (modified)
			sqrdists,		// dist to near neighbors (modified)
		eps);			// error bound
	annDeallocPt(queryPt);
	if(nodist)
		delete[] sqrdists;

}
void ann::query_fixed_radius(const math::vec<double> &query_point,double radius, int k, int *indices,double *sqrdists)
{
	bool nodist=false;

	if(sqrdists == NULL)
	{
		nodist = true;
		sqrdists = new double[k];
	}

	ANNpoint queryPt  = const_cast<double*>(query_point.begin());
	
	kdTree->annkFRSearch(
		queryPt, // query point
		radius*radius,			// squared radius of query ball
		k,				// number of neighbors to return
			indices,	// nearest neighbor array (modified)
			sqrdists,		// dist to near neighbors (modified)
		eps);		// error bound
	if(nodist)
		delete[] sqrdists;

}

void ann::query_knn(const math::vec<double> &query_point, int k, int *indices, double *sqrdists)
{
	assert(kdTree != NULL);
			
	bool nodist=false;

	if(sqrdists == NULL)
	{
		nodist = true;
		sqrdists = new double[k];
	}
	ANNpoint queryPt  = const_cast<double*>(query_point.begin());
	kdTree->annkSearch( // search
		queryPt, // query point
		k, // number of near neighbors
		indices, // nearest neighbors (returned)
		sqrdists, // distance (returned)
		eps); // error bound
	if(nodist)
		delete[] sqrdists;

}

void ann::query_knn(const math::vec<double> &query_point,int k, math::mat<double>& result_points)
{
	int *indices = new int[k];
	double *sqrdists = new double[k];
	result_points.resize(3,k);
	query_knn(query_point, k, indices,sqrdists );
	for(int i = 0; i < k; i++)
	{
		math::vec<double> p(3,dataPts[indices[i]]);
		result_points.set_col(i,p);
	}
	delete[] indices;
	delete[] sqrdists;
}


int ann::query_fixed_radius(const math::vec<double> &query_point,
	double radius,math::mat<double>& result_points)
{
	int k =query_num_points_in_FR(query_point, radius);
	int *indices = new int[k];
	double *sqrdists = new double[k];
	result_points.resize(3,k);
	query_fixed_radius(query_point, radius,  k, indices,sqrdists);
	for(int i = 0; i < k; i++)
	{
		math::vec<double> p(3,dataPts[indices[i]]);
		result_points.set_col(i,p);
	}
	delete[] indices;
	delete[] sqrdists;
	return k;
}

unsigned ann::num_points()
{
	return maxPts;
}

void ann::query_knn(unsigned i,int k, math::mat<double>& result_points)
{
	assert(kdTree != NULL);
	int *indices = new int[k];
	double *sqrdists = new double[k];
	result_points.resize(3,k);
		
				
	kdTree->annkSearch( // search
		dataPts[i], // query point
		k, // number of near neighbors
		indices, // nearest neighbors (returned)
		sqrdists, // distance (returned)
		eps); // error bound
		
	for(int i = 0; i < k; i++)
	{
		math::vec<double> p(3,dataPts[indices[i]]);
		result_points.set_col(i,p);
	}
	delete[] indices;
	delete[] sqrdists;
}


}

}