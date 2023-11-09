/******************************************************************************
 *                                                                            *
 *  Copyright (c) 2023 Ojaswa Sharma. All rights reserved.                    *
 *                                                                            *
 *  Author: Ojaswa Sharma                                                     *
 *  E-mail: ojaswa@iiitd.ac.in                                                *
 *                                                                            *
 *  This code is provided solely for the purpose of the CSE 333/533 course    *
 *  at IIIT Delhi. Unauthorized reproduction, distribution, or disclosure     *
 *  of this code, in whole or in part, without the prior written consent of   *
 *  the author is strictly prohibited.                                        *
 *                                                                            *
 *  This code is provided "as is", without warranty of any kind, express      *
 *  or implied, including but not limited to the warranties of                *
 *  merchantability, fitness for a particular purpose, and noninfringement.   *
 *                                                                            *
 ******************************************************************************/

#include "utils.h"
#include <cmath>
#include <iostream>
#include <cstdlib>
#include <map>
#include <unistd.h>
#include <random>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#define ANSI_DECLARATORS
// #include "triangle.h"

using namespace std;

#define REAL double

// struct triangulateio{
//     int a;
//G }
// Model, View and Projection Transformation Matrices
float a = 0.0;
float b = 0.0;
bool drawingInProgress = false;
int c = 0;
int p = 0;
#define DRAW_CUBIC_BEZIER 0   // Use to switch Linear and Cubic bezier curves
#define SAMPLES_PER_BEZIER 10 // Sample each Bezier curve as N=10 segments and draw as connected lines

// GLobal variables
std::vector<float> controlPoints;
std::vector<float> controlPoints_triangle;

std::vector<float> linearBezier;
std::vector<float> cubicBezier;
int width = 640, height = 640;
bool controlPointsUpdated = false;
bool controlPointsFinished = false;
int selectedControlPoint = -1;

void calculatePiecewiseLinearBezier()
{
    linearBezier.clear();
    int sz = controlPoints.size(); // Contains 3 points/vertex. Ignore Z
    float x[2], y[2];
    float delta_t = 1.0 / (SAMPLES_PER_BEZIER - 1.0);
    float t;
    for (int i = 0; i < (sz - 3); i += 3)
    {
        x[0] = controlPoints[i];
        y[0] = controlPoints[i + 1];
        x[1] = controlPoints[i + 3];
        y[1] = controlPoints[i + 4];
        linearBezier.push_back(x[0]);
        linearBezier.push_back(y[0]);
        linearBezier.push_back(0.0);
        t = 0.0;
        for (float j = 1; j < (SAMPLES_PER_BEZIER - 1); j++)
        {
            t += delta_t;
            linearBezier.push_back(x[0] + t * (x[1] - x[0]));
            linearBezier.push_back(y[0] + t * (y[1] - y[0]));
            linearBezier.push_back(0.0);
        }
        // No need to add the last point for this segment, since it will be added as first point in next.
    }
    // However, add last point of entire piecewise curve here (i.e, the last control point)
    linearBezier.push_back(x[1]);
    linearBezier.push_back(y[1]);
    linearBezier.push_back(0.0);
}


void triangulation(vector<float> controlPoints){
    ofstream MyFile("new.poly");
    MyFile << to_string(controlPoints.size()/3) << " 2 0 0" << endl;

    for(int i=0;i<controlPoints.size();i+=3){
        MyFile << to_string(i/3+1) << " " << to_string(controlPoints[i]) << " " << to_string(controlPoints[i+1]) << endl;
    }
    MyFile << to_string(controlPoints.size()/3) << "0" << endl;
    for(int i=0;i<controlPoints.size()-3;i+=3){
        MyFile << to_string(i/3+1) << " " << to_string(i/3) << " " << to_string(i/3+1) << endl;
    }
    MyFile << to_string((controlPoints.size()-3)/3+1) << " " << to_string((controlPoints.size()-3)/3) << " " << to_string(0) << endl;
    MyFile << "0" << endl;

    MyFile.close();
    
    system("cd src");
    system("triangle -p new");
}


map<int,pair<double,double>> mp;
vector<int> readele_vector;

vector<int> readele(){
    // cout<<"readele"<<endl;
    string myText;
    fstream MyReadfile("new.1.ele");
    int i=0;
    while (getline(MyReadfile, myText))
    {   if(i==0){
            i++;
            continue;
        }
        istringstream iss(myText);
        int num;
        while(iss>>num){
            readele_vector.push_back(num);
        }
        // Output the text from the file
        // cout << myText;
    }
    

    // for(int n:readele_vector){
    //     cout<<n<<" ";
    // }
    // Close the file
    MyReadfile.close();
}

