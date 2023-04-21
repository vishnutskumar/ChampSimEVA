#include <iostream>
#include <vector>


#define ARR_SIZEBIG 128*1024*8
#define ARR_SIZESMALL 16*1024*8
#define LOOP 1000*1000*1000


int main(){
    volatile uint64_t temp1, temp2;
    uint64_t *V_small = new uint64_t[ARR_SIZESMALL];  // Uint64_t used since we need 16K Lines
    uint64_t *V_big = new uint64_t[ARR_SIZEBIG];
    uint64_t i = 0;
    while(1){
        temp1 = V_small[i&(ARR_SIZESMALL-1)];
        temp2 = V_big[i&(ARR_SIZEBIG-1)];
        i=i+8; // To get 64 Bytes
        // std::cout << (i&(ARR_SIZESMALL-1)) << "\n";
        // std::cout << ARR_SIZESMALL << "\n";
        if(i>LOOP){
            std::cout << "Code Done \n"; 
            break;
        }
    }
    return 0;
}