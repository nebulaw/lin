#ifndef LIN_OBJECTS_H_
#define LIN_OBJECTS_H_

#define INNER_GROUP_LEN 3
#define OBJECT_WRITE 0
#define OBJECT_APPEND 1

int lin_object_create(const char *group_name, const char *object_name, FILE *fptr, int append);

int lin_object_remove(const char *group_name, const char *object_name);

int lin_object_exists(const char *group_name, const char *object_name);

int lin_object_inner_group_merge(const char group_name[GROUP_MAX_LEN], const char inner_group[INNER_GROUP_LEN]);

void lin_object_inner_group_from_str(char dest[INNER_GROUP_LEN], const char *src);

#endif // LIN_OBJECTS_H_