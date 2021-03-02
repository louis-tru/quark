
#include <iostream>
#include "../ftr/util/dict.h"
#include "../ftr/util/string.h"

using namespace ftr;

int test2_dict(int argc, char *argv[])
{
	Dict<String, String> dict;
	
	dict.set("", "");
	
	printf("%s \n", "ok");
	
	return 0;
}
