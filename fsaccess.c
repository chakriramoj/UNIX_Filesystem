#include<stdio.h>
#include<sys/types.h>
#include<unistd.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<stdlib.h>
#include<math.h>

typedef struct {//super block utilization-1023 bytes
unsigned int isize;
unsigned int fsize;
unsigned int nfree;
unsigned int free[150];
unsigned int ninode;
unsigned int inode[100];
char flock;
char ilock;
char fmod;
unsigned short time[2];
}superblock;

typedef struct {//inode size-64bytes
unsigned short flags;
unsigned short nlinks;
unsigned short uid;
unsigned short gid;
unsigned short size0; // -2 bytes of size
unsigned short size1;//-2 bytes of size-toatal 4 bytes,makes max 4gb file possible
unsigned int addr[12];  //one level of triple indirect block can support 4gb.
unsigned short actime;
unsigned short modtime[2];
}inode;

typedef struct {
unsigned short inode_number;//inode which is allocated to the directory 
char name[14];//name corresponding to the directory
}directory;

//free array structure for chaining the blocks
typedef struct {
unsigned int nfree;
unsigned int free[150];
}freeArray;



int initfs(int fd,unsigned short num_blck,unsigned short num_of_inodes ){
superblock super;

//setting fsize;
super.fsize=num_blck;
int number_of_blocks=num_blck;
int inodesize=64;
int inode_mem=num_of_inodes*inodesize;
int blocksize=1024;
int blocks_into_freearray;
int inodes_into_inodearray;
//calculating iszie
if(inode_mem%blocksize==0){
 super.isize=inode_mem/blocksize;
}
else{
super.isize=inode_mem/blocksize+1;
}
/*isize number of blocks to inodes,1st two blocks are bootloader and superblock 
and the 1st data block is allocated to the contents of root dierctory*/

int filled_blocks=3+super.isize;
int remaining_blocks=num_blck-filled_blocks;

/*setting free array and nfree*/

super.nfree=150;//setting nfree as 150
//free blocks are less than 150
if(remaining_blocks<150){
//setting nfree
blocks_into_freearray=remaining_blocks;
remaining_blocks=0;
}
//free blocks are more than 150
else if(remaining_blocks>150){
blocks_into_freearray=150;
remaining_blocks=remaining_blocks-150;
}
for(int i=0;i<blocks_into_freearray;i++){
super.free[i]=filled_blocks+i;
}

/*setting nionode and inode array*/

super.ninode=100;//number of free inodes in the list of inodes
int first_free_inode_number=2;//inode 1 will be allocated during the initialization of the root directory
if(num_of_inodes<=100){
inodes_into_inodearray=num_of_inodes-1;
}
else if(num_of_inodes>100){
inodes_into_inodearray=100;
}
super.ninode=inodes_into_inodearray;
for(int j=0;j<inodes_into_inodearray;j++){
super.inode[j]=first_free_inode_number+j;
}
super.fmod = 'N';
super.flock = 'Y';
super.ilock='N';
lseek(fd,1024,SEEK_SET);//moving to the superblock in the file
write(fd, &super, 1024);//write the superblock in the next 1024 bytes of the file which will be the superblock
superblock s2;
lseek(fd,1024,SEEK_SET);
read(fd,&s2,1024);
printf("s2 components: isize = %d, fsize = %d,nfree= %d,ninode= %d\n",s2.isize, s2.fsize,s2.nfree,s2.ninode);//printing the initialized components
printf("printing superblock free array\n");
for(int i=0;i<150;i++){
printf("%d",s2.free[i]);//printing free array
printf(", ");
}
printf("\n");

/*initialize inode 1 with root directory information*/
initialize_rootdir(fd,filled_blocks);

/*link the free array for chaining free data blocks*/
chainfreearray(fd,remaining_blocks,filled_blocks);

/*testing chaining of free blocks*/
print_next_set_of_chainedblocks(fd,filled_blocks,remaining_blocks);//to test if chaining is correct

return 1;
}

void print_next_set_of_chainedblocks(int fd, int filled_blocks,int remaining_blocks){
int to_add_to_free_array;
while(remaining_blocks>0){
if(remaining_blocks<150){
	to_add_to_free_array=remaining_blocks;
	remaining_blocks=0;
}
else{
	to_add_to_free_array=150;
	remaining_blocks=remaining_blocks-150;
}
lseek(fd,filled_blocks*1024,SEEK_SET);//move to first data block which conatins the list of next set of free datablocks
freeArray f1;
read(fd,&f1,604);
printf("chained free array\n");
printf("nfree value: %d\n",f1.nfree);
for(int i=0;i<to_add_to_free_array;i++){
	printf("%d",f1.free[i]);
	printf(", ");
}
printf("\n");
filled_blocks=filled_blocks+150;
}
} 

