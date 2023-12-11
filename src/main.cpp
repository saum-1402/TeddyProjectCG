#include "utils/utils.hpp"
#include "drawable.hpp"
#include "camera/camera.hpp"
#include "mesh/mesh.hpp"


#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include "imgui.h"
#include <cmath>
#include <iostream>
#include <cstdlib>
#include <map>
#include <unistd.h>
#include <random>
#include <iostream>
#include <set>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <stdlib.h>
#include <vector>


using namespace std;

vector<glm::vec3> points; // Store entered points
vector<glm::vec3> cuttingpoints; // Store entered points
void createAxesLine(unsigned int &, unsigned int &);

vector<double> Pointswithoutdups;
vector<vector<int>> terminal_triangles;
vector<glm::vec3> midpoints;
map<vector<int>, vector<int>> pointsToCheck;
map<int, pair<double, double>> mp;  //contains the point to coordinate mapping(x,y,z)
vector<int> readele_vector;
map<vector<double>, vector<vector<double>>> spineTovertices;
int onlyfirstvertex = 0;
int flagforcutting = 0;
vector<glm::vec3> unique_midpoints;
vector<glm::vec3> spine_pts;
vector<glm::vec3> boundary_pts;

int numSegments = 10; 

// Function to calculate the midpoint of an edge
glm::vec3 calculateEdgeMidpoint(const glm::vec3 &p1, const glm::vec3 &p2) {
    return glm::vec3((p1.x + p2.x) / 2.0f, (p1.y + p2.y) / 2.0f, (p1.z + p2.z) / 2.0f);
}

// Function to check if an edge is internal (not a boundary edge)
bool isInternalEdge(const glm::vec3 &p1, const glm::vec3 &p2)
{
    // Iterate over triangles to check if the edge is shared by more than one triangle
    int sharedTriangleCount = 0;

    for (size_t i = 0; i < readele_vector.size(); i += 3)
    {
        int v1_index = readele_vector[i];
        int v2_index = readele_vector[i + 1];
        int v3_index = readele_vector[i + 2];

        const glm::vec3 &triV1 = glm::vec3(Pointswithoutdups[3*(v1_index-1)],Pointswithoutdups[3*(v1_index-1) +1],Pointswithoutdups[3*(v1_index-1)+2]);
        const glm::vec3 &triV2 = glm::vec3(Pointswithoutdups[3*(v2_index-1)],Pointswithoutdups[3*(v2_index-1) +1],Pointswithoutdups[3*(v2_index-1)+2]);
        const glm::vec3 &triV3 = glm::vec3(Pointswithoutdups[3*(v3_index-1)],Pointswithoutdups[3*(v3_index-1) +1],Pointswithoutdups[3*(v3_index-1)+2]);

        // Check if the edge (p1, p2) matches any edge of the current triangle
        if ((triV1 == p1 && triV2 == p2) || (triV2 == p1 && triV3 == p2) || (triV3 == p1 && triV1 == p2) ||
            (triV1 == p2 && triV2 == p1) || (triV2 == p2 && triV3 == p1) || (triV3 == p2 && triV1 == p1))
        {
            sharedTriangleCount++;
        }

        // Break early if the edge is shared by more than one triangle
        if (sharedTriangleCount > 1)
        {
            return true;
        }
    }

    // If the edge is shared by exactly one triangle, it is likely a boundary edge
    return false;
}


double getdistance3d(double x1, double y1, double x2, double y2, double z1, double z2){
	double dist = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)+(z1-z2)*(z1-z2));
	return dist;
}


std::vector<glm::vec3> interpolateSemicircle(const glm::vec3 &start, const glm::vec3 &end, float radius, int numSegments)
{
	std::vector<glm::vec3> curvePoints;
	glm::vec3 center = (start + end) / 2.0f;     // Midpoint of the edge  //need to check if this is the correct way to calculate the center

	// Calculate the normal vector of the plane containing the semicircle
	glm::vec3 normal = glm::normalize(end - start);

	// Use the normal vector to create a rotation matrix
	glm::mat3 rotationMatrix = glm::mat3(glm::rotate(glm::mat4(1.0f), glm::half_pi<float>(), normal));

	// Generate points along the semicircle using the rotation matrix
	for (int i = 0; i <= numSegments; ++i)
	{
		float theta = 2.0f*glm::pi<float>() * static_cast<float>(i) / numSegments;   //instead of making a semicircle, we can make a circle
		glm::vec3 pointoncircle = center + rotationMatrix * (radius * glm::vec3(std::cos(theta), std::sin(theta), 0.0f));
		curvePoints.push_back(pointoncircle);
	}
	return curvePoints;
}



