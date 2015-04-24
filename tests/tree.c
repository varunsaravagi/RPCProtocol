#include <stdio.h>
#include <stdlib.h>
#include <dirtree.h>
#include <string.h>


char* serialize(struct dirtreenode* tree, char* buffer, int* offset, size_t *size);
struct dirtreenode* deserialize(struct dirtreenode* treecopy, char* buffer, int* offset);
int level = 1;
int main(){
	char *path = "/home/vsaravag/Downloads/";
	printf("Path %s\n", path);
	struct dirtreenode* tree = getdirtree(path);
	size_t length;
	unsigned int temp;
	int offset = 0;
	
	printf("--------FUNCTION OUTPUT--------\n");
	traverse(tree);
	//printf("Name %s | Count %d\n", tree->name, tree->num_subdirs);
	//printf("Size of struct %d\n", (int)sizeof(struct dirtreenode));
	
	size_t size=0;
	char* buffer = NULL;
	buffer = serialize(tree, buffer, &offset, &size);
	//printf("Address buffer %u\n", buffer);
	buffer[offset]=0;
	//printf("size of buffer %d\n", strlen(buffer));
	struct dirtreenode *treecopy=NULL;
	offset = 0;
	treecopy = deserialize(treecopy, buffer, &offset);
	//printf("Out deserialize\n");
	//printf("Name %s | Count %d\n", treecopy->name, treecopy->num_subdirs);
	printf("-------SELF OUTPUT--------\n");
	traverse(treecopy);
	freedirtree(tree);
	freedirtree(treecopy);
	return 0;
}

traverse(struct dirtreenode* treecopy){
	int i;
	//printf("In Traverse\n");
	printf("Name %s | Count %d\n", treecopy->name, treecopy->num_subdirs);
	//printf("Subdir address %u\n", treecopy->subdirs);
	for(i = 0; i < treecopy->num_subdirs;i++){
		//printf("Subdir %d address %u\n", i,(treecopy->subdirs[i]));
		traverse((treecopy->subdirs[i]));
	}
}

char* serialize(struct dirtreenode* tree, char* buffer, int *offset, size_t *size){
	//printf("Serialize\n");
	size_t length;
	unsigned int temp;
	int i;
	length = strlen(tree->name);
	*size += sizeof(size_t)+length+sizeof(int);
	//printf("Realloc parameters %u %d\n", buffer, *size);
	buffer = realloc(buffer, *size);
	//printf("Size %d Offset %d Address Buffer %u\n", *size, *offset, buffer);	
	if (!buffer)
		printf("Realloc failed\n");
	//printf("starting\n");
	//printf("Offset %d\n", *offset);
	temp = htonl(length);
	memcpy(buffer+ *offset, &temp, sizeof(temp));
	*offset += sizeof(temp);
	
	memcpy(buffer+ *offset, tree->name, length);
	*offset+=length;

	temp = htonl(tree->num_subdirs);
	memcpy(buffer+ *offset, &temp, sizeof(temp));
	*offset+=sizeof(temp);
	//printf("Level %d\n", level);
	//printf("Name %s\n", tree->name);
	for(i=0;i<tree->num_subdirs;i++){
		//printf("Loop %d",i);
		//printf("Name %s\n", (tree->subdirs[i])->name);
		serialize((tree->subdirs[i]),buffer, offset,size);
	}
	return buffer;
}

struct dirtreenode* deserialize(struct dirtreenode *treecopy, char* buffer, int *offset){
	//printf("In Deserialize\n");
	treecopy = malloc(sizeof(struct dirtreenode));
	if (!treecopy)
		printf("Malloc fail\n");
	unsigned int temp;
	size_t length;
	int i;
	memcpy(&temp, buffer+ *offset, sizeof(temp));
	length = ntohl(temp);
	*offset+=sizeof(temp);
	//printf("Length %d\n", length);
	treecopy->name = malloc(length);
	
	memcpy(treecopy->name, buffer+ *offset, length);
	treecopy->name[length]=0;
	*offset+=length;

	memcpy(&temp, buffer+ *offset, sizeof(temp));
	treecopy->num_subdirs = ntohl(temp);
	*offset += sizeof(temp);
	//printf("Length %d Name %s Count %d Address %u\n", length, treecopy->name, treecopy->num_subdirs, treecopy);
		
	if (treecopy->num_subdirs > 0){
		treecopy->subdirs = malloc(treecopy->num_subdirs * sizeof(struct dirtreenode*));
	//	printf("Subdir address %u\n", treecopy->subdirs);
	}
	for (i=0; i< treecopy->num_subdirs; i++){
		//(treecopy->subdirs[i]) = malloc(sizeof(struct dirtreenode));
		treecopy->subdirs[i] = deserialize((treecopy->subdirs[i]), buffer, offset);
//	printf("Subdir %d address %u\n", i, treecopy->subdirs[i]);
	}
	return treecopy;
}

