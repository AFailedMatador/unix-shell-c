#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>


//Helper to print stuff to stdout
void myPrint(char *msg)
{
    write(STDOUT_FILENO, msg, strlen(msg));
}

void freeStuff(char* arr[]){
    for(int i =0; arr[i] != NULL; i++){
        free(arr[i]);
    }
}
//Helper to print the error message when needed
void printAnError(){
    char error_message[30] = "An error has occurred\n";
    write(STDOUT_FILENO, error_message, strlen(error_message));
}

bool checkIfBlank(char *line){
    int i = 0;
    while(line[i] != '\0'){
        char car = line[i];
        if (car != ' ' && car != '\n' && car != '\t'){
            return false;
        }
        i += 1;
    }
    return true;
}

bool checkifNL(char* line){
    for(int i = 0; line[i] != '\0'; i++){
        if(line[i] == '\n'){
            return true;
        }
    }
    return false;
}

void removeLastIfNL(char *line){
    int length = strlen(line);
    int last = length -1;
    if(length > 0 && line[last] == '\n'){
        line[last] = '\0';
    }
}

char* mallocCopy(char *line){
    int length = strlen(line);
    char *copy = malloc(length +1);
    strcpy(copy, line);
    return copy;
}

//Helper to check if a given command is a built in command or not
char* checkIfBuiltIn(char* cmd){
    if(strcmp(cmd, "exit") == 0){
        return cmd;
    }else if(strcmp(cmd, "cd") == 0){
        return cmd;
    }else if(strcmp(cmd, "pwd") == 0){
        return cmd;
    }
    return NULL;
}

int findCharIdx(char *line, char target){
    for(int i = 0; line[i] != '\0'; i++){
        if(line[i] == target){
            return i;
        }
    }
    return -1;

}

int countCharAmount(char *line, char target){
    int count = 0;
    for(int i = 0; line[i] != '\0'; i++){
        if(line[i] == target){
            count += 1;
        }
    }
    return count;
}

char* getBefore(char* line, int idx){
    char *before_char = malloc(idx +1);
    strncpy(before_char, line, idx);
    before_char[idx] = '\0';
    return before_char;
}

int tokenize(char *line, char *args[]){
    int idx = 0;
    char *tok = strtok(line, " \t");
    while(tok != NULL){
        args[idx] = tok;
        idx += 1;
        tok = strtok(NULL, " \t");
    }
    args[idx] = NULL;
    return idx;

}

void doCommand(char *args[], int num){
    char* built = checkIfBuiltIn(args[0]);

    if(built != NULL){
        if(strcmp(args[0], "exit") == 0){
            if(num > 1){
                printAnError();
                return;
            }else{
                exit(0);
            }
                return;
        }
    

        if(strcmp(args[0], "pwd") == 0){
            if(num > 1){
                printAnError();
                return;
            }else{
                char *dir = getcwd(NULL, 0);

                if(dir != NULL){
                    myPrint(dir);
                    myPrint("\n");
                    free(dir);
                }else{
                    printAnError();
                }
            }
            return;
        }

        if(strcmp(args[0], "cd") == 0){
            if(num > 2){
                printAnError();
                return;
            }
            if(num == 1){
                char home[514];
                char *home_dir = getenv("HOME");
                if(home_dir == NULL){
                    printAnError();
                    return;
                }
                strcpy(home, home_dir);
                int result = chdir(home);
                if(result != 0){
                    printAnError();
                    return;
                }
                return;
            }
            else if(num == 2){
                char *path_copy = mallocCopy(args[1]);

                strcpy(path_copy, args[1]);

                int result = chdir(path_copy);
                if(result != 0){
                    printAnError();
                    return;
                }
                return;
            }

        }
    }else{
        pid_t id = fork();
        if(id < 0){
            printAnError();
        }
        if(id == 0){
            execvp(args[0], args);
            printAnError();
            exit(0);
        }
        int state; 
        waitpid(id, &state, 0);
    }
    return;
}

bool checkIfTooLong(char* cmd_buff, FILE *file, bool batch_mode_trigger){
    if(checkifNL(cmd_buff)){
        return false;
    }
    if(feof(file)){
        return false;
    }
    if(batch_mode_trigger){
        myPrint(cmd_buff);
    }
    
    bool no_NL = true;
    char garbage[514];
    while(no_NL){
        char *curr = fgets(garbage, 514, file);
            if(curr == NULL){
                break;
            }
            if(batch_mode_trigger){
                myPrint(garbage);
            }
            if(checkifNL(garbage)){
                printAnError();
                return true;
            }
    }
    return false;
}

bool invalidStart(char* arr){
    if(arr[0] == '.' || arr[0] == '-' || arr[0] == '_'){
        return true;
    }
    return false;
}

bool forkit(char *name, char* cmds[]){
    pid_t ff = fork();

    if(ff < 0){
        printAnError();
        return false;
    }

    if(ff == 0){
        int fd = creat(name, 0666);
        dup2(fd, 1);
        close(fd);
        execvp(cmds[0], cmds);
        printAnError();
        exit(0);
    }

    int state;
    waitpid(ff, &state, 0);
    return true;
}
bool redirectDo(char* before, char* after, char *cmds[], int *count, char **name){
    char* arr[514];
    char* after_c = mallocCopy(after);
    int fcount = tokenize(after_c, arr);

    if(fcount != 1){
        printAnError();
        free(after_c);
        return false;
    }

    *name = mallocCopy(arr[0]);
    free(after_c);

    char *before_copy = mallocCopy(before);
    *count = tokenize(before_copy, cmds);

    if(*count == 0){
        printAnError();
        free(*name);
        return false;
    }

    if(checkIfBuiltIn(cmds[0]) != NULL){
        printAnError();
        free(*name);
        return false;
    }

    if(invalidStart(*name)){
        printAnError();
        free(*name);
        return false;
    }

    return true;

}

