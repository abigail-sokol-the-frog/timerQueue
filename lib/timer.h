#ifndef TIMER_H
#define TIMER_H

#include <iostream>
#include <mutex>
#include <chrono>
#include "semephore.h"
#include <thread>
using namespace std;
using namespace chrono;

typedef void (*task)();
typedef void (*taskFailHandler)(exception);

struct taskToken{
    task main;
    taskFailHandler error;
    time_point<system_clock, milliseconds> when;
};

void defualtExceptionHandler(exception e){
    cout << "An exception occured: " << e.what() << endl;
}

class timer{
    private:
        taskToken* list;
        int size, totalSize;
        mutex lock;
        semephore sync;
        bool done;
        thread worker;

        int getNextTask(){
            if(size == 0){
                return -1;
            }
            int index = 0;
            time_point<system_clock, milliseconds> next = list[0].when;
            for(int i = 1; i < size; i++){
                if(list[i].when < next){
                    next = list[i].when;
                    index = i;
                }
            }
            return index;
        }

        time_point<system_clock, milliseconds> now(){
            time_point<system_clock, milliseconds> time(duration_cast<milliseconds>(system_clock::now().time_since_epoch()));
            return time;
        }

    public:
        timer(int startingSize = 1){
            list = (taskToken*)malloc(startingSize * sizeof(taskToken));
            totalSize = startingSize;
            size = 0;
            done = false;
            worker = thread([this]{ run(); });
        }

        ~timer(){
            worker.join();
            free(list);
        }

        void scedual(task main, unsigned int when, taskFailHandler fallBack = defualtExceptionHandler){
            time_point<system_clock, milliseconds> time(duration_cast<milliseconds>(system_clock::now().time_since_epoch()));
            time += milliseconds(when);
            taskToken token{main, fallBack, time};
            lock.lock();
            if(size == totalSize){
                totalSize *= 2;
                list = (taskToken*)realloc(list, totalSize * sizeof(taskToken));
            }
            list[size] = token;
            size++;
            sync.notify();
            lock.unlock();
        }

        void execute(int index){
            if(list[index].when <= now()){
                thread t([this](int index){
                    try{
                        list[index].main();
                    } catch(exception e){
                        list[index].error(e);
                    }
                }, index);
                t.detach();
                lock.lock();
                    if(index + 1 == size){ size--; lock.unlock(); return; }
                    for(int i = index; i < size - 1; i++){
                        list[i] = list[i+1];
                    }
                    size--;
                lock.unlock();
                return;
            }
        }

        void run(){
            while(!done){
                int index = getNextTask();
                if(index != -1){
                    if(list[index].when > now()){
                        sync.waitUntil( list[index].when );
                    }
                } else {
                    sync.wait();
                }
                if(index != -1){
                    execute(index);
                }
            }
        }

        void stop(){
            done = true;
            sync.notify();
        }
};

#endif
