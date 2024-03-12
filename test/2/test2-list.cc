
#include <iostream>
#include <list>
#include <vector>
#include <string>
#include <quark/util/array.h>
#include <quark/util/string.h>
#include <map>
#include "quark/util/list.h"

using namespace std;
using namespace qk;

class B {
	public:
	int b() {
		printf("%s \n", "test B");
		return 100;
	}
};

class Test {
	public:
	Test() {
		_b = new B;
	}
	
	B* b() const {
		return _b;
	}

	private:
	B* _b;
};


template<typename T> struct Compare {
	static uint64_t hashCode(const T& key) {
		return key.hashCode();
	}
};

template<typename T> struct Compare<T*> {
	typedef T* Type;
	static uint64_t hashCode(const Type& key) {
		return (uint64_t)key;
	}
};

template<class T, typename C = Compare<T>>
class Map {
	public:
	Map(T t): _t(t) {}
	uint64_t hashCode() {
		return C::hashCode(_t);
	}
	private:
	T _t;
};

int test2_vector(int argc, char *argv[]) {

	Test t;
	
	Map<Test*> map(&t);
	Map<String> map2("AA");
	
	uint64_t h = map.hashCode();
	uint64_t h2 = map2.hashCode();
	
	printf("%d, %llu， %llu \n", t.b()->b(), h, h2);

	vector<char> vec = { 'a', 'b' };;
	
	Array<char> a = { 'a', 'b' };
	Array<char> b( vec);
	
	auto i = a.begin();
	auto j = a.end();
	
	printf("%s, %c, %c \n", "ok", *i, *j);
	
	for (auto o: a) {
		printf("foreact, %c \n", o);
	}
	
	for (auto i = a.begin(); i != a.end(); i++) {
		auto j = *i;
		printf("foreact, %c \n", j);
	}
	
	std::map<string, string> m;
	
	auto begin = m.begin();

	return 0;
}

int test2_list2(int argc, char *argv[]) {

	// initializing lists
	List<std::string> l1 = { "1", "2", "3" };
	List<std::string> l2{ "4", "5" };
	List<std::string> l3{ "6", "7", "8", "20" };
	
	printf("%s \n", l1.front().c_str());
	
	auto id0 = l3.end();

	// transfer all the elements of l2
	l1.splice(l1.begin(), l2);

	// at the beginning of l1
	cout << "list l1 after splice operation" << endl;
	for (auto x : l1)
			cout << x << " ";

	// transfer all the elements of l1
	l3.splice(l3.end(), l1);
	
	// l1.erase(const_iterator __f, const_iterator __l)
	
	auto it = l3.insert(l3.end(), "100");
	
	auto it2 = l3.insert(it, "101");
	
	typedef List<std::string>::Iterator ID;
	
	auto id = ID();
	auto id2 = l3.end();
	l3.pushBack("A");
	l3.pushFront("B");
	auto id3 = l3.end();
	
	auto id4 = l3.begin();
	
	if (id0 == id2) {
		cout << "\n end id4 \n" << endl;
	}
	
	id2++;
	
	if (id2 == id3) {
		cout << "\n end id0 \n" << endl;
	}
	
	cout << "\n id3 " << *id2 << "," << *id4 << endl;
	
	//cout << *id0 << endl
	//cout << *id3 << endl
	
	cout << "\n id " << *it2 << endl;
	
	if (id == id2) {
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

int test2_list1(int argc, char *argv[]) {
	
	// initializing lists
	list<std::string> l1{ "1", "2", "3" };
	list<std::string> l2{ "4", "5" };
	list<std::string> l3{ "6", "7", "8", "20" };
	
	printf("%s \n", l1.front().c_str());
	
	auto id0 = l3.end();

	// transfer all the elements of l2
	l1.splice(l1.begin(), l2);

	// at the beginning of l1
	cout << "list l1 after splice operation" << endl;
	for (auto x : l1)
			cout << x << " ";

	// transfer all the elements of l1
	l3.splice(l3.end(), l1);
	
	// l1.erase(const_iterator __f, const_iterator __l)

	auto it = l3.insert(l3.end(), "100");
	
	auto it2 = l3.insert(it, "101");
	
	typedef list<std::string>::iterator ID;
	
	auto id = ID();
	auto id2 = l3.end();
	l3.push_back("A");
	l3.push_front("B");
	auto id3 = l3.end();
	
	auto id4 = l3.begin();
	
	if (id0 == id2) {
		cout << "\n end id4 \n" << endl;
	}
	
	id2++;
	
	if (id2 == id3) {
		cout << "\n end id0 \n" << endl;
	}
	
	cout << "\n id3 " << *id2 << "," << *id4 << endl;
	
	//cout << *id0 << endl
	//cout << *id3 << endl
	
	cout << "\n id " << *it2 << endl;
	
	if (id == id2) {
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

int test2_list(int argc, char *argv[])
{
	test2_vector(argc, argv);
	
	printf("\n\n---------------------------------------\n\n");
	
	test2_list2(argc, argv);

	printf("\n\n---------------------------------------\n\n");

	test2_list1(argc, argv);

	return 0;
}
