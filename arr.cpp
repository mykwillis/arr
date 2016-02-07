#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

#define WORLD_SIZE  59

int WorldA[WORLD_SIZE];
int WorldB[WORLD_SIZE];
int WorldSnapshot[WORLD_SIZE];
int WorldSnapshotGeneration;
int CycleDetectedGeneration;
int *World;
int Generation;
int PulsePeriod;
int Law[2][2];
int Physics;    // 0-15

int PrintInterval;

//
// USING CURRENT STATE + STATE OF LEFT NEIGHBOR:
//
//
// 0 - everything's zero
// 0001 - All the randomness gets pushed off to the right, while the remaining
//          pattern settles to alternate generations of all 0's and all 1's
// 0010 - Everything steps to the right each generation.  If the leftmost cell
//          is 1, this settles into an alternating 0101010 pattern that inverts 
//          every generation.  IF the leftmost is 0, this goes to zero.
//          If any 1 cells are adjacent to other 1 cells, only the right-most
//          1 will survive.
// 0011 - The initial random pattern is inverted on every generation.
//
// 0100 - After the first generation, any 1 cells that are to the right of another
//          1 cell will become zero.  The world stays stable after the first generation.
//
// 0101 - The inverse of every cell moves to the right every generation.
//          This results in a "101010" pattern growing from the
//          left and eventually taking over.
// 0110 - When a 1 is to the left of a 0, the 1 'wins' and moves over to the 0's
//          spot.  When on the right of a 0, the 1 will simply hold its ground.
//          this configuration creates upside-down triangles frequently whose border
//          is 1's and whose interior is all 0.  This appears to happen because groups
//          of 1's will kill each other off, leaving a pool of 0's that will be invaded
//          from the left.  Pretty long cycle; creates relatively complex patterns.
// 0111 - initial randomness is eaten by alternating generations of 00000.. and 1111...
// 1000 - After a brief struggle by the 1's, the 0's win.
// 1001 - Inverse of 0110; the triangles have 1's in the middle.
// 1010 - While we would expect the inverse of 0101, this physics results in all 1's
//          or all 0's winning after so many generations.
// 1011 - Everything goes to the right.  A leader of 1 results in all 1's winning,
//          while a leader of 0 results in an inversing 101010 pattern.  Inverse
//          to 0010?
// 1100 - Identity.
// 1101 - The first generation sees all 0's that are to the right of another 0 being
//          obliterated.  After that, identity.  Inverse to 0100?
// 1110 - The 1's win after brief scuffle.  Inverse of 1000.
// 1111 - The 1's dominate (obviously).
//


//
// USING LEFT AND RIGHT NEIGHBOR, AND NOT CURRENT STATE
//
// again, 6 & 9 are the most complex.  Some of the other ones exhibit slightly
//  less boring behavior as above, but mostly limited to simple alternations.
//
// 1001 (9) - become a 1 only if I'm next to either two 1's or two 0's.
//


void
GenerateRandomWorld()
{
    World = (int *)WorldA;

    srand( (unsigned)time(NULL) );

    for (int i=0;i<WORLD_SIZE;i++)  {
        World[i] = rand()%2;
    }
    WorldB[0] = World[0];
    WorldB[WORLD_SIZE-1] = World[WORLD_SIZE-1];

    Law[0][0] = (Physics & 1) >> 0;
    Law[0][1] = (Physics & 2) >> 1;
    Law[1][0] = (Physics & 4) >> 2;
    Law[1][1] = (Physics & 8) >> 3;

    Generation = 0;
}

void
AgeWorldByLeftHandedUniformPhysics()
{
    int *OldWorld;

    Generation++;

    if ( Generation % 2 )  {
        OldWorld = (int *)WorldA;
        World = (int *)WorldB;
    } else {
        OldWorld = (int *)WorldB;
        World = (int *)WorldA;
    }

    if ( PulsePeriod && ( Generation % PulsePeriod == 0 ) )  {
        World[0] = !OldWorld[0];
        OldWorld[0] = World[0];
    }


    for (int i=1;i<WORLD_SIZE;i++)  { 
        int CellValue;
        int NeighborCellValue;

        CellValue = OldWorld[i];
        NeighborCellValue = OldWorld[i-1];

        World[i] = Law[CellValue][NeighborCellValue];
    }

}


void
AgeWorldByUniformPhysics()
{
    int *OldWorld;

    Generation++;

    if ( Generation % 2 )  {
        OldWorld = (int *)WorldA;
        World = (int *)WorldB;
    } else {
        OldWorld = (int *)WorldB;
        World = (int *)WorldA;
    }

    if ( PulsePeriod && ( Generation % PulsePeriod == 0 ) )  {
        World[0] = !OldWorld[0];
        OldWorld[0] = World[0];
    }


    for (int i=1;i<WORLD_SIZE-1;i++)  { 
        int LeftCellValue;
        int RightCellValue;

        LeftCellValue = OldWorld[i-1];
        RightCellValue = OldWorld[i+1];

        World[i] = Law[LeftCellValue][RightCellValue];
    }

}



void
PrintWorld()
{
    printf("%08d: ", Generation);

    for (int i = 0;i<WORLD_SIZE;i++)    {
        printf("%1d", World[i]);
    }    
    printf("\n");
}

int
CompareWorlds(
    int *w1,
    int *w2
    )
{
    for (int i=0;i<WORLD_SIZE;i++)  {
        if ( w1[i] != w2[i] ) {
            return -1;
        }
    }

    return 0;
}

void
CheckForCycle()
{
    if ( Generation < WorldSnapshotGeneration )
        return;

    if ( CycleDetectedGeneration != 0 )
        return;

    if ( Generation == WorldSnapshotGeneration )    {
        for (int i=0;i<WORLD_SIZE;i++)  {
            WorldSnapshot[i] = World[i];
        }
        return;
    }

    if ( !CompareWorlds( WorldSnapshot, World ) )    {
        CycleDetectedGeneration = Generation;
    }
}

int main(int argc, char **argv)
{
    int c;
    int i=0;

    Physics = 1;
    PulsePeriod = 0;

    WorldSnapshotGeneration = 1000;
    CycleDetectedGeneration = 0;
    PrintInterval = 5000;

    if ( argc > 1 )
        Physics = atoi( argv[1] );
    if ( argc > 2 )
        PulsePeriod = atoi( argv[2] );

    printf( "Using physics %d.\n", Physics);

    GenerateRandomWorld();

    do {

        if ( !(Generation % PrintInterval) )
            PrintWorld();

        AgeWorldByUniformPhysics();

        CheckForCycle();

        if ( _kbhit() )
            c = _getch();

        if ( c == ' ' ) {
            c = _getch();
        }

    //} while ( !_kbhit() || _getch() != 'q' );
    } while ( c != 'q' && CycleDetectedGeneration == 0 );

    printf("Physics used:          %d\n", Physics);
    printf("Number of generations: %d\n", Generation);
    if ( CycleDetectedGeneration != 0 ) {
        printf("World at generation %d repeated at generation %d\n", 
            WorldSnapshotGeneration, CycleDetectedGeneration);
    }

    return 0;
}