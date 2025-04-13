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
} dt2;

static int mincost[4] = {3, 1, 0, 2};  // Direct costs from node 2

void rtinit2() 
{
    printf("rtinit2 called at time %f\n", clocktime);
    
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            if(i == 2) dt2.costs[i][j] = 0;
            else if(j == 2) dt2.costs[i][j] = 999;
            else if(i == j) dt2.costs[i][j] = mincost[i];
            else dt2.costs[i][j] = 999;
        }
    }
    
    struct rtpkt pkt;
    creatertpkt(&pkt, 2, 2, mincost);
    pkt.destid = 0; tolayer2(pkt);
    pkt.destid = 1; tolayer2(pkt);
    pkt.destid = 3; tolayer2(pkt);
    printf("Sent initial packets to nodes 0, 1, and 3\n");
    
    printdt2(&dt2);
}

void rtupdate2(struct rtpkt *rcvdpkt)
{
    printf("rtupdate2 called at time %f, received packet from node %d\n", 
           clocktime, rcvdpkt->sourceid);
           
    int updated = NO;
    int src = rcvdpkt->sourceid;
    
    for(int i = 0; i < 4; i++) {
        int newcost = mincost[src] + rcvdpkt->mincost[i];
        if(newcost < dt2.costs[i][src]) {
            dt2.costs[i][src] = newcost;
            updated = YES;
        }
    }
    
    if(updated) {
        for(int i = 0; i < 4; i++) {
            int min = 999;
            for(int j = 0; j < 4; j++) {
                if(dt2.costs[i][j] < min) min = dt2.costs[i][j];
            }
            if(min != mincost[i]) {
                mincost[i] = min;
                struct rtpkt pkt;
                creatertpkt(&pkt, 2, 2, mincost);
                pkt.destid = 0; tolayer2(pkt);
                pkt.destid = 1; tolayer2(pkt);
                pkt.destid = 3; tolayer2(pkt);
                printf("Sent updated packets to nodes 0, 1, and 3\n");
            }
        }
    }
    
    printdt2(&dt2);
    printf("Distance table %supdated\n", updated ? "" : "not ");
}

void printdt2(struct distance_table *dtptr)
{
    printf("                via     \n");
    printf("   D2 |    0     1    3 \n");
    printf("  ----|-----------------\n");
    printf("     0|  %3d   %3d   %3d\n", dtptr->costs[0][0],
           dtptr->costs[0][1], dtptr->costs[0][3]);
    printf("dest 1|  %3d   %3d   %3d\n", dtptr->costs[1][0],
           dtptr->costs[1][1], dtptr->costs[1][3]);
    printf("     3|  %3d   %3d   %3d\n", dtptr->costs[3][0],
           dtptr->costs[3][1], dtptr->costs[3][3]);
}