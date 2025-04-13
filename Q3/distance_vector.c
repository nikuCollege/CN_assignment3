#include <stdio.h>
#include <stdlib.h>

#define LINKCHANGES 1 

struct event {
   float evtime;
   int evtype;
   int eventity;
   struct rtpkt *rtpktptr;
   struct event *prev;
   struct event *next;
};
struct event *evlist = NULL;
float clocktime = 0.000;
#define FROM_LAYER2 2
#define LINK_CHANGE 10

struct rtpkt {
  int sourceid;
  int destid;
  int mincost[4];
};

int TRACE = 1;
int YES = 1;
int NO = 0;

void rtinit0(void);
void rtinit1(void);
void rtinit2(void);
void rtinit3(void);
void rtupdate0(struct rtpkt *rcvdpkt);
void rtupdate1(struct rtpkt *rcvdpkt);
void rtupdate2(struct rtpkt *rcvdpkt);
void rtupdate3(struct rtpkt *rcvdpkt);
void linkhandler0(int linkid, int newcost);
void linkhandler1(int linkid, int newcost);
void tolayer2(struct rtpkt packet);
void insertevent(struct event *p);
void init(void);

int creatertpkt(struct rtpkt *initrtpkt, int srcid, int destid, int mincosts[])
{
  int i;
  initrtpkt->sourceid = srcid;
  initrtpkt->destid = destid;
  for (i = 0; i < 4; i++)
    initrtpkt->mincost[i] = mincosts[i];
  return 0;
}  

int main(void)
{
   struct event *eventptr;
   
   init();
   
   while (1) {
     
        eventptr = evlist;
        if (eventptr == NULL)
           goto terminate;
        evlist = evlist->next;
        if (evlist != NULL)
           evlist->prev = NULL;
        if (TRACE > 1) {
          printf("MAIN: rcv event, t=%.3f, at %d",
                          eventptr->evtime, eventptr->eventity);
          if (eventptr->evtype == FROM_LAYER2) {
            printf(" src:%2d,", eventptr->rtpktptr->sourceid);
            printf(" dest:%2d,", eventptr->rtpktptr->destid);
            printf(" contents: %3d %3d %3d %3d\n", 
              eventptr->rtpktptr->mincost[0], eventptr->rtpktptr->mincost[1],
              eventptr->rtpktptr->mincost[2], eventptr->rtpktptr->mincost[3]);
            }
          }
        clocktime = eventptr->evtime;
        if (eventptr->evtype == FROM_LAYER2) {
            if (eventptr->eventity == 0) 
              rtupdate0(eventptr->rtpktptr);
            else if (eventptr->eventity == 1) 
              rtupdate1(eventptr->rtpktptr);
            else if (eventptr->eventity == 2) 
              rtupdate2(eventptr->rtpktptr);
            else if (eventptr->eventity == 3) 
              rtupdate3(eventptr->rtpktptr);
            else { printf("Panic: unknown event entity\n"); exit(0); }
          }
        else if (eventptr->evtype == LINK_CHANGE) {
            if (clocktime < 10001.0) {
              linkhandler0(1, 20);
              linkhandler1(0, 20);
              }
            else {
              linkhandler0(1, 1);
              linkhandler1(0, 1);
              }
          }
          else
             { printf("Panic: unknown event type\n"); exit(0); }
        if (eventptr->evtype == FROM_LAYER2) 
          free(eventptr->rtpktptr);
        free(eventptr);
      }
   
terminate:
   printf("\nSimulator terminated at t=%f, no packets in medium\n", clocktime);
   return 0;
}

void init(void)
{
  int i;
  float sum, avg;
  float jimsrand(void);
  
  printf("Enter TRACE:");
  scanf("%d", &TRACE);

  srand(9999);
  /*sum = 0.0;
  for (i = 0; i < 1000; i++)
     sum = sum + jimsrand();
  avg = sum / 1000.0;
  if (avg < 0.25 || avg > 0.75) {
    printf("It is likely that random number generation on your machine\n" );
    printf("is different from what this emulator expects.  Please take\n");
    printf("a look at the routine jimsrand() in the emulator code. Sorry. \n");
    exit(1);
  }*/

  clocktime = 0.0;
  rtinit0();
  rtinit1();
  rtinit2();
  rtinit3();

  if (LINKCHANGES == 1) {
    struct event *evptr;
    evptr = (struct event *)malloc(sizeof(struct event));
    evptr->evtime = 10000.0;
    evptr->evtype = LINK_CHANGE;
    evptr->eventity = -1;
    evptr->rtpktptr = NULL;
    insertevent(evptr);
    evptr = (struct event *)malloc(sizeof(struct event));
    evptr->evtype = LINK_CHANGE;
    evptr->evtime = 20000.0;
    evptr->rtpktptr = NULL;
    insertevent(evptr);    
  }
}

