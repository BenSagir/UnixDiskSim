#include <iostream>
#include <vector>
#include <map>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

#define DISK_SIZE 256

void decToBinary(int n, unsigned char &c)
{
    // array to store binary number
    int binaryNum[8];

    // counter for binary array
    int i = 0;
    while (n > 0)
    {
        // storing remainder in binary array
        binaryNum[i] = n % 2;
        n = n / 2;
        i++;
    }

    // printing binary array in reverse order
    for (int j = i - 1; j >= 0; j--)
    {
        if (binaryNum[j] == 1)
            c = c | 1u << j;
    }
}

// #define SYS_CALL
// ============================================================================
class fsInode
{
    int fileSize;
    int block_in_use;

    int *directBlocks;
    int singleInDirect;
    int num_of_direct_blocks;
    int block_size;

public:
    fsInode(int _block_size, int _num_of_direct_blocks){    //constructor
        fileSize = 0;
        block_in_use = 0;
        block_size = _block_size;
        num_of_direct_blocks = _num_of_direct_blocks;
        directBlocks = new int[num_of_direct_blocks];
        assert(directBlocks);
        for (int i = 0; i < num_of_direct_blocks; i++)
            directBlocks[i] = -1;
        singleInDirect = -1;
    }
    int getFileMaxSize(){           //returns the maximum size of a file
        return (num_of_direct_blocks + block_size) * block_size;
    }
    int getBlockInUse(){            //returns how many block are in use
        return block_in_use;
    }
    int getFileSize(){              //return the current size of a file
        return fileSize;
    }
    int getDirectBlock(int i){      //returns the number of the direct block in index i
        return directBlocks[i];
    }
    int isDirectFree(){             //if there is a free direct block, return his index. if there is not, return -1
        for (int i = 0; i < num_of_direct_blocks; i++)
            if (directBlocks[i] == -1)
                return i;
        return -1;
    }
    int getInDirect(){              //return the number of the single indirect block
        return singleInDirect;
    }
    int inDirect(int bit){          //if the single indirect block hasnt assined yet, it will be "bit"
        if (singleInDirect == -1)   {
            singleInDirect = bit;
            return bit;
        }
        return -1;                  //if it already assigned return -1
    }
    void incBlockInUse(){           //increment block_in_use 
        block_in_use++;
    }
    void decBlockInUse(){           //decrement block_in_use
        block_in_use--;
    }
    void setDirectBlock(int i, int n){  //set the direct block at index i to be n
        directBlocks[i] = n;
    }
    void setFileSize(int n){        //set the file size to be n
        fileSize = n;
    }
    ~fsInode(){                     //destructor
        delete directBlocks;
    }
};

// ============================================================================
class FileDescriptor
{
    pair<string, fsInode *> file;
    bool inUse;
    bool deleted;

public:
    FileDescriptor(string FileName, fsInode *fsi){  //constructor
        file.first = FileName;
        file.second = fsi;
        inUse = true;
        deleted = false;
    }
    string getFileName(){           //return the name of the file
        return file.first;
    }
    fsInode *getInode(){            //return a pointer to the Inode of the file
        return file.second;
    }
    bool isInUse(){                 //return true if the file is in use (created or opened)
        return inUse;
    }
    bool isDeleted(){               //return true if the has been deleted
        return deleted;
    }
    void setInUse(bool _inUse){     //set in use
        inUse = _inUse;
    }
    void setDeleted(bool _deleted){ //set deleted
        deleted = _deleted;
    }
    void setFileName(string s){     //set the file name
        file.first = s;
    }
    void setInode(fsInode *fsi){    //set the pointer to the Inode
        file.second = fsi;
    }
};

#define DISK_SIM_FILE "DISK_SIM_FILE.txt"
// ============================================================================
class fsDisk
{
    FILE *sim_disk_fd;

    bool is_formated;

    // BitVector - "bit" (int) vector, indicate which block in the disk is free
    //              or not.  (i.e. if BitVector[0] == 1 , means that the
    //             first block is occupied.
    int BitVectorSize;
    int *BitVector;

    // Unix directories are lists of association structures,
    // each of which contains one filename and one inode number.
    map<string, fsInode *> MainDir;

    // OpenFileDescriptors --  when you open a file,
    // the operating system creates an entry to represent that file
    // This entry number is the file descriptor.
    vector<FileDescriptor> OpenFileDescriptors;

