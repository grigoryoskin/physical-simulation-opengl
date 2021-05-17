#ifndef physics_h
#define physics_h

#include "../utils/draw_shapes.h"
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <tuple>
#include <algorithm>

typedef Eigen::SparseMatrix<float> SparseMatrixf;
typedef Eigen::Triplet<double> T;

class PhysicalMesh {
    
private:
    // A 3xn vector of coordinates of all vertices. q = (x0,y0,z0,x1,y1,z1,...)
    Eigen::VectorXf q;
    // Vector of derivatives of coordinates of all vertices.
    Eigen::VectorXf q_dot;
    // List of indices of stationary points.
    std::vector<unsigned int> fixed_points;
    // List of edges between vertices. Each element is a tuple of
    // indices of positions of two vertices and the undeformed distance between them.
    std::vector<std::tuple<int,int, float>> edges;
    // Number of vertices in a mesh.
    unsigned long n;
    // Spring constant (stiffness).
    float k;
    // Mass of one particle. (Assumes that all particles have the same mass)
    float m;
    // A 3nx3n mass matrix. Here it's m*I
    SparseMatrixf M;
    // Acceleration of gravity.
    float g;
    
    // Updating q and q dot using backward Euler method.
    void backwardEulerStep(SparseMatrixf &K,
                           Eigen::VectorXf &f,
                           float &h) {
        Eigen::SimplicialLDLT<SparseMatrixf> solverLDLT;
        auto rightHandSide = (M * q_dot + h*f);
        solverLDLT.compute(M + h*h*K);
        Eigen::VectorXf new_q_dot = solverLDLT.solve(rightHandSide);
        
        //Setting velocity of fixed points to zero.
        for (int i: fixed_points) {
            new_q_dot.segment(i*3, 3) *= 0;
        }
        
        q += h * new_q_dot;
        q_dot = new_q_dot;
    }
    
public:
    PhysicalMesh(Mesh &mesh, float m, float k, float g, std::vector<unsigned int> fixed_points): k(k), m(m), g(g), fixed_points(fixed_points){
        n = mesh.positions.size();
        q = Eigen::VectorXf::Zero(n*3);
        q_dot = Eigen::VectorXf::Zero(n*3);
        M = SparseMatrixf(3*n,3*n);
        
        int n_triangles = mesh.indices.size()/3;
        std::cout <<"N triangles: " << n_triangles << std::endl;
        
        // Filling up positions vector from original mesh.
        for(int i = 0; i<n; i++) {
            q.segment(i*3, 3) = mesh.positions[i];
        }
        
        // Creating a list of unique edges from triangles of the original mesh, since some triangles share edges.
        for(int i = 0; i<n_triangles; i++) {
            for(int p = 0; p<3; p++) {
                for(int o = p; o<3; o++) {
                    int i1 = 3*i + p;
                    int i2 = 3*i + o;
                    if(o != p && mesh.indices[i1] != mesh.indices[i2]) {
                        // Check if the list of edges already contains current edge.
                        auto it = std::find_if(edges.begin(), edges.end(),
                                               [mesh,i1,i2](std::tuple<int,int, float> e){
                            int e1 = std::get<0>(e);
                            int e2 = std::get<1>(e);
                            return (e1 == mesh.indices[i1] && e2 == mesh.indices[i2])
                                    || (e1 == mesh.indices[i2] && e2 == mesh.indices[i1]);
                        });
                        
                        // If edges list does not contain current edge, add it.
                        if(it == edges.end()) {
                            Eigen::Vector3f q1 = mesh.positions[mesh.indices[i1]];
                            Eigen::Vector3f q2 = mesh.positions[mesh.indices[i2]];
                            Eigen::Vector3f l = q2-q1;
                            float l0 = l.norm();
                            edges.push_back(
                            std::make_tuple(mesh.indices[i1], mesh.indices[i2],l0));
                        }
                    }
                }
            }
        }
        
        std::cout <<"N edges: " << edges.size() << std::endl;

        // Constructing a mass matrix.
        for(int i = 0; i<3*n; i++) {
            M.insert(i,i) = m;
        }
    };
    
    // Move fixed points by vector r.
    void moveFixedPoints(Eigen::Vector3f r) {
        for (int i: fixed_points) {
            q.segment(i*3, 3) += r;
        }
    }
    
    SparseMatrixf K_tmp;
    Eigen::VectorXf f_tmp;
    bool enableHessian = false;
    void simulationStep(float &h) {
        f_tmp = Eigen::VectorXf::Zero(3*n);
        K_tmp = SparseMatrixf(3*n,3*n);
        
        for (auto edge : edges) {
            int i = std::get<0>(edge);
            int j = std::get<1>(edge);
            float l0 = std::get<2>(edge);
        
            // Calculating the distance between vertices of an edge.
            Eigen::Vector3f r = q.segment(i*3, 3)-q.segment(j*3, 3);
            float r_i = r.norm();
            if (abs(r_i) < 0.001f) {
                r_i = 0.01;
            }

            // Calculating force vector.
            float f_i = -k*(1.0f - l0/r_i);
            if(abs(r_i - l0) < 0.01f) {f_i = 0;}
            f_tmp.segment(i*3,3) += f_i*r;
            f_tmp.segment(j*3,3) += -f_i*r;
            
            // Calculating stiffness matrix.
            if(enableHessian){
                float a = (r_i - l0)/(r_i*r_i*r_i);
                float b = 1/(r_i*r_i);
                float gamma = (r_i - l0)/r_i;
                SparseMatrixf K_i = SparseMatrixf(3*n,3*n);
                std::vector<T> tripletList;
                
                for(int p = 0; p<3; p++) {
                    for(int o = 0; o<3; o++) {
                        float coef = k * (p == o? (b-a) : (b - gamma));
                        float constant = p == o ? a/b : 0;
                        float k_i = (coef*r[p]*r[o] + constant);
                        tripletList.push_back(T(3*i + p, 3*j + o, -k_i));
                        tripletList.push_back(T(3*j + p, 3*i + o, -k_i));
                        tripletList.push_back(T(3*j + p, 3*j + o, k_i));
                        tripletList.push_back(T(3*i + p, 3*i + o, k_i));
                    }
                }
                K_i.setFromTriplets(tripletList.begin(), tripletList.end());
                K_tmp += K_i;
            }
        }
        
        // Adding gravitational force
        for(int i = 0; i<n; i++){
            f_tmp(i*3 + 1) += -m*g;
        }
        
        backwardEulerStep(K_tmp,f_tmp,h);
    }
    
    // Updating original mesh with new positions.
    void updateMesh(Mesh &mesh){
        for(int i = 0; i<n; i++) {
            mesh.positions[i] = q.segment(i*3, 3);
        }
    };
};
#endif /* physics_h */
