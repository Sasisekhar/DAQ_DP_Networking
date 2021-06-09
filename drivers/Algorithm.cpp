#include "Algorithm.h"
#include<iostream>

/** \brief Calculate Support Degree Matrix.
 *
 *  Creates the support degree matrix as a 1D array instead of a 2D array
 *  to make it easier for the calculation of Eigen Values and Eigen Vectors
 *  using the gsl library.
 *
 *  @param[in] sensorinputs Readings of all sensors for a specific timestamp.
 *  @param[in] size The number of sensors being considered.
 *
 *  \return A 1D array which is the Support Degree Matrix.
 */
MatrixXd sdm_calculator(double sensorinputs[], int size){

    int i,j,k=0;
   // printf("size %d\n", size);
    double *dmatrix = (double *) malloc(sizeof(double)*(size*size));

    for(i=0; i<size;i++){

        for(j=0;j<size;j++){
            double temp = sensorinputs[i]-sensorinputs[j];

            if(temp<0){
                temp = -temp;
            }

            temp = exp(-(temp));
            dmatrix[k]= temp;
           // printf("%lf\t",dmatrix[k]);
            k++;

            if(k%size==0){
            //printf("\n");
            }
        }
    }
    MatrixXd dmatrix1 = Map<MatrixXd>( dmatrix, size, size );
 //   cout<<dmatrix1<<endl;
    return dmatrix1;
}


/** \brief Calculates EigenValues for a given Support Degree Matrix.
 *
 *  Creates an 1D array of EigenValues for a given support degree matrix
 *  using functions from the gsl external library.
 *
 *  @param[in] dmatrix The Support Degree matrix produced by sdm_calculator
 *   function.
 *  @param[in] size The number of sensors being considered which is same as
 *   size of dmatrix.
 *
 *  \return A 1D array of EigenValues in descending order for the given dmatrix.
 */
MatrixXd eigen_value_calculation(MatrixXd dmatrix){
    EigenSolver<MatrixXd> solver(dmatrix);
    MatrixXd eigenvalues;
    eigenvalues = solver.eigenvalues().real();
  //  cout << "The eigenvalues of dmatrix are:" << endl << eigenvalues << endl;
    return eigenvalues;
//cout << "The eigenvectors of dmatrix are:" << endl << solver.eigenvectors() << endl;
}


// /** \brief Produces EigenVectors for a specific EigenValue of
//  *   Support Degree Matrix.
//  *
//  *  Creates an 1D array of EigenVectors for a given support degree matrix
//  *  using functions from the gsl external library. The actual EigenValue is
//  *  not supplied but the position of EigenValue in a descending sorted
//  *  array of EigenValues as returned by eigen_value_calculation method is
//  *  given.
//  *
//  *  @param[in] dmatrix The Support Degree matrix produced by sdm_calculator
//  *   function.
//  *  @param[in] size The number of sensors being considered which is same as
//  *   size of dmatrix.
//  *  @param[in] column The column number of the EigenVector matrix produced
//  *   by the gsl library. This is same as the position of EigenValue when
//  *   the EigenValues are sorted in descending order.
//  *
//  *  \return A 1D array of EigenVectors for the given EigenValue.
//  */
MatrixXd eigen_vector_calculation(MatrixXd dmatrix){
    EigenSolver<MatrixXd> solver(dmatrix);
    MatrixXd eigenvectors;
    eigenvectors = solver.eigenvectors().real();
  // cout << "The eigenvectors of dmatrix are:" << endl << eigenvectors << endl;
    return eigenvectors;
 }

 MatrixXd compute_alpha(MatrixXd eigenvalues) {
     MatrixXd list_of_alphas;
     double sum_of_evals;
     //cout << "in compute alpha"<<endl;
    sum_of_evals = eigenvalues.sum();
        //cout<< sum_of_evals;
//     //Constructing an array of alphas a.k.a contribution rates
     list_of_alphas = (eigenvalues / sum_of_evals)*100;

   // cout<<"List of alphas:"<< endl << list_of_alphas << endl;
     return list_of_alphas;
 }