    int direct_enteris;
    int block_size;

public:
    // ------------------------------------------------------------------------
    fsDisk(){   //constructor
        sim_disk_fd = fopen(DISK_SIM_FILE, "r+");
        assert(sim_disk_fd);
        for (int i = 0; i < DISK_SIZE; i++){
            int ret_val = fseek(sim_disk_fd, i, SEEK_SET);
            ret_val = fwrite("\0", 1, 1, sim_disk_fd);
            assert(ret_val == 1);
        }
        fflush(sim_disk_fd);
    }
    int getDiskFreeSpace(){         //return how many free bytes are there in the disk
        int actualSize = 0;
        for (int i = 0; i < OpenFileDescriptors.size(); i++)        //for loop to sum up all the sizes in the open FD vector
            actualSize += OpenFileDescriptors[i].getInode()->getFileSize();
        return (DISK_SIZE - actualSize);
    }

    int getBit(){                   //return the first free block in disk
        for (int i = 0; i < BitVectorSize; i++)
            if (BitVector[i] == 0)
                return i;
        return -1;                  //if the disk is full return -1
    }
    void setBit(int i, int n){      //set the bit in index i to be n
        BitVector[i] = n;
    }
    static char *substring(int starts, int length, char *text){     //a method to get subsring of text in size length the starts from starts
        char *sub = (char *)malloc(sizeof(char) * (length + 1));    //allocate new char*
        if(sub == NULL){                                            //check malloc
            cout << "substring allocation failed" << endl;
            exit(EXIT_FAILURE);
        }
        int c = 0;                                                  //init index c
        while (c < length){
            *(sub + c) = *(text + starts + c);
            c++;
        }
        sub[c] = '\0';                                              //seal the new string with '\0'
        return sub;                                                 //return sub
    }
    void blockDeleter(int block){                                   //a method to delete the block "block" from the disk
        for (int i = 0; i < block_size; i++){                       //writing '\0' into the block that we are deleting
            int ret_val = fseek(sim_disk_fd, (block * block_size) + i, SEEK_SET);
            ret_val = fwrite("\0", 1, 1, sim_disk_fd);
            assert(ret_val == 1);
        }
        setBit(block, 0);                                           //set his bit to 0
        fflush(sim_disk_fd);
    }
    ~fsDisk(){                                                      //destructor 
        delete[] BitVector;                                         //delete the bitVector array
        delete sim_disk_fd;                                         //delete the strem to file
        for (auto it = begin(MainDir); it != end(MainDir); ++it)    //delete every fsInode left in the MainDir
            delete it->second;
    }
    //=============================================================================================
    // function no.1 - print all the file descriptors opened and the content of the disk
    //=============================================================================================
    void listAll()
    {
        int i = 0;
        for (auto it = begin(OpenFileDescriptors); it != end(OpenFileDescriptors); ++it)
        {
            cout << "index: " << i << ": FileName: " << it->getFileName() << " , isInUse: " << it->isInUse() << endl;
            i++;
        }
        char bufy;
        cout << "Disk content: '";
        for (i = 0; i < DISK_SIZE; i++)
        {
            int ret_val = fseek(sim_disk_fd, i, SEEK_SET);
            ret_val = fread(&bufy, 1, 1, sim_disk_fd);
            cout << bufy;
        }
        cout << "'" << endl;
    }

