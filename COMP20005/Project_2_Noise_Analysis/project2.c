/* Program to create sound-level maps.

   Skeleton written by Alistair Moffat, ammoffat@unimelb.edu.au,
   April 2023, with the intention that it be modified by students
   to add functionality, as required by the assignment specification.

   Student Authorship Declaration:

   (1) I certify that except for the code provided in the initial skeleton
   file, the  program contained in this submission is completely my own
   individual work, except where explicitly noted by further comments that
   provide details otherwise.  I understand that work that has been developed
   by another student, or by me in collaboration with other students, or by
   non-students as a result of request, solicitation, or payment, may not be
   submitted for assessment in this subject.  I understand that submitting for
   assessment work developed by or in collaboration with other students or
   non-students constitutes Academic Misconduct, and may be penalized by mark
   deductions, or by other penalties determined via the University of
   Melbourne Academic Honesty Policy, as described at
   https://academicintegrity.unimelb.edu.au.

   (2) I also certify that I have not provided a copy of this work in either
   softcopy or hardcopy or any other form to any other student, and nor will I
   do so until after the marks are released. I understand that providing my
   work to other students, regardless of my intention or any undertakings made
   to me by that other student, is also Academic Misconduct.

   (3) I further understand that providing a copy of the assignment
   specification to any form of code authoring or assignment tutoring service,
   or drawing the attention of others to such services and code that may have
   been made available via such a service, may be regarded as Student General
   Misconduct (interfering with the teaching activities of the University
   and/or inciting others to commit Academic Misconduct).  I understand that
   an allegation of Student General Misconduct may arise regardless of whether
   or not I personally make use of such solutions or sought benefit from such
   actions.

   Signed by: Kim CHY
   Dated:     12/05/2023

*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#define MAXDATA 100
#define NUM_DATA_PER_LINE 3
#define P0 0.00002
#define PI 3.141592653589793238
#define W0 0.000000000001
#define MAX_X 74
#define MAX_Y 100
#define NUM_GRID 3
#define Q 2
#define ALPHA 0.5
#define ORIGIN 0
#define GRID_SIZE {1,0.5,0.25}
#define DANGER_SOUND_LEVEL 80
#define SMALLEST_GRID 0.25
#define TOTAL_X_GRID ((int)(MAX_X/SMALLEST_GRID+1))
#define TOTAL_Y_GRID ((int)(MAX_Y/SMALLEST_GRID+1))
#define ONE 1
#define TEN 10
#define SOUND_LOWER_BOUND 20
#define SOUND_UPPER_BOUND 90


//Defining a struct for our data;
typedef struct {
    double x_coord, y_coord, power;
    double dis_orign, SPI_orign;
} DATA_t;

typedef struct {
    double sound_power;
} SPLi_DATA_t;


//Protoyping Function
int read_data(DATA_t sound[MAXDATA]);
void do_section1(DATA_t sound[], int num_source);
double comp_dist(double x1, double y1, double x2, double y2);
double sound_pow(double Wi);
double surface_absorption(double r);
double SPLi(double r, double Wi);
void do_section2(SPLi_DATA_t zone[][TOTAL_X_GRID][TOTAL_Y_GRID],
                     DATA_t sound[], int num_source);
void comp_SPLi_total(SPLi_DATA_t zone[][TOTAL_X_GRID][TOTAL_Y_GRID],
                     DATA_t sound[], int num_source);
int count_danger_zone(SPLi_DATA_t zone[][TOTAL_X_GRID][TOTAL_Y_GRID], 
                     double grid_size);
double compute_percentage(double n, double sample_size);
int SPLtoS3Value(double n);
void do_section3(SPLi_DATA_t zone[][TOTAL_X_GRID][TOTAL_Y_GRID]);


int main(int argc, char *argv[]) {

    DATA_t sound[MAXDATA];
    int num_source = read_data(sound);
    do_section1(sound, num_source);

    printf("\n");
    //Defining index for SPLi Total
    SPLi_DATA_t zone_25cm[ONE][TOTAL_X_GRID][TOTAL_Y_GRID];

    do_section2(zone_25cm,sound,num_source);
    printf("\n");

    do_section3(zone_25cm);

    printf("\nta daa!\n");
}

void do_section1(DATA_t sound[], int num_source){
       for(int i = 0; i<num_source;i++){
        sound[i].dis_orign =comp_dist(ORIGIN,ORIGIN,sound[i].x_coord,
                                             sound[i].y_coord);
        sound[i].SPI_orign = SPLi(sound[i].dis_orign,sound[i].power);
        
    }


    printf("S1, number of sound sources: %d\n", num_source);

    for(int index = 0; index<num_source; index++){
        printf("S1, %4.1lfm E, %4.1lfm N, power %.5lfW, contributes %.1lf dB at"
                " origin\n", sound[index].x_coord,sound[index].y_coord, 
                sound[index].power, sound[index].SPI_orign);
    }
}


void do_section2(SPLi_DATA_t zone[][TOTAL_X_GRID][TOTAL_Y_GRID],
                     DATA_t sound[], int num_source){
    
    double grid_val[NUM_GRID] = GRID_SIZE;
    int danger_grid, total_grid;
    double percentage;
    comp_SPLi_total(zone,sound,num_source);

    for(int i = 0; i<NUM_GRID;i++){
        total_grid = (MAX_X/grid_val[i]+1)*(MAX_Y/grid_val[i]+1);
        danger_grid = count_danger_zone(zone,grid_val[i]);
        percentage = compute_percentage(danger_grid,total_grid);
        printf("S2, grid = %.2lfm, danger points = %6i / %6i = %.2lf%%\n", 
              grid_val[i], danger_grid, total_grid, percentage);
    }

}

void do_section3(SPLi_DATA_t zone[][TOTAL_X_GRID][TOTAL_Y_GRID]){
    
    //Setting Starting Print Position and grid size increment.
    int x_start_pos = 2;
    int y_start_pos = TOTAL_Y_GRID-5;
    int x_grid_increment = 1/SMALLEST_GRID;
    int y_grid_increment = 2/SMALLEST_GRID;
    
    int val;
    for(int y = y_start_pos; y>= ORIGIN; y-= y_grid_increment){
    printf("S3, ");
    for(int x = x_start_pos ; x<TOTAL_X_GRID; x+=x_grid_increment)
        {
            val = SPLtoS3Value(zone[ORIGIN][x][y].sound_power);
            if(val != 0){
                printf("%i",val);
             } else {
                printf(" ");
             }
            
        }
           printf("\n");
        }

}

//Scan the data into our array and return number line for our data
int read_data(DATA_t sound[MAXDATA]){
    int pos = 0;
    while(scanf("%lf%lf%lf", &sound[pos].x_coord, &sound[pos].y_coord,
          &sound[pos].power) == NUM_DATA_PER_LINE){
         pos++;
    }
    return pos;
}

//computing distance between two point
double comp_dist(double x1, double y1, double x2, double y2){
    double d1 = x1-x2;
    double d2 = y1-y2;
    return sqrt(pow(d1,2)+pow(d2,2));
}

//Computing strength of sound base on the formula given
double sound_pow(double Wi){
    return 10*log10(Wi/W0);
}


//Computing surface_absorption base on the formula given
double surface_absorption(double r){
    return (2+ALPHA)*PI*pow(r,2);
}

//Computing SPLi base on the formula given
double SPLi(double r, double Wi){
    
    double POWi = sound_pow(Wi);
    if(r == 0) return HUGE_VAL;

    double R = surface_absorption(r);
    double x = Q/(4*PI*pow(r,2))+4/R;
    return POWi + 10*log10(x);
}


void comp_SPLi_total(SPLi_DATA_t zone[][TOTAL_X_GRID][TOTAL_Y_GRID],
                     DATA_t sound[], int num_source){

    double r, x_val, y_val, Wi, Cur_SPL, Sum_10_SPL;
    //Computing SPL tatal for each grid base on the smallest grid size
    for(int x = 0; x<TOTAL_X_GRID;x++){
        x_val = x*SMALLEST_GRID;
        for(int y = 0; y<TOTAL_Y_GRID;y++){
            y_val = y*SMALLEST_GRID;
            Sum_10_SPL = 0;
            for(int i=0;i<num_source;i++){
                r = comp_dist(sound[i].x_coord,sound[i].y_coord,x_val,y_val);
                /*If it is a source location set to infinite value else
                compute SPL as normal*/
                if(r==0){
                    Sum_10_SPL = HUGE_VAL;
                    break;
                }
                Wi = sound[i].power;
                Cur_SPL = SPLi(r,Wi);
                Sum_10_SPL += pow(TEN,Cur_SPL/TEN);
            }

            zone[0][x][y].sound_power = 10*log10(Sum_10_SPL);
        }
    }
}

//Counting Number of danger zone base grid size
int count_danger_zone(SPLi_DATA_t zone[][TOTAL_X_GRID][TOTAL_Y_GRID], 
                     double grid_size){
    if(grid_size < SMALLEST_GRID) return 0;

    int count_size = grid_size/SMALLEST_GRID;
    int count = 0;
    for(int x=0; x<TOTAL_X_GRID; x += count_size){
        for(int y=0; y<TOTAL_Y_GRID; y += count_size){
            if(zone[ORIGIN][x][y].sound_power >= DANGER_SOUND_LEVEL) count++;
        }
    }
    return count;
}

//Simple percentage calcuator
double compute_percentage(double n, double sample_size){
    return n/sample_size*100;
}


//Decoding function for our print value in section 3
int SPLtoS3Value(double n) {
    int ans = 0;
    double factor = 5;
    if(n >= SOUND_UPPER_BOUND){
        ans = SOUND_UPPER_BOUND/TEN;
    }else if(n >= SOUND_UPPER_BOUND){
        if(fmod(n, TEN) < factor) ans = (int)(n / TEN);
    }
    return ans;
}

//Programming is fun!