//link the free array for chaining free data blocks
void chainfreearray(int fd,int remaining_blocks,int filled_blocks)
{
freeArray freearr;
int to_add_data_blocks;// number of datablocks to be added to freelist
while(remaining_blocks>0){//whenever data blocks remain
lseek(fd,(filled_blocks*1024),SEEK_SET);//move pointer to the first available data block
filled_blocks+=150;//increment the freeblocks by the freearray size
if(remaining_blocks<150){
to_add_data_blocks=remaining_blocks;
freearr.nfree = remaining_blocks;
remaining_blocks = 0;
}
else{
to_add_data_blocks=150;
freearr.nfree =150;
remaining_blocks =remaining_blocks-150;
}
for(int k=0;k<to_add_data_blocks;k++){
freearr.free[k]=filled_blocks+k;
}
/*printf("set of chained logical blocks");
for(int i=0;i<to_add_data_blocks;i++){
printf("%d\n",freearr.free[i]);
}*/

write(fd, &freearr,604);

}
}

void initialize_rootdir(int fd,int filled_blocks){
inode inodestr;//instance of inode struct
//setting inode flags
inodestr.flags=140777;//1-inode is allocated,4-directory,0-userid and groupid on execution not provided,each 7-r,w,execute to ownwner,group,others
inodestr.nlinks = '2';
inodestr.uid = '0';
inodestr.gid = '0';
inodestr.size0 = '0';
inodestr.size1 = 0;
inodestr.addr[0] = filled_blocks-1;//alloting first data block to inode 1's addr[] array.
inodestr.actime = 0;
inodestr.modtime[0] = 0;
inodestr.modtime[1] = 0;
lseek(fd, 2*1024,SEEK_SET);//going to the first inode block position
write(fd, &inodestr,64);//writing the root directory information into the inode1 which is the first 64 bytes
//init root directory's data block
directory dir;
dir.inode_number = 1;//setting inode number for the root to be 1
strncpy(dir.name, ".", 14);//cpying '.' in the name field of the directory
int logical_datablockstomove=filled_blocks-1;//first datablock which corresponds to the root
int totalbytes=logical_datablockstomove*1024;
lseek(fd,totalbytes,SEEK_SET);//moving to the first datablock which corresponds to root directory
write(fd, &dir, 16);//writing the directory structure there
directory dir2;
lseek(fd,totalbytes,SEEK_SET);
read(fd,&dir2,16);
printf("priniting root directory data\n");//verifying root directory information by read from file
printf("name: %s\n",dir2.name);
printf("inode_number: %d\n",dir2.inode_number);
strncpy(dir.name, "..", 14);//setting parent directory name
int to_seek=totalbytes+16;
lseek(fd,to_seek,SEEK_SET);
write(fd, &dir, 16);//writing the directory again
directory dir3;
lseek(fd,to_seek,SEEK_SET);
read(fd,&dir3,16);//verifying root info by reading from file
printf("priniting parent of root directory data\n");
printf("name: %s\n",dir3.name);
printf("inode_number: %d\n",dir3.inode_number);
}



#define TOKEN " "

int main(int args, char *arg[]){
char input[512];
char *input_command;
char *filepath;
char *a, *b, *c;
printf("Enter a command\n");
   while( 1 )
	{	
		scanf(" %[^\n]s", input);
      input_command = strtok(input," ");
		if(strcmp(input_command, "initfs")==0)
		{
			a = strtok(NULL, TOKEN);
			b = strtok(NULL, TOKEN);
			c = strtok(NULL, TOKEN);
         if (!a | !b | !c)
			{
				printf("enter proper arguments");
				continue;
			}
			else{
				filepath = a;
				int num_blck = atoi(b);
				int number_of_inodes = atoi(c);
            if( access(filepath, F_OK) == -1){
					int fd = open(filepath, O_RDWR | O_CREAT , 0777);
					if(fd < 0)
					{
						printf("please enter proper path");
						continue;
					}
               else{			
					printf("initialized new file system that supports files upto 4GB\n");
					initfs(fd, num_blck, number_of_inodes);
               
               }
				}
			}
		}
      else if(strcmp(input_command, "q")==0){
      printf("saving and exiting\n");
      exit(0);
      }

}
}