#include <iostream>
// #include "utils.h"
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
#include <string>
#include <math.h>
#include <map>

vector<int> readele_vector;
vector<glm::vec3> points;
vector<vector<int>> terminal_triangles;
vector<vector<int>> deleted_internalEdge;
map<vector<int>, vector<int>> pointsToCheck;
map<int, pair<double, double>> mp;
vector<double> Pointswithoutdups;
//terminal triangles need to be found(triangles with 2 external edges)
//assumptions: the triangle vertices are added in consecutive order

//check this function
int check_terminal(int a, int b, int c){
    if(abs(a-b)==1 && abs(b-c)==1 && abs(a-c)==1){
        return 0;
    }
    if(abs(a-b)==1 || abs(b-c)==1 || abs(a-c)==1){
        return 1;
    }
    return 0; 
}

void get_sign(int pt,vector<int> line){
    int a = l[0];
    int b = l[1];

    double sign = (mp[pt].second - mp[a].second)*(mp[b].first - mp[a].first) - (mp[b].second - mp[a].second)*(mp[pt].first - mp[a].first);
    if(sign>0){
        return 1;
    }
    else if(sign<0){
        return -1;
    }
    else{
        return 0;
    }
}


double getdistance(double x1, double y1, double x2, double y2){
    double dist = sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
    return dist;
}

int semicircleCheck(int pt, double radius, double centrex, double centrey){
    double x = mp[pt].first;
    double y = mp[pt].second;

    double dist = sqrt((x-centrex)*(x-centrex) + (y-centrey)*(y-centrey));
    if(dist<=radius){
        return 1;
    }
    return 0;

}

//need to make function to fill pts to check for each internal edge 
void fill_pointsTocheck(){
    for(int i=0;i<terminal_triangles.size();i++){
        //need to find the internal edge to procceed
        int a = terminal_triangles[i][0];
        int b = terminal_triangles[i][1];
        int c = terminal_triangles[i][2];
        int sign=1;

        vector<int> x;
        vector<int> y;
        vector<int> z; // z is the internal edge

        if(abs(a-b)==2){ 
            x = {a,b};
            y = {a,c};
            z = {a,b};
            
            pointsToCheck.push_back(c);
            sign = get_sign(c,z);
        }
        else if(abs(b-c)==2){
            x = {a,b};
            y = {a,c};
            z = {b,c};
            pointsToCheck.push_back(a);
            sign = get_sign(a,z);
        }
        else{
            x = {a,b};
            y = {b,c};
            z = {c,a};
            pointsToCheck.push_back(b);
            sign = get_sign(b,z);
        }

        double pt1x = mp[z[0].first];
        double pt1y = mp[z[0].second];

        double pt2x = mp[z[1].first];
        double pt2y = mp[z[1].second];

        for(int i=1;i<mp.size()+1;i+=1){
            if(sign==get_sign(i,z)){
                pointsTocheck[z].push_back(i);
            }
        }
        
    }
}

void prune(){
    for(int i=0;i<readele_vector.size();i+=3){
        int a = readele_vector[i];
        int b = readele_vector[i+1];
        int c = readele_vector[i+2];
        
        int x,y;
        if(check_terminal(a,b,c)==1){
            vector<int> temp;
            temp.push_back(a);
            temp.push_back(b);
            temp.push_back(c);
            terminal_triangles.push_back(temp);
        }
    }

    //now we have the terminal triangles
    //we need to find the terminal edges and do the pruning

    for(int i=0;i<terminal_triangles.size();i++){
        //need to find the internal edge to procceed
        int a = terminal_triangles[i][0];
        int b = terminal_triangles[i][1];
        int c = terminal_triangles[i][2];

        vector<int> x;
        vector<int> y;
        vector<int> z; // z is the internal edge

        if(abs(a-b)==2){ 
            x = {a,b};
            y = {a,c};
            z = {a,b};
        }
        else if(abs(b-c)==2){
            x = {a,b};
            y = {a,c};
            z = {b,c};
        }
        else{
            x = {a,b};
            y = {b,c};
            z = {c,a};
        }
    }

    //make semicircle and check with the rest of the edges
    for(auto i: pointsTocheck){
        vector<int> line = i.first;
        vector<int> points = i.second;

        double radius = getdistance(mp[line[0]].first,mp[line[0]].second,mp[line[1]].first,mp[line[1]].second)/2;
        double centrex = (mp[line[0]].first + mp[line[1]].first)/2;
        double centrey = (mp[line[0]].second + mp[line[1]].second)/2;
        if(semicircleCheck(pointsTocheck[i],radius,centrex,centrey)==0){
            points.push_back(glm::vec3(mp[line[0]].first,mp[line[0]].second,10.3));
        }
    
    }



}

std::vector<glm::vec3> interpolateSemicircle(const glm::vec3 &start, const glm::vec3 &end, float radius, int numSegments)
{
    std::vector<glm::vec3> curvePoints;

    // Calculate the direction vector between start and end points
    glm::vec3 direction = glm::normalize(end - start);

    // Find an axis perpendicular to the direction vector using the cross product
    glm::vec3 perpendicularAxis = glm::cross(direction, glm::vec3(0.0f, 0.0f, 1.0f));

    // Calculate the rotation matrix to align the semicircle with the direction vector
    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::atan(direction.y, direction.x), perpendicularAxis);

    // Generate points along the semicircle
    for (int i = 0; i <= numSegments / 2; ++i)
    {
        float theta = (glm::pi<float>() / static_cast<float>(numSegments)) * static_cast<float>(i);
        glm::vec3 pointOnSemicircle = start + glm::vec3(rotationMatrix * glm::vec4(radius * std::cos(theta), radius * std::sin(theta), 0.0f, 0.0f));
        curvePoints.push_back(pointOnSemicircle);
    }

    return curvePoints;
}

int main(){
    
}