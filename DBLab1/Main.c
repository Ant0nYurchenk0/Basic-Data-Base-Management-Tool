#include <stdio.h>
#include "Slave.h"
#include "Master.h"

int main() {
	struct Master master;
	struct Slave slave;
	int id;
	int slaveId;
	int choice;
	printf("Print 9 for help.\n");
	while (1) {
		printf("\x1b[32mprompt>\x1b[0m ");
		scanf("%d", &choice);

		switch (choice) {
		case 0:
			return 0;
		case 1:			
			printf("Enter master's Id: ");
			scanf("%d", &id);
			MasterOutput( MasterTryGetValue(id));
			break;
		case 2:
			MasterInput(&master);
			MasterInsert(master);
			break;
		case 3: 
			printf("Enter master's Id: ");
			scanf("%d", &id);
			MasterTryGetValue(id, NULL);
			master.id = id;
			MasterInput(&master);
			MasterUpdate(master);
			printf("\x1b[33m*Updated*\x1b[0m\n");
			MasterOutput(&master);
			break;
		case 4:
			printf("Enter master's Id: ");
			scanf("%d", &id);
			MasterDelete(id);
			break;
		case 5:
			printf("Enter master's ID: ");
			scanf("%d", &id);
			if (MasterTryGetValue(id))
			{
				printf("Enter slave's ID: ");
				scanf("%d", &slaveId);
				SlaveOutput(SlaveTryGetValue(slaveId, id));
			}
			break;
		case 6:
			printf("Enter master's Id: ");
			scanf("%d", &id);
			if (MasterTryGetValue(id)) {
				slave.masterId = id;
				SlaveInput(&slave);
				SlaveInsert(slave);
			}
			break;
		case 7:
			printf("Enter master's ID: ");
			scanf("%d", &id);
			if (MasterTryGetValue(id))
			{
				printf("Enter slave's ID: ");
				scanf("%d", &id);

				if (SlaveTryGetValue(slaveId, id))
				{
					SlaveInput(&slave);
					SlaveUpdate(slave);
					printf("\x1b[33m*Updated*\x1b[0m\n");
					SlaveOutput(&slave);
				}
			}
			break;
		case 8:
			break;
		case 9: 
			printf("\x1b[33mHelp:\x1b[0m\n1 - Get Master\n2 - Insert Master\n3 - Update Master\n4 - Delete Master\n5 - Get Slave\n6 - Insert Slave\n7 - Update Slave\n8 - Delete Slave\n9 - Help\n0 - Quit\n");
			break;
		default: 
			printf("\x1b[31m*Invalid command*\x1b[0m\n");
			break;
		}
	}
	return 0;
}