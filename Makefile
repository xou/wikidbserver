CXXFLAGS=-g -pthread -std=c++11 -lbz2 -O2 -lboost_program_options -Wall -Wextra -fPIC

wikidbserver: wikidbserver.cpp data.hpp commandline_interface.hpp read.hpp read.o parseutil.o graph_bfs.hpp
	g++ $(CXXFLAGS) -c wikidbserver.cpp -o wikidbserver.o
	g++ $(CXXFLAGS) wikidbserver.o read.o parseutil.o -o wikidbserver 
	
read.o: read.cpp read.hpp bzreader.hpp escaped_list_ignore.hpp producer_consumer_queue.hpp data.hpp
	g++ $(CXXFLAGS) -c read.cpp -o read.o

parseutil.o: parseutil.cpp parseutil.hpp
	g++ $(CXXFLAGS) -c parseutil.cpp -o parseutil.o

clean:
	rm -f wikidbserver parseutil.o read.o wikidbserver.o