    //=============================================================================================
    // function no.2 - format the disk by deciding the size of the blocks and the number of direct entries in each file
    //=============================================================================================
    void fsFormat(int blockSize = 4, int direct_Enteris_ = 3)
    {
        if(is_formated){                                            //if the disk is already been formated
            cout << "the disk is already formated" << endl;
            exit(EXIT_FAILURE);                                     
        }
        else{
            is_formated = true;                                     //set is_formated to be true
            this->block_size = blockSize;                           //set the block size
            this->direct_enteris = direct_Enteris_;                 //set the direct enteris
            BitVectorSize = DISK_SIZE / block_size;                 //calculate the size of bitVector
            BitVector = new int[BitVectorSize];                     //init the vector
            assert(BitVector);
            cout << "FORMAT DISK: number of blocks: " << BitVectorSize << endl;
        }
    }
    //=============================================================================================
    // function no.3 - create a file, insert it into open fd vector and into the MainDir
    //=============================================================================================
    int CreateFile(string fileName)
    {
        if (!is_formated){
            cout << "the disk has not been formated yet" << endl;
            return -1;
        }
        for (auto it = MainDir.find(fileName); it != end(MainDir); ++it){
            cout << "there is a file with the same name. pleae try again" << endl;
            return -1;
        }
        int delInx = -1;                                                //init a index for deleted fd spot in the vector
        for (int i = 0; i < OpenFileDescriptors.size(); i++)
            if (OpenFileDescriptors[i].getFileName().compare("") == 0)  //if there is a free spot in the vector because of deleted file
                delInx = i;                                             //delInx is the index of thefree spot
        fsInode *node = new fsInode(block_size, direct_enteris);        //create new fsInode object
        MainDir.insert({fileName, node});                               //insert the new file into the main dir
        if (delInx == -1){                                              //in case there is no deleted spot free
            FileDescriptor toInsert(fileName, node);                    //create new file descriptor object
            OpenFileDescriptors.push_back(toInsert);                    //put it into the open FD vector
            for (int i = 0; i < OpenFileDescriptors.size(); i++)
                if (fileName.compare(OpenFileDescriptors[i].getFileName()) == 0)
                    return i;                                           //return the index of the file in the open FD vector
        }
        else{                                                           //in case that there is a free spot in "delInx"
            OpenFileDescriptors[delInx].setInode(node);                 //set the Inode in this FD
            OpenFileDescriptors[delInx].setFileName(fileName);          //set the file name
            OpenFileDescriptors[delInx].setDeleted(false);              //mark it as not deleted
            OpenFileDescriptors[delInx].setInUse(true);                 //mark it as set in use
            return delInx;                                              //return the index
        }
    }