void calculateAndDrawChordalAxis(vector<double>Pointswithoutdups, unsigned int &program,GLuint vao)
{
	
    // Iterate over the triangulation vertices and connect midpoints to form the chordal axis
    for (size_t i = 0; i < readele_vector.size(); i += 3)
    {
        int v1_index = readele_vector[i];
        int v2_index = readele_vector[i + 1];
        int v3_index = readele_vector[i + 2];

        // Retrieve vertices from the points vector
   		const glm::vec3 &v1 = glm::vec3(Pointswithoutdups[3*(v1_index-1)],Pointswithoutdups[3*(v1_index-1) +1],Pointswithoutdups[3*(v1_index-1)+2]);
        const glm::vec3 &v2 = glm::vec3(Pointswithoutdups[3*(v2_index-1)],Pointswithoutdups[3*(v2_index-1) +1],Pointswithoutdups[3*(v2_index-1)+2]);
        const glm::vec3 &v3 = glm::vec3(Pointswithoutdups[3*(v3_index-1)],Pointswithoutdups[3*(v3_index-1) +1],Pointswithoutdups[3*(v3_index-1)+2]);
		
        if(isInternalEdge(v1,v2)){
			cout << "Internal edge: (" << v1_index << ", " << v2_index << ")" << endl;
			glm::vec3 midpoint = calculateEdgeMidpoint(v1, v2);   
			midpoints.push_back(midpoint);
			// add_midpoint(midpoint,v1,v2,midpointMap,midpoints);
			vector<double> temp;
			double x = midpoint.x;
			double y = midpoint.y;
			double z = midpoint.z;
			temp.push_back(x);
			temp.push_back(y);
			temp.push_back(z);

			vector<vector<double>> temp1;
			temp1.push_back({v1.x,v1.y,v1.z});
			temp1.push_back({v2.x,v2.y,v2.z});
			spineTovertices[temp]=temp1;
		}
        if(isInternalEdge(v2,v3)){
			cout << "Internal edge: (" << v2_index<< ", " << v3_index<< ")" << endl;
			glm::vec3 midpoint = calculateEdgeMidpoint(v2, v3);
			midpoints.push_back(midpoint);
			vector<double> temp;
			double x = midpoint.x;
			double y = midpoint.y;
			double z = midpoint.z;
			temp.push_back(x);
			temp.push_back(y);
			temp.push_back(z);

			vector<vector<double>> temp1;
			temp1.push_back({v2.x, v2.y, v2.z});
			temp1.push_back({v3.x, v3.y, v3.z});
			spineTovertices[temp] = temp1;
		}
			//add_midpoint(midpoint,v1,v2,midpointMap,midpoints);}
	
        if(isInternalEdge(v3,v1)){
			 cout << "Internal edge: (" << v3_index << ", " << v1_index << ")" << endl;
			glm::vec3 midpoint = calculateEdgeMidpoint(v3, v1);
			midpoints.push_back(midpoint);
			vector<double> temp;
			double x = midpoint.x;
			double y = midpoint.y;
			double z = midpoint.z;
			temp.push_back(x);
			temp.push_back(y);
			temp.push_back(z);

			vector<vector<double>> temp1;
			temp1.push_back({v3.x, v3.y, v3.z});
			temp1.push_back({v1.x, v1.y, v1.z});
			spineTovertices[temp] = temp1;
		}
			//add_midpoint(midpoint,v1,v2,midpointMap,midpoints);}
    }
	unique_midpoints.clear();
	set<pair<float, float>> s;
	for (int i = 0; i < midpoints.size(); i += 1)
	{
		s.insert(make_pair(static_cast<float>(midpoints[i].x), static_cast<float> (midpoints[i].y)));
	}
	// cout<<s.size()<<endl;
	for (auto i : s)
	{
		// unique_midpoints.push_back(glm::vec3(i.first,i.second,10.3f));
		points.push_back(glm::vec3(i.first,i.second,10.3f));
		spine_pts.push_back(glm::vec3(i.first,i.second,10.3f));
		
	}

}

