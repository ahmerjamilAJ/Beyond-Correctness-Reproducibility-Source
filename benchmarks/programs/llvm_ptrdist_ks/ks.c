/* Beyond Correctness source packaging notice, 2026-10-25.
 * This is the research-adapted/amalgamated benchmark version.
 * Its upstream copyright and license notices are retained below.
 * This release adds only this notice and a line-number directive.
 * See ../../licenses/ and the benchmark catalog for notices.
 */
#line 1
/*
 *      program:        Graph partition via Kernighan-Lin, modified
 *                      Kernighan-Lin, or Kernighan-Schweikert
 *
 *      author:         Todd M. Austin
 *                      ECE 756
 *
 *      date:           Thursday, February 25, 1993
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*
 *      module configuration
 */

/* define WEIGHTED for 1/(1-n) weighted node cost matrix, otherwise 1 */
/* #define WEIGHTED */

/* define KS_MODE to execute with Kernighan-Schweikert algorithm, rather
   than the Kernighan-Lin algorithm */
/* #undef KS_MODE */

#define BUF_LEN 1024    /* maximum line length */
#define G_SZ    1024    /* maximum group size */

/* simple exception handler */
#define TRY(exp, accpt_tst, fn, fail_fmt, arg1, arg2, arg3, fail_action) { \
              (exp); \
              if (!(accpt_tst)) { \
                  fprintf(stderr, "(%s:%s():%d): ", __FILE__, fn, __LINE__); \
                  fprintf(stderr, fail_fmt, arg1, arg2, arg3); \
                  fprintf(stderr, "\n"); \
                  fail_action; \
                  } \
                  }

/*
 *      the network, first the modules, then the nets
 */
/* modular view */
typedef struct _Net {
    struct _Net * next;
    unsigned long net;
} Net;
typedef Net * NetPtr;

extern NetPtr modules[G_SZ];	/* all modules -> nets */
extern unsigned long numModules;

/* net-ular view */
typedef struct _Module {
    struct _Module * next;
    unsigned long module;
} Module;
typedef Module * ModulePtr;

extern ModulePtr nets[G_SZ];	/* all nets -> modules */
extern unsigned long numNets;

typedef struct _ModuleRec {
    struct _ModuleRec * next;
    unsigned long module;
} ModuleRec;
typedef ModuleRec * ModuleRecPtr;

typedef struct _ModuleList {
    ModuleRecPtr head;
    ModuleRecPtr tail;
} ModuleList;
typedef ModuleList * ModuleListPtr;

extern ModuleList groupA, groupB;		/* current A, B */
extern ModuleList swapToA, swapToB;		/* swapped from A,B, ordered */
extern float GP[G_SZ];		/* GPs, ordered */

typedef enum { GroupA, GroupB, SwappedToA, SwappedToB } Groups;

extern Groups moduleToGroup[G_SZ];	/* current inverse mapping */
extern float D[G_SZ];		/* module costs */
extern float cost[G_SZ];		/* net costs */

void ReadNetList(char *fname);
void NetsToModules(void);
void ComputeNetCosts(void);
void InitLists(void);
void ComputeDs(ModuleListPtr group, Groups myGroup, Groups mySwap);
float CAiBj(ModuleRecPtr mrA, ModuleRecPtr mrB);
void SwapNode(ModuleRecPtr maxPrev, ModuleRecPtr max,
	      ModuleListPtr group, ModuleListPtr swapTo);
void UpdateDs(ModuleRecPtr max, Groups group);
float FindMaxGpAndSwap();
void SwapSubsetAndReset(unsigned long iMax);
void PrintResults(int verbose);
int main(int argc, char **argv);

/*
 *	program:	Graph partition via Kernighan-Lin, modified
 *			Kernighan-Lin, or Kernighan-Schweikert
 *
 *	author:		Todd M. Austin
 *			ECE 756
 *
 *	date:		Thursday, February 25, 1993
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>



NetPtr modules[G_SZ];		/* all modules -> nets */
unsigned long numModules;

ModulePtr nets[G_SZ];	     	/* all nets -> modules */
unsigned long numNets;

