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


#define MAX_TOKEN_LEN 20
#define MAX_LEVELS 20

int json_parse(char *json,char *hunt,int match_index,char *match_key,char *match_val) {

// json: char buffer that holds the JSON data
// hunt: the key pattern to hunt for
// match_index: the index of the match we want this function to return in:
// match_key: mathed item's key
// match_val: and the actual value as string
// Returns: number of matched records

  int i,j,level=0,state=0;
  char token[MAX_TOKEN_LEN+1];
  int token_len=0;
  int token_type=0;

  int last_level=0;
  int last_type=0;

  int cnt=0;

  char key[MAX_TOKEN_LEN+1];
  char keys[MAX_LEVELS][MAX_TOKEN_LEN];
  int level_index[MAX_LEVELS];
  int level_array[MAX_LEVELS]; //arraymode per level -- fix this
  int match_count=0;
  char match[MAX_LEVELS][MAX_TOKEN_LEN];
  int match_level=0;

  void do_data(int level,char *token,int state) {
    int i,mismatch;
    char val[MAX_TOKEN_LEN+1];
    char keybuf[MAX_TOKEN_LEN*MAX_LEVELS+1];
    if (state==1)
      strcpy(val,token);
    else
    if (strchr(token,'.'))
      sprintf(val,"%f",atof(token));
    else
      sprintf(val,"%d",atoi(token));
    strcpy(keys[level],key);
    keybuf[0]=0;
    for (i=0,mismatch=0; i<=level; i++) {
      sprintf(&keybuf[strlen(keybuf)],"/%s",keys[i]);
      if (strcmp(match[i],"+")==0)
        ;
      else if (strcmp(match[i],"#")==0) {
        if (!mismatch)
          mismatch=-10000;
      } else if (strcmp(keys[i],match[i])!=0)
        mismatch++;
    }
    if (level+1!=match_level)
      mismatch+=100;

    if (mismatch<=0) {
      if (match_count==match_index) {
        if (match_key)
          strcpy(match_key,keybuf);
        if (match_val)
        strcpy(match_val,val);
      }
      match_count++;
    }
    //printf("%s : %s  |  %d\n",keybuf,val,mismatch);

  }




  void do_token(char *token,int token_type,int state,int level) {
    if (token_type==0 && level>last_level && last_type==0 && !level_array[level]) {
      strcpy(keys[level],token); // store current key, as we are going deeper...
    }
    if (token_type==0) {
      if (!level_array[level]) {
        strcpy(key,token);
        strcpy(keys[level],token); //
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


  match_count=0;
  match_key[0]=0;
  match_val[0]=0;
  for (i=0,j=0,match_level=0; i<strlen(hunt)+1; i++) {
    if (hunt[i]=='/' || hunt[i]==0) {
      j=0;
      if (match_level++>MAX_LEVELS) {
        printf("Error: too deep levels at match string, %d\n",match_level);
        exit (-1);
      }
    } else {
      match[match_level][j++]=hunt[i];
      if (j>MAX_TOKEN_LEN) {
        printf("Error: hunt token too long\n");
        exit(-1);
      }
      match[match_level][j]=0;
      if (strcmp(match[match_level],"#")==0) {
        match_level++;
        break; // need not go deeper!
      }
    }
  }
  //for (i=0;i<match_level;i++)
  //  printf("match: '%s'\n",match[i]);
  for (i=0,level=-1; i<strlen(json); i++) {
    if (state) {   // inside quote or number
      if (state==2) {
        if (json[i]>='0' && json[i]<='9' || json[i]=='.') {
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
      if (level>=0)
        if (level_array[level])
          sprintf(keys[level],"%d",level_index[level]++);
      if (level+1>MAX_LEVELS) {
        printf("Error: too deep levels, %d\n",level+1);
        exit (-1);
      }
      level+=1;
      state=0;
      level_array[level]=0;
      token_type=0;
    } else if (json[i]=='}') {
      level-=1;
      continue;
    } else if (json[i]=='[') {
      if (level+1>MAX_LEVELS) {
        printf("Error: too deep levels, %d\n",level+1);
        exit (-1);
      }
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
  if (level!=-1)
    printf("Error: structure mismatch\n");
  return(match_count);
}


#define MAX_JSON_LEN 4000
char json[MAX_JSON_LEN];

int main(int argc, char **argv)
{
  if (argc < 3)
  {
    printf("usage: c-json-path fname.json match [index]\n");
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
  //printf("Red: %ld bytes\n",fsize);
  //puts(json);
  char match_key[MAX_TOKEN_LEN*MAX_LEVELS+1];
  char match_val[MAX_TOKEN_LEN+1];
  int hits;

  if (argc==3) {
    hits=json_parse(json,argv[2],0,match_key,match_val);
    printf("results: %d, first hit: '%s'='%s'\n",hits,match_key,match_val);
  } else {
    if (strcmp(argv[3],"all")==0) {
      hits=json_parse(json,argv[2],0,match_key,match_val);
      int i;
      for (i=0; i<hits; i++) {
        json_parse(json,argv[2],i,match_key,match_val);
        printf("results: %d, hit# %d: '%s'='%s'\n",hits,i,match_key,match_val);
      }
    } else {
      hits=json_parse(json,argv[2],atoi(argv[3]),match_key,match_val);
      printf("results: %d, hit# %d: '%s'='%s'\n",hits,atoi(argv[3]),match_key,match_val);
    }
  }
}