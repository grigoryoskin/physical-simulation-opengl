#ifndef physics_h
#define physics_h

#include "../utils/draw_shapes.h"
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <tuple>
#include <algorithm>
#include "physical_mesh.h"
#include "gradient.h"
#include "hessian.h"

typedef Eigen::SparseMatrix<float> SparseMatrixf;
typedef Eigen::Triplet<double> T;

const float h = 0.001f;

Eigen::VectorXf PhysicalMesh::dVdQ(Eigen::VectorXf &qq) {
    Eigen::VectorXf dVdQ = Eigen::VectorXf::Zero(3*n);
    
    for(int i = 0; i< n_tet; i++){
        auto ff_i = getFFlat(i, qq);
        auto grad = gradPsi(C, D, ff_i);
        Eigen::VectorXf dVdQ_i = volumes[i] * Bs[i].transpose() * grad;
            
        for(int k = 0; k< 4; k++) {
            int index = tetIndices[i][k];
            dVdQ.segment(index*3, 3) += dVdQ_i.segment(k*3, 3);
            dVdQ[index*3+1] += volumes[i]*g;
        }
    }
    
    return dVdQ;
}

SparseMatrixf PhysicalMesh::ddVddQ(Eigen::VectorXf &qq) {
    SparseMatrixf ddVddQ = SparseMatrixf(3*n,3*n);
    for(int i = 0; i< n_tet; i++){
        auto ff_i = getFFlat(i, qq);
        auto hessian = psi_hessian(C, D, ff_i);
        Eigen::MatrixXf ddVddQ_i = volumes[i] * Bs[i].transpose() * hessian * Bs[i];
        std::vector<T> tripletList;
        for (int j = 0; j<12; j++) {
            for (int k = 0; k<12; k++) {
                tripletList.push_back(T(3*tetIndices[i][(int)(j/3)] + j%3,
                                        3*tetIndices[i][(int)(k/3)] + k%3,
                                        ddVddQ_i(j,k)));
            }
        }
        ddVddQ.setFromTriplets(tripletList.begin(), tripletList.end());
    }
    return ddVddQ;
}

float PhysicalMesh::V(Eigen::VectorXf &qq) {
    float V = 0;
    for(int i = 0; i< n_tet; i++){
        auto ff_i = getFFlat(i, qq);
        V += volumes[i]*psi(C, D, ff_i);
    }
    return V;
}

Eigen::VectorXf PhysicalMesh::dEdV(Eigen::VectorXf &v) {
    Eigen::VectorXf q_i = q + h*v;
    return M*(v - q_dot) + h * dVdQ(q_i);
}

// Updating q and q dot using backward Euler method.
bool inverseCalculated = false;
Eigen::MatrixXf M_inv;
Eigen::VectorXf PhysicalMesh::forwardEulerStep() {
    if (!inverseCalculated) {
        M_inv = Eigen::MatrixXf::Identity(3*n, 3*n);
        Eigen::MatrixXf I = Eigen::MatrixXf::Identity(3*n, 3*n);
        Eigen::SimplicialLDLT<SparseMatrixf> solverLDLT;
        solverLDLT.compute(M);
        M_inv = solverLDLT.solve(I);
        inverseCalculated = true;
    }
    Eigen::VectorXf f = -dVdQ(q);
    
    auto rightHandSide = (M * q_dot + h*f);
    
    return M_inv * rightHandSide;
}

// Updating q and q dot using backward Euler method.
Eigen::VectorXf PhysicalMesh::backwardEulerLinearStep() {
    Eigen::VectorXf f = -dVdQ(q);
    SparseMatrixf K = -ddVddQ(q);
    Eigen::SimplicialLDLT<SparseMatrixf> solverLDLT;
    auto rightHandSide = (M * q_dot + h*f);
    solverLDLT.compute(M + h*h*K);
    return solverLDLT.solve(rightHandSide);
}

Eigen::VectorXf PhysicalMesh::gradiendDescent(float a, float tol, bool verbose) {
    Eigen::VectorXf v_i = q_dot;
    
    for(int i = 0; i< 100; i++){
        Eigen::VectorXf g_i = dEdV(v_i);
        if(verbose) {
            std::cout << "i: " << i << "g: " << g_i.norm() << std::endl;
        }
        if(g_i.norm()<tol) {
            break;
        }
        
        Eigen::VectorXf d = -g_i;
        v_i += a*d;
    }
    return v_i;
}


void PhysicalMesh::simulationStep() {
    Eigen::VectorXf new_q_dot = forwardEulerStep();
    //Eigen::VectorXf new_q_dot = gradiendDescent(20.0f, 0.0009f, false);
    //Eigen::VectorXf new_q_dot = backwardEulerLinearStep();
    
    for (int i = 0; i<n; i++) {
        if((q + h * new_q_dot)[3*i + 1] <= -3){
            new_q_dot[3*i + 1] = 0;
        }
    }
    q += h * new_q_dot;
    q_dot = new_q_dot;
}

#endif /* physics_h */
