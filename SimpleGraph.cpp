#include "SimpleGraph.h"

SimpleGraph::SimpleGraph(int vertex_number)
{
	vertex_number_ = vertex_number;
	relations_ = new std::list<int>[vertex_number_];
}

void SimpleGraph::ShowRelations()
{
    for (size_t vertex_id = 0; vertex_id < vertex_number_; vertex_id++)
    {
        std::cout << "vertex: "<<vertex_id << " has ";
        for (auto& child : relations_[vertex_id])
        {
            std::cout << " " << child;
        }
        std::cout << std::endl;
    }
}

void SimpleGraph::ShowRelationsR()
{
    for (size_t vertex_id = 0; vertex_id < vertex_number_; vertex_id++)
    {
        std::cout << "vertex: " << vertex_id << " has ";
        for (auto& child : r_relations_[vertex_id])
        {
            std::cout << " " << child;
        }
        std::cout << std::endl;
    }
}

void SimpleGraph::AddRelation(int parent_id, int child_id)
{

	relations_[parent_id].push_back(child_id);
}

void SimpleGraph::RemoveRelation(int parent_id, int child_id,bool on_raw)
{
    if (on_raw)
    {
        relations_[parent_id].remove(child_id);
    }
    else
    {
        r_relations_[parent_id].remove(child_id);
    }
   
}

std::vector<int> SimpleGraph::RunBFS(int start, bool on_raw)
{
    std::vector<int> bfs_result;
    std::list<int>* adj;
    if (on_raw)
    {
        adj = relations_;
    }
    else
    {
        adj = r_relations_;
    }


    // Mark all the vertices as not visited
    bool* visited = new bool[vertex_number_];
    for (int i = 0; i < vertex_number_; i++)
        visited[i] = false;

    // Create a queue for BFS
    std::list<int> queue;

    // Mark the current node as visited and enqueue it
    visited[start] = true;
    queue.push_back(start);

    // 'i' will be used to get all adjacent
    // vertices of a vertex
    std::list<int>::iterator i;

    int is_root_vertex = 1;

    while (!queue.empty())
    {
        // Dequeue a vertex from queue and print it
        start = queue.front();
        if (is_root_vertex)
        {
            is_root_vertex = 0;
        }
        else 
        {
            bfs_result.push_back(start);
        }
       
        //std::cout << start << " ";
        queue.pop_front();

        // Get all adjacent vertices of the dequeued
        // vertex s. If a adjacent has not been visited,
        // then mark it visited and enqueue it
        for (i = adj[start].begin(); i != adj[start].end(); ++i)
        {
            if (!visited[*i])
            {
                visited[*i] = true;
                queue.push_back(*i);
            }
        }
    }
    std::cout << std::endl;

    return bfs_result;

}




void SimpleGraph::RemoveConflicts()
{
    r_relations_ = relations_;
    // Find ancestors of each vertex
	ancestors_ = new std::list<int>[vertex_number_];
    for (size_t vertex_id = 0; vertex_id < vertex_number_; vertex_id++)
    {
        ancestors_[vertex_id] = FindAncestorsOf(vertex_id);
        
        if (false)
        {
            //std::cout << "vertex " << vertex_id << " has ancestor: ";
            for (auto& anc:ancestors_[vertex_id])
            {
                std::cout << anc << " ";
            }
            std::cout << std::endl;
        }
    }

    // Find the parent vertex among those parents
    for (size_t vertex_id = 0; vertex_id < vertex_number_; vertex_id++)
    {
        auto curr_ancestor = FindAncestorsOf(vertex_id);
        int curr_parent = FindParentOf(curr_ancestor);

        
        if (curr_parent!=-1)
        {
            //std::cout << vertex_id << " has paretn " << curr_parent << std::endl;

            for (auto& a:curr_ancestor)
            {
                if (a != curr_parent) 
                {
                    RemoveRelation(a, vertex_id, false);
                }
            }
        }
       
    }

}

std::list<int> SimpleGraph::FindAncestorsOf(int vertex_id)
{
    std::list<int> ret;

    for (int v_id = 0; v_id < vertex_number_; v_id++)
    {
        for (auto child : relations_[v_id])
        {
            if (child == vertex_id)
            {
                ret.push_back(v_id);
            }
        }
    }
    return ret;
}

int SimpleGraph::FindParentOf(std::list<int> parents)
{
 
    // Check each ancestor, if it has a child in this list, discard it.
    std::list<int> candidates = parents;
    for (auto& v:parents)
    {
        std::list<int> others = parents;
        others.remove(v);

        for (auto& other:others)
        {
            if (CheckRelation(v, other))
            {
                candidates.remove(v);
            }
        }
    }
    if (candidates.size()==1)
    {
        for (auto& element:candidates)
        {
            return element;
        }
    }
    else 
    {
        return -1;
    }

}

bool SimpleGraph::CheckRelation(int parent_id, int child_id)
{

    for (auto& child:relations_[parent_id])
    {
        if (child==child_id)
        {
            return true; // relations found
        }
    }
    return false;
}
