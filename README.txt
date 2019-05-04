Ashutosh Agrawala axa180037
Arjit Yadav axy170007
Chakriramoj sandireddy cxs180003


**The program will print the list of all chained free blocks,the information about root directory data after initialization,isize,fsize,nfree and ninode**
**There is an output file for the sample run of 8000 datablocks and 300 inodes**

steps to compile the code:
1.use the command  cc -std=c99 fsaccess.c

specifications:
1.super block of size 1023 bytes.
2.inode structure of size 64 bytes.
3.free array of size 150.
4.inode array of size 100;
5.file size limited to 4gb by making the size of 4bytes using short for each of the low and high bytes of the size0 and size1 variable

Methods implemented:
1.initfs-To initialize the V6 file system.
  -calculate the isize
  -calculate the number of available data blocks
  -calculate the number of used data blocks
  -set the field of the superblock
  Other methods used inside initfs:
  (i) chaining of the free array:
      (A) The first set of 150 logical blocks that are free are strored in the super blocks free array.
      (B) The next set of 150 blocks are copied into a struct of free array conataining the nfree value and free array logical block numbers.
      (C) The freearray struct is then written to the logical block pointed by free[0] of the super block to maintain thye chaining.
  (ii)initializing the root directory inode:
      (A) inode 1 is allocated to the root directory.
      (B) The addr array of the inode corresponding to the root directory is made to point to the first data block(blocks after that have been added to the free array).
      (C) The flag field is made 140777.1 corresponds that the inode is allocated,4 corresponds that the type is a directory and 777 are the read,write and excecute permissions of the file.
      (D) Then we make a directory structure to store the parent directory info and the info about the directory itself with the corresponding inode numbers in the first 2 bytes and the filename in the next 14 bytes.
      (E) We then copy the directory structures content to the logical data block corresponding to the root directory which is the first data block that we have allocated.
  (iii)Printing a set of chained free array data blocks:
      (A) we are printing the set of next available free blocks.
      (B) this is done by using lseek() and moving to the 1st data block after the one allocated for the root directory and printing its contents.
2. q-whenever a user enters q we exit the filesystem by saving its contents.As the file contents are already written during the initfs we do not need to write again,just an exit(0) is enough.
 
 
 