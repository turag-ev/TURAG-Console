#include <stdio.h>
#include <stdarg.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <math.h>

#include "base64.h"

#define TURAG_DEBUG_GRAPH_PREFIX    		"D"
#define TURAG_DEBUG_GRAPH_CREATE    		"n"
#define TURAG_DEBUG_GRAPH_CHANNEL   		"b"
#define TURAG_DEBUG_GRAPH_CHANNEL_FIXED     "B"
#define TURAG_DEBUG_GRAPH_DATA      		"d"
#define TURAG_DEBUG_LINE_PREFIX "\x02"
#define TURAG_DEBUG_NEWLINE "\n"
#define TURAG_DEBUG_GRAPH_VERTICAL_MARKER  	"m"
#define TURAG_DEBUG_GRAPH_CHANNELGROUP  	"g"

# define turag_graph_add_channel(index, title) \
    printf(TURAG_DEBUG_LINE_PREFIX TURAG_DEBUG_GRAPH_CHANNEL TURAG_DEBUG_GRAPH_PREFIX "%d 0 %s" TURAG_DEBUG_NEWLINE, index, title)


static unsigned turag_graph_index = 0;

unsigned turag_graph_create(const char* name) {
    printf(TURAG_DEBUG_LINE_PREFIX TURAG_DEBUG_GRAPH_CREATE TURAG_DEBUG_GRAPH_PREFIX "%u %s" TURAG_DEBUG_NEWLINE, turag_graph_index, name);
    return turag_graph_index++;
}

void turag_graph_data(unsigned index, float time, float* args, int count) {
    printf(TURAG_DEBUG_LINE_PREFIX TURAG_DEBUG_GRAPH_DATA TURAG_DEBUG_GRAPH_PREFIX "%u ", index);

    uint8_t encoded[7] = {0};

    turag_base64_encode((uint8_t*)&time, 4, encoded);
    printf("%s", (char*)encoded);


    int i;
    for ( i = 0; i < count; ++i ) {
        turag_base64_encode((uint8_t*)args, 4, encoded);
        printf("%s", (char*)encoded);
        ++args;
    }

    printf("%s", TURAG_DEBUG_NEWLINE);
}

void turag_graph_add_vertical_marker(unsigned index, const char* label, float time) {
    printf(TURAG_DEBUG_LINE_PREFIX TURAG_DEBUG_GRAPH_VERTICAL_MARKER TURAG_DEBUG_GRAPH_PREFIX "%u ", index);

    uint8_t encoded[7] = {0};

    turag_base64_encode((uint8_t*)&time, 4, encoded);
    printf("%s", (char*)encoded);

	printf(" %s" TURAG_DEBUG_NEWLINE, label);	
}

void turag_graph_add_channel_group(unsigned index, const char* label, unsigned count, ...) {
	printf(TURAG_DEBUG_LINE_PREFIX TURAG_DEBUG_GRAPH_CHANNELGROUP TURAG_DEBUG_GRAPH_PREFIX "%u %u", index, count);
	
	va_list indices;
	va_start ( indices, count );
	
	unsigned i;
	for (i = 0; i < count; ++i) {
		printf(" %d", va_arg ( indices, int ));
	}
	
	va_end ( indices );

	printf(" %s" TURAG_DEBUG_NEWLINE, label);	
}



int main(int argc, char **argv) {
	int i;
	
	unsigned index = turag_graph_create("Diagramm");
	
	for (i = 0; i < 30; ++i) {
		turag_graph_add_channel(index, "Channel");
	}


	float angle = 0.0f;
	int time = 0;
	
	while(time < 30000) {
		angle += 3.14f / 50;
		++time;
		
		if (time % 1000 == 0) {
			turag_graph_add_vertical_marker(index, "joo", time);
		}
			
		float data[30];		
		for (i = 0; i < 30; ++i) {
			data[i] = sin(angle) * (i + 1);
		}
		turag_graph_data(index, time, data, 30);
	}
	

	
	return 0;
}
