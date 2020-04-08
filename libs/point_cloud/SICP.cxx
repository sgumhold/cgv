#include "point_cloud.h"
#include <fstream>
#include "ann_tree.h"
#include "SICP.h"

SICP::SICP() {
	sourceCloud = new point_cloud();
	targetCloud = new point_cloud();
    movingCloud = new point_cloud();
    lambda = new point_cloud();
    nml_est = new normal_estimator(*sourceCloud, ng);
}

SICP::~SICP() {

}

void SICP::set_source_cloud(const point_cloud& inputCloud) {
	sourceCloud = &inputCloud;
}

void SICP::set_target_cloud(const point_cloud& inputCloud) {
	targetCloud = &inputCloud;
}

void SICP::get_center_point(const point_cloud &input, Pnt &center_point) {
    center_point.zeros();
    for (unsigned int i = 0; i < input.get_nr_points(); i++)
        center_point += input.pnt(i);
    center_point /= (float)input.get_nr_points();
}

int SICP::perform_sparceICP()
{
    if (sourceCloud->get_nr_points() == 0 || targetCloud->get_nr_points() == 0)
        return 1;
    /*if (method == pointToPoint)
        cout << "Beginning ICP with method Point to Point" << endl;
    else if (method == pointToPlane)
        cout << "Beginning ICP with method Point to Plane" << endl;*/
    //Initialize the point cloud that is going to move
    Mat rotation_mat;
    Dir translation_vec;
    ///Maybe we need to overload operator =
    //movingCloud = sourceCloud;
    copy_pc(*sourceCloud, *movingCloud);
    //movingNormals = firstNormals;

    //Beginning of the algorithm itself
    for (int iter = 0; iter < num_iterations; iter++)
    {
        std::cout << "Iteration " << iter << std::endl;
        //1st step : Computing correspondances
        std::vector<int> matchIndice = compute_correspondances(*targetCloud, *movingCloud);
        point_cloud matchCloud = select_subset_pc(*targetCloud, matchIndice); //Selecting y
        /*if (method == pointToPlane)
            selectedNormals = selectSubsetPC(secondNormals, matchIndice);*/

        //2nd step : Computing transformation
        //RigidTransfo iterTransfo;
        cgv::math::fmat<float, 4, 4> iterTransfo;
        for (int iterTwo = 0; iterTwo < num_iterations_in; iterTwo++)
        {
            // step 2.1 Computing z

            //Compute h
            point_cloud h = movingCloud - matchCloud + division_pc(*lambda, mu);
            //Optimizing z with the shrink operator
            point_cloud z;
            for (int i = 0; i < h.get_nr_points(); i++)
                z.pnt(i) = shrink(h.pnt(i));

            // step 2.2 point-to-point ICP

            //Compute C
            point_cloud c = matchCloud + z - division_pc(*lambda, mu);
            //Make a standard ICP iteration
            if (method == pointToPoint)
                rigid_transform_point_to_point(*movingCloud, c, rotation_mat, translation_vec);
            else if (method == pointToPlane)
                rigid_transform_point_to_plane(*movingCloud, c, selectedNormals);
            else
                std::cout << "Warning ! The method you try to use is incorrect !" << std::endl;

            //Updating the moving pointCloud
            /*movingCloud = movePointCloud(*movingCloud, iterTransfo);
            movingNormals = (iterTransfo.first * movingNormals.transpose()).transpose();
            computedTransfo = compose(iterTransfo, computedTransfo);
            updateIter(iterTransfo); //Updating the iterations measure*/

            // step 2.3 Updating the Lagrange multipliers
            point_cloud delta = movingCloud - matchPC - z;
            lambda = lambda + mu * delta;
        }
    }

    hasBeenComputed = true;
    return 0;
}

std::vector<int> SICP::compute_correspondances(const point_cloud& refCloud, point_cloud& queryCloud){
    //Create an adapted kd tree for the point cloud
    ann_tree* tree = new ann_tree();
    tree->build(refCloud);
    //Create an index
    //my_kd_tree_t   mat_index(3, refCloud, 10 /* max leaf */);
    //mat_index.index->buildIndex();

    std::vector<int> nearestIndices;
    for (int i = 0; i < queryCloud.get_nr_points(); i++)
    {
        //Current query point
        Pnt queryPoint = queryCloud.pnt(i);

        //Do a knn search
        const size_t num_results = 1; //We want the nearest neighbour
        std::vector<const Pnt*> knn;
        std::vector<size_t>  ret_indexes(num_results);
        std::vector<double> out_dists_sqr(num_results);

        //KNNResultSet<double> resultSet(num_results);
        //resultSet.init(&ret_indexes[0], &out_dists_sqr[0]);

        //mat_index.index->findNeighbors(resultSet, &queryPoint[0], SearchParams(10));
        tree->find_closest_points(queryPoint, num_results, knn);
        //Updating the resulting index vector
        //nearestIndices.push_back();

        if (verbose)
        {
            //std::cout << queryPoint(0, 0) << " " << queryPoint(0, 1) << " " << queryPoint(0, 2) << " refPoint" << std::endl;
            //std::cout << refCloud(ret_indexes[0], 0) << " " << refCloud(ret_indexes[0], 1) << " " << refCloud(ret_indexes[0], 2) << " closestPoint" << std::endl << std::endl << std::endl;
        }
    }
    return nearestIndices;
}

