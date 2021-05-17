#ifndef draw_shapes_h
#define draw_shapes_h

#include <Eigen/Dense>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <regex>
#include <iterator>
#include <algorithm>


struct Mesh {
public:
    std::vector<Eigen::Vector3f> positions;
    std::vector<Eigen::Vector2f> uv;
    std::vector<Eigen::Vector3f> normals;
    std::vector<unsigned int> indices;
    
    Mesh() =default;
    
    Mesh(std::vector<Eigen::Vector3f> positions,
         std::vector<Eigen::Vector2f> uv,
         std::vector<Eigen::Vector3f> normals,
         std::vector<unsigned int> indices
         ): positions(positions), uv(uv), normals(normals), indices(indices) {};
    
    // Construct from obj file (only vertices and triangles).
    Mesh(std::string path) {
        std::ifstream mesh_file(path);
        if (mesh_file.is_open()) {
        std::string line;
        std::cout << "Parsing mesh file" << std::endl;
        std::vector<Eigen::Vector3f> positions_tmp;
        std::vector<Eigen::Vector3f> normals_tmp;
            
        std::vector<unsigned int> normals_indices;
            
        while (std::getline(mesh_file, line))
        {
            std::istringstream iss(line);
            std::string prefix;
            iss >> prefix;
            
            if(prefix.compare("v") == 0) {
                float x;
                float y;
                float z;
                
                iss >> x >> y >> z;
                if(!iss.fail()) {
                    Eigen::Vector3f v(x, y ,z);
                    positions.push_back(0.7*v);
                }
            }
            if(prefix.compare("vn") == 0) {
                float x;
                float y;
                float z;
                
                iss >> x >> y >> z;
                if(!iss.fail()) {
                    Eigen::Vector3f v(x, y ,z);
                    normals_tmp.push_back(v);
                }
            }
            else if (prefix.compare("f") == 0) {
                std::string q[3];
                
                iss >> q[0] >> q[1] >> q[2];
                
                if(!iss.fail()) {
                    for(int i = 0; i < 3; i++) {
                       std::regex regex("\\/");
                        
                       std::vector<std::string> out(
                                       std::sregex_token_iterator(q[i].begin(), q[i].end(), regex, -1),
                                       std::sregex_token_iterator()
                                       );
                        indices.push_back(atoi(out[0].c_str())-1);
                        normals_indices.push_back(atoi(out[2].c_str())-1);
                    }
                }
            }
        }
                mesh_file.close();
            
            for (int i = 0; i < positions.size(); i++) {
                unsigned int index = std::find(indices.begin(), indices.end(), i) - indices.begin();
                normals.push_back(normals_tmp[normals_indices[index]]);
            }
    }
        std::cout << "N vertices: " << positions.size() << std::endl;
        std::cout << "N triangles: " << indices.size()/3 << std::endl;
    }
};

struct TetrahedralMesh {
public:
    std::vector<Eigen::Vector3f> positions;
    std::vector<unsigned int> indices;
    
    TetrahedralMesh() = default;
    
    TetrahedralMesh(
         std::vector<Eigen::Vector3f> positions,
         std::vector<unsigned int> indices
    ): positions(positions), indices(indices) {};
    
    TetrahedralMesh(std::string path) {
        std::ifstream tet_file(path);
        if (tet_file.is_open()) {
        std::string line;
        std::cout << "Parsing msh file" << std::endl;
        bool isReadingVerts = false;
        bool isReadingTetrahedra = false;
        while (std::getline(tet_file, line))
        {
            if(isReadingVerts) {
                std::istringstream iss(line);
                
                float x;
                float y;
                float z;
                
                iss >> x >> y >> z;
                if(!iss.fail()) {
                    Eigen::Vector3f v(x, y ,z);
                    if(v.norm() > 10) {
                        std::cout << v << std::endl;
                    }
                    positions.push_back(v);
                }
            }
            if (isReadingTetrahedra) {
                std::istringstream iss(line);
                
                unsigned int i;
                unsigned int q0;
                unsigned int q1;
                unsigned int q2;
                unsigned int q3;
                
                iss >> i >> q0 >> q1 >> q2 >> q3;
                
                if(!iss.fail()) {
                    indices.push_back(q0-1);
                    indices.push_back(q1-1);
                    indices.push_back(q2-1);
                    indices.push_back(q3-1);
                }
            }
            
            if (line.find("$Nodes") != std::string::npos) {
                isReadingVerts = true;
            }
            if (line.find("$EndNodes") != std::string::npos) {
                isReadingVerts = false;
            }
            if (line.find("$Elements") != std::string::npos) {
                isReadingTetrahedra = true;
            }
            if (line.find("$EndElements") != std::string::npos) {
                isReadingTetrahedra = false;
            }
        }
                tet_file.close();
        }
        std::cout << "N vertices: " << positions.size() << std::endl;
        std::cout << "N tetrahedra: " << indices.size()/4 << std::endl;
    };
};

