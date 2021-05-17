#ifndef physical_mesh_h
#define physical_mesh_h

#include "../utils/draw_shapes.h"
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <tuple>
#include <algorithm>
#include "gradient.h"

typedef Eigen::SparseMatrix<float> SparseMatrixf;
typedef Eigen::Triplet<double> T;

class PhysicalMesh {
    
private:
    // A 3xn vector of coordinates of all vertices. q = (x0,y0,z0,x1,y1,z1,...)
    Eigen::VectorXf q;
    // Vector of derivatives of coordinates of all vertices.
    Eigen::VectorXf q_dot;
    // List of indices of vertices of terahedra.
    std::vector<std::vector<int>> tetIndices;
    // List of volumes of tetrahedra.
    std::vector<float> volumes;
    
    // Utility matrices for calculating various tetrahedral parameters.
    std::vector<Eigen::Matrix3f> Ts;
    std::vector<Eigen::MatrixXf> Bs;
    std::vector<Eigen::MatrixXf> Ds;
    
    // Number of vertices in a mesh.
    unsigned long n;
    // Number of tetrahedra in a mesh.
    int long n_tet;
    
    // A 3nx3n mass matrix.
    SparseMatrixf M;
    
    // Stiffness parameters.
    float C = 170;
    float D = 169.5;
    // Acceleration of gravity.
    float g = 3;
    
    Mesh skinMesh;
    // Reletionship between skin mesh vertices and tetraheda. Mesh index - <Tetrahedron index, skin matrix>.
    std::map<int, std::tuple<int, Eigen::MatrixXf>> skinMeshTetrahedra;
    
    // Coordinates of i-th tetrahedron flattened into 12x1 vector.
    Eigen::VectorXf getQTet(int i, Eigen::VectorXf &qq) {
        Eigen::VectorXf q_i = Eigen::VectorXf::Zero(12);
        for (int j = 0; j < 4; j++) {
            for(int k = 0; k < 3; k++){
                q_i[3*j + k] = qq[3*tetIndices[i][j] + k];
            }
        }
        return q_i;
    }
    
    Eigen::Matrix3f getFMat(int i, Eigen::VectorXf &qq) {
        Eigen::MatrixXf pos = Eigen::MatrixXf::Zero(3, 4);
        Eigen::VectorXf qTet = getQTet(i, qq);
        for (int j = 0; j < 4; j ++) {
            pos.col(j) = qTet.segment(j*3, 3).transpose();
        }
        Eigen::MatrixXf F = pos*Ds[i];
        return F;
    }
    
    
    Eigen::VectorXf getFFlat(int i, Eigen::VectorXf &qq) {
        Eigen::VectorXf ff = Eigen::VectorXf::Zero(9);
        auto fMat = getFMat(i, qq);
        for(int k = 0; k<3; k++) {
            for(int j = 0; j<3; j++) {
                ff[k*3 + j] = fMat(k,j);
            }
        }
        return ff;
    }
    Eigen::VectorXf forwardEulerStep();
    Eigen::VectorXf backwardEulerLinearStep();
    Eigen::VectorXf gradiendDescent(float a, float tol, bool verbose);
    float V(Eigen::VectorXf &qq);
    float E(Eigen::VectorXf &v);
    Eigen::VectorXf dVdQ(Eigen::VectorXf &qq);
    SparseMatrixf ddVddQ(Eigen::VectorXf &qq);
    Eigen::VectorXf dEdV(Eigen::VectorXf &v);
public:
    void simulationStep();
    void moveFixedPoints(Eigen::Vector3f r);
    
