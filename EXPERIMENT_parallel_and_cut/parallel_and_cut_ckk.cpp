#include <stdio.h>
/* standard I/O library */
#include <math.h>
/* mathematical library */
#include <stdlib.h>
/* standard library*/
#include <time.h>
/* compute the time library*/
#include <pthread.h>
/* parallel computing library*/
#include <sys/time.h>
/* compute the real time library*/
#include <fstream>
#define MAXK 12
/* maximum number of subsets to partition values into */
#define MAXN 201
/* maximum number of values being partitioned */
#define MAXSETS 65536
/* maximum number of values being partitioned 2^(MAXN/4)*/
#define MAXLIST 100000
/* maximum size of cdlists */
#define INITSEED  13070
/* initial random seed, in decimal */
#define ACONST 25214903917
/* constant multiplier */
#define C 11
/* additive constant */
#define MASK 281474976710655
/* 2^{48}-1 */
# define thread_count 32
/* number of threads*/

using namespace std;




int thread = 0;
/* next thread index */
pthread_t *thread_handles;
/* handles of pthread*/

int K;
/* number of sets to partition numbers into */
int N;
/* problem size: number of values in number array */
int TRIALS;
/* number of trials to run */
long long seed;
/* current random number seed */

long long sorted[MAXN];
/* original numbers sorted in decreasing order */
long long sumall;
/* sum of all original numbers */
long long diffthresh;
/* difference threshold at which to terminate CKK */

long long calls[MAXK+1];
/* counters for number of partitionings for each K */

long long alpha = 100000000000;
/* best difference found for 2-way CKK partitioning */
int finished_thread = 0;

clock_t start_normal, end_normal, start_parallel, end_parallel;
/* record time */

int N_set[MAXN];
/* record progress */

double ave_time_normal, ave_time_parallel = 0;
/* INIT initializes it's argument array with random numbers from zero to
   2^{MAXNUM}-1.  It returns their sum. */

struct func_para{
    long long a[MAXN];
    int n;
    long long total;
    int thread;
}para[thread_count];

int count = 0;

// init 01
long long init (long long a[MAXN], int n){
    int i;
    /* index into array */
    long long sum;
    /* sum of all numbers */

    sum = 0;
    /* initialize sum of all numbers */
    for (i = 0; i < n; i++){
        /* for each element of array */
        seed = (ACONST * seed + C) & MASK;
//        seed = 20 * seed;
        /* next seed in random sequence */
        a[i] = seed;
        /* random value from zero to 2^{48}-1 */
        sum += a[i];
        /* compute sum of all numbers */
    }
    return (sum);
    /* return sum of numbers */
}
    /* SORTDOWN takes an array of longs, and the length of the array,
    and sorts the array in decreasing order using insertion sort. */

//sortdown 01
void sortdown (long long a[MAXN], int n){
    int i,j;
    /* indices into array for sorting */
    long long temp;
    /* temporary value for swapping */

    for(i = 1; i < n; i++){
        temp = a[i];
        for(j = i - 1; j >= 0; j--)
            if (a[j] < temp)
                a[j+1] = a[j];
            else
                break;
        a[j+1] = temp;
    }
}

//pthread_ckk 02
void *pthread_ckk(void *args);


/* CKK takes an array of numbers A sorted in decreasing order, its
   length N, and their TOTAL sum, and finds the best partition,
   leaving the resulting difference in the global variable ALPHA. It
   runs branch-and-bound, starting with the Karmarkar-Karp
   solution. At each point, the largest two remaining numbers are
   selected, and replaced with either their difference or their sum,
   representing assigning them to different sets or the same set,
   respectively. */