bool doRedirection(char* cmd_buff){
    int redirectCount = countCharAmount(cmd_buff, '>');

    if(redirectCount == 0){
        return false;
    }
    if(redirectCount > 1){
        printAnError();
        return true;
    }

    int redirect_idx = findCharIdx(cmd_buff, '>');
    char *before_char = getBefore(cmd_buff, redirect_idx);

    if(checkIfBlank(before_char) == true){
        printAnError();
        return true;
    }

    char *after_char = mallocCopy(cmd_buff + redirect_idx + 1);
    
    char* arr[514];
    int cmd_amm = 0;
    char* name = NULL;

    if(redirectDo(before_char, after_char, arr, &cmd_amm, &name) == false){
        char *clean_arr[] = {before_char, after_char, NULL};
        freeStuff(clean_arr);
        return true;
    }

    char* clean_arr[] = {before_char, after_char, name, NULL};

    int check = open(name, 0);
    if(check != -1){
        close(check);
        printAnError();
        return true;
    }

    if(!forkit(name, arr)){
        freeStuff(clean_arr);
        return true;
    }

    freeStuff(clean_arr);
    return true;
}


bool doAdvancedRedirection(char* cmd_buff){
    int arrow_plus_count = countCharAmount(cmd_buff, '>');
    if(arrow_plus_count == 0){
        return false;
    }

    int arrow_plus_idx = findCharIdx(cmd_buff, '>');

    if(cmd_buff[arrow_plus_idx + 1] != '+'){
        return false;
    }

    if(arrow_plus_count > 1){
        printAnError();
        return true;
    }

    char* before_char = getBefore(cmd_buff, arrow_plus_idx);

    if(checkIfBlank(before_char)){
        printAnError();
        return true;
    }

    char* after_char = mallocCopy(cmd_buff + arrow_plus_idx + 2);

    char* cmds[514];
    int cmd_amm = 0;
    char *name = NULL;

    if(!redirectDo(before_char, after_char, cmds, &cmd_amm, &name)){
        char* clean_arr[] = {before_char, after_char, NULL};
        freeStuff(clean_arr);
        return true;
    }

    char* clean_arr[] = {before_char, after_char, name, NULL};

    int check = open(name, 0);

    if(check == -1){
        if(!forkit(name, cmds)){
            freeStuff(clean_arr);
            return true;
        }

    }else{
        int seek = lseek(check, 0, SEEK_END);
        lseek(check, 0, SEEK_SET);

        char* prev = malloc(seek + 1);
        read(check, prev, seek);
        prev[seek] = '\0';
        close(check);

        pid_t ff = fork();
        if(ff < 0){
            printAnError();
            freeStuff(clean_arr);
            return true;
        }

        if(ff == 0){
            int ffd = creat(name, 0666);
            dup2(ffd, 1);
            close(ffd);
            execvp(cmds[0], cmds);
            printAnError();
            exit(0);
        }

        int state;
        waitpid(ff, &state, 0);

        int new_stuff = open(name, 1);
        lseek(new_stuff, 0, SEEK_END);
        write(new_stuff, prev, seek);
        close(new_stuff);
    }

    freeStuff(clean_arr);
    return true;

}

int splitCommands(char *line){
    char *cc = mallocCopy(line);

    char* cmds[514];
    int num = 0;

    char* tok = strtok(cc, ";");
    while(tok != NULL){
        cmds[num] = tok;
        num += 1;
        tok = strtok(NULL, ";");
    }

    for(int i = 0; i < num; i++){
        if(checkIfBlank(cmds[i]) == true){
            continue;
        }

        char *curr_cmd = mallocCopy(cmds[i]);

        if(doAdvancedRedirection(curr_cmd)){
            continue;
        }

        if(doRedirection(curr_cmd) == true){
            free(curr_cmd);
            continue;
        }
        
        char* arr[514];
        int tot = tokenize(curr_cmd, arr);

        if(!tot){
            free(curr_cmd);
            continue;
        }
        doCommand(arr, tot);
        free(curr_cmd);
    }
    return 0;
}


int main(int argc, char *argv[]) 
{
    //**Arching Variables
    //Too-long command line: A command line that is too long consists of more than 512 of any characters excluding the newline character (Hint: so you must create an array of 514 characters to carry the newline and null-termination character). If you type more than 512 white spaces, it is considered as an invalid command line.
    char cmd_buff[514];
    char *keyboard_input;
    bool batch_mode_trigger = false; 
    FILE *file_input = stdin;

    //Argument # Check
    if(argc > 2){
        printAnError();
        exit(0);
    }else if(argc == 2){
        file_input = fopen(argv[1], "r");
        if(file_input == NULL){
            printAnError();
            exit(0);
        }
        batch_mode_trigger = true;
    }

    //Super Loop
    while(true){
        if(!batch_mode_trigger){
            myPrint("myshell> ");
        }

        keyboard_input = fgets(cmd_buff, 514, file_input);
        if(keyboard_input == NULL){
            exit(0);
        }

        if(checkIfTooLong(cmd_buff, file_input, batch_mode_trigger)){
            continue;
        }


        if(batch_mode_trigger){
            if(checkIfBlank(cmd_buff)){
                continue;
            }else{
                myPrint(cmd_buff);
            }
            
        }

        if(checkIfBlank(cmd_buff) == true){
                continue;
        }

       removeLastIfNL(cmd_buff);
       splitCommands(cmd_buff);


    }
    
}