ModuleList groupA, groupB;			/* current A, B */
ModuleList swapToA, swapToB;			/* swapped from A,B, ordered */
float GP[G_SZ];			/* GPs, ordered */

Groups moduleToGroup[G_SZ];	/* current inverse mapping */
float D[G_SZ];			/* module costs */
float cost[G_SZ];			/* net costs */


/* read the netlist into the nets[] structure */
void
ReadNetList(char *fname)
{
    FILE *inFile;
    char line[BUF_LEN];
    char *tok;
    unsigned long net, dest;
    ModulePtr node, prev, head;

    TRY(inFile = fopen(fname, "r"),
	inFile != NULL, "ReadData",
	"unable to open input file [%s]", inFile, 0, 0,
	exit(1));

    TRY(fgets(line, BUF_LEN, inFile),
	sscanf(line, "%lu %lu", &numNets, &numModules) == 2, "ReadData",
	"unable to parse header in file [%s]", inFile, 0, 0,
	exit(1));

    for (net = 0; net < numNets; net++) {
	fgets(line, BUF_LEN, inFile);
	
	/* net connections for "dest" */
	dest = atol(strtok(line, " \t\n"))-1;

	/* parse out all the net module connections */
	TRY(head = prev = (Module *)malloc(sizeof(Module)),
	    prev != NULL, "ReadData",
	    "unable to allocate a module list node", 0, 0, 0,
	    exit(1));
	(*prev).module = atol(strtok(NULL, " \t\n"))-1;
	(*prev).next = NULL;
	while ((tok = strtok(NULL, " \t\n")) != NULL) {
	    TRY(node = (Module *)malloc(sizeof(Module)),
		node != NULL, "ReadData",
		"unable to allocate a module list node", 0, 0, 0,
		exit(1));
	    (*node).module = atol(tok)-1;
	    (*node).next = NULL;
	    (*prev).next = node;
	    prev = node;
	}
	nets[dest] = head;
    }
}

/* invert the previously read nets, into a module to net structure */
void
NetsToModules(void)
{
    unsigned long net, mod;
    ModulePtr modNode;
    NetPtr netNode;

    for (mod = 0; mod<numModules; mod++)
	modules[mod] = NULL;

    for (net=0; net<numNets; net++) {
	for (modNode = nets[net]; modNode != NULL; modNode = (*modNode).next) {
	    TRY(netNode = (Net *)malloc(sizeof(Net)),
		netNode != NULL, "NetsToModules",
		"unable to allocate net list node", 0, 0, 0,
		exit(1));
	    (*netNode).net = net;
	    (*netNode).next = modules[(*modNode).module];
	    modules[(*modNode).module] = netNode;
	}
    }
}


/* compute the net edge costs, based on the weighting strategy */
void
ComputeNetCosts(void)
{
#ifdef WEIGHTED
    ModulePtr nn;
    unsigned long count;
#endif /* WEIGHTED */
    unsigned long i;

    for (i=0; i<numNets; i++) {
#ifndef WEIGHTED
	cost[i] = 1.0;
#else
	count = 0;
	for (nn = nets[i]; nn != NULL; nn = (*nn).next)
	    count++;

	cost[i] = 1.0/((float)count - 1.0);
#endif /* WEIGHTED */
    }
}

