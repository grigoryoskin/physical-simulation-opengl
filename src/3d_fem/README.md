# Soft body simulation made with OpenGL and Eigen.

Soft body simulation using finate element method on a tetrahedral mesh. For strain I used Neo-Hookean model. (Took the formula from https://en.wikipedia.org/wiki/Neo-Hookean_solid)

Each step the system is updated using backward Euler method and rendered with OpenGL using a lil pbr shader. By default it uses linear approximation to solve equation of motion, so it's a little unstable. It is possible to use gradient descent to get more precision.

![ezgif com-video-to-gif](https://user-images.githubusercontent.com/44236259/118449727-62dd9700-b72e-11eb-96e6-411ca4f9c83a.gif)


# Build on Mac OS
This is how you could build this project:

1. Go to root folder.
2. Pull glfw and glad dependencies.
```
git submudule init
git submodule update
```
3. Install Eigen
`brew install eigen`
4. Create an output directory and move to it.
```
mkdir build
cd build
```
5. Create a makefile with cmake.
```
cmake -S ../ -B ./ -DTARGET_NAME=3d_fem
```
6. Build executable.
```
make
```
7. Run it.
```
./3d_fem
```
