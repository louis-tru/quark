
#include <iostream>
#include <list>
using namespace std;

int test2_list(int argc, char *argv[])
{ 
	// initializing lists 
	list<int> l1 = { 1, 2, 3 }; 
	list<int> l2 = { 4, 5 }; 
	list<int> l3 = { 6, 7, 8 }; 

	// transfer all the elements of l2 
	l1.splice(l1.begin(), l2); 

	// at the beginning of l1 
	cout << "list l1 after splice operation" << endl; 
	for (auto x : l1) 
			cout << x << " "; 

	// transfer all the elements of l1 
	l3.splice(l3.end(), l1); 

	
	auto it = l3.insert(l3.end(), 100);
	
	auto it2 = l3.insert(it, 101);
	
	typedef list<int>::iterator ID;
	
	auto id = ID();
	
	cout << "\n id " << *it2 << endl;
	
	if (id == l3.end()) {
		cout << "\n end \n" << endl;
	}
	
	if (id == ID()) {
		cout << "\n end2 \n" << endl;
	}
	

	// at the end of l3
	cout << "\nlist l3 after splice operation" << endl;
	for (auto x : l3)
			cout << x << " ";
	
	cout << endl;

	return 0;
}
