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
#include <random>

using namespace std;

float a = 0.0;
float b = 0.0;
bool drawingInProgress = false;
int c = 0;
int p = 0;
#define DRAW_CUBIC_BEZIER 1   // Use to switch Linear and Cubic bezier curves
#define SAMPLES_PER_BEZIER 10 // Sample each Bezier curve as N=10 segments and draw as connected lines

// GLobal variables
std::vector<float> controlPoints;

std::vector<float> linearBezier;
std::vector<float> cubicBezier;
int width = 640, height = 640;
bool controlPointsUpdated = false;
bool controlPointsFinished = false;
int selectedControlPoint = -1;


void calculatePiecewiseCubicBezier()
{
    cubicBezier.clear();        // Clear the vector to accomodate new points such that they maintain C1 continuity
    c++;
    int sz = controlPoints.size(); // Contains 3 points/vertex. Ignore Z
    float x[4], y[4];
    float delta_t = 1.0 / (SAMPLES_PER_BEZIER - 1.0);
    float t;
    int k = 0;
    for (int i = 0; i < (sz -3); i += 3)    // runs such that atleast 2 points are present
    {
        x[0] = controlPoints[i];               //x coordinate of P0
        y[0] = controlPoints[i + 1];            //y coordinate of P0
        x[3] = controlPoints[i + 3];            //x coordinate of P3
        y[3] = controlPoints[i + 4];            //y coordinate of P3
        float o = max(x[0], x[3]) - min(x[0], x[3]);
        float p = y[0];
        if (i == 0)
        {
            x[1]=x[0]+o/4;                  //x coordinate of P1
            y[1]=o;                         //y coordinate of P1
            x[2]=x[0]+3*(o/4);              //x coordinate of P2
            y[2]=o+0.5;                     //y coordinate of P2
        }
        else
        {
            x[1] = 2*x[0]-a;                  //x coordinate of P1
            y[1] = 2*y[0]-b;                  //y coordinate of P1
            x[2] = a + 0.05;                    //x coordinate of P2
            y[2] = b;                           //y coordinate of P2    
        }

        t = 0.0;
        //generate points on Bezier curve at different values of t between P0 and P3
        for (float j = 1; j < (SAMPLES_PER_BEZIER - 1); j++)
        {
            t += delta_t;
            float xp = pow(1 - t, 3) * x[0] + 3 * pow(1 - t, 2) * t * x[1] + 3 * (1 - t) * pow(t, 2) * x[2] + pow(t, 3) * x[3];
            cubicBezier.push_back(xp);
            float yp = pow(1 - t, 3) * y[0] + 3 * pow(1 - t, 2) * t * y[1] + 3 * (1 - t) * pow(t, 2) * y[2] + pow(t, 3) * y[3];
            cubicBezier.push_back(yp);
            cubicBezier.push_back(0.0);
        }

        a = x[2];
        b = y[2];

        // k++;
    }
    // adding the last controol point to the curve P3
    cubicBezier.push_back(x[3]);
    cubicBezier.push_back(y[3]);
    cubicBezier.push_back(0.0);
    
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


            
            // Update VAO/VBO for piecewise cubic Bezier curve
            // TODO:
            calculatePiecewiseCubicBezier();
            glBindVertexArray(VAO_cubicBezier);
            glBindBuffer(GL_ARRAY_BUFFER, VBO_cubicBezier);
            glBufferData(GL_ARRAY_BUFFER, cubicBezier.size() * sizeof(GLfloat), &cubicBezier[0], GL_DYNAMIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
            glEnableVertexAttribArray(0); // Enable first attribute buffer (vertices)

            controlPointsUpdated = false; // Finish all VAO/VBO updates before setting this to false.
            // controlPointsUpdated_n = false;
        }

        glUseProgram(shaderProgram);

        // Draw control points
        glBindVertexArray(VAO_controlPoints);
        glDrawArrays(GL_POINTS, 0, controlPoints.size() / 3); // Draw points


        

        

#if DRAW_CUBIC_BEZIER
        // TODO:
        // Draw cubic Bezier
        // glBindVertexArray(VAO_cubicBezier);
        // glDrawArrays(GL_LINE_STRIP, 0, cubicBezier.size() / 3); // Draw lines


#else

        

#endif

        glUseProgram(0);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Delete VBO buffers
    glDeleteBuffers(1, &VBO_controlPoints);
    // glDeleteBuffers(1, &VBO_linearBezier);
    // TODO:
    glDeleteBuffers(1, &VBO_cubicBezier);

    // Cleanup
    cleanup(window);
    return 0;
}