/* set up the initial groups, just split down the middle */
void
InitLists(void)
{
    unsigned long p;
    ModuleRecPtr mr;

    groupA.head = groupA.tail = NULL;
    groupB.head = groupB.tail = NULL;

    /* for all modules */
    for (p = 0; p<numModules/2; p++) {

	/* build the group A module list */
	TRY(mr = (ModuleRec *)malloc(sizeof(ModuleRec)),
	    mr != NULL, "main",
	    "unable to allocate ModuleRec", 0, 0, 0,
	    exit(1));
	(*mr).module = p;
	if (groupA.head == NULL) {
	    /* first item */
	    groupA.head = groupA.tail = mr;
	    (*mr).next = NULL;
	}
	else {
	    /* add to tail */
	    (*mr).next = NULL;
	    (*groupA.tail).next = mr;
	    groupA.tail = mr;
	}
	moduleToGroup[p] = GroupA;

	/* build the group B module list */
	TRY(mr = (ModuleRec *)malloc(sizeof(ModuleRec)),
	    mr != NULL, "main",
	    "unable to allocate ModuleRec", 0, 0, 0,
	    exit(1));
	(*mr).module = (numModules/2) + p;
	if (groupB.head == NULL) {
	    /* first item */
	    groupB.head = groupB.tail = mr;
	    (*mr).next = NULL;
	}
	else {
	    /* add to tail */
	    (*mr).next = NULL;
	    (*groupB.tail).next = mr;
	    groupB.tail = mr;
	}
	moduleToGroup[(numModules/2) + p] = GroupB;
    }

    /* initially clear the swap chains */
    swapToA.head = swapToA.tail = NULL;
    swapToB.head = swapToB.tail = NULL;
}


/* compute the cost of switching every node in group to the other group */
void
ComputeDs(ModuleListPtr group, Groups myGroup, Groups mySwap)
{
#ifdef KS_MODE

    NetPtr netNode;
    ModulePtr modNode;
    ModuleRecPtr groupNode;
    unsigned long numInG, numInNet;
    ModulePtr oneInG;

    /* for all modules in group */
    for (groupNode = (*group).head;
	 groupNode != NULL;
	 groupNode = (*groupNode).next) {

	assert(moduleToGroup[(*groupNode).module] == myGroup);

	/* for all nets on this module, check if groupNode move unifies net */
	for (netNode = modules[(*groupNode).module];
	     netNode != NULL;
	     netNode = (*netNode).next) {

	    /* look for single node nets, or single partition nets */
	    numInG = numInNet = 0;
	    oneInG = NULL;
	    /* for all modules on this net */
	    for (modNode = nets[(*netNode).net];
		 modNode != NULL;
		 modNode = (*modNode).next) {
		if ((moduleToGroup[(*modNode).module] == myGroup) ||
		    (moduleToGroup[(*modNode).module] == mySwap)) {
		    numInG++;
		    oneInG = modNode;
		}
		numInNet++;
	    }
	    /* single node net? */
	    if ((numInG == 1) && ((*oneInG).module == (*groupNode).module))
		D[(*groupNode).module] = D[(*groupNode).module] + 1;
	    /* single partition net? */
	    if (numInG == numInNet)
		D[(*groupNode).module] = D[(*groupNode).module] - 1;
	}
    }

#else /* !KS_MODE */

    float I, E;

    NetPtr netNode;
    ModulePtr modNode;
    ModuleRecPtr groupNode;

    /* for all modules in group */
    for (groupNode = (*group).head;
	 groupNode != NULL;
	 groupNode = (*groupNode).next) {

	assert(moduleToGroup[(*groupNode).module] == myGroup);

	/* initial conditions */
	I = E = 0.0;

	/* for all nets on this module */
	for (netNode = modules[(*groupNode).module];
	     netNode != NULL;
	     netNode = (*netNode).next) {
	    
	    /* for all modules on this net */
	    for (modNode = nets[(*netNode).net];
		 modNode != NULL;
		 modNode = (*modNode).next) {

		/* only check nodes other than self, and not swapped */
		if (((*modNode).module != (*groupNode).module) &&
		    (moduleToGroup[(*modNode).module] < SwappedToA)) {
		    if (moduleToGroup[(*modNode).module] == myGroup)
			I = I + cost[(*netNode).net]; /* internal */
		    else
			E = E + cost[(*netNode).net]; /* external */
		}
	    }
	}
	D[(*groupNode).module] = E - I;
    }

#endif /* !KS_MODE */

}

