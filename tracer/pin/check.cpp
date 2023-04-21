#include <iostream>
#include <vector>


#define ARR_SIZEBIG 128*1024*1024
#define ARR_SIZESMALL 16*1024
#define LOOP 1000*1000*1000

int main(){
    int a;
    uint64_t *V_small = new uint64_t[ARR_SIZESMALL];  // Uint64_t used since we need 16K Lines
    uint64_t *V_big = new uint64_t[ARR_SIZEBIG];
    uint64_t i = 0;
    while(1){
        V_small[i%ARR_SIZESMALL]++;
        V_big[i%ARR_SIZEBIG]++;
        i++;
        if(i>LOOP){
            std::cout << "Code Done \n"; 
            break;
        }
    }
    return 0;
}