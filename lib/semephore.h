#ifndef SEMEPHORE_H
#define SEMEPHORE_H

#include <mutex>
#include <condition_variable>
using namespace std;
using namespace chrono;

class semephore{
    private:
        mutex lock;
        condition_variable condition;
        int count;

    public:
        semephore(){ count = 0; }

        void notify(){
            unique_lock<mutex> l(lock);
            count++;
            condition.notify_one();
        }

        void wait(){
            unique_lock<mutex> l(lock);
            condition.wait(l, [this](){ return count > 0; } );
            count ^= count;
        }

        void waitUntil(time_point<system_clock, milliseconds> point){
            unique_lock<mutex> l(lock);
            condition.wait_until(l, point, [this]() { return count > 0; });
            count ^= count;
        }
};

#endif
