#include <stdio.h>

extern struct rtpkt {
  int sourceid;
  int destid;
  int mincost[4];
};

extern int TRACE;
extern int YES;
extern int NO;
extern float clocktime;

struct distance_table 
{
  int costs[4][4];
} dt3;

static int mincost[4] = {7, 999, 2, 0};  // Direct costs from node 3

void rtinit3() 
{
    printf("rtinit3 called at time %f\n", clocktime);
    
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            if(i == 3) dt3.costs[i][j] = 0;
            else if(j == 3) dt3.costs[i][j] = 999;
            else if(i == j) dt3.costs[i][j] = mincost[i];
            else dt3.costs[i][j] = 999;
        }
    }
    
    struct rtpkt pkt;
    creatertpkt(&pkt, 3, 3, mincost);
    pkt.destid = 0; tolayer2(pkt);
    pkt.destid = 2; tolayer2(pkt);
    printf("Sent initial packets to nodes 0 and 2\n");
    
    printdt3(&dt3);
}

void rtupdate3(struct rtpkt *rcvdpkt)
{
    printf("rtupdate3 called at time %f, received packet from node %d\n", 
           clocktime, rcvdpkt->sourceid);
           
    int updated = NO;
    int src = rcvdpkt->sourceid;
    
    for(int i = 0; i < 4; i++) {
        int newcost = mincost[src] + rcvdpkt->mincost[i];
        if(newcost < dt3.costs[i][src]) {
            dt3.costs[i][src] = newcost;
            updated = YES;
        }
    }
    
    if(updated) {
        for(int i = 0; i < 4; i++) {
            int min = 999;
            for(int j = 0; j < 4; j++) {
                if(dt3.costs[i][j] < min) min = dt3.costs[i][j];
            }
            if(min != mincost[i]) {
                mincost[i] = min;
                struct rtpkt pkt;
                creatertpkt(&pkt, 3, 3, mincost);
                pkt.destid = 0; tolayer2(pkt);
                pkt.destid = 2; tolayer2(pkt);
                printf("Sent updated packets to nodes 0 and 2\n");
            }
        }
    }
    
    printdt3(&dt3);
    printf("Distance table %supdated\n", updated ? "" : "not ");
}

void printdt3(struct distance_table *dtptr)
{
    printf("             via   \n");
    printf("   D3 |  0   2 \n");
    printf("  ----|-----------\n");
    printf("     0|  %3d %3d\n", dtptr->costs[0][0], dtptr->costs[0][2]);
    printf("dest 1|  %3d %3d\n", dtptr->costs[1][0], dtptr->costs[1][2]);
    printf("     2|  %3d %3d\n", dtptr->costs[2][0], dtptr->costs[2][2]);
}