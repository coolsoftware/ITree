// TestITree.cpp : Defines the entry point for the console application.
//

// VIT 2014 test project for ITREE.H
// http://blog.coolsoftware.ru/

#include "stdafx.h"
#include "itree.h"
#include <time.h>
#include <limits.h>
#include <iostream>

class observation {
public:
   typedef double range_type;
   typedef unsigned int id_type;

   observation(range_type rHigh, range_type rLow, unsigned short sID) 
      : m_rHigh(rHigh > rLow ? rHigh : rLow), m_rLow(rLow <= rHigh ? rLow : rHigh), m_sID(sID) {}
   range_type low() const { return m_rLow; }
   range_type high() const { return m_rHigh; }
   id_type id() const { return m_sID; }

private:
   range_type m_rHigh, m_rLow;
   id_type m_sID;
};

#define MAX_OBS       100000
//#define QUERY_VALUE   1.0

int _tmain(int argc, _TCHAR* argv[])
{
	itree<observation> Tree;    // The interval tree

	srand((unsigned)time(NULL)); // Initialize radnom generator

	std::cout << "Constructing intervals vector..." << std::endl;

	// Add some intervals to the tree
	for (observation::id_type s = 0; s < MAX_OBS; ++s)
	{
		observation::range_type rHigh, rLow;

		rLow = (double)rand() / RAND_MAX * 70.0;
		rHigh = rLow + (double)rand() / RAND_MAX * 40.0;

		observation obs(rHigh, rLow, s);
		Tree.push_back(obs);  // use a vector insertion function
	}

	std::cout << "Constructing query vector..." << std::endl;

	std::vector<observation::range_type> query_vec;

	for (unsigned int i = 0; i < 100; i++)
	{
		observation::range_type queryValue = (double)rand() / RAND_MAX * 100.0; //QUERY_VALUE;
		query_vec.push_back(queryValue);
	}

	std::cout << "Constructing itree (" << MAX_OBS <<" items) ..." << std::endl;

	clock_t cbegin = clock();

	// Put the tree in query mode
	itree<observation> const * ConstTree = Tree.construct();

	clock_t cend = clock();

	std::cout << double(cend - cbegin) / CLOCKS_PER_SEC << " sec." << std::endl;

	bool bCheckResults = false;

	std::set<observation::id_type> bfresults;
	int bfn = 0;

	// Use the const iterator to access all intervals sequentially
	std::cout << "Brute-force search..." << std::endl;

	clock_t bfbegin = clock();

	for (unsigned int i = 0; i < query_vec.size(); i++)
	{
		observation::range_type queryValue = query_vec[i];
		//std::cout << "Query value: " << queryValue << std::endl;

		for (itree<observation>::const_iterator citer = ConstTree->begin(); citer != ConstTree->end(); ++citer)
		{
			if ((citer->low() <= queryValue) && 
				(citer->high() >= queryValue))
			{
				if (bCheckResults) bfresults.insert(citer->id());
				bfn++;
				//        std::cout << "ID: " << citer->id()
				//                  << ", Low: " << citer->low() 
				//                  << ", High: " << citer->high() << std::endl;
			}
		}
	}

	clock_t bfend = clock();

	std::cout << bfn << " brute-force search results, " << double(bfend - bfbegin) / CLOCKS_PER_SEC << " sec." << std::endl;

	std::set<observation::id_type> qresults;
	int qn = 0;

	// Use the query iterator to access intervals
	std::cout << "Itree search..." << std::endl;

	clock_t qbegin = clock();

	for (unsigned int i = 0; i < query_vec.size(); i++)
	{
		observation::range_type queryValue = query_vec[i];
		//std::cout << "Query value: " << queryValue << std::endl;

		//itree<observation>::query_iterator iter;
		for (itree<observation>::query_iterator iter = ConstTree->qbegin(queryValue); iter != ConstTree->qend(queryValue); ++iter)
		{
			if (bCheckResults) qresults.insert(iter->id());
			qn++;
			//       std::cout << "ID: " << iter->id() 
			//                 << ", Low: " << iter->low() 
			//                 << ", High: " << iter->high() << std::endl;
		}
	}

	clock_t qend = clock();

	std::cout << qn << " itree search results, " << double(qend - qbegin) / CLOCKS_PER_SEC << " sec." << std::endl;

	if (bCheckResults)
	{
		std::cout << "Checking results..."  << std::endl;
		assert(bfn == qn);
		for (std::set<observation::id_type>::const_iterator it = qresults.begin(); it != qresults.end(); ++it)
		{
			assert(bfresults.find(*it) != bfresults.end());
		}
		std::cout << "Ok." << std::endl;
	}

	return 0;
}