// Function to calculate and draw the chordal axis or spine

// int flag=0;
void elevation(){

	for (auto i : spineTovertices)
	{
		vector<double> temp = i.first;			 // spine pt
		vector<vector<double>> temp1 = i.second; // vertices
		vector<double> st = temp1[0];
		vector<double> end = temp1[1];
		float radius = getdistance3d(st[0], st[1], end[0], end[1], st[2], end[2]) / 2;
		points.push_back(glm::vec3(temp[0], temp[1], temp[2] + radius));
		points.push_back(glm::vec3(temp[0], temp[1], temp[2] - radius));
	}
}

void make3dMesh(){
	// Example usage:

	for(auto i:spineTovertices){
		vector<double> temp = i.first; //spine pt
		vector<vector<double>> temp1 = i.second; //vertices

		vector<double> st = temp1[0];
		vector<double> end = temp1[1];
		float radius = getdistance3d(st[0], st[1], end[0], end[1], st[2], end[2])/2;
		glm::vec3 startPoint(st[0], st[1], st[2]);
		glm::vec3 midpt(temp[0], temp[1], temp[2]);
		glm::vec3 endPoint(end[0], end[1], end[2]);
		std::vector<glm::vec3> curvePoints1 = interpolateSemicircle(endPoint, startPoint, radius, numSegments);
		for (auto i : curvePoints1)
		{
			points.push_back(i);
		}

	}
}



// void cutting_plane(){
     //take 3 pts as inputs from the user and make a plane and then using the equation find which pts lie on the left and remove them from the points vector
// }

vector<double> remove_duplicates(vector<double> Pointswp){
	// Pointswithoutdups.clear();
	Pointswp.clear();
	set<vector<float>> vectorSet;

	for (int i = 0; i < points.size(); i += 1)
	{
		vector<float> temp;
		temp.push_back(points[i].x);
		temp.push_back(points[i].y);
		temp.push_back(points[i].z);
		if(find(cuttingpoints.begin(),cuttingpoints.end(),points[i])!=cuttingpoints.end()){
			continue;
		}
		vectorSet.insert(temp);
	}

	for (auto i : vectorSet)
	{
		Pointswp.push_back(i[0]);
		Pointswp.push_back(i[1]);
		Pointswp.push_back(i[2]);
	}
	
	return Pointswp;


}

vector<double> remove_duplicatesFor3dmesh(vector<double> Pointswp)
{
	// Pointswithoutdups.clear();
	Pointswp.clear();
	set<vector<float>> vectorSet;

	for (int i = 0; i < points.size(); i += 1)
	{   
		if(points[i].z==10.3f+0.01f){
			continue;
		}
		vector<float> temp;
		temp.push_back(points[i].x);
		temp.push_back(points[i].y);
		temp.push_back(points[i].z);
		vectorSet.insert(temp);
	}

	for (auto i : vectorSet)
	{
		Pointswp.push_back(i[0]);
		Pointswp.push_back(i[1]);
		Pointswp.push_back(i[2]);
	}

	return Pointswp;
}

// Function to perform linear interpolation between two 3D points
// Function to interpolate points along a circular arc in 3D

void triangulation(vector<double> Pointswd)
{
	ofstream MyFile("new.node");
	MyFile << to_string(Pointswd.size() / 3) << " 2 0 1" << endl;
	double x_past = 0;
	double y_past = 0;
	int vnumber = 1;
	for (int i = 0; i < Pointswd.size(); i += 3)
	{
		MyFile << to_string(i / 3 + 1) << " " << to_string(Pointswd[i]) << " " << to_string(Pointswd[i + 1]) << endl;
	}
	MyFile.close();
	system("cd src");
	system("triangle new");
}

