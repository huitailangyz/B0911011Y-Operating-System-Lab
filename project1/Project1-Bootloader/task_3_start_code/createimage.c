#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


uint8_t extend = 0;
uint16_t num_section=0;
uint32_t base;

void write_bootblock(FILE **image_file, FILE **boot_file)
{
	FILE *image=*image_file;
	FILE *boot=*boot_file;
	uint8_t buf[1024];
	Elf32_Ehdr *boot_ehdr;
	Elf32_Phdr *boot_phdr;
	fread(buf,1,1024,boot);
	boot_ehdr=(void *)buf;
	uint32_t magic=0x464c457f;
	uint32_t *boot_magic=(uint32_t *)buf;
	assert(*boot_magic==magic);

	int i=0;
	for (boot_phdr=(void *)buf+boot_ehdr->e_phoff; i<boot_ehdr->e_phnum; i++){
		if (boot_phdr[i].p_type==PT_LOAD){
			num_section++;
			base=boot_phdr[i].p_vaddr;
			fseek(boot,boot_phdr[i].p_offset,SEEK_SET);
			uint8_t newbuf[512];
			memset(newbuf,0,512);
			fread(newbuf,1,boot_phdr[i].p_filesz,boot);
			fwrite(newbuf,1,512,image);
			fflush(image);
	
			if (extend){
				printf("bootblock image info\n");
				printf("sectors: %d\n",num_section);
				printf("offset of segment in the file: 0x%x\n",boot_phdr[i].p_offset);
				printf("the image's virtural address of segment in memory: 0x%x\n",boot_phdr[i].p_vaddr);
				printf("the file image size of segment: 0x%x\n",boot_phdr[i].p_filesz);
				printf("the size of write to the OS iamge: 0x%x\n",boot_phdr[i].p_memsz);
				printf("padding up to 0x%x\n",512);		
			}
		}
	}	
	
}

void write_kernel(FILE **image_file, FILE **kernel_file,char *filename)
{
	FILE *image=*image_file;
	FILE *kernel=*kernel_file;
	uint8_t buf[1024];
	Elf32_Ehdr *kernel_ehdr;
	Elf32_Phdr *kernel_phdr;
	fread(buf,1,1024,kernel);
	kernel_ehdr=(void *)buf;
	uint32_t magic=0x464c457f;
	uint32_t *kernel_magic=(uint32_t *)buf;
	assert(*kernel_magic==magic);
	int i=0,j,read_len;
	num_section=0;
	for (kernel_phdr=(void *)buf+kernel_ehdr->e_phoff; i<kernel_ehdr->e_phnum; i++){
		if (kernel_phdr[i].p_type==PT_LOAD){

			base=kernel_phdr[i].p_vaddr;
			fseek(kernel,kernel_phdr[i].p_offset,SEEK_SET);
			uint8_t newbuf[512];
			for (j=0; j<(kernel_phdr[i].p_filesz+511)/512; j++){
				num_section++;
				memset(newbuf,0,512);
				read_len=(j==(int)(kernel_phdr[i].p_filesz+511)/512-1)?kernel_phdr[i].p_filesz-j*512:512;
				fread(newbuf,1,read_len,kernel);
				fwrite(newbuf,1,512,image);
				fflush(image);
			}
			if (extend){
				printf("%s image info\n",filename);
				printf("sectors: %d\n",num_section);
				printf("offset of segment in the file: 0x%x\n",kernel_phdr[i].p_offset);
				printf("the image's virtural address of segment in memory: 0x%x\n",kernel_phdr[i].p_vaddr);
				printf("the file image size of segment: 0x%x\n",kernel_phdr[i].p_filesz);
				printf("the size of write to the OS iamge: 0x%x\n",kernel_phdr[i].p_memsz);
				printf("padding up to 0x%x\n",512);
			}
		}
	}	
	
}

int main(int argc, char *argv[]){

	if (argc>1 && strcmp(argv[1],"--extended")==0)
		extend=1;


	FILE *image=fopen("image","wb");

	FILE *boot=fopen(argv[extend+1],"rb");
	write_bootblock(&image,&boot);
	fclose(boot);

	int i=0;
	for (i=extend+2; i<argc; i++){
		FILE *kernel=fopen(argv[i],"rb");
		write_kernel(&image,&kernel,argv[i]);
		fclose(kernel);
	}
	num_section*=512;
	fseek(image,0x40,SEEK_SET);
	fwrite(&num_section,sizeof(num_section),1,image);
	fflush(image);
	fclose(image);
	return 0;
}
