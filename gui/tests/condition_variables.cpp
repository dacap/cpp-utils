#include <gui/thread.h>
#include <queue>
#include <iostream>
#include <iomanip>
#include <exception>

using namespace gui;
using namespace std;

class synchronized_queue
{
  static const size_t MAX = 4;

  queue<int> items;
  mutex monitor;
  condition_variable full;
  condition_variable empty;

public:

  synchronized_queue()
  {
  }

  void add(int producer_nth, int item)
  {
    lock_guard<mutex> lock(monitor);

    while (items.size() == MAX) {
      cout << ">  [producer " << producer_nth << "] is full" << endl;
      full.wait(lock);
      cout << ">  [producer " << producer_nth << "] now i can produce" << endl;
    }

    assert(items.size() < MAX);
    items.push(item);
    cout << setw(2) << items.size()
	 << " [producer " << producer_nth << "] item " << setw(3) << item << " produced" << endl;

    assert(items.size() <= MAX);

    if (items.size() == 1)
      empty.notify_one();
  }

  int remove(int consumer_nth)
  {
    lock_guard<mutex> lock(monitor);

    while (items.size() == 0) {
      cout << ">  [consumer " << consumer_nth << "] is empty" << endl;
      empty.wait(lock);
      cout << ">  [consumer " << consumer_nth << "] now i can consume" << endl;
    }

    assert(items.size() <= MAX);

    int res = items.front();
    items.pop();

    cout << setw(2) << items.size()
	 << " [consumer " << consumer_nth << "] item " << setw(3) << res << " consumed" << endl;

    if (items.size() == MAX-1)
      full.notify_one();

    return res;
  }

};

synchronized_queue the_queue;
bool stop_flag = false;

void producer(int producer_nth)
{
  while (!stop_flag) {
    // Produce the "item"
    int item = rand() & 255;
    this_thread::sleep_for(item);

    // Add it to the queue
    the_queue.add(producer_nth, item);
  }
}

void consumer(int consumer_nth)
{
  while (!stop_flag) {
    // Consume the "item" from the queue
    int item = the_queue.remove(consumer_nth);

    // Here we can use the item...
    this_thread::sleep_for(rand() & 255);
  }
}

int main()
{
  srand(NULL);

  // Create 3 producers and 3 consumers
  thread p1(&producer, 1);
  thread p2(&producer, 2);
  thread p3(&producer, 3);
  thread c1(&consumer, 1);
  thread c2(&consumer, 2);
  thread c3(&consumer, 3);

  // Wait 1 second
  this_thread::sleep_for(1000);
  stop_flag = true;

  cout << "stop all threads" << endl;

  // Join everything
  p1.join();
  p2.join();
  p3.join();
  c1.join();
  c2.join();
  c3.join();

  // You should redirect the output of this program to see the result...
  // Something like "condition_variables.exe > log.txt"
  return 0;
}