int main(int, char *argv[])
{
    
    GLFWwindow *window = setupWindow(width, height);
    ImGuiIO &io = ImGui::GetIO(); // Create IO object

    ImVec4 clear_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

    unsigned int shaderProgram = createProgram("./shaders/vshader.vs", "./shaders/fshader.fs");


    glUseProgram(shaderProgram);

    // Create VBOs, VAOs
    unsigned int VBO_controlPoints, VBO_linearBezier, VBO_cubicBezier;
    unsigned int VAO_controlPoints, VAO_linearBezier, VAO_cubicBezier;
    glGenBuffers(1, &VBO_controlPoints);
    glGenVertexArrays(1, &VAO_controlPoints);

    glGenBuffers(1, &VBO_linearBezier);
    glGenVertexArrays(1, &VAO_linearBezier);
    // TODO:
    // glGenBuffers(1, &VBO_controlPoints);
    // glGenVertexArrays(1, &VAO_controlPoints);
    glGenBuffers(1, &VBO_cubicBezier);
    glGenVertexArrays(1, &VAO_cubicBezier);

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);

    int button_status = 0;


    // glutDisplayFunc(display);

    // Display loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Rendering
        showOptionsDialog(controlPoints, io);
        ImGui::Render();

        // Add a new point on mouse click
        float x, y;
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(VAO);

        if (!ImGui::IsAnyItemActive())
        {
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
                // Set a flag to indicate that drawing/selection is in progress
                drawingInProgress = true;
            }

            if (drawingInProgress && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
            {
                // Drawing/selection is complete when the left mouse button is released
                drawingInProgress = false;
            }

            if (drawingInProgress)
            {
                x = io.MousePos.x;
                y = io.MousePos.y;

                if (!controlPointsFinished)
                {
                    // Add points
                    addControlPoint(controlPoints, x, y, width, height);
                    controlPointsUpdated = true;
                }
                else
                {
                    // Select point
                    searchNearestControlPoint(x, y);
                }
            }

            if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
            {
                controlPointsFinished = true;
            }
        
        }

        if (controlPointsUpdated)
        {
            // Update VAO/VBO for control points (since we added a new point)
            glBindVertexArray(VAO_controlPoints);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_controlPoints);
            glBufferData(GL_ARRAY_BUFFER, controlPoints.size() * sizeof(GLfloat), &controlPoints[0], GL_DYNAMIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
            glEnableVertexAttribArray(0); // Enable first attribute buffer (vertices)

            calculatePiecewiseLinearBezier();
            glBindVertexArray(VAO_linearBezier);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_linearBezier);
            glBufferData(GL_ARRAY_BUFFER, linearBezier.size() * sizeof(GLfloat), &linearBezier[0], GL_DYNAMIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
            glEnableVertexAttribArray(0); // Enable first attribute buffer (vertices)

            // TODO:
  
            controlPointsUpdated = false; // Finish all VAO/VBO updates before setting this to false.
            // controlPointsUpdated_n = false;
        }

        glUseProgram(shaderProgram);

        // Draw control points
        glBindVertexArray(VAO_controlPoints);
        glDrawArrays(GL_POINTS, 0, controlPoints.size() / 3); // Draw points

        for(int i=0;i<controlPoints.size();i+=3){
            mp[i/3+1] = make_pair(controlPoints[i],controlPoints[i+1]);
        }


        if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Tab))){
            triangulation(controlPoints);

        }

        // if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter))){
            // readele();
        // }

        for(int i=0;i<readele_vector.size();i++){
            controlPoints_triangle.push_back(mp[readele_vector[i]].first);
            controlPoints_triangle.push_back(mp[readele_vector[i]].second);
        }
    
        

        

#if DRAW_CUBIC_BEZIER
        // TODO:
        // Draw cubic Bezier
        // glBindVertexArray(VAO_cubicBezier);
        // glDrawArrays(GL_LINE_STRIP, 0, cubicBezier.size() / 3); // Draw lines


#else
    glBindVertexArray(VAO_linearBezier);
    glDrawArrays(GL_LINE_STRIP, 0, linearBezier.size() / 3); // Draw lines

#endif

        glUseProgram(0);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Delete VBO buffers
    glDeleteBuffers(1, &VBO_controlPoints);
    glDeleteBuffers(1, &VBO_linearBezier);
    // TODO:
    // glDeleteBuffers(1, &VBO_cubicBezier);

    // Cleanup
    cleanup(window);
    return 0;
}
