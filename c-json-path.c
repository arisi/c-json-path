/* Minimal JSONPath library for C*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
Minimal JSONPath library for C
Copyright (C) 2014  Ari Siitonen

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#define MAX_JSON_LEN 4000
#define MAX_TOKEN_LEN 20
#define MAX_LEVELS 20
char json[MAX_JSON_LEN];

int last_level=0;
int last_type=0;
//int array=0;
int cnt=0;

char key[MAX_TOKEN_LEN+1];
char keys[MAX_LEVELS][MAX_TOKEN_LEN];
int level_index[MAX_LEVELS];
int level_array[MAX_LEVELS]; //arraymode per level -- fix this

void do_data(int level,char *token,int state) {
  int i;
  for (i=1;i<level;i++)
    printf("['%s']",keys[i]); 
  if (state==1)
    printf("['%s']='%s'\n",key,token);
  else
    if (strchr(token,'.'))
      printf("['%s']=%f\n",key,atof(token));
    else
      printf("['%s']=%d\n",key,atoi(token));
}
  
void do_token(char *token,int token_type,int state,int level) {
  if (token_type==0 && level>last_level && last_type==0 && ! level_array[level]) {
   strcpy(keys[level],token);
  }
  if (token_type==0) {
    if (!level_array[level]) {
      strcpy(key,token);
      strcpy(keys[level],token);
    } else {
      sprintf(key,"%d",level_index[level]);
      sprintf(keys[level],"%d",level_index[level]++);
      do_data(level,token,state);
    }
  } else if (token_type==1 && last_type!=0) 
    printf("Error: no value for %s\n",key);
  else if (token_type==1 && last_type==0) {
    do_data(level,token,state);
  }
  last_type=token_type;
  last_level=level;      
}


int main(int argc, char**argv)
{
  if (argc != 2)
  {
    printf("usage: c-json-path fname.json \n");
    exit(1);
  }
  FILE *f = fopen(argv[1], "r");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);
  if (fsize>MAX_JSON_LEN) {
    printf("Error: Too long JSON file: %ld , max %d \n",fsize,MAX_JSON_LEN);
    exit(1);
  }
  fread(json, fsize, 1, f);
  fclose(f);
  printf("Red: %ld bytes\n",fsize);
  puts(json);
  printf("Analysis:\n");
  int i,j,level=0,state=0;
  char token[MAX_TOKEN_LEN+1];
  int token_len=0;
  int token_type=0;
  for (i=0;i<fsize;i++) {
      if (state) { // inside quote or number
        if (state==2) {
          if (json[i]>='0' && json[i]<='9'  || json[i]=='.') {
            if (token_len<MAX_TOKEN_LEN)
              token[token_len++]=json[i];
            else
              printf("Token Overflow\n");
            continue;
          } else {
            token[token_len]=0;
            do_token(token,token_type,state,level);
            state=0;
          }
        } else {
          if (json[i]!='"') {
            if (token_len<MAX_TOKEN_LEN)
              token[token_len++]=json[i];
            else
              printf("Token Overflow\n");
          } else {
            token[token_len]=0;
            do_token(token,token_type,state,level);
            state=0;
          }
          continue;
        }
      }

      if (json[i]==',') {
        token_type=0;
      } else if (json[i]==':') {
        token_type=1;
      } else if (json[i]>='0' && json[i]<='9' || json[i]=='-' || json[i]=='+' || json[i]=='.') {
        token_len=1;
        token[0]=json[i];
        state=2;
      } else if (json[i]=='{') {
        if (level_array[level]) {
          sprintf(keys[level],"%d",level_index[level]++);
        }
        level+=1;
        state=0;
        level_array[level]=0;
        token_type=0;
      } else if (json[i]=='}') {
        level-=1;
        continue; 
      } else if (json[i]=='[') {
        level+=1;
        state=0;
        token_type=0;
        level_array[level]=1;
        level_index[level]=0;
     } else if (json[i]==']') {
        level_array[level]=0;
        level-=1;
       continue;
      } else if (json[i]=='"' && state==0) {
        state=1;
        token_len=0;
        continue;
      } 
    }
    if (level)
      printf("Error: structure mismatch\n");
}