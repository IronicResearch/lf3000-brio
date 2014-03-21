
#include <VNProfiler.h>
#include <map>
#include <stack>
#include <string>
#include <iostream>
#include <time.h>
#include <iomanip>


static const int spacing = 20;

struct ProfileEntry {
	clock_t startTime;
	float totalTime;
	int iterations;
	
	ProfileEntry() :
		startTime(0),
		totalTime(0),
		iterations(0)
	{}
	
	void start() {
		startTime = clock();
	}
	
	void end() {
		float time = (float)((clock() - startTime)/float(CLOCKS_PER_SEC));
		totalTime += time;
		iterations++;
	}
	

	void print( const std::string& name ) {

		std::cout	<< std::setw(spacing) << name
				<< std::setw(spacing) << iterations
				<< std::setw(spacing) << totalTime
				<< std::setw(spacing) <<  (1000.0f * (totalTime / float(iterations))) 
				<< std::endl;
	
	}
};

static std::map<std::string, ProfileEntry> entries;
static std::stack<ProfileEntry*> stack;

 
void _profile_block_start(const char* func_name) {
	std::map<std::string, ProfileEntry>::iterator it = entries.find(std::string(func_name));
	if( it == entries.end() ) {
		ProfileEntry entry;
		entries[std::string(func_name)] = entry;
		it = entries.find(std::string(func_name));
	}
	
	it->second.start();

	stack.push( &it->second );
}

void _profile_block_end() {
	ProfileEntry* entry = stack.top();
	entry->end();
	stack.pop();
}

void _profile_print( void ) {
	
	std::cout 	<< std::setw(spacing) << "name"
			<< std::setw(spacing) << "iterations"	
			<< std::setw(spacing) << "total (s)"
			<< std::setw(spacing) << "avg (ms)"
			<< std::endl;

	std::map<std::string, ProfileEntry>::iterator it = entries.begin();
	for(; it != entries.end(); it++ ) {
		it->second.print(it->first);
	}
}


void _profile_reset() {
	entries.clear();
}
void foobar() {
	std::cout << "\afoobar" << std::endl;
	//printf("\afoobar\n\n\n");
}

