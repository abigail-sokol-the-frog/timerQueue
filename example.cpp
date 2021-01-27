#include "lib/timer.h"
#include <unistd.h>

void testA(){
    cout << "Test A was called!" << endl;
}

void testB(){
    cout << "Test B was called!" << endl;
}

int main(){
    cout << "Running..." << endl;
    timer t{};
    t.scedual(testA, 5000);
    t.scedual(testB, 2000);
    sleep(7);
    cout << "Stopping..." << endl;
    t.stop();
    return 0;
}