    PhysicalMesh(TetrahedralMesh &mesh, Mesh &skinMesh): skinMesh(skinMesh) {
        n = mesh.positions.size();
        q = Eigen::VectorXf::Zero(n*3);
        q_dot = Eigen::VectorXf::Zero(n*3);
        n_tet = mesh.indices.size()/4;
        
        // Filling up positions vector from original mesh.
        for(int i = 0; i<n; i++) {
            q.segment(i*3, 3) = mesh.positions[i];
        }
        
        M = SparseMatrixf(3*n,3*n);
        
        // Matrix multiplier individual tetrahedron mass matrix.
        Eigen::MatrixXf M_i = Eigen::MatrixXf::Identity(12,12);
        Eigen:: VectorXf v = Eigen::VectorXf::Zero(12);
        v[2] = 1; v[5] = 1; v[8] = 1; v[11] = 1;
        for (int j = 0; j<12; j++) {
            Eigen:: VectorXf v_temp = Eigen::VectorXf::Zero(12);
            int last = v[11];
            v_temp[0] = last;
            for (int k = 0; k<11; k++) {
                v_temp[k+1] = v[k];
            }
            v = v_temp;
            
            M_i.row(j) += v.transpose();
        }
        
        for (int i = 0; i<n_tet; i++) {
            std::vector<int> tetIndex;
            tetIndex.push_back(mesh.indices[i*4]);
            tetIndex.push_back(mesh.indices[i*4 + 1]);
            tetIndex.push_back(mesh.indices[i*4 + 2]);
            tetIndex.push_back(mesh.indices[i*4 + 3]);
            tetIndices.push_back(tetIndex);
            
            Eigen::VectorXf qTet = getQTet(i, q);
            
            Eigen::Vector3f q0 = qTet.segment(0,3);
            Eigen::Vector3f q1 = qTet.segment(3,3);
            Eigen::Vector3f q2 = qTet.segment(6,3);
            Eigen::Vector3f q3 = qTet.segment(9,3);
             
            // Calculating volumes.
            float vol = abs(((q1 - q0).cross(q2-q0)).dot(q3-q0)/6);
            //std::cout << vol << std::endl;
            volumes.push_back(vol);
            
            // Calculating utility matrices.
            Eigen::Matrix3f T_i = Eigen::MatrixXf::Zero(3,3);
            
            T_i.col(0) = q1 - q0;
            T_i.col(1) = q2 - q0;
            T_i.col(2) = q3 - q0;
        
            Ts.push_back(T_i);
            
            Eigen::Matrix3f T_i_inv = T_i.inverse();
    
            Eigen::MatrixXf hm = - Eigen::Vector3f::Ones(3).transpose() * T_i_inv;
            Eigen::MatrixXf D_i = Eigen::MatrixXf::Zero(4,3);
            D_i.block(0, 0, 1, 3) = hm;
            D_i.block(1, 0, 3, 3) = T_i_inv;

            Ds.push_back(D_i);
            
            Eigen::MatrixXf B_i = Eigen::MatrixXf::Zero(9,12);
            for (int j = 0; j<4; j++) {
                auto v = D_i.row(j);
                for (int k= 0; k<3;k++) {
                B_i.block(k*3, j*3 + k, 3, 1) = v.transpose();
                }
            }
            Bs.push_back(B_i);
            
            // Assembling mass matrix.
            for (int j = 0; j<12; j++) {
                for (int k = 0; k<12; k++) {
                    M.coeffRef(3*tetIndex[(int)(j/3)] +  j%3, 3*tetIndex[(int)(k/3)] + k%3) += vol*M_i(j,k)/20;
                }
            }
            
        // Calculating skinning matrices.
        for(int j = 0; j< skinMesh.positions.size(); j++){
                Eigen::Vector3f v = skinMesh.positions[j];
                Eigen::Vector3f phi = T_i_inv*(v - q0);
                if(phi[0] < 1 && phi[0] > 0 && phi[1] < 1 && phi[1] > 0 && phi[2] < 1 && phi[2] > 0) {
                    Eigen::MatrixXf w = Eigen::MatrixXf::Zero(3,12);
                    w.block(0,0,3,3) = (1 - phi[0] - phi[1] - phi[2]) * Eigen::MatrixXf::Identity(3,3);
                    w.block(0,3,3,3) = phi[0] * Eigen::MatrixXf::Identity(3,3);
                    w.block(0,6,3,3) = phi[1] * Eigen::MatrixXf::Identity(3,3);
                    w.block(0,9,3,3) = phi[2] * Eigen::MatrixXf::Identity(3,3);
                    skinMeshTetrahedra[j] = std::make_tuple(i, w);
                }
            }
        }
    };
    
    Mesh getSkinMesh() {
        Mesh skinnedMesh = skinMesh;
        for ( const auto &skinMeshPair : skinMeshTetrahedra ) {
            int i_vert = skinMeshPair.first;
            int i_tet = std::get<0>(skinMeshPair.second);
            Eigen::MatrixXf w = std::get<1>(skinMeshPair.second);
            skinnedMesh.positions[i_vert] = w * getQTet(i_tet, q);
        }
        
        return skinnedMesh;
    }

};

#endif /* physical_mesh_h */
