#include <stdio.h>
#include "Master.h"

int main() {
	struct Master master;
	int id;
	printf("Print 9 for help.\n");
	while (1) {
		int choice;
		printf("\x1b[32mprompt>\x1b[0m ");
		scanf("%d", &choice);
		switch (choice) {
		case 0:
			return 0;
		case 1:			
			printf("Enter master's Id: ");
			scanf("%d", &id);
			MasterTryGetValue(id);
			break;
		case 2:
			MasterInput(&master);
			MasterInsert(master);
			break;
		case 3: 
			printf("Enter master's Id: ");
			scanf("%d", &id);
			MasterUpdate(id);
			break;
		case 4:
			printf("Enter master's Id: ");
			scanf("%d", &id);
			MasterDelete(id);
			break;
		case 9: 
			printf("\x1b[33mHelp:\x1b[0m\n1 - Get Master\n2 - Insert Master\n3 - Update Master\n4 - Delete Master\n5 - Get Slave\n6 - Insert Slave\n7 - Update Slave\n8 - Delete Slave\n9 - Help\n0 - Quit\n");
			break;
		default: 
			printf("\x1b[31m*Invalid command*\x1b[0m\n");
		}
	}
	return 0;
}