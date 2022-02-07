#pragma once
#include <stdlib.h>
#include <stdio.h>
#include "Slave.h" 
#include "Indexer.h"

#define MASTER_DATA "master.fl"
#define MASTER_IND "master.ind"
#define MASTER_GARBAGE "masterGarbage.txt"
#define MASTER_SIZE sizeof(struct Master)

struct Master {
	int id;
	char name[16];
	long firstSlaveAddress;
	int slavesCount;
};

void masterUseFreeSpaces(int , FILE* , struct Master* );

void masterMarkAsDeleted(int );

void MasterOutput(struct Master* );

struct Master* MasterTryGetValue(int , int );

void MasterInput(struct Master* );

void MasterInsert(struct Master );

void MasterUpdate(struct Master , int );

void MasterDelete(int );