/*
 *	program:	Graph partition via Kernighan-Lin, modified
 *			Kernighan-Lin, or Kernighan-Schweikert
 *
 *	author:		Todd M. Austin
 *			ECE 756
 *
 *	date:		Thursday, February 25, 1993
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>



/* handle special cases where both nodes are switched */
float
CAiBj(ModuleRecPtr mrA, ModuleRecPtr mrB)
{
    NetPtr netNode;
    ModulePtr modNode;
    float gain = 0.0;
    float netCost;
    unsigned long module = (*mrB).module;

    /* is mrA connected to mrB? */
    /* mrA and mrB are both un-Swapped */
    for (netNode = modules[(*mrA).module];
	 netNode != NULL;
	 netNode = (*netNode).next) {
	netCost = cost[(*netNode).net];
	for (modNode = nets[(*netNode).net];
	     modNode != NULL;
	     modNode = (*modNode).next) {
	    if ((*modNode).module == module) {
		gain = gain + netCost;
	    }
	}
    }
    return gain;
}

/* swap a node out of the current group, and into a target group */
void
SwapNode(ModuleRecPtr maxPrev, ModuleRecPtr max,
	 ModuleListPtr group, ModuleListPtr swapTo)
{
    if (maxPrev == NULL) {	/* found at head of list */
	if ((*group).head == (*group).tail)	{ /* only one in the list */
	    (*group).head = NULL;
	    (*group).tail = NULL;
	    (*max).next = NULL;
	}
	else {
	    (*group).head = (*max).next;
	    (*max).next = NULL;
	}
    }
    else {	/* middle or end of list */
	if ((*group).tail == max)		/* end of list */
	    (*group).tail = maxPrev;
	(*maxPrev).next = (*max).next;
	(*max).next = NULL;
    }

    /* put max on the tail of swapTo */
    if ((*swapTo).tail == NULL) {	/* empty */
#if 0
	(*swapTo).head = (*swapTo).tail = max;
#endif
	(*swapTo).tail = max;
	(*swapTo).head = max;
    }
    else { /* end of list */
	(*(*swapTo).tail).next = max;
	(*swapTo).tail = max;
    }
    (*max).next = NULL;
}

/* incrementally update the D values in Kernighan-Lin algorithm */
void
UpdateDs(ModuleRecPtr max, Groups group)
{
    NetPtr net;
    ModulePtr mod;

    /* for all nets this is connected to */
    for (net = modules[(*max).module]; net != NULL; net = (*net).next) {

	/* for a modules this net is connected to */
	for (mod = nets[(*net).net]; mod != NULL; mod = (*mod).next) {

	    if (moduleToGroup[(*mod).module] < SwappedToA) {
		if (moduleToGroup[(*mod).module] == group)
		    D[(*mod).module] = D[(*mod).module] + cost[(*net).net];
		else
		    D[(*mod).module] = D[(*mod).module] - cost[(*net).net];
	    }
	}
    }
}

/* find the best swap available and do it */
float
FindMaxGpAndSwap()
{
    ModuleRecPtr mrA, mrPrevA, mrB, mrPrevB;
    ModuleRecPtr maxA, maxPrevA, maxB, maxPrevB;
    float gp, gpMax;

    gpMax = -9999999;
    maxA = maxPrevA = maxB = maxPrevB = NULL;
    for (mrA = groupA.head, mrPrevA = NULL;
	 mrA != NULL;
	 mrPrevA = mrA, mrA = (*mrA).next) {

	for (mrB = groupB.head, mrPrevB = NULL;
	     mrB != NULL;
	     mrPrevB = mrB, mrB = (*mrB).next) {

#ifdef KS_MODE
	    gp = D[(*mrA).module] + D[(*mrB).module] - CAiBj(mrA, mrB);
#else /* !KS_MODE */
	    gp = D[(*mrA).module] + D[(*mrB).module] - 2*CAiBj(mrA, mrB);
#endif /* !KS_MODE */
	    if (gp > gpMax) {
		gpMax = gp;
		maxA = mrA; maxPrevA = mrPrevA;
		maxB = mrB; maxPrevB = mrPrevB;
	    }
	}
    }

    /* swap the nodes out, into the swap lists */
    assert(maxA != NULL);
    SwapNode(maxPrevA, maxA, &(groupA), &(swapToB));
    assert(maxB != NULL);
    SwapNode(maxPrevB, maxB, &(groupB), &(swapToA));


    /* update the inverse mapping, these two node are now gone */
    assert(moduleToGroup[(*maxA).module] == GroupA);
    moduleToGroup[(*maxA).module] = SwappedToB;

    assert(moduleToGroup[(*maxB).module] == GroupB);
    moduleToGroup[(*maxB).module] = SwappedToA;

#ifndef KS_MODE
    /* update the Ds */
    UpdateDs(maxA, GroupA);
    UpdateDs(maxB, GroupB);
#endif /* !KS_MODE */

    return gpMax;
}

