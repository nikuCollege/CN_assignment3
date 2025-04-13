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
} dt0;

static int mincost[4] = {0, 1, 3, 7};  // Direct costs from node 0

void rtinit0() 
{
    printf("rtinit0 called at time %f\n", clocktime);
    
    // Initialize distance table
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            if(i == 0) dt0.costs[i][j] = 0;
            else if(j == 0) dt0.costs[i][j] = 999;
            else if(i == j) dt0.costs[i][j] = mincost[i];
            else dt0.costs[i][j] = 999;
        }
    }
    
    // Send initial distance vector to neighbors (1, 2, 3)
    struct rtpkt pkt;
    creatertpkt(&pkt, 0, 0, mincost);
    for(int i = 1; i < 4; i++) {
        pkt.destid = i;
        tolayer2(pkt);
        printf("Sent initial packet to node %d\n", i);
    }
    
    printdt0(&dt0);
}

void rtupdate0(struct rtpkt *rcvdpkt)
{
    printf("rtupdate0 called at time %f, received packet from node %d\n", 
           clocktime, rcvdpkt->sourceid);
           
    int updated = NO;
    int src = rcvdpkt->sourceid;
    
    // Update distance table with received costs
    for(int i = 0; i < 4; i++) {
        int newcost = mincost[src] + rcvdpkt->mincost[i];
        if(newcost < dt0.costs[i][src]) {
            dt0.costs[i][src] = newcost;
            updated = YES;
        }
    }
    
    // Check if minimum costs changed
    if(updated) {
        for(int i = 0; i < 4; i++) {
            int min = 999;
            for(int j = 0; j < 4; j++) {
                if(dt0.costs[i][j] < min) {
                    min = dt0.costs[i][j];
                }
            }
            if(min != mincost[i]) {
                mincost[i] = min;
                // Send updated distance vector to neighbors
                struct rtpkt pkt;
                creatertpkt(&pkt, 0, 0, mincost);
                for(int k = 1; k < 4; k++) {
                    pkt.destid = k;
                    tolayer2(pkt);
                    printf("Sent updated packet to node %d\n", k);
                }
            }
        }
    }
    
    printdt0(&dt0);
    printf("Distance table %supdated\n", updated ? "" : "not ");
}

void printdt0(struct distance_table *dtptr)
{
    printf("                via     \n");
    printf("   D0 |    1     2    3 \n");
    printf("  ----|-----------------\n");
    printf("     1|  %3d   %3d   %3d\n", dtptr->costs[1][1],
           dtptr->costs[1][2], dtptr->costs[1][3]);
    printf("dest 2|  %3d   %3d   %3d\n", dtptr->costs[2][1],
           dtptr->costs[2][2], dtptr->costs[2][3]);
    printf("     3|  %3d   %3d   %3d\n", dtptr->costs[3][1],
           dtptr->costs[3][2], dtptr->costs[3][3]);
}

void linkhandler0(int linkid, int newcost)
{
    printf("linkhandler0 called at time %f, link to %d cost changed to %d\n",
           clocktime, linkid, newcost);
           
    int oldcost = mincost[linkid];
    mincost[linkid] = newcost;
    
    // Update distance table
    for(int i = 0; i < 4; i++) {
        if(i != 0) {
            dt0.costs[i][linkid] = (dt0.costs[i][linkid] - oldcost) + newcost;
        }
    }
    
    // Send updated distance vector
    struct rtpkt pkt;
    creatertpkt(&pkt, 0, 0, mincost);
    for(int i = 1; i < 4; i++) {
        pkt.destid = i;
        tolayer2(pkt);
        printf("Sent updated packet to node %d\n", i);
    }
    
    printdt0(&dt0);
}