
#include <iostream>

using namespace std;

double Div(int a, int b) throw(const char*) {
	if (b == 0) throw "Zero Div";//抛出一个字符串常量
	cout << "要是异常抛出, 自我及其一下全部腰斩, 不会执行" << endl;
	return (double)a / (double)b;
}

int test2_try(int argc, char *argv[]) noexcept {
	try {
		cout << Div(4, 0) << endl;
	}
	catch (int errid) {
		//捕获错误码整形进行处理
		cout << "错误编号: " << errid << endl;
	}
	catch (const char* msg) {
		cout << "错误信息" << msg << endl;
	}
	cout << "异常处理结束了, 继续向后执行呀, 除非异常处理进行了中断" << endl;
	return 0;
}