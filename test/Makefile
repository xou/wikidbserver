CXXFLAGS=-std=c++11 -lgtest -g

all: test_wikidata

test: all
	./test_wikidata

clean:
	rm -f test_wikidata

producer_consumer_queue_test: producer_consumer_queue_test.cpp ../producer_consumer_queue.hpp
	$(CXX) $(CXXFLAGS) producer_consumer_queue_test.cpp -pthread -o producer_consumer_queue_test