//ckk *1
long long ckk (long long a[MAXN], int n, long long total, bool flag){
    if(finished_thread > 0.80 * 32)
        return 1;
/* array of numbers */
/* number of elements in array */
/* sum of all numbers */
//    printf("-----------------------------\n");
//    for(int count = 0; count < n;count++)
//        printf("count-%d:%lld\n", count, a[count]);
//    printf("\n");
    long long diff2;
    /* difference of largest 2    numbers */
    long long sum2;
    /* sum of largest 2    numbers */
    long long difference;
    /* difference of  partition */
    long long b[MAXN];
    /* new copy of list for recursive calls */
    long long rest;
    /* sum of all elements except first 2 */
    int i;
    /* index into array */
//    bool flag = false;
    /* make sure when create pthread */
//    if (N_set[n] == 10 && n % 2 == 0)
//        printf("process: %d\n", n);

//    if (n == 60)
//        for (int i = 0; i < n; i++)
//        printf("--- test %d -----: %lld\n", n, a[i]);
//    if (N - n == 4){
//        flag = true;
//        printf("thread %d: ----- deep N - n = %d -----\n", thread, N - n);
//    }
    diff2 = a[0] - a[1];
    /* difference of two largest elements */
    for (i = 2; i < n; i++)
    /* copy list and insert difference in order */
        if (diff2 < a[i])
            b[i-2] = a[i];
    /* new number is less, keep copying */
        else
            break;
    /* found correct place, exit loop */
    b[i-2] = diff2;
    /* insert new element into array */
    for (i = i; i < n; i++)
    /* copy remaining elements */
        b[i-1] = a[i];

    if (n == 5)
    /* when only 4 numbers are left, KK is optimal */
    {
        diff2 = b[0] - b[1];
        /* difference of 2 largest elements */
        if (diff2 < b[2])
            difference = b[2] - b[3] - diff2;
        /* b[2] is biggest */
        else
            difference = diff2 - b[2] - b[3];
        /* diff2 is biggest */
        if (difference < 0)
            difference = -difference;

        /* take absolute value */
        if (difference < alpha)
        /* better than best previous partition */
            alpha = difference;
        return alpha;
    }
    /* reset alpha to better value */

    else{
        rest = total - a[0] - a[1] - b[0];
        /* sum of all elements except first */
        if (b[0] >= rest){
            /* if largest element >= sum of rest */

            if (b[0] - rest < alpha)
            /* best better than best so far */
                alpha = b[0] - rest;

            /* reset alpha to better value */
            return alpha;
        }
            /* if difference is larger than rest, so is the sum */

        if (flag && N - n == 4){
//            printf("thread %d: ----- deep N - n = %d -----\n", thread, N - n);
            para[thread].n = n - 1;
            para[thread].total = total - a[0] - a[1];
            para[thread].thread = thread;
            for (int i = 0; i < n - 1; i++)
                para[thread].a[i] = b[i];
            thread++;
        }
        else
            ckk (b, n-1, total - a[0] - a[1], flag);
        /* one less element, new total */
    }
    /*good enough solution based on maxsofar*/

    sum2 = a[0] + a[1];
    /* sum of largest two numbers */
    if (n == 5){
        /* when only 4 numbers are left, KK is optimal */
        diff2 = sum2 - a[2];
        /* difference of 2 largest elements */
        if (diff2 < a[3])
            difference = a[3] - a[4] - diff2;
        /* a[3] biggest */
        else
            difference = diff2 - a[3] - a[4];
        /* diff2 is biggest */
        if (difference < 0)
            difference = - difference;
        /* take absolute value */
        if (difference < alpha)
        /* better than best previous partition */
            alpha = difference;

            /* reset alpha to better value */
        return alpha;
    }
    /* stop searching and return */

    rest = total - a[0] - a[1];
    /* sum of all elements except first two */
    if (sum2 >= rest){
        /* if largest element >= sum of rest */
        if (sum2 - rest < alpha)
        /* best better than best so far */
            alpha = sum2 - rest;

            /* reset alpha to better value */
        return alpha;
    }
    /* if difference is larger than rest, so is the sum */

    a[1] = sum2;
    /* sum of two largest is new largest element */
    if (flag && N - n == 4){
//        printf("thread %d: ----- deep N - n = %d -----\n", thread, N - n);
        para[thread].n = n - 1;
        para[thread].total = total;
        para[thread].thread = thread;
        for (int i = 0; i < n - 1 ; i++){
            para[thread].a[i] = a[i + 1];
//            printf("&&&&&& %lld &&&&&& ", para[thread].a[i]);
        }
        thread++;
    }
    else
        ckk (a+1, n-1, total, flag);
    /* call on subarray with one less element */
    a[1] = sum2 - a[0];
    /* restore array to previous state */
//    if (N - n == 3 && thread == 16)
//        for(int i = 0; i < thread; i++)
//            pthread_join (thread_handles[i], NULL);
//

    return alpha;
}