void CDTtriangulation()
{
	ofstream MyFile("new.poly");
	MyFile << to_string(Pointswithoutdups.size() / 3) << " 2 0 1" << endl; // 1 means it is a boundary marker
	double x_past = 0;
	double y_past = 0;
	int vnumber = 1;
	for (int i = 0; i < Pointswithoutdups.size(); i += 3)
	{
		MyFile << to_string(i / 3 + 1) << " " << to_string(Pointswithoutdups[i]) << " " << to_string(Pointswithoutdups[i + 1]) << " 2" << endl;
	}
	MyFile << to_string(Pointswithoutdups.size() / 3) << " 1" << endl;
	int t = 1;
	int k = 0;
	for (int i = 0; i < Pointswithoutdups.size(); i += 3)
	{
		if (t + 1 > Pointswithoutdups.size() / 3)
		{
			k = 1;
		}
		else
		{
			k = t + 1;
		}
		MyFile << to_string(i / 3 + 1) << " " << to_string(t) << " " << to_string(k) << " 2" << endl;
		t++;
	}
	MyFile << "0" << endl;
	MyFile.close();
	system("cd src");
	system("triangle -pqD new");
}

void readele()
{
	readele_vector.clear();
	//cout << "readele" << endl;
	string line;
	ifstream MyReadfile("new.1.ele");
	//cout << "readele1" << endl;
	int i = 0;
	// int t = 0;

	while (std::getline(MyReadfile, line))
	{
		std::istringstream iss(line);
		std::string word;
		int t = 0;
		if (i == 0)
		{
			i++;
			continue;
		}
		int count = 0;
		double first;

		// int t=0;
		while (iss >> word)
		{

			// Check if the word is a number
			bool isNumber = true;
			for (char c : word)
			{
				if (!std::isdigit(c) && c != '.' && c != '-')
				{
					isNumber = false;
					break;
				}
			}
			if (isNumber)
			{
				// Convert the word to a number and store it
				int number;
				std::istringstream(word) >> number;
				if (t == 0)
				{
					t++;
					continue;
				}
				if (count == 0)
				{
					first = number;
				}

				readele_vector.push_back(number);
				// count++;
				// if (count == 3 && flag == 0)
				// {
				// 	readele_vector.push_back(first);
				// 	count = 0;
				// 	flag = 1;
				// 	// onlyfirstvertex=1;
				// }
			}
		}
	}
	MyReadfile.close();
}

Mesh *createPointsMesh(const vector<glm::vec3> &points)
{
	vector<GLfloat> verts;
	for (const auto &point : points)
	{
		verts.push_back(point.x);
		verts.push_back(point.y);
		verts.push_back(point.z);
	}
	GLuint indices[] = {};
	return new Mesh(verts.data(), indices, verts.size() / 3, 0);
}

Mesh *newMesh(){
	vector<GLfloat> verts;
	for (auto p : Pointswithoutdups)
	{
		verts.push_back(p);
	}
	vector<GLint> indices;
	for(auto i:readele_vector){
		indices.push_back(i-1);
	}

	return new Mesh(verts.data(), reinterpret_cast<GLuint *> (indices.data()), verts.size()/3, indices.size()/3);

	
}

