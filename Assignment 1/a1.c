#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


void list(char *dir_path, char *initial_dir_path, int recursive, int name_ends_with, char *ending_word, int has_perm_execute) 
{
    DIR* dir;
    struct dirent *dir_entry;
    dir = opendir(dir_path);
    if (dir == 0) 
    {
        printf("ERROR\n");
        printf("invalid directory path\n");
    }
    else 
    {   if(strcmp(dir_path, initial_dir_path) == 0) 
            printf("SUCCESS\n");
        while ((dir_entry = readdir(dir)) != 0) 
        {   
            if(strcmp(dir_entry->d_name, ".") != 0 && strcmp(dir_entry->d_name, "..") != 0)
            {
                char new_dir_path[100];
                strcpy(new_dir_path, dir_path);
                strcat(new_dir_path, "/");
                strcat(new_dir_path, dir_entry->d_name);

                int is_ok = 1;
                if(name_ends_with == 1) 
                {   
                    int i, j = strlen(dir_entry->d_name) - 1;
                    for(i = strlen(ending_word) - 1; i >= 0 && j >= 0; i--, j--)
                        if(ending_word[i] != dir_entry->d_name[j]) 
                            is_ok = 0;
                }
                int is_ok2 = 0;
                if(has_perm_execute == 1)
                {
                    struct stat file_metadata;
                    if(stat(new_dir_path, &file_metadata) == 0) 
                        if(file_metadata.st_mode & S_IXUSR)
                            is_ok2 = 1;
                }
                if(name_ends_with == 0 && has_perm_execute == 0)
                        printf("%s/%s\n", dir_path, dir_entry->d_name);
                else if(name_ends_with == 1 && is_ok == 1)
                         printf("%s/%s\n", dir_path, dir_entry->d_name);
                else if(has_perm_execute == 1 && is_ok2 == 1)
                         printf("%s/%s\n", dir_path, dir_entry->d_name);
                if(recursive == 1) 
                    if(strstr(dir_entry->d_name, ".") == 0)
                        list(new_dir_path, dir_path, recursive, name_ends_with, ending_word, has_perm_execute);
            }
        }
        closedir(dir);
    }
}

void parse(char *dir_path) 
{   
    int fd = open(dir_path, O_RDONLY);
    int file_size;
    struct stat st;
    if (stat(dir_path, &st) == 0)
        file_size = st.st_size;

    lseek(fd, -4, SEEK_END);
    char magic[5];
    read(fd, &magic[0], 1);
    read(fd, &magic[1], 1);
    read(fd, &magic[2], 1);
    read(fd, &magic[3], 1);
    magic[4] = 0;
    if(strcmp(magic, "ZKas") != 0)
    {
        printf("ERROR\n");
        printf("wrong magic\n");
        return;  
    }
    int header_size;
    header_size = 0;
    lseek(fd, -6, SEEK_END);
    read(fd, &header_size, 2);
    lseek(fd, file_size - header_size, SEEK_SET);
    int version = 0;
    read(fd, &version, 4);
    if(version > 71 || version < 45)
    {
        printf("ERROR\n");
        printf("wrong version\n");
        return;  
    }
    int sect_nr;
    sect_nr = 0;
    read(fd, &sect_nr, 1);
    if(sect_nr > 15 || sect_nr < 2)
    {
        printf("ERROR\n");
        printf("wrong sect_nr\n");
        return;
    }

    char **sect_name = (char**)malloc(sizeof(char*) * sect_nr);
    int sect_type[17], sect_offset[17], size[17], i;
    int wrong = 0;

    for(i = 0; i <= sect_nr - 1; i++) 
    {   
        sect_name[i] = (char*)malloc(12);
        read(fd, sect_name[i], 12);
        sect_type[i] = 0;
        read(fd, &sect_type[i], 4);
        if(sect_type[i] != 60 && sect_type[i] != 83 && sect_type[i] != 36) 
        {
			printf("ERROR\n");
            printf("wrong sect_types\n");
            wrong = 1;
		}
        sect_offset[i] = 0; 
        read(fd, &sect_offset[i], 4);
        size[i] = 0;
        read(fd, &size[i], 4);
    }   
    if(wrong == 1) 
    {
        for(i = 0; i <= sect_nr - 1; i++) 
		    free(sect_name[i]);
	    free(sect_name);
    }
    else 
    {
        printf("SUCCESS\n");
        printf("version=%d\n", version);
        printf("nr_sections=%d\n", sect_nr);
        for(i = 0; i <= sect_nr - 1; i++) 
			printf("section%d: %s %d %d\n", i + 1, *(sect_name + i), sect_type[i], size[i]);
        for(i = 0; i <= sect_nr - 1; i++) 
		    free(sect_name[i]);
	    free(sect_name);
	}
}

