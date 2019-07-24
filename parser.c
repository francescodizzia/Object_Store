#define MAX_ACTION_LENGTH 9
#define MAX_NAME_LENGTH 101

int parse_request(char* header){
 char action[MAX_ACTION_LENGTH],name[MAX_NAME_LENGTH];
 size_t len;
 sscanf(header, "%s %s %u \n", action, name, &len);
 printf("Action: %s   Name: %s  len: %u",action, name, len);
}