int main(int, char **)
{
	// Setup window
	GLFWwindow *window = setupWindow(SCREEN_W, SCREEN_H, "Parametric Representations of Surfaces");
	ImGuiIO &io = ImGui::GetIO(); // Create IO

	double last_time = 0;
	double delta_time = 0;

	unsigned int shader_program = createProgram("shaders/vshader.vs", "shaders/fshader.fs");

	Camera *cam = new Camera(glm::vec3(0.0f, 0.0f, 10.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
							 45.0f, 0.1f, 10000.0f, window);

	// Camera *cam = new Camera(glm::vec3(-5.0f, 3.0f, 3.5f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
	// 						 45.0f, 0.1f, 10000.0f, window);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	GLuint vao, vbo;

	// Create a VAO and VBO for the points
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	// Bind the VAO
	glBindVertexArray(vao);

	// Bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// Reserve space for the points (optional but can be more efficient)
	glBufferData(GL_ARRAY_BUFFER, points.capacity() * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);

	// Specify the layout of the vertex data
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
	glEnableVertexAttribArray(0);

	// Unbind the VAO and VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	Mesh *pointsMesh = nullptr;

	// Mesh *newmesh = newMesh();
	unsigned int axis_VAO;
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	int t=0;
	
	while (!glfwWindowShouldClose(window))
	{
		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		double curr_time = static_cast<double>(glfwGetTime());
		delta_time = curr_time - last_time;
		last_time = curr_time;

		if (!io.WantCaptureMouse)
		{
			cam->process_input(window, delta_time);
			cam->setViewTransformation(shader_program);
			cam->setProjectionTransformation(shader_program);
		}

		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(WHITE.x, WHITE.y, WHITE.z, WHITE.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		int drawingInProgress = 1;
		if (!ImGui::IsAnyItemActive())
		{
			if (drawingInProgress && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
			{
				double xpos, ypos;
				int screen_width = SCREEN_W;
				int screen_height = SCREEN_H;
				glfwGetCursorPos(glfwGetCurrentContext(), &xpos, &ypos);
				cout << "Screen coordinates: (" << xpos << ", " << ypos << ")" << endl;
				glm::vec3 winPos = glm::vec3(xpos, screen_height - ypos, 0.5f);
				glm::mat4 modelMatrix = cam->getView();
				glm::mat4 viewProjection = cam->getProjection();
				glm::vec4 viewport = glm::vec4(0.0f, 0.0f, screen_width, screen_height);
				glm::vec3 worldPos = glm::unProject(winPos, modelMatrix, viewProjection, viewport);

				if(flagforcutting==1){
					cuttingpoints.push_back(worldPos);
					points.push_back(worldPos);
				}
				else{
				// worldPos.z = 0.0f;
				points.push_back(worldPos);
				boundary_pts.push_back(worldPos);
				int x = xpos/100;
				int y = ypos/100;
				// cout<<x<<" "<<y<<endl;
				// points.push_back(glm::vec3(x, y, 0.0f));
				// cout << "World coordinates: (" << worldPos.x << ", " << worldPos.y << ", " << worldPos.z << ")" << endl;
				}
			}

			if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
			{
				drawingInProgress = 0;
			}
		}

		if (ImGui::IsMouseReleased(ImGuiMouseButton_Middle))
		{
			drawingInProgress = 1;
		}

		

		// Update the Mesh only if points have changed
		Mesh *newPointsMesh = createPointsMesh(points);
		if (newPointsMesh != pointsMesh)
		{
			delete pointsMesh;
			pointsMesh = newPointsMesh;
		}

		// Draw the Mesh
		pointsMesh->draw(shader_program);

		// createAxesLine(shader_program, axis_VAO);

		// Use the shader program
		glUseProgram(shader_program);

		// Bind the VAO
		glBindVertexArray(vao);

		// Update the VBO with the current points
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec3), points.data(), GL_DYNAMIC_DRAW);

		// Render the points
		glPointSize(10.0f);
		glDrawArrays(GL_POINTS, 0, points.size());

		// Unbind the VAO
		glBindVertexArray(0);

		// Unuse the shader program
		glUseProgram(0);

		// Check for right mouse button
		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Tab)))
		{
			Pointswithoutdups = remove_duplicates(Pointswithoutdups);
			triangulation(Pointswithoutdups);
			readele();
			// cout << readele_vector.size() << endl;
			// for (int i = 0; i < readele_vector.size(); i++)
			// {
			// 	cout << readele_vector[i] << " ";
			// 	if (i % 3 == 2)
			// 	{
			// 		cout << endl;
			// 	}
			// }
			t++;
			
			
		}
		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape))){
			calculateAndDrawChordalAxis(Pointswithoutdups,shader_program,vao);
		}

		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter))){
			elevation();

		}

		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_X))){  //to complete the mesh
			// cout<<"pruning"<<endl;
			make3dMesh();
			// prune();
		}

		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_C))){
			numSegments+=10;
			make3dMesh();
		}
		
		if(t>0){
			Mesh *newmesh = newMesh();
			newmesh->draw(shader_program);
		}
		
		
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Clean up Mesh
	delete pointsMesh;

	// Cleanup
	cleanup(window);

	return 0;
}

