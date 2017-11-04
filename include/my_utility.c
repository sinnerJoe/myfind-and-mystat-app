#include<my_utility.h>
#include<string.h>

int cur_pos = 0;
int total = 0;
short positions[10];
char * last_str;

char * spacetok(char * str){
    if(str == NULL){
        
        if (total == 0)
            return NULL;
        else if(cur_pos >= total){
            total = 0;
            cur_pos = 0;
            return NULL;
        }
        else{
            cur_pos++;
            return last_str + positions[cur_pos-1];
        }
        
    }
    else{
        total = 0;
        last_str = str;
        int sz = strlen(str);
        char is_quoted = 0;
        char searching = 0;

        if(str[0] != ' '){
            total = 1;
            positions[0] = 0;
        }

        for(int i=0; i<sz; i++){
            if(searching && str[i] != ' '){
                positions[total] = i;
                total++;
                searching = 0;
            }

            if(str[i]=='\"'){
                is_quoted = !is_quoted;
            }
            else if(!is_quoted && str[i] == ' '){
                searching = 1;
                str[i] = '\0';
            }
        }
        if(total == 0){
            cur_pos = 0;
            return NULL;
        }
        else
            cur_pos = 1;
        
        return str + positions[0];
    }
}

short word_count(){
    return total;
}

char * removeQuotes(char * quotedString){
    int sz = strlen(quotedString);
    if(quotedString[sz-1] == '\"')
        quotedString[sz-1] = '\0';
    if(quotedString[0] == '\"'){
        quotedString[0] = '\0';
        return quotedString + 1;
    }
    return quotedString;
}