/* find the best point, during the last numModules/2 swaps */
float
FindGMax(unsigned long * iMax)
{
    int i;
    float gMax;

    gMax = -9999999;
    *iMax = 0xffffffff;
    for (i=0; i<numModules/2; i++) {
	if (GP[i] > gMax) {
	    gMax = GP[i];
	    *iMax = i;
	}
    }
    return gMax;
}

/* swap groupA and groupB from [0..iMax] */
void
SwapSubsetAndReset(unsigned long iMax)
{
    unsigned long i;
    ModuleRecPtr mrPrevA, mrA, mrPrevB, mrB;

    /* re-splice the lists @ iMax pointers into the lists */
    for (mrPrevA = NULL, mrA = swapToA.head,
	 mrPrevB = NULL, mrB = swapToB.head, i=0;
	 i <= iMax;
	 mrPrevA = mrA, mrA = (*mrA).next,
	 mrPrevB = mrB, mrB = (*mrB).next,
	 i++);

    /* must at least select one to swap, case where gMax is first */
    assert(mrPrevA != NULL && mrPrevB != NULL);

    if (mrA == NULL) {	
	/* swap entire list */
	groupA = swapToA;
	groupB = swapToB;
    }
    else {
	/* splice the lists */
	(*mrPrevA).next = mrB;
	groupA.head = swapToA.head;
	groupA.tail = swapToB.tail;

	(*mrPrevB).next = mrA;
	groupB.head = swapToB.head;
	groupB.tail = swapToA.tail;
    }

    /* reset the inverse mappings */
    for (mrA = groupA.head; mrA != NULL; mrA = (*mrA).next)
	moduleToGroup[(*mrA).module] = GroupA;
    for (mrB = groupB.head; mrB != NULL; mrB = (*mrB).next)
	moduleToGroup[(*mrB).module] = GroupB;

    /* clear the swap lists */
    swapToA.head = swapToA.tail = NULL;
    swapToB.head = swapToB.tail = NULL;
}


struct {
    unsigned long total;
    unsigned long edgesCut;
    unsigned long netsCut;
} netStats[256];
long maxStat;

