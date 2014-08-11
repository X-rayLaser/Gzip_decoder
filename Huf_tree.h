/*
 * huf_tree.h
 *
 *  Created on: 01 авг. 2014 г.
 *      Author: User
 */

#ifndef HUF_TREE_H_
#define HUF_TREE_H_

////add copy constructor, overload operator=

#include <vector>


namespace tree {

const int NOVALUE = -8000;

class empty_tree{
};

class bad_tree{
};

class bad_code{

};

struct node {
	int value;
	int length;
	struct node* left;
	struct node* right;
};


struct pair {
	int value;
	int length;
	pair(){ value=0; length=0; }
	pair(int val, int len){ value = val; length = len; }
} ;


class Huf_tree{
	mutable struct node* cur_node;
	struct node* root;						//root of the huffman tree
	std::vector<pair> alphabet;				//pairs of code lengths and values
	std::vector<pair>::iterator cur_symbol;	//points to the current symbol in a vector

	static bool compareByLength(const pair& p1, const pair& p2){
		if (p1.length == p2.length)
			return p1.value < p2.value;
		else
			return p1.length < p2.length ;
	}

	void left_edge(struct node* parent );		//create a left edge
	void right_edge(struct node* parent);		//create a right edge
	void build_tree();							//create a huffman tree
	void delete_node(struct node* descendant);	//free memory for a given descendant node
public:
	Huf_tree(const std::vector<pair> &alphabt);
	Huf_tree(const Huf_tree& htr);
	int down_left()  const;			    		//walk down the left edge
	int down_right() const;			    		//walk down the right edge
	~Huf_tree();
};

}

#endif /* HUF_TREE_H_ */