float jimsrand(void)
{
  double mmm = 2147483647;
  float x;
  x = rand() / mmm;
  return x;
}

void insertevent(struct event *p)
{
   struct event *q, *qold;

   if (TRACE > 3) {
      printf("            INSERTEVENT: time is %lf\n", clocktime);
      printf("            INSERTEVENT: future time will be %lf\n", p->evtime); 
   }
   q = evlist;
   if (q == NULL) {
        evlist = p;
        p->next = NULL;
        p->prev = NULL;
   }
   else {
        for (qold = q; q != NULL && p->evtime > q->evtime; q = q->next)
              qold = q; 
        if (q == NULL) {
             qold->next = p;
             p->prev = qold;
             p->next = NULL;
        }
        else if (q == evlist) {
             p->next = evlist;
             p->prev = NULL;
             p->next->prev = p;
             evlist = p;
        }
        else {
             p->next = q;
             p->prev = q->prev;
             q->prev->next = p;
             q->prev = p;
        }
   }
}

void printevlist(void)
{
  struct event *q;
  printf("--------------\nEvent List Follows:\n");
  for (q = evlist; q != NULL; q = q->next) {
    printf("Event time: %f, type: %d entity: %d\n", q->evtime, q->evtype, q->eventity);
  }
  printf("--------------\n");
}

void tolayer2(struct rtpkt packet)
{
 struct rtpkt *mypktptr;
 struct event *evptr, *q;
 float jimsrand(void), lastime;
 int i;

 int connectcosts[4][4];

 connectcosts[0][0] = 0;  connectcosts[0][1] = 1;  connectcosts[0][2] = 3;
 connectcosts[0][3] = 7;
 connectcosts[1][0] = 1;  connectcosts[1][1] = 0;  connectcosts[1][2] = 1;
 connectcosts[1][3] = 999;
 connectcosts[2][0] = 3;  connectcosts[2][1] = 1;  connectcosts[2][2] = 0;
 connectcosts[2][3] = 2;
 connectcosts[3][0] = 7;  connectcosts[3][1] = 999;  connectcosts[3][2] = 2;
 connectcosts[3][3] = 0;
    
 if (packet.sourceid < 0 || packet.sourceid > 3) {
   printf("WARNING: illegal source id in your packet, ignoring packet!\n");
   return;
 }
 if (packet.destid < 0 || packet.destid > 3) {
   printf("WARNING: illegal dest id in your packet, ignoring packet!\n");
   return;
 }
 if (packet.sourceid == packet.destid) {
   printf("WARNING: source and destination id's the same, ignoring packet!\n");
   return;
 }
 if (connectcosts[packet.sourceid][packet.destid] == 999) {
   printf("WARNING: source and destination not connected, ignoring packet!\n");
   return;
 }

 mypktptr = (struct rtpkt *)malloc(sizeof(struct rtpkt));
 mypktptr->sourceid = packet.sourceid;
 mypktptr->destid = packet.destid;
 for (i = 0; i < 4; i++)
    mypktptr->mincost[i] = packet.mincost[i];
 if (TRACE > 2) {
   printf("    TOLAYER2: source: %d, dest: %d\n              costs:", 
          mypktptr->sourceid, mypktptr->destid);
   for (i = 0; i < 4; i++)
        printf("%d  ", mypktptr->mincost[i]);
   printf("\n");
 }

 evptr = (struct event *)malloc(sizeof(struct event));
 evptr->evtype = FROM_LAYER2;
 evptr->eventity = packet.destid;
 evptr->rtpktptr = mypktptr;

 lastime = clocktime;
 for (q = evlist; q != NULL; q = q->next) 
    if ((q->evtype == FROM_LAYER2 && q->eventity == evptr->eventity)) 
      lastime = q->evtime;
 evptr->evtime = lastime + 2. * jimsrand();

 if (TRACE > 2)  
     printf("    TOLAYER2: scheduling arrival on other side\n");
 insertevent(evptr);
} 