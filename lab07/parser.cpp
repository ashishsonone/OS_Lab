#include <iostream>
#include <string>
using namespace std;

struct command{
	int operation;
	int reciever;
	char message[100];
};


command * parser(FILE * readline){
	if (readline == NULL) perror ("Error opening file");

	string tempstr;
	getline(readline, tempstr);

	int sizeofstr = string,size();
	int count = 0, place = 0;
	string tmp1, tmp2, tmp3;

	for (int i = 0 ; i < sizeofstr ; i ++){
		if (tempstr.at(i) == '\n' || count >=3 ) break;
		else if (tempstr.at(i) == ' '){
			if (count == 0) {
				tmp1 = tempstr.substr(0, i);
				place = i;
			}
			else if (count == 1) {
				tmp2 = tempstr.substr(place+1, i- place);
				place = i;
			}
			else {
				tmp3 = tempstr.substr(place+1, sizeofstr-i);
				break;
			}
			count++;	
		}

		
	}

}