MatrixXd compute_phi(MatrixXd list_of_alphas, int size){
    MatrixXd list_of_phi;
     //The Contribution rate and the Accumulated Contribution rate of first
     //sensor is the same
     list_of_phi = list_of_alphas;
//     //printf("list of phi = %lf\n",list_of_phi[0]*100);

     for(int i=1; i<size;i++){
         list_of_phi(i,0) = list_of_phi(i-1,0) + list_of_alphas(i,0);
         //printf("list of phi = %lf\n",list_of_phi[i]*100);
     }
     //cout<<list_of_phi;
     return list_of_phi;
 }

MatrixXd compute_integrated_support_degree_score(double sensorinputs[],
             MatrixXd list_of_alphas, MatrixXd list_of_phi, MatrixXd dmatrix,
             double criterion, int size){
                 MatrixXd Z, Z_final;
                 MatrixXd list_of_m_phi;
            int i;
    MatrixXd evec_i = eigen_vector_calculation(sdm_calculator(sensorinputs,size));

    MatrixXd y = dmatrix * evec_i;
    VectorXd alphas(Map<VectorXd>(list_of_alphas.data(), list_of_alphas.cols()*list_of_alphas.rows()));
    //cout << alphas << endl;

   // Z = y.array().colwise() * alphas.array();
   Z = y.array().rowwise() * alphas.transpose().array();
  //  cout<< "Y = " << y << endl;
    Z = (Z /100);
   // cout<<"Z = "<<Z<<endl;
Z_final = Z.rowwise().sum();
//cout<< Z_final << endl;
   return Z_final;
 }

 double faulty_sensor_and_sensor_fusion(MatrixXd Z, double inputsensors[], double criterion,
 		int size){

     int i, tempfault=0,j=0;
     double *weight = (double *) malloc(sizeof(double)*(size));
     int *fault = (int *) malloc(sizeof(int)*(size));
     double average, sum=0,calculation=0,fusion_value=0;

    sum = Z.sum();
    //cout<<"sum of Z = "<<sum<<endl;


     average = fabs((sum/size))*criterion;
    double *Z_array = Z.data();
  //  cout<<"Z Matrix = "<<Z<<endl;
    // for(i=0;i<size;i++){
    //     cout<<"Z = "<<Z_array[i]<<endl;
    // }
//    // printf("Criterion: %lf",criterion);
   // printf("Average : %lf \n", average);
     for(i=0;i<size;i++){

     	//Identifying a faulty sensor and storing it's index in an int array called fault
        if(fabs(Z_array[i])<average){
             tempfault = i;
           //  printf("%d is a faulty sensor! \n",tempfault+1);
             fault[j]=tempfault;
             j++;
         }
 	}
//     printf("\n");

	//Making the reading and score of all faulty sensors to zero
    for(i=0;i<j;i++){
        Z_array[fault[i]]=0;
        inputsensors[fault[i]]=0;
    }
   // for(i=0;i<size;i++){
   //     printf("Z = %lf\n",Z[i]);
   // }

    //Assigning the sum of integrated support degree score of all
    //NON-FAULTY sensors to 'calculation' variable
    for(i=0;i<size;i++){
        calculation += Z_array[i];
    }

    //Creating an array of weight coefficients of each sensor by dividing a
    //sensor's integrated support degree score by sum of all scores
    for(i=0;i<size;i++){
        weight[i] = Z_array[i]/calculation;
    }
//    for(i=0;i<size;i++){
//        printf("Weight coefficient : %lf\n",weight[i]);
//    }

    //Calculating the fused value as a summation of product of
    //weight coefficient and sensor reading
    for(i=0;i<size;i++){
        fusion_value += weight[i] * inputsensors[i];
    }
    free(weight);
    free(fault);
    //printf("Fusion Value = %lf\n", fusion_value);
     return fusion_value;
 }