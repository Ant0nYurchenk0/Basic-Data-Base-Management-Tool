#pragma once
#include <stdio.h>
#include <stdlib.h>
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

void masterUseFreeSpaces(int garbageCount, FILE* garbageCollector, struct Master* master) {
	FILE* indexTable = fopen(MASTER_IND, "rb");
	int* deletedIds = malloc(garbageCount * sizeof(int));		// Виділяємо місце під список "сміттєвих" індексів

	for (int i = 0; i < garbageCount; i++)
	{
		fscanf(garbageCollector, "%d", deletedIds + i);				// Заповнюємо його
	}

	master->id = deletedIds[0];									// Для запису замість логічно видаленого "сміттєвого"

	fclose(garbageCollector);									// За допомогою цих двох команд
	fopen(MASTER_GARBAGE, "wb");							// повністю очищуємо файл зі "сміттям"
	fprintf(garbageCollector, "%d", garbageCount - 1);			// Записуємо нову кількість "сміттєвих" індексів

	for (int i = 1; i < garbageCount; i++)
	{
		fprintf(garbageCollector, " %d", deletedIds[i]);				// Записуємо решту "сміттєвих" індексів
	}

	free(deletedIds);											// Звільняємо виділену під масив пам'ять
	fclose(garbageCollector);
}


void masterMarkAsDeleted(id) {
	FILE* garbageCollector = fopen(MASTER_GARBAGE, "rb");		// "rb": відкриваємо бінарний файл для читання

	int garbageCount;
	fscanf(garbageCollector, "%d", &garbageCount);

	int* deletedIds = malloc(garbageCount * sizeof(int));		// Виділяємо місце під список "сміттєвих" індексів

	for (int i = 0; i < garbageCount; i++)
	{
		fscanf(garbageCollector, "%d", deletedIds + i);				// Заповнюємо його
	}
	fclose(garbageCollector);
	garbageCollector = fopen(MASTER_GARBAGE, "wb");				// повністю очищуємо файл зі "сміттям"
	fprintf(garbageCollector, "%d", garbageCount + 1);			// Записуємо нову кількість "сміттєвих" індексів

	for (int i = 0; i < garbageCount; i++)
	{
		fprintf(garbageCollector, " %d", deletedIds[i]);				// Заносимо "сміттєві" індекси назад...
	}

	fprintf(garbageCollector, " %d", id);						// ...і дописуємо до них індекс щойно видаленого запису
	free(deletedIds);											// Звільняємо виділену під масив пам'ять
	fclose(garbageCollector);
}

void MasterOutput(struct Master* master) {
	printf("	Name: %s\n", master->name);
	printf("	Id: %d\n", master->id);
	printf("	SlavesCount: %d\n", master->slavesCount);
}

struct Master* MasterTryGetValue(int id) {
	FILE* indexTable = fopen(MASTER_IND, "rb");				// "rb": відкрити бінарний файл
	FILE* dataBase = fopen(MASTER_DATA, "rb");				// тільки для читання
	struct Indexer indexer;

	if (indexTable == NULL || dataBase == NULL)
	{
		printf("\x1b[31m*Database does not exist*\x1b[0m\n");
		return NULL;
	}

	fseek(indexTable, 0, SEEK_END);

	fseek(indexTable, (id - 1) * INDEXER_SIZE, SEEK_SET);	// Отримуємо індексатор шуканого запису
	fread(&indexer, INDEXER_SIZE, 1, indexTable);			// за вказаним номером
	if ((!ftell(indexTable) || id * INDEXER_SIZE > ftell(indexTable)) || !indexer.exists)
	{
		printf("\x1b[31m*No record corresponds to this Id*\x1b[0m\n");
		return NULL;
	}

	struct Master master ;
	fseek(dataBase, indexer.address, SEEK_SET);				// Отримуємо шуканий запис з БД-таблички
	fread(&master, sizeof(struct Master), 1, dataBase);		// за знайденою адресою
	fclose(indexTable);										// Закриваємо файли
	fclose(dataBase);

	
	return &master;
}

void MasterInput(struct Master* master) {
	printf("Enter the name of master: ");
	scanf("%s", master->name);
}

