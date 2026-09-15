#include <stdio.h>

void dump_hex(FILE *fd, const *doc, const void* data, unsigned int size) {
	char ascii[17];
	unsigned int i, j;
	ascii[16] = '\0';
    fprintf(fd,"%s:\n",doc);
	for (i = 0; i < size; ++i) {
		fprintf(fd,"%02x ", ((unsigned char*)data)[i]);
		if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
			ascii[i % 16] = ((unsigned char*)data)[i];
		} else {
			ascii[i % 16] = '.';
		}
		if ((i+1) % 8 == 0 || i+1 == size) {
			fprintf(fd," ");
			if ((i+1) % 16 == 0) {
				fprintf(fd,"|  %s \n", ascii);
			} else if (i+1 == size) {
				ascii[(i+1) % 16] = '\0';
				if ((i+1) % 16 <= 8) {
					fprintf(fd," ");
				}
				for (j = (i+1) % 16; j < 16; ++j) {
					fprintf(fd,"   ");
				}
				fprintf(fd,"|  %s \n", ascii);
			}
		}
	}
}

int main() {
#if defined(__CPM86__)
    dump_hex(stdout,"ZERO_PAGE",(void *)0x0000,92);
    dump_hex(stdout,"8080",(void*)0x05,1);
    dump_hex(stdout,"LOAD_DRIVE(CP/M>3.0)",(void*)0x50,01);
    dump_hex(stdout,"FCB1",(void*)0x5C,12);
    dump_hex(stdout,"FCB2",(void*)0x6C,12);
    dump_hex(stdout,"TAIL_LENGTH",(void*)0x80,1);
    dump_hex(stdout,"TAIL",(void*)0x81,127);
#else
    dump_hex(stdout,"PSP",(void *)0x0000,92);
    dump_hex(stdout,"CPMEXIT",(void *)0x0000,2);
    dump_hex(stdout,"JMPTERM",(void*)0x05,5);
    dump_hex(stdout,"ENVSEG",(void*)0x2C,2);
    dump_hex(stdout,"VER(DOS>5.0)",(void*)0x40,2);
    dump_hex(stdout,"JMPINT",(void*)0x50,3);
    dump_hex(stdout,"FCB1",(void*)0x5C,12);
    dump_hex(stdout,"FCB2",(void*)0x6C,12);
    dump_hex(stdout,"ARGS_LENGTH",(void*)0x80,1);
    dump_hex(stdout,"ARGS",(void*)0x81,127);
#endif
    return 0;
}

