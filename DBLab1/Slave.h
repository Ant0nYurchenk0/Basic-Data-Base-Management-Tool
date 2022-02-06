#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "Indexer.h"
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

void SlaveInput(struct Slave* slave) {
	printf("Enter the name of slave: ");
	scanf("%s", slave->name);
}

void SlaveOutput(struct Slave* slave) {
	printf("	Name: %s\n", slave->name);
	printf("	Id: %d\n", slave->id);
	printf("	MasterId: %d\n", slave->masterId);
}

struct Slave* SlaveTryGetValue(int id, int masterId) {
	struct Master master = *MasterTryGetValue(masterId);
	struct Slave slave;
	if (!master.slavesCount)									// У постачальника немає поставок
	{
		printf("\x1b[31m*This master has no slaves yet*\x1b[0m");
		return NULL;
	}

	FILE* dataBase = fopen(SLAVE_DATA, "rb");


	fseek(dataBase, master.firstSlaveAddress, SEEK_SET);		// Отримуємо перший запис
	fread(&slave, SLAVE_SIZE, 1, dataBase);

	for (int i = 0; i < master.slavesCount; i++)				// Шукаємо потрібний запис по коду деталі
	{
		if (slave.id == id)						// Знайшли
		{
			fclose(dataBase);			
			return &slave;
		}

		fseek(dataBase, slave.nextSlave, SEEK_SET);
		fread(&slave, SLAVE_SIZE, 1, dataBase);
	}

	printf("\x1b[31m*No such slave in database*\x1b[0m");					// Не знайшли
	fclose(dataBase);

	return NULL;
}

void SlaveInsert(struct Slave slave) {
	slave.exists = 1;

	FILE* dataBase = fopen(SLAVE_DATA, "a+b");
	FILE* garbageCollector = fopen(SLAVE_GARBAGE, "rb");	
	int garbageCount;
	struct Master master = *MasterTryGetValue(slave.masterId);
	fscanf(garbageCollector, "%d", &garbageCount);
	if (0)											// Наявні видалені записи
	{
		//overwriteGarbageAddress(garbageCount, garbageZone, &slave);
		//reopenDatabase(database);								// Змінюємо режим доступу файлу
		//fseek(database, slave.selfAddress, SEEK_SET);			// Ставимо курсор на "сміття" для подальшого перезапису
	}
	else                                                        // Видалених немає, пишемо в кінець файлу
	{
		fseek(dataBase, 0, SEEK_END);
		int dbSize = ftell(dataBase);
		slave.self = dbSize;
		slave.nextSlave = dbSize;
	}
	slave.id = master.slavesCount + 1;
	fwrite(&slave, SLAVE_SIZE, 1, dataBase);					// Записуємо поставку до свого файлу
	if (!master.slavesCount)								    // Поставок ще немає, пишемо адресу першої
	{
		master.firstSlaveAddress = slave.self;
	}
	else                                                        // Поставки вже є, оновлюємо "адресу наступника" останньої
	{
		fclose(dataBase);
		dataBase = fopen(SLAVE_DATA, "r+b");
		struct Slave previous;
		fseek(dataBase, master.firstSlaveAddress, SEEK_SET);
		for (int i = 0; i < master.slavesCount; i++)		    // Пробігаємомо зв'язаний список до останньої поставки
		{
			fread(&previous, SLAVE_SIZE, 1, dataBase);
			fseek(dataBase, previous.nextSlave, SEEK_SET);
		}
		previous.nextSlave = slave.self;						// Зв'язуємо адреси
		fwrite(&previous, SLAVE_SIZE, 1, dataBase);				// Заносимо оновлений запис назад до файлу
	}
	fclose(dataBase);	
	fclose(garbageCollector);									// Закриваємо файл
	master.slavesCount++;										// Стало на одну поставку більше
	MasterUpdate(master);
	printf("\x1b[33mId of this slave:\x1b[0m %d\n", slave.id);
}