/*
Move the pointCloud according to the rigid transformation in t
*/
point_cloud SICP::move_pointcloud(point_cloud &pointCloud, Dir &t){
    return pointCloud;
}


/*
This function is the standard point to point ICP
a : moving cloud
b : reference cloud
*/
void SICP::rigid_transform_point_to_point(point_cloud &a, point_cloud &b, Mat& rotation_m, Dir& translation_v)
{
    Mat rotation_mat;
    Dir translation_vec;
    //Centering the point clouds
    Pnt centerA, centerB;
    get_center_point(a, centerA);
    get_center_point(b, centerB);
    //PointCloud aCenter = a - centerA.replicate(a.rows(), 1);
    //PointCloud bCenter = b - centerB.replicate(b.rows(), 1);
    //Computing the product matrix A
    Mat fA;
    fA.zeros();
    for (int i = 0; i < a.get_nr_points(); i++)
        fA += Mat(a.pnt(i), b.pnt(i));
   
    //Computing singular value decomposition
    //JacobiSVD<Matrix<double, 3, 3>> svd(W, Eigen::ComputeFullU | Eigen::ComputeFullV);
    cgv::math::mat<float> U, V;
    cgv::math::diag_mat<float> Sigma;
    U.zeros();
    V.zeros();
    Sigma.zeros();
    ///cast fA to A
    cgv::math::mat<float> A(3, 3, &fA(0, 0));
    cgv::math::svd(A, U, Sigma, V);

    Mat fU(3, 3, &U(0, 0)), fV(3, 3, &V(0, 0));
    rotation_mat = fU * cgv::math::transpose(fV);
    //Computing the rigid transformation
    //Mat rotation = svd.matrixV() * svd.matrixU().transpose();
    point_cloud_types::Pnt t;
    t.zeros();
    t = rotation_mat * centerA;
    translation_vec = centerB - t;

    //Outputing the transformation
    if (verbose)
    {
        std::cout << std::endl << std::endl << "Rotation Matrix : " << std::endl << rotation_mat << std::endl;
        std::cout << "Translation Matrix : " << std::endl << translation_vec << std::endl;
    }

    rotation_m = rotation_mat;
    translation_v = translation_vec;
    //return RigidTransfo(rotation, translation);
}

/*
This function is the standard point to plane ICP
a : moving cloud
b : reference cloud
n : normal to the reference cloud b
*/
void SICP::rigid_transform_point_to_plane(point_cloud& a, point_cloud& b, point_cloud &n)
{
    //Initialize linear system
    cgv::math::fmat<float, 6, 6> left_member;
    left_member.zeros();
    cgv::math::fmat<float, 6, 1> right_member;
    right_member.zeros();

    //PointCloud c = PointCloud::Zero(a.rows(),3);
    for (int i = 0; i < a.get_nr_points(); i++)
    {
        //Computing c = a x n (outer product)
        Mat c = Mat(a.pnt(i), n.pnt(i));

        //Updating left member
        /*leftMember.block(0, 0, 3, 3) += cgv::math::transpose(c) * c;                         //Top-left block
        leftMember.block(3, 3, 3, 3) += cgv::math::transpose(n.pnt(i).to_vec()) * n.pnt(i).to_vec();           //Bottom-right block
        leftMember.block(0, 3, 3, 3) +=
            n.row(i).replicate(3, 1).cwiseProduct(c.transpose().replicate(1, 3)); //Top-right block
        leftMember.block(3, 0, 3, 3) +=
            n.row(i).transpose().replicate(1, 3).cwiseProduct(c.replicate(3, 1)); //Bottom-left block*/

          //Updating right member
        /*double factor = (a.pnt(i) - b.pnt(i)) * cgv::math::transpose(n.pnt(i).to_vec());
        rightMember.block(0, 0, 3, 1) -= factor * cgv::math::transpose(c);        //Top 3 elements
        rightMember.block(3, 0, 3, 1) -= factor * cgv::math::transpose(n.pnt(i).to_vec()); //Bottom 3 elements*/
    }

    //Solving linear system
    /*LDLT<cgv::math::fmat<double, 6, 6>> ldlt(leftMember);
    cgv::math::fmat<double, 6, 1> solution = ldlt.solve(rightMember);*/

    //Expressing the resulting transformation
    /*RotMatrix rotation = (AngleAxisd(AngleAxisd::Scalar(solution(0, 0)), Vector3d::UnitX()) * AngleAxisd(AngleAxisd::Scalar(solution(1, 0)), Vector3d::UnitY()) * AngleAxisd(AngleAxisd::Scalar(solution(2, 0)), Vector3d::UnitZ())).matrix();
    TransMatrix translation = solution.block(3, 0, 3, 1);*/

    //return RigidTransfo(rotation, translation);
}

