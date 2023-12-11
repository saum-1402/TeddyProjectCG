# TeddyProjectCG

Steps to run the code:

1. Run "cmake CMakeLists.text" to generate the make file of the code
2. Run "make"
3. Now run the executable "./Project"
4. Add points using right mouse button.
5. Camera direction is changed by (WASD) keys or left mouse button(to move right and left).
6. Press tab to do delaunay triangulation(Move the camera a bit to make the lines visible).
7. Press Esc to find the spine pts and tab again to retriangulate.
8. Press Enter to elevate the spine points (By moving the camera back side of the object can also be observed).
9. Press X to draw semicircle around the edges that contain the spines.
10. Press tab to retriangulate and get the 3d object.
11. The object can be smoothened by pressing C and retriangulating again.

(make sure not to change the camera initially since entering pts at a new camera position may give unwanted results).