    //=============================================================================================
    // function no.4 - open a file - works only on file that has been closed
    //=============================================================================================
    int OpenFile(string fileName)
    {
        for (int i = 0; i < OpenFileDescriptors.size(); i++){       //find the file in the vector comaring fileName and the name of the FD in the vector
            if (fileName.compare(OpenFileDescriptors[i].getFileName()) == 0){
                if (OpenFileDescriptors[i].isInUse())               //if it already in use, return -1
                    return -1;
                else{  
                    if (OpenFileDescriptors[i].isDeleted())         //if the file is deleted
                        return -1;                                  //return -1
                    OpenFileDescriptors[i].setInUse(true);          //if the file is not deleted and not in use , set in use to true
                    return i;                                       //return the fd index
                }
            }
        }
        return -1;                                                  //in case we did not find "fileName" in open fd vector, return -1 
    }
    //=============================================================================================
    // function no.5 - close a file - making and opened file unavailable to write to or read from
    //=============================================================================================
    string CloseFile(int fd)
    {
        if (OpenFileDescriptors.size() <= fd)                       //if there is no file in index fd
            return "-1";                                            //return -1
        if (OpenFileDescriptors[fd].isInUse()){                     //if in use is true
            OpenFileDescriptors[fd].setInUse(false);                //set it to false
            return OpenFileDescriptors[fd].getFileName();           //return the name of the file
        }
        return "-1";                                                //if the file is already closed, return -1
    }
    //=============================================================================================
    // function no.6 - write to file - write data into the disk and save a pointer to it in the FD
    //=============================================================================================
    int WriteToFile(int fd, char *buf, int len)
    {
        if (!is_formated){
            cout << "the disk hasn't been formated yet" << endl;
            return -1;
        }
        if (OpenFileDescriptors[fd].isDeleted()){
            cout << "the file has been deleted" << endl;
            return -1;
        }
        if (!OpenFileDescriptors[fd].isInUse()){
            cout << "this file is closed right now" << endl;
            return -1;
        }
        char *buffer;                                               //initilaze variables
        char *temp;
        int bufferLen = len;
        buffer = (char *)malloc(sizeof(char) * bufferLen);
        strcpy(buffer, buf);
        unsigned char block_pointer = '\0';
        int diskFreeSpace = getDiskFreeSpace();
        int fileMaxSize = OpenFileDescriptors[fd].getInode()->getFileMaxSize();
        int curSize = OpenFileDescriptors[fd].getInode()->getFileSize();
        int blockInUse = OpenFileDescriptors[fd].getInode()->getBlockInUse();
        int change = curSize % block_size;                          //the amount of bytes in the part full block
        if ((fileMaxSize - (len + curSize)) < 0){       
            cout << "there is not room for " << buf << " in the file: " << OpenFileDescriptors[fd].getFileName() << endl;
            return -1;
        }
        if (diskFreeSpace < len){
            cout << "there is not enogh room for " << buf << " in the disk. please delete other files in order to preform this action" << endl;
            return -1;
        }
        if (change > 0){                                            //if there is part full block
            int block;
            if (blockInUse > direct_enteris){                       //if there are remote blocks
                int inDirect = OpenFileDescriptors[fd].getInode()->getInDirect();
                char dufy;
                int ret_val = fseek(sim_disk_fd, (block_size * inDirect) + (blockInUse - direct_enteris - 1), SEEK_SET);
                ret_val = fread(&dufy, 1, 1, sim_disk_fd);
                block = (int)dufy;                                  //read from the inDirect block in the disk the block number of the remote block
            }
            else{                                                   //assigned new block directly
                int directIndex = OpenFileDescriptors[fd].getInode()->isDirectFree();
                if (directIndex == -1)                              //if returns -1, therefor it is "direct enteris" blocks, so the part full is at direct_enteris -1 
                    block = OpenFileDescriptors[fd].getInode()->getDirectBlock(direct_enteris - 1);
                else                                                //if return a number the block is at (number-1) index
                    block = OpenFileDescriptors[fd].getInode()->getDirectBlock(directIndex - 1);
            }
            char *bf = substring(0, block_size - change, buffer);   //bf will be the "block_size-change" first chars 
            int mrkr = fseek(sim_disk_fd, (block * block_size) + change, SEEK_SET);
            mrkr = fwrite(bf, block_size - change, 1, sim_disk_fd);
            assert(mrkr == 1);
            temp = buffer;                                          //save a pointer to the current version of buffer
            buffer = substring(block_size - change, bufferLen - (block_size - change), buffer); //buffer will be now the rest of the char after index (block_size-change)
            OpenFileDescriptors[fd].getInode()->setFileSize(curSize + block_size - change);     //update the file size
            bufferLen -= (block_size - change);                                                 //update the buffer size
            free(temp);                                             //free the previos version of buffer
            free(bf);                                               //free bf
        }
                                                                    //at this point you know there is no part full blocks
        int numofblocks = bufferLen / block_size;                   //the amount of full blocks you need to write
        for (int i = 0; i < numofblocks; i++){
            int block = getBit();                                   //block is now a free block in bitVector
            blockInUse = OpenFileDescriptors[fd].getInode()->getBlockInUse();
            if (blockInUse < direct_enteris){                       //if there is less block in use that direct enteris
                OpenFileDescriptors[fd].getInode()->setDirectBlock(OpenFileDescriptors[fd].getInode()->isDirectFree(), block);
            }
            else{                                                   //if there is more
                int isInDirect = OpenFileDescriptors[fd].getInode()->inDirect(getBit());
                if (isInDirect != -1)                               //assign indirect block in bit vector
                    setBit(isInDirect, 1);
                block = getBit();                                   //get bit again
                decToBinary(block, block_pointer);                  //apply DecToBinary method to get representation of block in binary
                int diff = block - (int)block_pointer;              //difference between the orignal number and the (int)value of char
                if (diff != 0)                                      //if there is difference
                    block_pointer += diff;                          //fix it
                unsigned char *pointer = &block_pointer;            //pointer to the char
                int inDirectBlock = OpenFileDescriptors[fd].getInode()->getInDirect();
                int mrkr = fseek(sim_disk_fd, (inDirectBlock * block_size) + (blockInUse - direct_enteris), SEEK_SET);
                mrkr = fwrite(pointer, 1, 1, sim_disk_fd);          //write the coded char into the Single in direct block
                assert(mrkr == 1);
            }
            char *bf = substring(0, block_size, buffer);            //bf will be the "block_size" first chars
            int mrkr = fseek(sim_disk_fd, block * block_size, SEEK_SET);
            mrkr = fwrite(bf, block_size, 1, sim_disk_fd);          //write to the disk
            assert(mrkr == 1);
            setBit(block, 1);                                       //set the bitVector to 1 at block index
            OpenFileDescriptors[fd].getInode()->incBlockInUse();    //increment the block in use
            OpenFileDescriptors[fd].getInode()->setFileSize(OpenFileDescriptors[fd].getInode()->getFileSize() + block_size); //update size
            temp = buffer;                                          //save a pointer to the current version of buffer
            buffer = substring(block_size, bufferLen - block_size, buffer);     //cut the first "block_size" chars off buffer
            bufferLen -= block_size;                                //update the buffer size
            free(temp);                                             //free the previos version of buffer
            free(bf);                                               //free bf
        }
                    //at this point you know there is less that 'block_size' chars left to insert to the disk
        if (bufferLen > 0 && bufferLen < block_size){
            int block = getBit();                                   //get new free block index
            blockInUse = OpenFileDescriptors[fd].getInode()->getBlockInUse();
            if (blockInUse < direct_enteris)                        //if there is direct block
                OpenFileDescriptors[fd].getInode()->setDirectBlock(OpenFileDescriptors[fd].getInode()->isDirectFree(), block);
            else{
                int isInDirect = OpenFileDescriptors[fd].getInode()->inDirect(getBit());
                if (isInDirect != -1)                               //assign indirect block in bit vector
                    setBit(isInDirect, 1);
                block = getBit();                                   //get bit again
                decToBinary(block, block_pointer);                  //apply DecToBinary method to get representation of block in binary
                int diff = block - (int)block_pointer;              //difference between the orignal number and the (int)value of char
                if (diff != 0)                                      //if there is difference
                    block_pointer += diff;                          //fix it
                unsigned char *pointer = &block_pointer;            //pointer to the char
                int inDirectBlock = OpenFileDescriptors[fd].getInode()->getInDirect();
                int mrkr = fseek(sim_disk_fd, (inDirectBlock * block_size) + (blockInUse - direct_enteris), SEEK_SET);
                mrkr = fwrite(pointer, 1, 1, sim_disk_fd);          //write the coded char into the Single in direct block
                assert(mrkr == 1);
            }
            int mrkr = fseek(sim_disk_fd, block * block_size, SEEK_SET);
            mrkr = fwrite(buffer, bufferLen, 1, sim_disk_fd);       //write whats remain from buffer to the disk
            assert(mrkr == 1);
            setBit(block, 1);                                       //set the block bit to 1
            OpenFileDescriptors[fd].getInode()->incBlockInUse();    //increment the block in use
            OpenFileDescriptors[fd].getInode()->setFileSize(OpenFileDescriptors[fd].getInode()->getFileSize() + bufferLen);     //update file sizw
        }
        free(buffer); 
        fflush(sim_disk_fd);
    }
    //=============================================================================================
    // function no.7 - read from file - reads the first "len" data from the disk and put into buf
    //=============================================================================================
    int ReadFromFile(int fd, char *buf, int len)
    {
        int fileMaxSize = OpenFileDescriptors[fd].getInode()->getFileMaxSize();
        for (int i = 0; i < fileMaxSize; i++)                               //delete whatever is now at buf
            buf[i] = '\0';
        if (!is_formated){
            cout << "the disk has not been formated yet" << endl;
            return -1;
        }
        if (!OpenFileDescriptors[fd].isInUse()){
            cout << "the file is closed now. could not read from the disk" << endl;
            return -1;
        }
        char inDirect[block_size];                                          //initilaze variables
        char bufy;
        int block2read;
        int blocks = len / block_size;                                      //block needed to be read read
        if (len % block_size > 0)                                           //if there is part full block
            blocks++;                                                       //read 1 more block
        if (blocks > OpenFileDescriptors[fd].getInode()->getBlockInUse())   //if there is more block to read than the blocks the is writen on
            blocks = OpenFileDescriptors[fd].getInode()->getBlockInUse();   //make "block is use" the amount of blocks to read
        if (blocks > direct_enteris){                                       //if there is any indirect blocks
            int inDir = OpenFileDescriptors[fd].getInode()->getInDirect();  //read the Single InDirect block from the disk
            for (int l = 0; l < block_size; l++){
                int ret_val = fseek(sim_disk_fd, (block_size * inDir) + l, SEEK_SET);
                ret_val = fread(&inDirect[l], 1, 1, sim_disk_fd);
            }
        }
        //i runs for blocks, j runs for len, k runs for offset in block
        for (int i = 0, j = 0, k = 0; i < blocks && j < len && k < block_size; j++, k++){
            if (i < direct_enteris)                                         //if reading from direct blocks
                block2read = OpenFileDescriptors[fd].getInode()->getDirectBlock(i);
            else                                                            //if reading from inDirect blocks
                block2read = (int)(inDirect[i - direct_enteris]);           
            int mrkr = fseek(sim_disk_fd, (block_size * block2read) + k, SEEK_SET);
            mrkr = fread(&bufy, 1, 1, sim_disk_fd);                         //read the data into bufy
            char *ptr = &bufy;                                              //point to bufy
            ptr[1] = '\0';                                                  //add '\0' to get "string" in length of 1
            strcat(buf, ptr);                                               //append ptr to buf
            if (k + 1 == block_size){                                       //in case that k is at the end of the block
                k = -1;                                                     
                i++;
            }
        }
        return fd;                                                          //return the fd index
    }
    //=============================================================================================
    // function no.8 - delele a file - cleaning all his data from the disk
    //=============================================================================================
    int DelFile(string FileName)
    {
        int fd = -1, inDir;                                         //initilaze variables
        char inDirect[block_size];
        for (int i = 0; i < OpenFileDescriptors.size(); i++)
            if (FileName.compare(OpenFileDescriptors[i].getFileName()) == 0)
                fd = i;                                             //i will be the index in openFD vector of the file with the name "fileName"
        if (fd == -1){
            cout << "there is no file by the name :" << FileName << endl;
            return -1;
        }
        int blockInUse = OpenFileDescriptors[fd].getInode()->getBlockInUse();
        if (blockInUse > direct_enteris){                           //if there are indirect blocks
            inDir = OpenFileDescriptors[fd].getInode()->getInDirect();
            for (int l = 0; l < block_size; l++){                   //read the single indirect block from the disk
                int ret_val = fseek(sim_disk_fd, (block_size * inDir) + l, SEEK_SET);
                ret_val = fread(&inDirect[l], 1, 1, sim_disk_fd);
            }
        }
        for (int i = 0; i < blockInUse; i++){                       //for any block in the file
            if (i < direct_enteris)                                 //delete direct block
                blockDeleter(OpenFileDescriptors[fd].getInode()->getDirectBlock(i));
            else                                                    //delete indirect block
                blockDeleter((int)(inDirect[i - direct_enteris]));
        }
        if (blockInUse > direct_enteris)                            //if there is indirect blocks
            blockDeleter(inDir);                                    //delete the singleInDirect management block
        OpenFileDescriptors[fd].setDeleted(true);                   //mark the fd index in OpenFd vector as deleted
        OpenFileDescriptors[fd].setInUse(false);                    //mark it as not in use
        OpenFileDescriptors[fd].setFileName("");                    //set the file name to be empty string
        delete OpenFileDescriptors[fd].getInode();                  //delete the fsInode of the File Descriptor
        MainDir.erase(FileName);                                    //erase it from the Main Dir
        return fd;                                                  //return the index of the deleted FD
    }
};