/*
This function implements the shrink operator which optimizes the function
f(z) = ||z||_2^p + mu/2*||z-h||_2^2
*/
point_cloud_types::Pnt SICP::shrink(Pnt &h)
{
    double alpha_a = pow((2. / mu) * (1. - p), 1. / (2. - p));
    double hTilde = alpha_a + (p / mu) * pow(alpha_a, p - 1);
    double hNorm = norm(h);
    if (hNorm <= hTilde)
    {
        h.x() *= 0;
        h.y() *= 0;
        h.z() *= 0;
        return h;
    }
    double beta = ((alpha_a) / hNorm + 1.) / 2.;
    for (int i = 0; i < num_iter_shrink; i++)
        beta = 1 - (p / mu) * pow(hNorm, p - 2.) * pow(beta, p - 1);
    h.x() *= beta;
    h.y() *= beta;
    h.z() *= beta;
    return h;
}

double SICP::norm(Dir& h) {
    double norm = sqrt(pow(h.x(), 2) + pow(h.y(), 2) + pow(h.z(), 2));
    return norm;
}

void SICP::compose(Mat& r_new, Dir& t_new, Mat& r_old, Dir& t_old) {
    Mat temp_r = r_new* r_old;
    cgv::math::fvec<float, 3> temp_t = r_new * t_old + t_new;
}
///Selection of a subset in a PointCloud
point_cloud SICP::select_subset_pc(point_cloud p, std::vector<int> indice) {
    point_cloud selection;
    selection.resize(indice.size());
    for (int i = 0; i < indice.size(); i++)
        selection.pnt(i) = p.pnt(indice[i]);
    return selection;
}

///Updates the iterations measure by estimating the amplitude of rigid motion t
void SICP::update_iter(Mat& r, Dir& t) {
    cgv::math::fmat<double, 4, 4> id;
    cgv::math::fmat<double, 4, 4> curT;
    id.identity();
    curT.identity();
    //curT.block(0, 0, 3, 3) = r;
    //curT.block(0, 3, 3, 1) = t / referenceDist;
    cgv::math::fmat<double, 4, 4> diff = curT - id; //Difference between t and identity
    iterations.push_back((diff * cgv::math::transpose(diff)).trace()); //Returning matrix norm
}
///Save iterations to file
void SICP::save_iter(std::string pathToFile) {
    std::ofstream txtStream(pathToFile.c_str());
    for (int i = 0; i < iterations.size(); i++)
        txtStream << iterations[i] << std::endl;
    txtStream.close();
}

void SICP::division_pc(point_cloud& input, double d) {
    for (int i = 0; i < input.get_nr_points(); i++) {
        input.pnt(i) /= d;
    }
}
///Here is a demo of Cholesky_Decomposition, still needs to be improved
void SICP::cholesky_decomposition(int matrix[][100], int n) {
    int lower[100][100];
    memset(lower, 0, sizeof(lower));

    // Decomposing a matrix into Lower Triangular 
    for (int i = 0; i < n; i++) {
        for (int j = 0; j <= i; j++) {
            int sum = 0;

            if (j == i) // summation for diagnols 
            {
                for (int k = 0; k < j; k++)
                    sum += pow(lower[j][k], 2);
                lower[j][j] = sqrt(matrix[j][j] -
                    sum);
            }
            else {

                // Evaluating L(i, j) using L(j, j) 
                for (int k = 0; k < j; k++)
                    sum += (lower[i][k] * lower[j][k]);
                lower[i][j] = (matrix[i][j] - sum) /
                    lower[j][j];
            }
        }
    }

    // Displaying Lower Triangular and its Transpose 
    std::cout << std::setw(6) << " Lower Triangular"
        << std::setw(30) << "Transpose" << std::endl;
    for (int i = 0; i < n; i++) {

        // Lower Triangular 
        for (int j = 0; j < n; j++)
            std::cout << std::setw(6) << lower[i][j] << "\t";
        std::cout << "\t";

        // Transpose of Lower Triangular 
        for (int j = 0; j < n; j++)
            std::cout << std::setw(6) << lower[j][i] << "\t";
        std::cout << std::endl;
    }
}

void SICP::copy_pc(const point_cloud& input, point_cloud& output) {
    for (int i = 0; i < input.get_nr_points(); i++) {
        output.pnt(i) = input.pnt(i);
    }
}