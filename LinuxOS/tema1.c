#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT_LINE_SIZE 300

struct Dir;
struct File;

typedef struct Dir {
    char *name;
    struct Dir *parent;
    struct File *head_children_files;
    struct Dir *head_children_dirs;
    struct Dir *next;
} Dir;

typedef struct File {
    char *name;
    struct Dir *parent;
    struct File *next;
} File;

int searchIfExist(Dir *parent, char *name) {
    Dir *searchDir = parent->head_children_dirs;
    File *searchFile = parent->head_children_files;

    while (searchDir != NULL) {
        if (strcmp(searchDir->name, name) == 0) {
            return 0;
        }
        searchDir = searchDir->next;
    }
    while (searchFile != NULL) {
        if (strcmp(searchFile->name, name) == 0) {
            return 0;
        }
        searchFile = searchFile->next;
    }
    return 1;
}

void mkhomedir(Dir *home, char *name) {
    home->name = (char *) malloc(5 * sizeof(char));
    strcpy(home->name, name);
    home->parent = NULL;
    home->next = NULL;
    home->head_children_dirs = NULL;
    home->head_children_files = NULL;
}

void touch(Dir *parent, char *name) {
    File *file = (File *) malloc(sizeof(File));
    File *auxtouch = (File *) malloc(sizeof(File));
    auxtouch = parent->head_children_files;
    file->name = (char *) malloc(sizeof(name));

    if (searchIfExist(parent, name) == 1) {
        if (parent->head_children_files == NULL) {
            parent->head_children_files = file;
            strcpy(file->name, name);
            file->parent = parent;
            file->next = NULL;
        } else {
            while (auxtouch->next != NULL) {
                auxtouch = auxtouch->next;
            }
            auxtouch->next = file;
            strcpy(file->name, name);
            file->parent = parent;
            file->next = NULL;
        }
    } else {
        printf("File already exists\n");
    }
}

void mkdir(Dir *parent, char *name) {
    Dir *director = (Dir *) malloc(sizeof(Dir));
    Dir *auxmkdir = (Dir *) malloc(sizeof(Dir));
    auxmkdir = parent->head_children_dirs;
    director->name = (char *) malloc(sizeof(name));

    if (searchIfExist(parent, name) == 1) {
        if (parent->head_children_dirs == NULL) {
            parent->head_children_dirs = director;
            strcpy(director->name, name);
            director->parent = parent;
            director->next = NULL;
            director->head_children_dirs = NULL;
            director->head_children_files = NULL;
        } else {
            while (auxmkdir->next != NULL) {
                auxmkdir = auxmkdir->next;
            }
            auxmkdir->next = director;
            strcpy(director->name, name);
            director->parent = parent;
            director->next = NULL;
            director->head_children_dirs = NULL;
            director->head_children_files = NULL;
        }
    } else {
        printf("Directory already exists\n");
    }
}

void ls(Dir *parent) {
    Dir *auxlsDir = (Dir *) malloc(sizeof(Dir));
    File *auxlsFile = (File *) malloc(sizeof(File));
    auxlsDir = parent->head_children_dirs;
    auxlsFile = parent->head_children_files;

    while (auxlsDir != NULL) {
        printf("%s\n", auxlsDir->name);
        auxlsDir = auxlsDir->next;
    }
    while (auxlsFile != NULL) {
        printf("%s\n", auxlsFile->name);
        auxlsFile = auxlsFile->next;
    }

    free(auxlsDir);
    free(auxlsFile);
}

void rm(Dir *parent, char *name) {
    File *searchFile = parent->head_children_files;
    File *searchFileAux = parent->head_children_files;

    if (strcmp(searchFile->name, name) == 0) {
        parent->head_children_files = searchFile->next;
        free(searchFile);
    } else if (searchIfExist(parent, name) == 0) {
        while (searchFile != NULL) {
            if (strcmp(searchFile->name, name) == 0) {
                free(searchFile);
                searchFileAux->next = searchFile->next;
            }
            searchFileAux = searchFile;
            searchFile = searchFile->next;
        }
    } else {
        printf("Could not find the file\n");
    }
}

void rmdir(Dir *parent, char *name) {
    Dir *searchDir = parent->head_children_dirs;
    Dir *searchDirAux = parent->head_children_dirs;

    if (strcmp(searchDir->name, name) == 0) {
        parent->head_children_dirs = searchDir->next;
        free(searchDir);
    } else if (searchIfExist(parent, name) == 0) {
        while (searchDir != NULL) {
            if (strcmp(searchDir->name, name) == 0) {
                free(searchDir);
                searchDirAux->next = searchDir->next;
            }
            searchDirAux = searchDir;
            searchDir = searchDir->next;
        }
    } else {
        printf("Could not find the dir\n");
    }
}

void cd(Dir **target, char *name) {
    Dir *auxDir = (*target)->head_children_dirs;

    if (strcmp(name, "..") == 0) {
        if ((*target)->parent != NULL) {
            (*target) = (*target)->parent;
        }
    } else if (searchIfExist((*target), name) == 0) {
        while (strcmp(auxDir->name, name) != 0) {
            auxDir = auxDir->next;
        }
        (*target) = auxDir;
    } else {
        printf("No directories found!\n");
    }
}