void extract(char *dir_path, int line, int section) 
{
    int fd = open(dir_path, O_RDONLY);
    if(fd == -1) 
    {
        printf("ERROR\n");
        printf("invalid file\n");
        return;
    }

    int file_size;
    struct stat st;
    if (stat(dir_path, &st) == 0)
        file_size = st.st_size;

    lseek(fd, -4, SEEK_END);
    char magic[5];
    read(fd, &magic[0], 1);
    read(fd, &magic[1], 1);
    read(fd, &magic[2], 1);
    read(fd, &magic[3], 1);
    magic[4] = 0;
    if(strcmp(magic, "ZKas") != 0)
    {
        printf("ERROR\n");
        printf("invalid file\n");
        return;  
    }
    int header_size;
    header_size = 0;
    lseek(fd, -6, SEEK_END);
    read(fd, &header_size, 2);
    lseek(fd, file_size - header_size, SEEK_SET);
    int version = 0;
    read(fd, &version, 4);
    if(version > 71 || version < 45)
    {
        printf("ERROR\n");
        printf("invalid file\n");
        return;  
    }
    int sect_nr;
    sect_nr = 0;
    read(fd, &sect_nr, 1);
    if(sect_nr > 15 || sect_nr < 2)
    {
        printf("ERROR\n");
        printf("invalid section\n");
        return;
    }

    char sect_name[13];
    sect_name[12] = 0;
    int sect_type[17], sect_offset[17], size[17], i;

    for(i = 0; i <= sect_nr - 1; i++) 
    {   
        sect_type[i] = 0; 
        read(fd, sect_name, 12);
        sect_name[12] = 0;
        read(fd, &sect_type[i], 4);
        if(sect_type[i] != 60 && sect_type[i] != 83 && sect_type[i] != 36) 
        {
			printf("ERROR\n");
            printf("invalid file\n");
            return;
		}
        sect_offset[i] = 0;
        read(fd, &sect_offset[i], 4);
        size[i] = 0;
        read(fd, &size[i], 4);
    }   
    section--;
    int size_searched_section = size[section];
    int offset_searched_section = sect_offset[section];
    char *store_searched_line = (char*)malloc(sizeof(char) * size_searched_section); 
    char *read_section = (char*)malloc(sizeof(char) * size_searched_section);
    lseek(fd, offset_searched_section, SEEK_SET);
    read(fd, read_section, size_searched_section);
    int line_nr = 1, j = 0, k; 
    for(k = 0; k <= size_searched_section - 1 &&line_nr <= line ; k++)
    {
        if(line_nr == line) 
        {
            store_searched_line[j] = read_section[k];
            j++;
        }
        if(read_section[k] == '\n') 
            line_nr++;
    }
    if(line_nr < line) 
    {
        printf("ERROR\n");
        printf("invalid line\n");
        return;
    }
    else
    {  
        printf("SUCCESS\n");
        store_searched_line[j - 1] = 0; 
        printf("%s\n", store_searched_line);
    }
    free(store_searched_line);
    free(read_section);
}

