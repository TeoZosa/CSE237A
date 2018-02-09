//
// Created by Teofilo Zosa on 2/9/18.
//

//custom fast sort since we know the # of workloads in advance;
//idea originally from stack overflow, generated the swap orders from
static inline void sorting_network(WLxTime * workloads){
#define min(x, y) (((x).time)<((y).time)?(x):(y))
#define max(x, y) (((x).time)<((y).time)?(y):(x))
#define SWAP(x,y) { const WLxTime shorter = min(workloads[x], workloads[y]); \
                    const WLxTime longer = max(workloads[x], workloads[y]); \
                    workloads[x] = shorter; workloads[y] = longer; }
  SWAP(0, 1);
  SWAP(2, 3);
  SWAP(4, 5);
  SWAP(6, 7);
  SWAP(8, 9);
  SWAP(10, 11);
  SWAP(12, 13);
  SWAP(14, 15);
  SWAP(0, 2);
  SWAP(4, 6);
  SWAP(8, 10);
  SWAP(12, 14);
  SWAP(1, 3);
  SWAP(5, 7);
  SWAP(9, 11);
  SWAP(13, 15);
  SWAP(0, 4);
  SWAP(8, 12);
  SWAP(1, 5);
  SWAP(9, 13);
  SWAP(2, 6);
  SWAP(10, 14);
  SWAP(3, 7);
  SWAP(11, 15);
  SWAP(0, 8);
  SWAP(1, 9);
  SWAP(2, 10);
  SWAP(3, 11);
  SWAP(4, 12);
  SWAP(5, 13);
  SWAP(6, 14);
  SWAP(7, 15);
  SWAP(5, 10);
  SWAP(6, 9);
  SWAP(3, 12);
  SWAP(13, 14);
  SWAP(7, 11);
  SWAP(1, 2);
  SWAP(4, 8);
  SWAP(1, 4);
  SWAP(7, 13);
  SWAP(2, 8);
  SWAP(11, 14);
  SWAP(2, 4);
  SWAP(5, 6);
  SWAP(9, 10);
  SWAP(11, 13);
  SWAP(3, 8);
  SWAP(7, 12);
  SWAP(6, 8);
  SWAP(10, 12);
  SWAP(3, 5);
  SWAP(7, 9);
  SWAP(3, 4);
  SWAP(5, 6);
  SWAP(7, 8);
  SWAP(9, 10);
  SWAP(11, 12);
  SWAP(6, 7);
  SWAP(8, 9);
#undef SWAP
#undef min
#undef max
}