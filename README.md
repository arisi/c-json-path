c-json-path
===========

Basic Pure C implementation of XPath like query library for JSON-data. No allocs, No c++. Suitable to microcontrollers and sensors.

#API:

Only one function is needed:

``` c
int json_parse(char *json,char *hunt,int match_index,char *match_key,char *match_val);

// json: char buffer that holds the JSON data
// hunt: the key pattern to hunt for
// match_index: the index of the match we want this function to return in:
// match_key: mathed item's key
// match_val: and the actual value as string
// Returns: number of matched records


```
The pattern format is borrowed from MQTT.

If the input JSON is this:
``` json
{
  "test" : 123,
  "sub": [1,2,3],
  "rec": {
    "deep": "value",
    "deep_list": ["a","b"]
  }
}
``` 
So, to find "test", you would run:

``` c
hits=json_parse(json,"test",0,NULL,NULL);
```
NULLs at the match_key and match_val  indicate that we are not interested in values, we just want to find out if our key existst in the JSON, and how many times.

Having found the number of hits, you can traverse the JSON this way:
``` c
char key[MAX_TOKEN_LEN],val[MAX_TOKEN_LEN];
hits=json_parse(json,"rec/+",0,NULL,NULL);
for (i=0;i<hits;i++) {
  json_parse(json,"rec/+",i,key,val);
  printf("%s: %s\n",key,val);
}

Result:

rec/deep: value
```
Now we look for subvalues under top level "rec" (which can be a hash or array).
So the plus sign is the wildcard that matches any key at its level.

And to find all sublevels, as deep as the come: use the "#" wildcard, it matches all remaining levels:
``` c
char key[MAX_TOKEN_LEN],val[MAX_TOKEN_LEN];
hits=json_parse(json,"rec/#",0,NULL,NULL);
for (i=0;i<hits;i++) {
  json_parse(json,"rec/#",i,key,val);
  printf("%s: %s\n",key,val);
}

Result:

rec/deep: value
rec/deep_list/0: a
rec/deep_list/1: b
```
Notice that arrays generate indexes automatically, so you can easily match them with the same notation as hashes.