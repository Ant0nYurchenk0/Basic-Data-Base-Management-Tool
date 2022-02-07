#include "Slave.h"
#include <stdio.h>
#include <stdlib.h>
#include "Master.h"

void slaveUseFreeSpaces(int garbageCount, FILE* garbageCollector, struct Slave* record) {
	long* deletedIds = malloc(garbageCount * sizeof(long));		// Виділяємо місце під список "сміттєвих" адрес

	for (int i = 0; i < garbageCount; i++)
	{
		fscanf(garbageCollector, "%d", deletedIds + i);				// Заповнюємо його
	}

	record->self = deletedIds[0];						// Для запису замість логічно видаленої "сміттєвої"
	record->nextSlave = deletedIds[0];

	fclose(garbageCollector);									// За допомогою цих двох команд
	fopen(SLAVE_GARBAGE, "wb");							    // повністю очищуємо файл зі "сміттям"
	fprintf(garbageCollector, "%d", garbageCount - 1);			// Записуємо нову кількість "сміттєвих" адрес

	for (int i = 1; i < garbageCount; i++)
	{
		fprintf(garbageCollector, " %d", deletedIds[i]);				// Записуємо решту "сміттєвих" адрес
	}

	free(deletedIds);											// Звільняємо виділену під масив пам'ять
	fclose(garbageCollector);
}

void slaveRelinkAddresses(FILE* database, struct Slave previous, struct Slave slave, struct Master* master) {
	if (slave.self == master->firstSlaveAddress)		// Немає попередника (перший запис)...
	{
		if (slave.self == slave.nextSlave)			// ...ще й немає наступника (запис лише один)
		{
			master->firstSlaveAddress = -1;					// Неможлива адреса для безпеки
		}
		else                                                // ...а наступник є,
		{
			master->firstSlaveAddress = slave.nextSlave;  // робимо його першим
		}
	}
	else                                                    // Попередник є...
	{
		if (slave.self == slave.nextSlave)			// ...але немає наступника (останній запис)
		{
			previous.nextSlave = previous.self;    // Робимо попередник останнім
		}
		else                                                // ... а разом з ним і наступник
		{
			previous.nextSlave = slave.nextSlave;		// Робимо наступник наступником попередника
		}
		fseek(database, previous.self, SEEK_SET);	// Записуємо оновлений попередник
		fwrite(&previous, SLAVE_SIZE, 1, database);			// назад до таблички
	}

}

void slaveMarkAsDeleted(long adress) {
	FILE* garbageCollector = fopen(SLAVE_GARBAGE, "rb");			// "rb": відкриваємо бінарний файл для читання

	int garbageCount;
	fscanf(garbageCollector, "%d", &garbageCount);

	long* delAddresses = malloc(garbageCount * sizeof(long)); // Виділяємо місце під список "сміттєвих" адрес

	for (int i = 0; i < garbageCount; i++)
	{
		fscanf(garbageCollector, "%ld", delAddresses + i);		// Заповнюємо його
	}

	fclose(garbageCollector);									// За допомогою цих двох команд
	garbageCollector = fopen(SLAVE_GARBAGE, "wb");				// повністю очищуємо файл зі "сміттям"
	fprintf(garbageCollector, "%ld", garbageCount + 1);			// Записуємо нову кількість "сміттєвих" адрес

	for (int i = 0; i < garbageCount; i++)
	{
		fprintf(garbageCollector, " %ld", delAddresses[i]);		// Заносимо "сміттєві" адреси назад...
	}

	fprintf(garbageCollector, " %d", garbageCollector);					// ...і дописуємо до них адресу щойно видаленого запису
	free(delAddresses);										// Звільняємо виділену під масив пам'ять
	fclose(garbageCollector);									// Закриваємо файл
}

void SlaveInput(struct Slave* slave) {
	printf("Enter the name of slave: ");
	scanf("%s", slave->name);
}

void SlaveOutput(struct Slave* slave) {
	printf("	Name: %s\n", slave->name);
	printf("	Id: %d\n", slave->id);
	printf("	MasterId: %d\n", slave->masterId);
}

