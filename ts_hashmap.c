#include <limits.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ts_hashmap.h"

/**
 * Creates a new thread-safe hashmap.
 *
 * @param capacity initial capacity of the hashmap.
 * @return a pointer to a new thread-safe hashmap.
 */
ts_hashmap_t *initmap(int capacity)
{

  struct ts_hashmap_t *hashMap = malloc(sizeof(struct ts_hashmap_t));

  hashMap->table = malloc(capacity * sizeof(struct ts_entry_t *));
  hashMap->locks = malloc(capacity * sizeof(struct pthread_mutex_t *)); // allocate lock array

  // init all the locks in the array
  for (int i = 0; i < capacity; i++)
  {
    struct pthread_mutex_t *lock = malloc(sizeof(pthread_mutex_t));
    hashMap->locks[i] = lock;
    pthread_mutex_init(lock, NULL);
  }

  hashMap->capacity = capacity;
  hashMap->size = 0;
  return hashMap;
}

/**
 * Obtains the value associated with the given key.
 * @param map a pointer to the map
 * @param key a key to search
 * @return the value associated with the given key, or INT_MAX if key not found
 */
int get(ts_hashmap_t *map, int key)
{

  // calculate the array index
  int index = getKeyIndex(key, map->capacity);

  // lock the lock with the specific index
  pthread_mutex_t *l = map->locks[index];
  pthread_mutex_lock(l);

  struct ts_entry_t *entry = map->table[index];

  if (entry == NULL)
  {
    // key not found
    pthread_mutex_unlock(map->locks[index]);
    return INT_MAX;
  }

  struct ts_entry_t *current = entry;

  while (current != NULL)
  {
    if (current->key == key)
    {

      // value found. return
      pthread_mutex_unlock(map->locks[index]);
      return current->value;
    }
    current = current->next;
  }

  // value was not found in the correct array. return
  pthread_mutex_unlock(map->locks[index]);
  return INT_MAX;
}

/**
 * Associates a value associated with a given key.
 * @param map a pointer to the map
 * @param key a key
 * @param value a value
 * @return old associated value, or INT_MAX if the key was new
 */
int put(ts_hashmap_t *map, int key, int value)
{

  int index = getKeyIndex(key, map->capacity);

  pthread_mutex_t *l = map->locks[index];
  pthread_mutex_lock(l);
  struct ts_entry_t *entry = map->table[index];
  int oldValue = INT_MAX;
  if (entry == NULL)
  {

    // insert a new entry to the the table
    struct ts_entry_t *newEntry = malloc(sizeof(ts_entry_t));
    newEntry->key = key;
    newEntry->value = value;
    newEntry->next = NULL;
    map->table[index] = newEntry;

    // increment size
    map->size += 1;
  }
  else
  {

    // go through the index array till we find it or get to null
    struct ts_entry_t *current = entry;
    while (current->next != NULL && current->key != key)
    {
      current = current->next;
    }

    if (current->key == key)
    {
      // change the value
      oldValue = current->value;
      current->value = value;
    }
    else
    {
      // insert new entry at the end
      struct ts_entry_t *newEntry = malloc(sizeof(ts_entry_t));
      newEntry->key = key;
      newEntry->value = value;
      current->next = newEntry;
      map->size += 1;
    }
  }

  pthread_mutex_unlock(map->locks[index]);
  return oldValue;
}

/**
 * Removes an entry in the map
 * @param map a pointer to the map
 * @param key a key to search
 * @return the value associated with the given key, or INT_MAX if key not found
 */
int del(ts_hashmap_t *map, int key)
{

  // calc index
  int index = getKeyIndex(key, map->capacity);

  pthread_mutex_t *l = map->locks[index];
  pthread_mutex_lock(l);
  struct ts_entry_t *entry = map->table[index];
  int oldValue = INT_MAX;

  if (entry == NULL)
  {
    pthread_mutex_unlock(map->locks[index]);
    return INT_MAX; // key not found
  }

  // if first element, free up and change the array pointer
  if (entry->key == key)
  {
    oldValue = entry->value;

    map->table[index] = entry->next;
    free(entry);
    pthread_mutex_unlock(map->locks[index]);
    return oldValue;
  }

  // reccur through list until key is found or not found
  struct ts_entry_t *current = entry;
  
  while (current->next != NULL)
  {

    ts_entry_t *nextEntry = current->next;
    if (nextEntry->key == key)
    {
      
      // value found. switch the pointer and free up the delete entry.


      oldValue = nextEntry->value;
      current->next = nextEntry->next;
      
      free(nextEntry);

      // return the value
      pthread_mutex_unlock(map->locks[index]);
      return oldValue;

     
    }
    current = nextEntry;

   
  }
 

  // value was not found in the correct array. return
  pthread_mutex_unlock(map->locks[index]);
  return INT_MAX;
}

/**
 * @return the load factor of the given map
 */
double lf(ts_hashmap_t *map)
{
  return (double)map->size / map->capacity;
}

/**
 * Prints the contents of the map
 */
void printmap(ts_hashmap_t *map)
{
  for (int i = 0; i < map->capacity; i++)
  {
    printf("[%d] -> ", i);
    ts_entry_t *entry = map->table[i];
    while (entry != NULL)
    {
      printf("(%d,%d)", entry->key, entry->value);
      if (entry->next != NULL)
        printf(" -> ");
      entry = entry->next;
    }
    printf("\n");
  }
}

/**
 * returns the index of the key within the  hashamp given the size
 */
int getKeyIndex(int key, int capacity)
{
  unsigned int newKey = (unsigned int)key;

  if (capacity == 0)
  {
    return 0;
  }

  return newKey % capacity;
}