char *pwd(Dir *target) {
    if (target->parent == NULL) {
        char *cale = (char *) malloc(MAX_INPUT_LINE_SIZE * sizeof(char));
        strcpy(cale, "/home");
        return cale;
    } else {
        char *currentName = (char *) malloc(MAX_INPUT_LINE_SIZE * sizeof(char));
        char *cale = (char *) malloc(MAX_INPUT_LINE_SIZE * sizeof(char));
        Dir *srch = target->parent->head_children_dirs;
        strcpy(cale, "/");
        strcat(cale, srch->name);
        srch = srch->parent;
        while (srch != NULL) {
            strcpy(currentName, "/");
            strcat(currentName, srch->name);
            strcat(currentName, cale);
            strcpy(cale, currentName);
            srch = srch->parent;
        }
        return cale;
    }
}

void stop(Dir *target) {
    File *file;
    free(file);
    Dir *director;
    free(director);
}

void mv(Dir *parent, char *oldname, char *newname) {

    Dir *DirAux = (Dir *) malloc(sizeof(Dir));
    Dir *searchD = (Dir *) malloc(sizeof(Dir));
    searchD = parent->head_children_dirs;
    DirAux = parent->head_children_dirs;

    File *FileAux = (File *) malloc(sizeof(File));
    File *searchF = (File *) malloc(sizeof(File));
    searchF = parent->head_children_files;
    FileAux = parent->head_children_files;

    if (searchIfExist(parent, oldname) == 1) {
        printf("File/Director not found\n");
        return;
    }
    if (searchIfExist(parent, newname) == 0) {
        printf("File/Director already exists\n");
        return;
    }
    while (searchF != NULL) {
        if (strcmp(searchF->name, oldname) == 0) {
            File *newFile = (File *) malloc(sizeof(File));
            rm(parent, oldname);
            newFile->name = (char *) malloc(sizeof(newname));
            strcpy(newFile->name, newname);
            newFile->parent = parent;
            newFile->next = NULL;
            while (FileAux->next != NULL) {
                FileAux = FileAux->next;
            }
            FileAux->next = newFile;
            return;
        }
        searchF = searchF->next;
    }
    while (searchD != NULL) {
        if (strcmp(searchD->name, oldname) == 0) {
            if (searchD->head_children_dirs == NULL && searchD->head_children_files == NULL) {
                Dir *newDir = (Dir *) malloc(sizeof(Dir));
                rmdir(parent, oldname);
                newDir->name = (char *) malloc(sizeof(newname));
                strcpy(newDir->name, newname);
                newDir->parent = parent;
                newDir->head_children_dirs = NULL;
                newDir->head_children_files = NULL;
                newDir->next = NULL;
                while (DirAux->next != NULL) {
                    DirAux = DirAux->next;
                }
                DirAux->next = newDir;
                return;
            } else {
                Dir *newDir = (Dir *) malloc(sizeof(Dir));
                File *newDirHCF = (File *) malloc(sizeof(File));
                Dir *newDirHCD = (Dir *) malloc(sizeof(Dir));
                newDirHCD = searchD->head_children_dirs;
                newDirHCF = searchD->head_children_files;
                rmdir(parent, oldname);
                newDir->name = (char *) malloc(sizeof(newname));
                strcpy(newDir->name, newname);
                newDir->parent = parent;
                newDir->head_children_dirs = newDirHCD;
                newDir->head_children_files = newDirHCF;
                newDir->next = NULL;
                while (DirAux->next != NULL) {
                    DirAux = DirAux->next;
                }
                DirAux->next = newDir;
                return;
            }
        }
        searchD = searchD->next;
    }
}

int main() {
    Dir *targetDir = (Dir *) malloc(sizeof(Dir));
    mkhomedir(targetDir, "home");
    char *input;
    input = (char *) malloc(MAX_INPUT_LINE_SIZE * sizeof(char));
    scanf("%s", input);

    do {
        if (strcmp(input, "mkdir") == 0) {
            scanf("%s", input);
            mkdir(targetDir, input);
        }
        if (strcmp(input, "touch") == 0) {
            scanf("%s", input);
            touch(targetDir, input);
        }
        if (strcmp(input, "ls") == 0) {
            ls(targetDir);
        }
        if (strcmp(input, "rm") == 0) {
            scanf("%s", input);
            rm(targetDir, input);
        }
        if (strcmp(input, "rmdir") == 0) {
            scanf("%s", input);
            rmdir(targetDir, input);
        }
        if (strcmp(input, "cd") == 0) {
            scanf("%s", input);
            cd(&targetDir, input);
        }
        if (strcmp(input, "pwd") == 0) {
            printf("%s\n", pwd(targetDir));
        }
        if (strcmp(input, "mv") == 0) {
            char *old = (char *) malloc(sizeof(MAX_INPUT_LINE_SIZE));
            scanf("%s", input);
            strcpy(old, input);
            char *new = (char *) malloc(sizeof(MAX_INPUT_LINE_SIZE));
            scanf("%s", input);
            strcpy(new, input);
            mv(targetDir, old, new);
        }
        scanf("%s", input);
    } while (strcmp(input, "stop") != 0);
    stop(targetDir);

    return 0;
}
