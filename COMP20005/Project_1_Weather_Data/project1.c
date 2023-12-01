/* Program to analyze weather station data.

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

   Signed by: [Kim Seang CHY 998008]
   Dated:     [16/04/2023]

*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

/*******************************************************************/

#define MAX_DATA 99999
#define S1 "S1, "


int section1();

typedef struct {
    int year;
    int month;
    int day;
    int hour;
    int min;
    int solar;
    float wind;
    float temp;
} DATA;




DATA weather[MAX_DATA];

int main(void)
{
    
    int line = section1();


    //Average Calculator
    int Avg_Solar[12][24];

    for (int month = 1; month <= 12; month++)
        {
          
            for(int hour = 0; hour < 24; hour++)
                {  
                    int n = 0;
                    float sum = 0;
                    for(int i = 0; i < line; i++)
                    {
                        if((weather[i].month) == month && (weather[i].hour == hour))
                        {
                            n++;
                            sum += weather[i].solar;
                        }
                    }

                    Avg_Solar[month-1][hour] = (n==0) ? sum: round(sum/n);
                }
        }


}



int section1()
{

    //Define Starting Line of start and initial variable
    int start = 0, max_solar =0;
    float max_wind =0 , max_temp = 0;

    //Burn the First Line
    int ch;
    while ((ch = getchar()) != '\n');

 

    //Counter Number of Line and scanning data.
    int line = 0;
    while (1) {

        scanf("%d%d%d%d%d%d%f%f",
              &weather[line].year, &weather[line].month, &weather[line].day,
              &weather[line].hour, &weather[line].min, 
              &weather[line].solar,&weather[line].wind, &weather[line].temp);
            
            if(max_solar < weather[line].solar) max_solar = weather[line].solar;
            if(max_wind < weather[line].wind) max_wind = weather[line].wind;
            if(max_temp < weather[line].temp) max_temp = weather[line].temp;
            
            if((ch=getchar()) == EOF) break;
            line++;
    }

    printf(S1"%i data rows in total\n", line);

    printf(S1"row %5i: at %02i:%02i on %02i/%02i/%4i, solar =%5i, wind = %6.2f, temp = %5.1f\n",
            start, weather[start].hour,weather[start].min, 
            weather[start].day, weather[start].month, weather[start].year,
            weather[start].solar, weather[start].wind, weather[start].temp);

    printf(S1"row %5i: at %02i:%02i on %02i/%02i/%4i, solar =%5i, wind = %6.2f, temp = %5.1f\n",
            line-1, weather[line-1].hour,weather[line-1].min, 
            weather[line-1].day, weather[line-1].month, weather[line-1].year,
            weather[line-1].solar, weather[line-1].wind, weather[line-1].temp);

    printf(S1"max solar =%5i\n", max_solar);
    printf(S1"max wind  =%5.1f\n", max_wind);
    printf(S1"max temp  =%5.1f\n", max_temp);

    return line;

}