#include <stdio.h>
#include <conio.h>
#include <process.h>
#include <io.h>
#include <alloc.h>

#define FALSE	0
#define TRUE	1

const char *ui = "ui.exe";
const char *temp_batch = "rloadrun.bat";
const char *err_format = "Error running: %s";

int main(int argc, char *argv[]) {
	char first_run = TRUE;
	char **args = (char **)malloc(sizeof(char *) * argc + 2);
	int i;

	for (i = 0; i < argc; ++i)
		args[i] = argv[i];

	args[argc] = NULL;

	while (1) {
		// Add /nosplash flag to display splash screen only on first opening.
		if (first_run == FALSE) {
			args[argc] = "/nosplash";
			args[argc + 1] = NULL;
		}

		// Lauch user interface.
		if (spawnv(P_WAIT, (char *)ui, args) == -1) {
			free(args);

			// Delete temporary files that may be left on disk.
			if (access(temp_batch, 0) == 0)
				unlink(temp_batch);
			printf(err_format, ui);
			exit(-1);
		}

		/* Check for existence of temporary batch file.
		 * If do not exists, there is no program to be
		 * runned and loop must end.
		 */
		if (access(temp_batch, 0) != 0) {
			free(args);
			exit(0);
		}

		// Run batch testing it for error.
		if (system(temp_batch) != 0) {
			free(args);
			printf(err_format, temp_batch);
			exit(-1);
		}

		first_run = FALSE;
	}
}
