#include "Huf_tree.h"
#include <algorithm>
#include <iostream>

namespace tree {

Huf_tree::Huf_tree(const Huf_tree& htr)
{
	alphabet = htr.alphabet;
	build_tree();
}

/* alphabt must have at least one symbol(value) of non-zero length
 */
Huf_tree::Huf_tree(const std::vector<pair> &alphabt) :
		alphabet( alphabt.size() )
{

	std::sort(alphabet.begin(), alphabet.end(), compareByLength);

	////erase nulls at the beginning of a vector
	std::vector<pair>::iterator p = alphabet.begin();
	while (p!=alphabet.end() && p->length!=0)
		p=alphabet.erase(p);

	if (alphabet.size()==0)
		throw empty_tree;

	build_tree();
}

void Huf_tree::build_tree()
{
	//// create a root
	root=new struct node;
	root->value  = NOVALUE;
	root->length = 0;
	root->left   = NULL;
	root->right  = NULL;

	//// building a huffman tree
	cur_symbol=alphabet.begin();
	left_edge(root);
	right_edge(root);

	if (cur_symbol != alphabet.end())
		throw bad_tree;

	cur_node=root;
}


void Huf_tree::left_edge(struct node* parent )
{
	////check if there are still elements to add
	if ( cur_symbol == alphabet.end() ){
		parent->left = NULL ;
		return ;
	}

	struct node* new_node=new struct node;

	new_node->length = parent->length + 1;
	parent->left = new_node;
	new_node->left  = NULL;
	new_node->right = NULL;

	////if this node is a leaf
	if (new_node->length == cur_symbol->length){
		new_node->value = cur_symbol->value;
		cur_symbol++;
		return ;
	}
	else{
		new_node->value = NOVALUE;
		left_edge(new_node);
		right_edge(new_node);
	}


}

void Huf_tree::right_edge(struct node* parent )
{
	////check if there are still elements to add
	if ( cur_symbol == alphabet.end() ){
		parent->left = NULL ;
		return ;
	}

	struct node* new_node=new struct node;

	new_node->length = parent->length + 1;
	parent->right = new_node;
	new_node->left  = NULL;
	new_node->right = NULL;

	////if this node is a leaf
	if (new_node->length == cur_symbol->length){
		new_node->value = cur_symbol->value;
		cur_symbol++;
		return ;
	}
	else{
		new_node->value = NOVALUE;
		left_edge(new_node);
		right_edge(new_node);
	}
}

/* walk down the left edge
 * return NOTVALUE if this is not a leaf
 * else return leaf's value
 */
int  Huf_tree::down_left() const
{
	if (cur_node == NULL)
		throw bad_code;

	cur_node = cur_node->left;

	int val = cur_node->value;

	if (val != NOVALUE)
		cur_node = root;

	return val ;
}

/* walk down the right edge
 * return NOTVALUE if this is not a leaf
 * else return leaf's value
 */
int  Huf_tree::down_right() const
{
	if (cur_node == NULL)
		throw bad_code;

	cur_node = cur_node->right;

	int val = cur_node->value;

	if (val != NOVALUE)
		cur_node = root;

	return val ;
}


Huf_tree::~Huf_tree()
{

	delete_left(root->left);
	delete_right(root->right);
	delete root;
}


void Huf_tree::delete_left(struct node* ancestor)
{
	if (ancestor->value == NOVALUE){
		delete ancestor;
		return ;
	}

	if (ancestor->left != NULL)
		delete_left(ancestor->left);

	if (ancestor->right != NULL)
		delete_right(ancestor->right);

}

void Huf_tree::delete_right(struct node* ancestor)
{
	if (ancestor->value != NOVALUE){
		delete ancestor;
		return ;
	}

	if (ancestor->left != NULL)
		delete_left(ancestor->left);

	if (ancestor->right != NULL)
		delete_right(ancestor->right);
}


}
