#pragma once

#include <eosio/datastream.hpp>
#include <list> 
#include <chrono>
#include <cstring>

using namespace eosio; 
using namespace std; 

class Sum
{
  name Owner;
  int Value;
  
  public:  
    Sum(name o, int v)
    {
      Owner = o;
      Value = v;
    };
    
    name getOwner() {
      return Owner;
    }
    int getValue() {
      return Value;
    }
    int addValue(int val) {
      Value = Value + val;
      return Value;
    }
};

class SumTracker
{
  public:
    list<Sum> sums;

    list<Sum> getSums() {
      return sums;
    }

    void addSum(Sum val) {
      bool found = false;
      for(auto sum=sums.begin(); sum!=sums.end(); sum++){
        if (sum->getOwner() == val.getOwner()) {
          sum->addValue(val.getValue());
          found = true;
        }
      }
      if (!found) {
        sums.push_back(val);
      }
    }
};

class Edge
{
  public:
    int Stuff;
    int Id;

    void setStuff(int stuff) {
      Stuff = stuff;
    }
    void setId(int id) {
      Id = id;
    }
    int getStuff() {
      return Stuff;
    }
    int getId() {
      return Id;
    }
};

// A directed graph using adjacency list representation 
class Graph 
{ 
    int V; // No. of vertices in graph 
    list<Edge> *adj; // Pointer to an array containing adjacency lists 
    vector<vector<int>> paths; // List of paths to return

    // A recursive function used by getTradePath() 
    void getTradePathUtil(int , int , bool [], int [], int &, int); 
  
  public: 
    Graph(int V); // Constructor 
    void addEdge(int u, int v, int id); 
    vector<vector<int>> getTradePath(int supply, int demand, int id); 
}; 

Graph::Graph(int V) 
{ 
    this->V = V; 
    adj = new list<Edge>[V]; 
    // print("Created graph: ", V, "\n");
} 
  
void Graph::addEdge(int u, int v, int id) 
{ 
    Edge e;
    e.setStuff(v);
    e.setId(id);
    adj[u].push_back(e); // Add v to u’s list. 
    // print("Added edge [", u, "] mapped to {stuff: ", e.getStuff(), ", id: ", e.getId(), "}\n");
} 
  
// Prints all paths from 's' to 'd' 
vector<vector<int>> Graph::getTradePath(int supply, int demand, int id) 
{ 
    // Mark all the vertices as not visited 
    bool *visited = new bool[V]; 
  
    // Create an array to store paths 
    int *path = new int[V]; 
    int index = 0; // Initialize path[] as empty 
  
    // Initialize all vertices as not visited 
    for (int i = 0; i < V; i++) 
        visited[i] = false; 
  
    // Call the recursive helper function to print all paths 
    getTradePathUtil(supply, demand, visited, path, index, id); 

    // Return the paths
    return paths;
} 
  
// A recursive function to print all paths from 'u' to 'd'. 
// visited[] keeps track of vertices in current path. 
// path[] stores IDs of our desired offers
// index is current index in path[] 
void Graph::getTradePathUtil(int supply, int demand, bool visited[], 
                            int path[], int &index, int id) 
{ 
    // Mark the current node and store it in path[]
    visited[supply] = true; 
    path[index] = id; 
    index++; 
    
    
    // If current vertex is same as destination, then print 
    // current path[] 
    if (supply == demand) 
    { 
      // print("Trade found: ");

      vector<int> value;

      for (int i = 0; i<index; i++) {
        int id = path[i];
        value.push_back(id);
        // print(id);
        
        // if (index - i != 1) {
        //   print("->");
        // }
      }

      paths.push_back(value);
      // print("\n");
    } 
    else // If current vertex is not destination 
    { 
        
        // Recur for all the vertices adjacent to current vertex 
        list<Edge>::iterator i; 
        for (i = adj[supply].begin(); i != adj[supply].end(); ++i) 
            if (!visited[i->Stuff]) 
                getTradePathUtil(i->Stuff, demand, visited, path, index, i->Id); 
    } 
  
    // Remove current vertex from path[] and mark it as unvisited 
    index--; 
    visited[supply] = false; 
} 