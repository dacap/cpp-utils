#include "ui"
#include <queue>
#include <iostream>
#include <iomanip>
#include <exception>

using namespace ui;
using namespace std;

class stopped_exception : exception { };

class synchronized_queue
{
  static const size_t MAX = 4;

  queue<int> items;
  mutex monitor;
  condition_variable full;
  condition_variable empty;
  bool stop_flag;

public:

  synchronized_queue()
  {
    stop_flag = false;
  }

  void add(int producer_nth, int item)
  {
    lock_guard<mutex> lock(monitor);

    while (items.size() == MAX) {
      cout << "> [producer " << producer_nth << "] is full" << endl;
      full.wait(lock);
      if (stop_flag)
	throw stopped_exception();
      cout << "> [producer " << producer_nth << "] now i can produce" << endl;
    }

    assert(items.size() < MAX);
    items.push(item);
    cout << "[producer " << producer_nth << "] item " << setw(3) << item << " produced" << endl;

    assert(items.size() <= MAX);

    if (items.size() == 1)
      empty.notify_one();
  }

  int remove(int consumer_nth)
  {
    lock_guard<mutex> lock(monitor);

    while (items.size() == 0) {
      cout << "> [consumer " << consumer_nth << "] is empty" << endl;
      empty.wait(lock);
      if (stop_flag)
	throw stopped_exception();
      cout << "> [consumer " << consumer_nth << "] now i can consume" << endl;
    }

    assert(items.size() <= MAX);

    int res = items.front();
    items.pop();

    cout << "[consumer " << consumer_nth << "] item " << setw(3) << res << " consumed" << endl;

    if (items.size() == MAX-1)
      full.notify_one();

    return res;
  }

  void break_execution()
  {
    stop_flag = true;

    full.notify_all();
    empty.notify_all();
  }

};

synchronized_queue the_queue;

void producer(int producer_nth)
{
  try {
    while (true) {
      // Produce the "item"
      int item = rand() % 256;

      // Add it to the queue
      the_queue.add(producer_nth, item);
    }
  }
  catch (stopped_exception&) {
  }
}

void consumer(int consumer_nth)
{
  try {
    while (true) {
      // Consume the "item" from the queue
      int item = the_queue.remove(consumer_nth);

      // Here we can use the item...
    }
  }
  catch (stopped_exception&) {
  }
}

int main()
{
  // Create 3 producers and 3 consumers
  thread p1(&producer, 1);
  thread p2(&producer, 2);
  thread p3(&producer, 3);
  thread c1(&consumer, 1);
  thread c2(&consumer, 2);
  thread c3(&consumer, 3);

  // Wait 1 second
  this_thread::sleep_for(1000);
  the_queue.break_execution();

  cout << "the_queue.break_execution" << endl;

  // Join everything
  p1.join();
  p2.join();
  p3.join();
  c1.join();
  c2.join();
  c3.join();

  return 0;
}
