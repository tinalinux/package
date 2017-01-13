#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

long long atoll_s(const char *nptr)
{
	int pos;
	char suffix;
	long long factor = 1;
	long long value;

	if ((pos = strlen (nptr) - 1) < 0) {
		fprintf(stderr, "invalid string\n");
		exit (1);
    }
	switch (suffix = nptr[pos]) {
		case 's':
		case 'S':
			factor = 1;
			break;
		case 'm':
		case 'M':
		factor = 60;
		break;
		case 'h':
		case 'H':
		factor = 60 * 60;
			break;
	case 'd':
	case 'D':
		factor = 60 * 60 * 24;
		break;
	case 'y':
	case 'Y':
		factor = 60 * 60 * 24 * 365;
			break;
		default:
		if (suffix < '0' || suffix > '9') {
			fprintf(stderr, "unrecognized suffix: %c\n", suffix);
			exit (1);
		}
	}

		if (sscanf (nptr, "%lli", &value) != 1) {
		fprintf(stderr, "invalid number: %s\n", nptr);
		exit (1);
	}

		value = value * factor;
		return value;
}
long long atoll_b (const char *nptr)
{
	int pos;
	char suffix;
	long long factor = 0;
	long long value;

	if ((pos = strlen (nptr) - 1) < 0) {
		fprintf(stderr, "invalid string\n");
		exit (1);
	}

	switch (suffix = nptr[pos])
	{
		case 'b':
		case 'B':
			factor = 0;
			break;
		case 'k':
		case 'K':
			factor = 10;
		break;
		case 'm':
		case 'M':
			factor = 20;
			break;
		case 'g':
		case 'G':
			factor = 30;
			break;
			default:
		if (suffix < '0' || suffix > '9') {
			fprintf(stderr, "unrecognized suffix: %c\n", suffix);
			exit (1);
		}
	}

	if (sscanf (nptr, "%lli", &value) != 1) {
		fprintf(stderr, "invalid number: %s\n", nptr);
		exit (1);
	}

	value = value << factor;

	return value;
}
