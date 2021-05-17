# Mass spring simulation made with OpenGL and Eigen.

Simulates a sphere with mass in each vertex and a spring between each pair of verices.

 ![spring-simulation-complete](https://user-images.githubusercontent.com/44236259/114544494-abb8b080-9c95-11eb-8185-851dc37e6173.gif)

Each step the system is updated using backward Euler method and rendered with OpenGL using a simple shader that outputs world - space normals. Some OpenGL - related code is based on https://github.com/JoeyDeVries/LearnOpenGL with glm replaced by Eigen. 

