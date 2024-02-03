#include <iostream>  // cout
#include <stdlib.h>  // rand
#include <math.h>    // sqrt, pow
#include <omp.h>     //OpenMP
#include <string.h>  // memset
#include "Timer.h"
#include "Trip.h"
#include <bits/stdc++.h>
#include<unordered_map>

using namespace std;

// Comparing trips based on the calculated fitness value
int compareTrip(const void *a, const void *b) {
    return ((struct Trip*)a)->fitness - ((struct Trip*)b)->fitness;
}
std::unordered_map<char, char> createComplementMap() {
    std::unordered_map<char, char> complementMap;
    char cit[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    for (int i = 0; i < CITIES; ++i) {
        char complementChar;

        if ((cit[i] - 1) >= 'A') {
            complementChar = cit[CITIES - (cit[i] - 1) - 'A'];
        } else {
            complementChar = cit[CITIES - (cit[i] - 1) - 'A' + 26];
        }

        complementMap[cit[i]] = complementChar;
    }

    return complementMap;
}



// Evaluates the distance of all given trips and sorts them on the basis of their fitness.
void evaluate(Trip trip[CHROMOSOMES], int coordinates[CITIES][2]) {
//Parallelizing
#pragma omp parallel for
    for (int i = 0; i < CHROMOSOMES; ++i) {
        int indexFir,indexSec;
        char citychar[CITIES];
        strncpy(citychar, trip[i].itinerary, CITIES); //copying CITIES number of elements from trip[i].itinerary to citychar
        if(citychar[0]>='A')//Checks if its a character that comes after 'A'
        {
            indexFir= citychar[0] - 'A';
        }
        else{
            indexFir = citychar[0]-'0' + 26;
        }
        double distanceScore = sqrt( pow( (coordinates[indexFir][0]-0),2) + pow( (coordinates[indexFir][1]-0),2)); //Calculating distance between origin and first city using the distance formula.
        for (int j = 1; j < CITIES; ++j) {
            if(citychar[j]>='A'){//Checks if its a character that comes after 'A'
                indexSec = citychar[j] - 'A';
            }
            else{
                indexSec= citychar[j] - '0' + 26;
            }
            
            distanceScore+= sqrt (pow((coordinates[indexFir][0] - coordinates[indexSec][0]),2) + pow((coordinates[indexFir][1] - coordinates[indexSec][1]),2) ); // Calculating distance between the two cities.
            indexFir = indexSec;//assigning the index of second city to first city to proceed with the trip intinerary.
        }
        trip[i].fitness = (float) distanceScore;
    }
    qsort(trip, CHROMOSOMES, sizeof(struct Trip), compareTrip);//sorting the values with shortest trip first.
    
}


// Generate 25000 offsprings

void crossover(Trip parents[TOP_X], Trip offsprings[TOP_X], int coordinates[CITIES][2]) {
//Parallelizing
#pragma omp parallel for
    for (int i = 0; i < TOP_X; i+=2) {
        char child1[CITIES + 1]; 
        char child2[CITIES + 1]; 
        char *parent1 = parents[i].itinerary; 
        char *parent2 = parents[i + 1].itinerary; 
        child1[0] = parent1[0]; 
        int visited[100] = {0};
        visited[child1[0]] = 1;
        const char* found;
        const char* found2;
        int indexA,indexB, indexInPar1, indexInPar2;
        
        
        for (int j = 0; j < (CITIES - 1); j++) {

            found= strchr ( parent1, child1[j]);
            indexInPar1= (int)(found - parent1) + 1;
            found2= strchr ( parent2, child1[j]);
            indexInPar2= (int)(found2 - parent2) + 1;
            float distanceA = FLT_MAX;
            float distanceB = FLT_MAX;
            if (visited[parent1[indexInPar1]] == 0) {//checking if that city is not visited 
            if(child1[j] >= 'A'){
                indexA = child1[j] - 'A';
            }
            else{
                indexA = child1[j] - '0' + 26;
            }
            if( parent1[indexInPar1]>='A'){
                indexB= parent1[indexInPar1] - 'A';
            }
            else{
                indexB= parent1[indexInPar1] - '0' + 26;
            }
                distanceA=(float) sqrt (pow((coordinates[indexA][0] - coordinates[indexB][0]),2) + pow((coordinates[indexA][1] - coordinates[indexB][1]),2) );//calculating distance between two cities. 
            }
            if (visited[parent2[indexInPar2]] == 0) {//checking if that city is not visited

                if(child1[j] >= 'A'){
                indexA = child1[j] - 'A';
            }
            else{
                indexA = child1[j] - '0' + 26;
            }
            if( parent2[indexInPar2]>='A'){
                indexB= parent2[indexInPar2] - 'A';
            }
            else{
                indexB= parent2[indexInPar2] - '0' + 26;
            }
                distanceB=(float) sqrt (pow((coordinates[indexA][0] - coordinates[indexB][0]),2) + pow((coordinates[indexA][1] - coordinates[indexB][1]),2) );//calculating distance between two cities
                
            }
            
            if ( (visited[parent1[indexInPar1]] == 1)&& (visited[parent2[indexInPar2]] == 1)) {//checking if both cities are visited 
                int random = rand()%CITIES;// if both cities are visited, a random number is chosen
                int flag = 1;
                while (flag) {
                    if (visited[parent1[random]] != 1) {
                        flag = 0;
                        child1[j + 1] = parent1[random];
                    }
                    if((random+1)>=CITIES){
                        random=0;
                    }
                    else{
                        random=random+1;
                    }
                }
            } else {
                child1[j + 1] = (distanceA <= distanceB) ? parent1[indexInPar1] : parent2[indexInPar2];
            }
            
            visited[child1[j + 1]] = 1;
        }


    
    std::unordered_map<char, char> complementMap = createComplementMap();
    char* childB = (char *)malloc(CITIES);
    for (int i = 0; i < CITIES; ++i) {
        childB[i] = complementMap[child1[i]];
    }
        strcpy(child2, childB);
        memcpy(offsprings[i].itinerary, child1, CITIES);
        memcpy(offsprings[i + 1].itinerary, child2, CITIES);
    }
}

void mutate(Trip offsprings[TOP_X]) {
    for (int i = 0; i < TOP_X; ++i) {
        int rate = rand() % 100; //choosing a random  number 
        if ( rate < MUTATE_RATE) {
            int pos1 = rand() % CITIES;
            int pos2 = rand() % CITIES;
            char y1 = offsprings[i].itinerary[pos1];
            char y2 = offsprings[i].itinerary[pos2];
            offsprings[i].itinerary[pos1] = y2; //swapping values
            offsprings[i].itinerary[pos2] = y1; //swapping values
        }
    }
}



