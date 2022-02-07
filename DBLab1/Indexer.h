#pragma once
#define INDEXER_SIZE sizeof(struct Indexer)

struct Indexer
{
	int id;
	int address;
	int exists;
};