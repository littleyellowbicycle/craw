#include<iostream>
using namespace std;
int main(int argc,char** argv){
	const char* test ="for test";
	const char* test1;
	if (argc<2)
		test1=test;
	else
		test1=argv[1];

	cout<<test1<<endl;
	return 0;
}
