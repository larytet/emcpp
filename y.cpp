#include <vector>
#include <string>
#include <sstream>
#include <algorithm> 
#include <tuple> 

typedef std::vector<int> VertexIDs;
std::vector<int> readInput() 
{
  std::string inputStr;
  VertexIDs vertexIDs;
  std::getline(std::cin, inputStr);
  std::stringstream ss(inputStr);

  int i;
  while (ss >> i)
  {
    vertexIDs.push_back(i);

    if (ss.peek() == ',')
    ss.ignore();
  }
  return vertexIDs;
}


int main() 
{
    auto vertexIDs = readInput();
    std::unordered_map<std::string, std::string> edges;
    // populate the map of edges
    for(std::vector<int>::size_type i = 0; i != vertexIDs.size(); i+= 3) {
        v0 = vertexIDs[i];
        v1 = vertexIDs[i+1];
        v2 = vertexIDs[i+2];
        int triangle = {v0, v1, v2}; 
        sort(triangle, triangle + 3);
        edge0 = std::make_tuple(triangle[0], triangle[1]);
        edge1 = std::make_tuple(triangle[1], triangle[2]);
        edge2 = std::make_tuple(triangle[2], triangle[0]);

        edges.insert(std::make_pair(edge0, i+0));
        edges.insert(std::make_pair(edge1, i+1));
        edges.insert(std::make_pair(edge2, i+2));
    }

    // output the edges
    for(std::vector<int>::size_type i = 0; i != vertexIDs.size(); i+= 3) {
        auto foundEdges = edges.find(edge0);
        if foundEdges == edges.end() {
            printf("-1,-1,");
            continue;
        } 
        // check if any of the found edges is not this edge 
        if (foundEdges->second > i+2) {
            printf("%d,", foundEdges->second)
        }
    }


    return 0;
}