int main()
{
    int blockSize;
    int direct_entries;
    string fileName;
    char str_to_write[DISK_SIZE];
    char str_to_read[DISK_SIZE];
    int size_to_read;
    int _fd;

    fsDisk *fs = new fsDisk();
    int cmd_;
    while (1)
    {
        cin >> cmd_;
        switch (cmd_)
        {
        case 0: // exit
            delete fs;
            exit(0);
            break;

        case 1: // list-file
            fs->listAll();
            break;

        case 2: // format
            cin >> blockSize;
            cin >> direct_entries;
            fs->fsFormat(blockSize, direct_entries);
            break;

        case 3: // creat-file
            cin >> fileName;
            _fd = fs->CreateFile(fileName);
            cout << "CreateFile: " << fileName << " with File Descriptor #: " << _fd << endl;
            break;

        case 4: // open-file
            cin >> fileName;
            _fd = fs->OpenFile(fileName);
            cout << "OpenFile: " << fileName << " with File Descriptor #: " << _fd << endl;
            break;

        case 5: // close-file
            cin >> _fd;
            fileName = fs->CloseFile(_fd);
            cout << "CloseFile: " << fileName << " with File Descriptor #: " << _fd << endl;
            break;

        case 6: // write-file
            cin >> _fd;
            cin >> str_to_write;
            fs->WriteToFile(_fd, str_to_write, strlen(str_to_write));
            break;

        case 7: // read-file
            cin >> _fd;
            cin >> size_to_read;
            fs->ReadFromFile(_fd, str_to_read, size_to_read);
            cout << "ReadFromFile: " << str_to_read << endl;
            break;

        case 8: // delete file
            cin >> fileName;
            _fd = fs->DelFile(fileName);
            cout << "DeletedFile: " << fileName << " with File Descriptor #: " << _fd << endl;
            break;
        default:
            break;
        }
    }
}