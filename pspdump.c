#include <stdio.h>

void dump_hex(FILE *fd, char *doc, char* data, unsigned int size) {
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


static char psp_buffer[256+16];
static char * psp_target;

#if !defined(__CPM86__)

new_psp_target() {
#asm
	;push ax
	push cx
	push dx
	mov dx, psp_target_
	mov cl, 4
	shr dx, cl
	mov cx, ds
	add dx, cx
	mov ax, dx
	mov ah, 26h
	int 21h
	pop dx
	pop cx
	;pop ax
#endasm
}

#endif

int main(int argc, char *argv[])
 {
	psp_target = 0;
#if defined(__CPM86__)
    dump_hex(stdout,"ZERO_PAGE",psp_target,92);
    dump_hex(stdout,"8080",psp_target+0x05,1);
    dump_hex(stdout,"LOAD_DRIVE(CP/M>3.0)",psp_target+0x50,01);
    dump_hex(stdout,"FCB1",psp_target+0x5C,12);
    dump_hex(stdout,"FCB2",psp_target+0x6C,12);
    dump_hex(stdout,"TAIL_LENGTH",psp_target+0x80,1);
    dump_hex(stdout,"TAIL",psp_target+0x81,127);
#else
	if(argc>1 && (!strcmp(argv[1],"-c") || !strcmp(argv[1],"-C"))) {
		memset(psp_buffer, 0xDD, sizeof(psp_buffer));
		psp_target = psp_buffer;
		psp_target += 16-((int)(psp_target) % 16);
		new_psp_target();
		dump_hex(stdout,"NEW_PSP",psp_target,92);
	} else {
		dump_hex(stdout,"PSP",psp_target,92);
	}
    dump_hex(stdout,"CPMEXIT",psp_target,2);
    dump_hex(stdout,"JMPTERM",psp_target+0x05,5);
    dump_hex(stdout,"ENVSEG",psp_target+0x2C,2);
    dump_hex(stdout,"VER(DOS>5.0)",psp_target+0x40,2);
    dump_hex(stdout,"JMPINT",psp_target+0x50,3);
    dump_hex(stdout,"FCB1",psp_target+0x5C,12);
    dump_hex(stdout,"FCB2",psp_target+0x6C,12);
    dump_hex(stdout,"ARGS_LENGTH",psp_target+0x80,1);
    dump_hex(stdout,"ARGS",psp_target+0x81,127);
#endif
    return 0;
}

