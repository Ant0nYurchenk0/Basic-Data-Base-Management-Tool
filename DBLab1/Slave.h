#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "Master.h"

#define SLAVE_DATA "slave.fl"
#define SLAVE_IND "slave.ind"
#define SLAVE_GARBAGE "slaveGarbage.txt"
#define SLAVE_SIZE sizeof(struct Slave)

struct Slave {
	int id;
	int masterId;
	char name[16];
	int exists;
	long self;
	long nextSlave;
};

void slaveUseFreeSpaces(int , FILE* , struct Slave* );

void slaveRelinkAddresses(FILE*, struct Slave, struct Slave, struct Master*);

void slaveMarkAsDeleted(long );							

void SlaveInput(struct Slave* );

void SlaveOutput(struct Slave* );

void SlaveUpdate(struct Slave );

struct Slave* SlaveTryGetValue(int id, int masterId);

void SlaveInsert(struct Slave );

void SlaveDelete(int , int );