/* print the current groups, and their edge and net cut counts */
void
PrintResults(int verbose)
{
    ModuleRecPtr mr;
    NetPtr nn;
    ModulePtr mn;
    unsigned long cuts;
    Groups grp;
    int i, netSz;

    fprintf(stdout, "----------------------------------------------\n");

    maxStat = -1;
    for (i=0; i<256; i++)
	netStats[i].total = netStats[i].edgesCut = netStats[i].netsCut = 0; 

    /* partitions */
    if (verbose) {
	fprintf(stdout, "Group A:  \n");
	for (mr = groupA.head; mr != NULL; mr = (*mr).next)
	    fprintf(stdout, "%3lu ", (*mr).module+1);
	fprintf(stdout, "\n");

	fprintf(stdout, "Group B:  \n");
	for (mr = groupB.head; mr != NULL; mr = (*mr).next)
	    fprintf(stdout, "%3lu ", (*mr).module+1);
	fprintf(stdout, "\n");
    }

    /* total edge cuts */
    cuts = 0;
    for (mr = groupA.head; mr != NULL; mr = (*mr).next) {

	assert(moduleToGroup[(*mr).module] == GroupA);

	/* for all nets on this module */
	for (nn = modules[(*mr).module]; nn != NULL; nn = (*nn).next) {
	    
	    netSz = 0;
	    for (mn = nets[(*nn).net]; mn != NULL; mn = (*mn).next)
		netSz++;
	    assert(netSz >= 2);

	    /* for all modules on this net */
	    for (mn = nets[(*nn).net]; mn != NULL; mn = (*mn).next) {

		/* only check nodes other than self, and not swapped */
		if (moduleToGroup[(*mr).module] != moduleToGroup[(*mn).module]) {
		    if (verbose)
			fprintf(stdout, "Conn %3lu - %3lu cut.\n",
				(*mr).module+1, (*mn).module+1);
		    netStats[netSz].edgesCut++;
		    cuts++;
		}
	    }
	}
    }
    fprintf(stdout, "Total edge cuts = %lu\n", cuts);

    /* total net cuts */
    cuts = 0;
    for (i=0; i<numNets; i++) {

	netSz = 0;
	for (mn = nets[i]; mn != NULL; mn = (*mn).next)
	    netSz++;
	assert(netSz >= 2);
	netStats[netSz].total++;
	if (netSz > maxStat)
	    maxStat = netSz;

	for (grp=moduleToGroup[(*(nets[i])).module],mn = (*(nets[i])).next;
	     mn != NULL;
	     mn = (*mn).next) {
	    
	    /* only check nodes other than self, and not swapped */
	    if (grp != moduleToGroup[(*mn).module]) {
		if (verbose)
		    fprintf(stdout, "Net %3lu cut.\n", i+1);
		cuts++;
		netStats[netSz].netsCut++;
		break;
	    }
	}
    }
    fprintf(stdout, "Total net cuts  = %lu\n", cuts);

    for (i=2; i<=maxStat; i++)
	fprintf(stdout,
		"sz:%5lu     total:%5lu     edgesCut:%5lu     netsCuts:%5lu\n",
		i, netStats[i].total,
		netStats[i].edgesCut, netStats[i].netsCut);
}

int
main(int argc, char **argv)
{
    unsigned long p, iMax;
    float gMax, lastGMax;
    ModuleRecPtr mr;
    ;

    /* parse argument */
    if (argc != 2) {
	fprintf(stderr, "Usage: KL <input_file>\n");
        ;
	exit(1);
    }

    /* prepare the data structures */
    ReadNetList(argv[1]);
    NetsToModules();
    ComputeNetCosts();

    assert((numModules % 2) == 0);

    /* initial partition */
    InitLists();
    lastGMax = 0;

    /* do until we don't make any progress */
    do {

#ifndef KS_MODE
	/* compute the swap costs */
	ComputeDs(&(groupA), GroupA, SwappedToA);
	ComputeDs(&(groupB), GroupB, SwappedToB);
#endif /* !KS_MODE */

	/* for all pairs of nodes in A,B */
	for (p = 0; p<numModules/2; p++) {

#ifdef KS_MODE
	    /* compute the swap costs */
	    ComputeDs(&(groupA), GroupA, SwappedToA);
	    ComputeDs(&(groupB), GroupB, SwappedToB);
#endif /* KS_MODE */

	    /* find the max swap opportunity, and swap */
	    GP[p] = FindMaxGpAndSwap();

	}
	/* lists should both be empty now */
	assert(groupA.head == NULL && groupA.tail == NULL);
	assert(groupB.head == NULL && groupB.tail == NULL);

	gMax = FindGMax(&iMax);

	/* debug/statistics */
	if (lastGMax == gMax)
	    fprintf(stdout, "No progress: gMax = %f\n", gMax);
	lastGMax = gMax;
	fprintf(stdout, "gMax = %f, iMax = %lu\n", gMax, iMax);

	if (gMax > 0.0)
	    SwapSubsetAndReset(iMax);
	PrintResults(0);
    } while (gMax > 0.0);	/* progress made? */

    /* all swaps rejected */
    groupA = swapToB;
    for (mr = groupA.head; mr != NULL; mr = (*mr).next)
	moduleToGroup[(*mr).module] = GroupA;
    groupB = swapToA;
    for (mr = groupB.head; mr != NULL; mr = (*mr).next)
	moduleToGroup[(*mr).module] = GroupB;

    ;

    /* all done, show results */
    PrintResults(1);
#ifdef PLUS_STATS
    PrintDerefStats(stderr);
    PrintHeapSize(stderr);
#endif /* PLUS_STATS */
    exit(0);
    return 0;
}
