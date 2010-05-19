#include <gui/thread.h>
#include <iostream>
#include <utility>
#include <vector>

using namespace gui;
using namespace std;

class philosopher;

vector<philosopher*> philosophers;
mutex philosophers_mutex;

enum state_t { THINKING, EATING };

class philosopher
{
public:
  philosopher(int id) {
    m_id = id;
    m_feed = 0;
    think();
  }

  void eat() {
    m_state = EATING;
    m_feed++;
    cout << "philosopher " << m_id << " is eating (" << m_feed << ")" << endl;
  }

  void think() {
    m_state = THINKING;
    cout << "philosopher " << m_id << " is thinking" << endl;
  }

  state_t state() const {
    return m_state;
  }

  philosopher* philosopher_at_left() const
  {
    return philosophers[m_id > 0 ? m_id-1: philosophers.size()-1];
  }

  philosopher* philosopher_at_right() const
  {
    return philosophers[m_id < philosophers.size()-1 ? m_id+1: 0];
  }
  
private:
  int m_id;
  size_t m_feed;
  state_t m_state;
};

static void control_philosopher(philosopher* phi)
{
  while (true) {
    philosophers_mutex.lock();

    philosopher* left = phi->philosopher_at_left();
    philosopher* right = phi->philosopher_at_right();

    if (left->state() == THINKING && 
	right->state() == THINKING) {
      phi->eat();

      philosophers_mutex.unlock();

      this_thread::sleep_for(100);

      philosophers_mutex.lock();
      phi->think();
    }

    philosophers_mutex.unlock();

    this_thread::yield();
  }
}

int main()
{
  vector<thread*> threads;

  for (int id=0; id<5; ++id)
    philosophers.push_back(new philosopher(id));

  {
    lock_guard<mutex> lock(philosophers_mutex);
    for (int id=0; id<philosophers.size(); ++id) {
      threads.push_back(new thread(&control_philosopher, philosophers[id]));
    }
  }

  for (int id=0; id<threads.size(); ++id)
    threads[id]->join();

  return 0;
}
