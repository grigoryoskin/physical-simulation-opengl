#ifndef draw_shapes_h
#define draw_shapes_h

#include <Eigen/Dense>
#include <vector>

struct Mesh {
public:
    std::vector<Eigen::Vector3f> positions;
    std::vector<Eigen::Vector2f> uv;
    std::vector<Eigen::Vector3f> normals;
    std::vector<unsigned int> indices;
    std::vector<unsigned int> creases;
};

void renderMesh(Mesh &mesh, uint &vao)
{
    int positions_size = mesh.positions.size() * sizeof(Eigen::Vector3f);
    int normals_size = mesh.normals.size() * sizeof(Eigen::Vector3f);
    int uv_size = mesh.uv.size() * sizeof(Eigen::Vector2f);
    int total_size = positions_size + normals_size + uv_size;
    
    if (vao == 0)
    {
        glGenVertexArrays(1, &vao);

        unsigned ebo, vbo;
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), &mesh.indices[0], GL_STATIC_DRAW);
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
        glVertexAttribPointer(
                              2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(size_t)(positions_size + normals_size));
    }
    
    glBindVertexArray(vao);
    glBufferSubData(GL_ARRAY_BUFFER, 0, positions_size, mesh.positions.data());

    glDrawElements(GL_TRIANGLE_STRIP, mesh.indices.size(), GL_UNSIGNED_INT, 0);
}

Mesh sphereMesh(unsigned int segments) {
    std::vector<Eigen::Vector3f> positions;
    std::vector<Eigen::Vector2f> uv;
    std::vector<Eigen::Vector3f> normals;
    std::vector<unsigned int> indices;
    std::vector<unsigned int> creases;

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
            if(y==0 || y == Y_SEGMENTS || x==0 || x == X_SEGMENTS) {
                creases.push_back(count);
            }
            
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
    return Mesh{positions, uv, normals, indices, creases};
}

// renders (and builds at first invocation) a sphere
// -------------------------------------------------
unsigned int sphereVAO = 0;
Mesh m;
void renderSphere(unsigned int segments)
{
    m = sphereMesh(segments);
    renderMesh(m, sphereVAO);
}
#endif /* draw_shapes_h */