//pthread_ckk 02 implement
void *pthread_ckk (void *args){
    func_para *para;
    para = (func_para*) args;
    long long a[MAXN];
    int n;
    long long total;
    n = para->n;
    total = para->total;
//    printf(" = = == = = =%d \n", para->thread);
    for (int i = 0; i < n; i++){
        a[i] = para->a[i];
    }
    ckk (a, n, total, false);
    finished_thread += 1;
    start_parallel = clock();
//    printf("finishied 1 %lf\n", (double)(start_parallel - start_normal)/CLOCKS_PER_SEC/TRIALS);
//    printf(" - %lld --- %d --\n", alpha, para->thread);
    return NULL;
}

/* This is the main function.  It calls init to generate the random numbers,
   then sorts them in decreasing order.  Next it calls KK5 to generate an
   approximate 5-way partition.  If there are more that 7 numbers, or KK5
   doesn't return an optimal partition, then it calls five-way to optimally
   partition the numbers. */

int main (int argc, char *argv[]){
    long long totalmax;
    /* total of all 2-way partition differences */
    long long totalcalls[MAXK+1];
    /* total number of recursive calls for each k */
    int i;
    /* utility index */
    int trial;
    /* number of current trial */
    long long minmax;
    /* the largest subset sum in an optimal solution */

//    sscanf(argv[1], "%d", &K);
    K = 2;
    /* read number of subsets from command line */
//    sscanf(argv[2], "%d", &N);
    N = atoi(argv[1]);
    /* read number of values from command line */
//    sscanf(argv[3], "%d", &TRIALS);
    TRIALS = 10;
    /* read number of values from command line */

    seed = INITSEED;
    /* initialize seed of random number generator */

    totalmax = 0;

    /* initialize total solution cost counter */
    thread_handles = (pthread_t *) malloc (thread_count * sizeof(pthread_t));
    start_normal = clock();
    float time_use=0;
    struct timeval start;
    struct timeval end;

    gettimeofday(&start,NULL); //gettimeofday(&start,&tz);结果一样
    ofstream outputfile;
    outputfile.open ("parallel_and_cut_output.txt", ios::app);
    outputfile << " -------------- " << N << " ------------------ "<<endl;
    for (trial = 1; trial <= TRIALS; trial++){
        finished_thread = 0;
        for (i = 0; i <= N; i++)
        /* initialize N_set */
            N_set[i] = 0;
        thread = 0;

        /* initialize pthread handles */
        /* initialize thread currently */
        alpha = 100000000000;
        /* for each independent trial */
        sumall = init(sorted, N);
        diffthresh = N * C;
        /* generate random values, return their sum */
        sortdown(sorted, N);
        /* sort numbers in decreasing order */


        ckk(sorted, N, sumall, true);
//        for (int i = 0; i < para[31].n; i++)
//            printf("****** %lld ** %d ***\n", para[31].a[i], para[31].thread);

        for (int i = 0; i < 32; i++)
            pthread_create(&thread_handles[i], NULL, pthread_ckk, &para[i]);

        for (int i = 0; i < 1; i++){
            pthread_join(thread_handles[i], NULL);
//            printf("tag_in_join\n");
        }
//        printf("tag_out_join\n");
        minmax = alpha;
//        end_normal = clock();

//        ave_time_normal = ave_time_normal + (double)(end_normal - start_normal);

        totalmax += minmax;
        /* increment total solution cost counter */

        outputfile << "TRIAL : " << trial << " --- " << minmax << endl;
        printf ("%02d %lld\n", trial, minmax);

    } /* output each individual instance */
    printf("-------------------------\n");
    end_normal = clock();
    printf("%d - way trial X %d \nnumbers of integers: %2d \ntotalmax of %d trials: %lld \naverage(trial) - CPU - time: %lf  ", K, TRIALS, N, TRIALS, totalmax, (double)(end_normal - start_normal)/CLOCKS_PER_SEC/TRIALS);
    /* solution statistics for run */
    gettimeofday(&end,NULL);
    time_use=(end.tv_sec-start.tv_sec)*1000000+(end.tv_usec-start.tv_usec);//微秒
    printf("time_use is %.6f\n", time_use/1000000);
    outputfile << " -------------- " << N << " ------------------ "<<endl;
    outputfile << K << " - way trial X " << TRIALS << endl;
    outputfile << "numbers of integers: " << N << endl;
    outputfile << "totalmax of " << TRIALS << " trials: " << totalmax << endl;
    outputfile << "average - CPU - time: " << (double)(end_normal - start_normal)/CLOCKS_PER_SEC/TRIALS << endl;
    outputfile << "the real time is below: " << time_use/1000000<<endl;
    outputfile << endl;
    outputfile.close();
    free(thread_handles);
    return 0;
}
