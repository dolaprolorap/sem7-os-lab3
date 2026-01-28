#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "port.h"

int main() {
	start_main_program(true);

	printf("s - запустить вручную программу, с <number> - установить значение счетчика, e - завершить\n");

	while(1)
	{
		char command[30];
		fgets(command, sizeof(command), stdin);

		if (command[0] == 's' || command[0] == 'S')
		{
			start_main_program(false);
			printf("Вручную запущена программа\n");
		}
		
		if (command[0] == 'c' || command[0] == 'C')
		{
			char* end;
			
			long new_counter = strtol(command + 1, &end, 10);

			if (command + 1 == end) 
			{
				printf("Невалидная команда\n");
			}
			else 
			{
				set_counter(new_counter);
				printf("Вручную установлено значение счетчика: %d\n", new_counter);
			}
		}

		if (command[0] == 'e' || command[0] == 'E')
		{
			break;
		}
	}

	clear_memory();

	return 0;
}

