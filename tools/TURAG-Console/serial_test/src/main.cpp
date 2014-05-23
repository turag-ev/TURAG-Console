#include <iostream>
#include <cstdio>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <cmath>

using namespace std;


int main(int argc, char **argv) {
	if (argc < 2) return -1;
	
	int handler = open(argv[1], O_WRONLY);
	cout << "[Trans] Open: " << handler << endl;
	
	char buf[100];


	float angle = 0.0f;
	int time = 0;
	
	while(1) {
		angle += 3.14f / 50;
		++time;
		
		if (time % 1000 == 0) angle = 0;
		
		int length = sprintf(buf, "%d %d %d\n", time, (int)(sin(angle) * angle * 1000), (int)(cos(angle) * angle * 1000));
		
		int result = write(handler, buf, length);
		
		if (result > 0) {
		} else {
			int errsv = errno;
			cout << "[Trans] error: " << strerror(errsv) << endl;
		}
		
		usleep(50000);
	}
	

	
	return 0;
}