void renderMesh(Mesh &mesh, uint &vao, uint &vbo)
{
    unsigned int positions_size = mesh.positions.size() * sizeof(Eigen::Vector3f);
    unsigned int normals_size = mesh.normals.size() * sizeof(Eigen::Vector3f);
    unsigned int uv_size = mesh.uv.size() * sizeof(Eigen::Vector2f);
    unsigned int total_size = positions_size + normals_size + uv_size;
    
    if (vao == 0)
    {
        glGenVertexArrays(1, &vao);

        unsigned int ebo;
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), &mesh.indices[0], GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, total_size,NULL, GL_DYNAMIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, positions_size, mesh.positions.data());
        glBufferSubData(GL_ARRAY_BUFFER, positions_size, normals_size, mesh.normals.data());
        glBufferSubData(GL_ARRAY_BUFFER, positions_size + normals_size, uv_size, mesh.uv.data());
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(size_t)(positions_size));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(size_t)(positions_size + normals_size));
    }
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, positions_size, mesh.positions.data());
    
    glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Mesh sphereMesh(unsigned int segments) {
    std::vector<Eigen::Vector3f> positions;
    std::vector<Eigen::Vector2f> uv;
    std::vector<Eigen::Vector3f> normals;
    std::vector<unsigned int> indices;
    std::vector<unsigned int> tetrahedra_indices;

    const unsigned int X_SEGMENTS = segments;
    const unsigned int Y_SEGMENTS = segments;
    const float PI = 3.14159265359;
    
    float r = 0;
    //std::cout << r;
    int count =0;
    for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
    {
        for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
        {
            //float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX/0.3);
            float xSegment = (float)x / (float)X_SEGMENTS;
            float ySegment = (float)y / (float)Y_SEGMENTS;
            float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            float yPos = std::cos(ySegment * PI);
            float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

            positions.push_back(Eigen::Vector3f(xPos + r, yPos + r, zPos + r));
            uv.push_back(Eigen::Vector2f(xSegment, ySegment));
            normals.push_back(Eigen::Vector3f(xPos, yPos, zPos));
            
            count++;
        }
    }

    bool oddRow = false;
    for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
    {
        if (!oddRow) // even rows: y == 0, y == 2; and so on
        {
            for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
            {
                indices.push_back(y       * (X_SEGMENTS + 1) + x);
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
            }
        }
        else
        {
            for (int x = X_SEGMENTS; x >= 0; --x)
            {
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                indices.push_back(y       * (X_SEGMENTS + 1) + x);
            }
        }
        oddRow = !oddRow;
    }
    return Mesh{positions, uv, normals, indices};
}

void skinTetMesh(TetrahedralMesh &tetMesh, Mesh &mesh) {
    mesh.positions = tetMesh.positions;
    int n_tet = tetMesh.indices.size()/4;
    
    if(mesh.indices.size() == 0) {
    for (int i = 0; i<n_tet; i++) {
        mesh.indices.push_back(tetMesh.indices[i*4]);
        mesh.indices.push_back(tetMesh.indices[i*4 + 1]);
        mesh.indices.push_back(tetMesh.indices[i*4 + 3]);
    }
    }
}

// renders (and builds at first invocation) a sphere
// -------------------------------------------------
unsigned int sphereVAO = 0;
unsigned int sphereVBO = 0;
Mesh m;
void renderSphere(unsigned int segments)
{
    m = sphereMesh(segments);
   renderMesh(m, sphereVAO, sphereVBO);
}
#endif /* draw_shapes_h */
