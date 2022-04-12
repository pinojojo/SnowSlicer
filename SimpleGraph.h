#pragma once
#include<iostream>
#include <list>
#include <vector>



class SimpleGraph
{
	int vertex_number_;
	std::list<int>* relations_;
	std::list<int>* r_relations_;
	std::list<int>* ancestors_;
	std::vector<int> parent_;
public:
	SimpleGraph(int vertex_number);

	void ShowRelations();
	void ShowRelationsR();

	void AddRelation(int parent_id, int child_id);
	void RemoveRelation(int parent_id, int child_id,bool on_raw);
	
	std::vector<int> RunBFS(int start,bool on_raw);
	void RemoveConflicts();
private:


	std::list<int> FindAncestorsOf(int vertex_id);
	int FindParentOf(std::list<int> parents);

	bool CheckRelation(int parent_id, int child_id);
};

