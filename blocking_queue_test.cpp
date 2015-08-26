#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include "producer_consumer_queue.hpp"

using namespace std;

ProducerConsumerQueue<string> q(10);

void consumerThread(int id) {
  while (true) {
    this_thread::sleep_for( chrono::seconds(1) );
    string out;
    cout << "consumerThread(" << id << ") waking up" << endl;
    if (!q.pop(out)) {
      cout << "consumerThread(" << id << ") exiting" << endl;
      return;
    }

    cout << "consumerThread(" << id << ") consumed " << out << endl;
  }
};

int main() {
  
  thread t1(consumerThread, 1);
  thread t2(consumerThread, 2);

  for (size_t i = 0; i < 15; ++i) {
    cout << "pushing Item " << i << endl;
    q.push("Item " + to_string(i));
    cout << " - " << i << " pushed " << endl;
  }

  q.terminate_consumers();

  cout << "Joining 1" << endl;
  t1.join();
  cout << "Joining 2" << endl;
  t2.join();

}
