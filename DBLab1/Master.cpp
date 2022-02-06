#include "Master.h"
#include <stdio.h>
#include <stdlib.h>

void masterUseFreeSpaces(int garbageCount, FILE* garbageCollector, struct Master* master) {
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
