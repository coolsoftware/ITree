#pragma once

#ifndef ITREE_H_
#define ITREE_H_

// Interval Trees
// By Yogi Dandass, January 01, 2000
// http://www.drdobbs.com/cpp/interval-trees/184401179

// VIT 2014 added some ommited declarations and optimized constructing of interval tree
// http://blog.coolsoftware.ru/

#include <vector>
#include <set>
#include <assert.h>
#include <algorithm>
#include <iterator>

template<class T, class A = std::allocator<T> > class itree 
{
public:
	typedef T                                  invl_type;
	typedef typename T::range_type             range_type;
	typedef std::vector<invl_type, A>          invl_vec;
	typedef std::vector<range_type>            range_vec;

	typedef typename invl_vec::iterator        iterator;
	typedef typename invl_vec::const_iterator  const_iterator;

	typedef typename invl_vec::reference	   reference;
	typedef typename invl_vec::const_reference const_reference;

	typedef typename invl_vec::size_type	   size_type;

	// other types derived from invl_vec omitted
	// ...

private:
	typedef struct node_tag       // Interval tree nodes
	{
		range_type      discrim;  // discriminant
		int             start;    // starting offset in AL and DH
		int             size;     // number of entris in AL and DH
		struct node_tag *left;    // left subtree
		struct node_tag *right;   // right subtree
	} node;

public:
	class query_iterator : public std::iterator<std::forward_iterator_tag, itree> 
	{
	public:
		// ... // constructors and operator*() omitted 

		query_iterator& operator++(void)  // prefix 
		{
			Increment();
			return *this;
		}  

		query_iterator operator++(int)  // postfix
		{ 
			query_iterator it(*this); 
			Increment(); 
			return it; 
		}

		const T* operator->() const 
		{ 
			if (cur_node->discrim >= value)
				return (*p_al)[cur_node->start + index]; 
			else
				return (*p_dh)[cur_node->start + index]; 
		}

		bool operator!=(query_iterator& it) const 
		{
			return !(*this == it);
		}

		bool operator==(query_iterator& it) const 
		{ 
			return (value == it.value) && 
				(cur_node == it.cur_node) && (index == it.index);
		}

	private:
		node*							cur_node;
		int					            index;
		const std::vector<invl_type*>	*p_al, *p_dh;
		range_type						value;

		query_iterator(node* cur_node, int index, range_type value,
			const std::vector<invl_type*> *p_al, 
			const std::vector<invl_type*> *p_dh) 
			: cur_node(cur_node), index(index), value(value), 
			p_al(p_al), p_dh(p_dh) {} // private constructor

		void Increment(void) 
		{
			index++;
			if (index == cur_node->size) 
			{
				if (cur_node->discrim == value) 
				{ // finished!
					cur_node = NULL;
					index = 0;
					return;
				}
				else if (cur_node->discrim > value)
					cur_node = cur_node->left;
				else
					cur_node = cur_node->right;
				init_node();
				return;
			}
			if (cur_node->discrim > value) 
			{
				if ((*p_al)[cur_node->start + index]->low() <= value)
					return;
				else
					cur_node = cur_node->left;
			}
			else if (cur_node->discrim < value) 
			{
				if((*p_dh)[cur_node->start + index]->high() >= value)
					return;
				else
					cur_node = cur_node->right;
			}
			else  //(cur_node->discrim == value)
				return;
			init_node();
			return;
		}

		void init_node(void) 
		{
			index = 0;        
			while (cur_node != NULL) 
			{
				if (value < cur_node->discrim) 
				{
					if ((cur_node->size != 0) &&
						((*p_al)[cur_node->start]->low() <= value))
						return;
					else
						cur_node = cur_node->left;
				}
				else if (value > cur_node->discrim) 
				{
					if ((cur_node->size != 0) && 
						((*p_dh)[cur_node->start]->high() >= value))
						return;
					else
						cur_node = cur_node->right;
				}
				else //(value == cur_node->discrim)
				{
					if (cur_node->size == 0) //VIT 2014
					{
						cur_node = NULL;
					}
					return;
				}
			}
		}

		friend itree;  // allow itree to use private constructor
	};    // end of query_iterator definition

	// ... // functions forwarded to std::vector omitted

	const_iterator begin() const { return invls.begin(); }

	const_iterator end() const { return invls.end(); }

	void push_back(const T& _Val) { invls.push_back(_Val); }

	const_reference operator[](size_type _Pos) const { return invls[_Pos]; }

	// Interval tree specific functionality follows

	itree() : root(NULL), count(0) {}
	~itree() { deconstruct(); }

	void deconstruct(void) // reverts initialization mode
	{ delete_tree(root); root = NULL; count = 0; }

	void delete_tree(node *cur_node)  // delete nodes in tree
	{
		if (cur_node != NULL) 
		{
			delete_tree(cur_node->left);    // left tree 
											//  recursively
			delete_tree(cur_node->right);   // right tree 
											//  recursively
			delete cur_node;
		}
	}

	bool constructed() const { return root != NULL; }

	// build the interval tree structure and
	// put the container into query mode
	itree const * construct(void) 
	{
		range_vec         values;
		int               num_al_dh = 0;

		extract_values(values);   

		al.resize(invls.size());
		dh.resize(invls.size());

		std::vector<invl_type*> temp_invls(invls.size(), 0); // temp vector of intervals	
		for (unsigned int i = 0; i < invls.size(); i++)
		{ temp_invls[i] = &invls[i]; }

		root = construct_tree(temp_invls, 0, temp_invls.size(), values, num_al_dh, 0, values.size() - 1);

		return this;
	}

	void extract_values(range_vec& values)
	{
	std::set<range_type> s;
	for (invl_vec::const_iterator it = invls.begin(); it != invls.end(); it++)
	{
		s.insert(it->low());
		s.insert(it->high());
	}
	values.assign(s.begin(), s.end());
	}

	// recursively construct the tree
	node* construct_tree(std::vector<invl_type*>& invls, int invls_start, int invls_size, const range_vec& values, int& num_al_dh, int start, int end) 
	{
		int         discrim_pos;
		range_type  discrim;
		int         list_start, list_size;
		node        *root, *left, *right;

		root = left = right = NULL;
      
		if (start > end) return root;

		discrim_pos = (start + end) / 2;
		discrim = values[discrim_pos];
 
		list_start = num_al_dh;
		list_size = 0;

		int invls_right = invls_size;

		for (int i = invls_start; i < invls_right; ) 
		{
			assert(invls[i]->low() <= invls[i]->high());
			if ((invls[i]->low() <= discrim) && 
				(invls[i]->high() >= discrim)) 
			{
				al[num_al_dh] = invls[i];
				dh[num_al_dh] = invls[i];

				num_al_dh++;
				list_size++;

				if (i < --invls_right)
				{
					invl_type* p = invls[i];
					invls[i] = invls[invls_right];
					invls[invls_right] = p;
				}
				if (invls_right < --invls_size)
				{
					invl_type* p = invls[invls_right];
					invls[invls_right] = invls[invls_size];
					invls[invls_size] = p;
				}
			}
			else
			{
				if (//(invls[i]->high() > discrim) &&
					(invls[i]->low() > discrim))
				{
					if (i < --invls_right)
					{
						invl_type* p = invls[i];
						invls[i] = invls[invls_right];
						invls[invls_right] = p;
					}
				}
				else
				//if (//(invls[i]->low() < discrim) && 
				//	(invls[i]->high() < discrim))
				{
					i++;
				}
			}
		}

		// see if left and/or right subtree needs to be built
		if (invls_right > invls_start && (start <= (discrim_pos - 1)))
			left = construct_tree(invls, invls_start, invls_right, values, num_al_dh, start, discrim_pos - 1);

		if (invls_right < invls_size && ((discrim_pos + 1) <= end))
			right = construct_tree(invls, invls_right, invls_size, values, num_al_dh, discrim_pos + 1, end);

		// this node is needed only if thre are entries in the 
		// AL/DH list or the left or right subtree exists.
		if ((list_size > 0) || (left != NULL) || (right != NULL)) 
		{
			if (list_size > 1)
			{
				std::sort(al.begin()+list_start,
					al.begin()+list_start+list_size,
					comp_for_al);
				std::sort(dh.begin()+list_start,
					dh.begin()+list_start+list_size,
					comp_for_dh);
			}

			root = new node;
			root->left = left;
			root->right = right;
			root->discrim = discrim;
			root->start = list_start;
			root->size = list_size;

			count++;
		}

		return root;
	}

	static bool comp_for_al(invl_type* p1, invl_type* p2)
	{
		return p1->low() < p2->low();
	}

	static bool comp_for_dh(invl_type* p1, invl_type* p2)
	{
		return p1->high() > p2->high();
	}

	query_iterator qbegin(range_type x) const
	{ 
		assert(constructed());
		query_iterator it(root, 0, x, &al, &dh); 
		it.init_node(); return it; 
	}

	query_iterator qend(range_type x) const
	{ 
		assert(constructed());
		query_iterator it(NULL, 0, x, NULL, NULL); 
		return it; 
	}

private:
	invl_vec                invls;  // vector of intervals
	std::vector<invl_type*> al, dh; // vectors of interval ptrs
	node                    *root;  // Interval tree root node
	int                     count;  // count of nodes
};

#endif //ITREE_H_