void MasterInsert(struct Master master) {
	FILE* indexTable = fopen(MASTER_IND, "a+b");
	FILE* dataBase = fopen(MASTER_DATA, "a+b");
	FILE* garbage = fopen(MASTER_GARBAGE, "rb");
	FILE* garbageCollector = fopen(MASTER_GARBAGE, "rb");
	struct Indexer indexer;
	int garbageCount;

	fscanf(garbageCollector, "%d", &garbageCount);

	if (garbageCount)										// Наявні "сміттєві" записи, перепишемо перший з них
	{
		masterUseFreeSpaces(garbageCount, garbageCollector, &master);

		fclose(indexTable);									// Закриваємо файли для зміни
		fclose(dataBase);									// режиму доступу в подальшому

		indexTable = fopen(MASTER_IND, "r+b");				// Знову відкриваємо і змінюємо режим на
		dataBase = fopen(MASTER_DATA, "r+b");				// "читання з та запис у довільне місце файлу"
		
		fseek(indexTable, (master.id - 1) * INDEXER_SIZE, SEEK_SET);
		fread(&indexer, INDEXER_SIZE, 1, indexTable);
		fseek(dataBase, indexer.address, SEEK_SET);			// Ставимо курсор на "сміття" для подальшого перезапису	
	}
	else {
		fseek(indexTable, 0, SEEK_END);

		if (ftell(indexTable)) {
			int indexerSize = INDEXER_SIZE;
			fseek(indexTable, -indexerSize, SEEK_END);		// Ставимо курсор на останній індексатор
			fread(&indexer, INDEXER_SIZE, 1, indexTable);	// Читаємо останній індексатор

			master.id = indexer.id + 1;						// Нумеруємо запис наступним індексом
		}
		else                                                // Індексна табличка порожня
		{
			master.id = 1;									// Індексуємо наш запис як перший
		}
	}
	master.firstSlaveAddress = -1;
	master.slavesCount = 0;

	fwrite(&master, MASTER_SIZE, 1, dataBase);				// Записуємо в потрібне місце БД-таблички передану структуру

	indexer.id = master.id;									// Вносимо номер запису в індексатор
	indexer.address = (master.id - 1) * MASTER_SIZE;		// Вносимо адресу запису в індексатор
	indexer.exists = 1;										// Прапорець існування запису		

	fseek(indexTable, (master.id - 1) * INDEXER_SIZE, SEEK_SET);
	fwrite(&indexer, INDEXER_SIZE, 1, indexTable);			// Записуємо індексатор у відповідну табличку, куди треба
	fclose(indexTable);										// Закриваємо файли
	fclose(dataBase);
	
	printf("\x1b[33mId of this master:\x1b[0m %d\n", master.id);
}

void MasterUpdate(struct Master master) {


	FILE* indexTable = fopen(MASTER_IND, "r+b");			// "r+b": відкрити бінарний файл
	FILE* database = fopen(MASTER_DATA, "r+b");
	int id = master.id;

	struct Indexer indexer;
	fseek(indexTable, (id - 1) * INDEXER_SIZE, SEEK_SET);	// Отримуємо індексатор шуканого запису
	fread(&indexer, INDEXER_SIZE, 1, indexTable);

	fseek(database, indexer.address, SEEK_SET);				// Позиціонуємося за адресою запису в БД
	fwrite(&master, MASTER_SIZE, 1, database);				// Оновлюємо запис
	fclose(indexTable);										// Закриваємо файли
	fclose(database);


}

void MasterDelete(int id) {
	if (!MasterTryGetValue(id, NULL)) {
		return;
	}
	int responce;
	printf("Are you sure you want to delete this record (1/0)? ");
	scanf("%d", &responce);
	if (responce) {
		FILE* indexTable = fopen(MASTER_IND, "r+b");			// "r+b": відкрити бінарний файл для читання та запису	

		struct Indexer indexer;

		fseek(indexTable, (id - 1) * INDEXER_SIZE, SEEK_SET);	// Отримуємо індексатор шуканого запису
		fread(&indexer, INDEXER_SIZE, 1, indexTable);			// за вказаним номером

		indexer.exists = 0;										// Запис логічно не існуватиме...

		fseek(indexTable, (id - 1) * INDEXER_SIZE, SEEK_SET);

		fwrite(&indexer, INDEXER_SIZE, 1, indexTable);			// ...але фізично буде присутній
		fclose(indexTable);										// Закриваємо файл [NB: якщо не закрити, значення не оновиться]

		masterMarkAsDeleted(id);									// Заносимо індекс видаленого запису до "сміттєвої зони"


		//if (master.slavesCount)								// Були поставки, видаляємо всі
		//{
		//	FILE* slavesDb = fopen(SLAVE_DATA, "r+b");
		//	struct Slave slave;
		//	fseek(slavesDb, master.firstSlaveAddress, SEEK_SET);
		//	for (int i = 0; i < master.slavesCount; i++)
		//	{
		//		fread(&slave, SLAVE_SIZE, 1, slavesDb);
		//		fclose(slavesDb);
		//		deleteSlave(master, slave, slave.productId, error);
		//		slavesDb = fopen(SLAVE_DATA, "r+b");
		//		fseek(slavesDb, slave.nextAddress, SEEK_SET);
		//	}
		//	fclose(slavesDb);
		//}

		printf("\x1b[33m*Deleted*\x1b[0m\n");
	}
}