void SlaveUpdate(struct Slave slave) {
	FILE* dataBase = fopen(SLAVE_DATA, "r+b");
	struct Slave oldSlave;
	fseek(dataBase, slave.self, SEEK_SET);
	fread(&oldSlave, SLAVE_SIZE, 1, dataBase);
	slave.exists = oldSlave.exists;
	slave.self = oldSlave.self;
	slave.nextSlave = oldSlave.nextSlave;
	fseek(dataBase, slave.self, SEEK_SET);
	fwrite(&slave, SLAVE_SIZE, 1, dataBase);
	fclose(dataBase);

	return 1;
}

struct Slave* SlaveTryGetValue(int id, int masterId) {
	struct Master master = *MasterTryGetValue(masterId, 0);
	struct Slave slave;
	if (!master.slavesCount)									// У постачальника немає поставок
	{
		printf("\x1b[31m*This master has no slaves yet*\x1b[0m\n");
		return NULL;
	}

	FILE* dataBase = fopen(SLAVE_DATA, "rb");

	if (dataBase == NULL) {
		printf("\x1b[31m*Database does not exist*\x1b[0m\n");
		return;
	}

	fseek(dataBase, master.firstSlaveAddress, SEEK_SET);		// Отримуємо перший запис
	fread(&slave, SLAVE_SIZE, 1, dataBase);

	for (int i = 0; i < master.slavesCount; i++)				// Шукаємо потрібний запис по коду деталі
	{
		if (slave.id == id)						// Знайшли
		{
			fclose(dataBase);
			SlaveOutput(&slave);
			return &slave;
		}

		fseek(dataBase, slave.nextSlave, SEEK_SET);
		fread(&slave, SLAVE_SIZE, 1, dataBase);
	}

	printf("\x1b[31m*No such slave in database*\x1b[0m\n");					// Не знайшли
	fclose(dataBase);

	return NULL;
}

void SlaveInsert(struct Slave slave) {
	slave.exists = 1;

	FILE* dataBase = fopen(SLAVE_DATA, "a+b");
	FILE* garbageCollector = fopen(SLAVE_GARBAGE, "rb");
	int garbageCount;
	struct Master master = *MasterTryGetValue(slave.masterId, 0);
	fscanf(garbageCollector, "%d", &garbageCount);
	if (0)											// Наявні видалені записи
	{
		slaveUseFreeSpaces(garbageCount, garbageCollector, &slave);
		fclose(dataBase);
		dataBase = fopen(SLAVE_DATA, "r+b");								// Змінюємо режим доступу файлу
		fseek(dataBase, slave.self, SEEK_SET);			// Ставимо курсор на "сміття" для подальшого перезапису
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
	MasterUpdate(master, 1);
	printf("\x1b[33mId of this slave:\x1b[0m %d\n", slave.id);
}

void SlaveDelete(int id, int masterId) {
	FILE* dataBase = fopen(SLAVE_DATA, "r+b");
	struct Slave slave = *SlaveTryGetValue(id, 0);
	struct Master master = *MasterTryGetValue(masterId, 0);
	struct Slave previous;
	fseek(dataBase, master.firstSlaveAddress, SEEK_SET);

	do		                                                    // Шукаємо попередника запису (його може й не бути,
	{															// тоді в попередника занесеться сам запис)
		fread(&previous, SLAVE_SIZE, 1, dataBase);
		fseek(dataBase, previous.nextSlave, SEEK_SET);
	} while (previous.nextSlave != slave.self && slave.self != master.firstSlaveAddress);

	slaveRelinkAddresses(dataBase, previous, slave, &master);
	slaveMarkAsDeleted(slave.self);						// Заносимо адресу видаленого запису у "смітник"

	slave.exists = 0;											// Логічно не існуватиме

	fseek(dataBase, slave.self, SEEK_SET);				// ...але фізично
	fwrite(&slave, SLAVE_SIZE, 1, dataBase);					// записуємо назад
	fclose(dataBase);

	master.slavesCount--;										// Однією поставкою менше
	MasterUpdate(master, 1);
	printf("\x1b[33m*Deleted*\x1b[0m\n");
}