void findall(char *dir_path, char *initial_dir_path)
{
    DIR* dir;
    struct dirent *dir_entry;

    dir = opendir(dir_path);
    if (dir == 0) 
    {
        printf("ERROR\n");
        printf("invalid directory path\n");
    }
    else 
    {   if(strcmp(dir_path, initial_dir_path) == 0) 
            printf("SUCCESS\n");
        while ((dir_entry = readdir(dir)) != 0) 
        {   
            if(strcmp(dir_entry->d_name, ".") != 0 && strcmp(dir_entry->d_name, "..") != 0)
            {
                char new_dir_path[100];
                strcpy(new_dir_path, dir_path);
                strcat(new_dir_path, "/");
                strcat(new_dir_path, dir_entry->d_name);
                struct stat path_stat;
                stat(new_dir_path, &path_stat);
                if(S_ISDIR(path_stat.st_mode))
                    findall(new_dir_path, dir_path);
                if(S_ISREG(path_stat.st_mode))
                {
                    int fd = open(new_dir_path, O_RDONLY);
                    int file_size;

                    struct stat st;
                    if (stat(new_dir_path, &st) == 0)
                        file_size = st.st_size;

                    lseek(fd, -4, SEEK_END);
                    char magic[5];
                    read(fd, &magic[0], 1);
                    read(fd, &magic[1], 1);
                    read(fd, &magic[2], 1);
                    read(fd, &magic[3], 1);
                    magic[4] = 0;
                    if(strcmp(magic, "ZKas") != 0)
                    {   
                        closedir(dir);
                        return; 
                    }
                    int header_size;
                    header_size = 0;
                    lseek(fd, -6, SEEK_END);
                    read(fd, &header_size, 2);
                    lseek(fd, file_size - header_size, SEEK_SET);
                    int version = 0;
                    read(fd, &version, 4);
                    if(version > 71 || version < 45)
                    {   
                        closedir(dir);
                        return; 
                    }
                    int sect_nr;
                    sect_nr = 0;
                    read(fd, &sect_nr, 1);
                    if(sect_nr > 15 || sect_nr < 2)
                    {   
                        closedir(dir);
                        return; 
                    } 
                    char sect_name[13];
                    int sect_type[17], sect_offset[17], size[17], i;

                    int wrong = 0;
                    for(i = 0; i <= sect_nr - 1; i++) 
                    {   
                        read(fd, sect_name, 12);
                        sect_name[12] = 0;
                        read(fd, &sect_type[i], 4);
                        if(sect_type[i] != 60 && sect_type[i] != 83 && sect_type[i] != 36) 
                        {   
                            closedir(dir);
                            return; 
                        }
                        sect_offset[i] = 0; 
                        read(fd, &sect_offset[i], 4);
                        size[i] = 0;
                        read(fd, &size[i], 4);
                        if(size[i] > 1062)
                            wrong = 1;
                    }
                    if(wrong == 0) 
                        printf("%s\n", new_dir_path);
                }
            }  
        }
        closedir(dir);
    }
}

int main(int argc, char **argv) 
{
    if(argc >= 2) {
        char dir_path[100] = "";
        if(strcmp(argv[1], "variant") == 0) 
            printf("61248\n");
        
        else if(strcmp(argv[1], "list") == 0) 
        {   
            int i, recursive = 0, name_ends_with = 0,  has_perm_execute = 0;
            char ending_word[50] = "";
            for(i = 2; i <= argc - 1; i++)
            {
                if(strcmp(argv[i], "recursive") == 0) 
                    recursive = 1;
                else if(strcmp(argv[i], "has_perm_execute") == 0)
                    has_perm_execute = 1;
                else if(strstr(argv[i], "name_ends_with=")) 
                {
                    name_ends_with = 1;
                    strcpy(ending_word, argv[i] + 15);
                }  
                else if(strstr(argv[i], "path=")) 
                     strcpy(dir_path, argv[i] + 5);
                 
            }
            
            list(dir_path, dir_path, recursive, name_ends_with, ending_word, has_perm_execute);  
        }
        else if(strcmp(argv[1], "parse") == 0)
        {
            strcpy(dir_path, argv[2] + 5);
            parse(dir_path);
        }
        else if(strcmp(argv[1], "extract") == 0)
        {
            char line[100], section[100];
            int i;
            for(i = 2; i <= argc - 1; i++)
            {
                if(strstr(argv[i], "line=")) 
                    strcpy(line, argv[i] + 5);
                  
                else if(strstr(argv[i], "section=")) 
                     strcpy(section, argv[i] + 8);
                else if(strstr(argv[i], "path=")) 
                     strcpy(dir_path, argv[i] + 5);
            }
            extract(dir_path, atoi(line), atoi(section));
        }
        else if(strcmp(argv[1], "findall") == 0)
        {
            strcpy(dir_path, argv[2] + 5);
            findall(dir_path, dir_path);
        }
    }
    return 0;
}


