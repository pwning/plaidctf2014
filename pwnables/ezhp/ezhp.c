#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// yay for a hilariously bad memory allocator..
struct mm_header {
  size_t sz;
  struct mm_header *next;
  struct mm_header *prev;
  char buf[0];
};

typedef struct mm_header mm_header;

mm_header *base;

void __attribute__((constructor)) init_heap(void) {
  size_t base_amount = 2*sizeof(mm_header) + (1 << 10);
  base = sbrk(base_amount);
  base->next = base->prev = NULL;
  base->sz = sizeof(mm_header);
  mm_header *n = base + 1;
  base->next = n;
  n->prev = base;
  n->next = NULL;
  n->sz = (1 << 10);
}

#define THRESHOLD (sizeof(mm_header) * 4)
void *allocate(size_t sz) {
  sz += sizeof(mm_header);
  sz += sizeof(mm_header) - (sz % sizeof(mm_header));
  mm_header *curr = base;
  while (curr && (curr->sz < sz || (curr->sz & 1))) {
    curr = curr->next;
  }
  if (!curr) {
    size_t base_amount = sizeof(mm_header) + (1 << 10);
    if (sz < base_amount) sz = base_amount;
    curr = sbrk(sz);
    curr->next = NULL;
    curr->sz = sz;
    mm_header *last = base;
    while (last->next) { last = last->next; }
    last->next = curr;
    curr->prev = last;
    curr->sz |= 1;
    return &curr->buf;
  }
  if (curr->sz - sz > THRESHOLD) {
    // split the block.
    mm_header *prev = curr->prev;
    mm_header *next = curr->next;
    mm_header *new_block = (mm_header *)((char *)curr + sz);
    new_block->prev = curr;
    new_block->next = next;
    new_block->sz = curr->sz - sz;
    if (next) next->prev = new_block;
    curr->next = new_block;
    curr->sz = sz;
    curr->sz |= 1;
    return &curr->buf;
  }
  curr->sz |= 1;
  return &curr->buf;
}

void deallocate(void *v) {
  if (!v) return;
  mm_header *curr = (mm_header *)((char *)v - sizeof(mm_header));
  mm_header *prev = curr->prev;
  mm_header *next = curr->next;
  // we don't bother coalescing.
  if (prev) prev->next = next;
  if (next) next->prev = prev;
  curr->next = base->next;
  if (base->next) base->next->prev = curr;
  base->next = curr;
  curr->sz &= ~1;
}

#define MAX_NOTES 1024

char *notes[MAX_NOTES];
int note_count = 0;

void add_note() {
  if (MAX_NOTES-1 <= note_count) { puts("The emperor says there are too many notes!"); fflush(stdout); return; }
  size_t sz;
  puts("Please give me a size.");
  fflush(stdout);
  scanf("%d%*c", &sz);
  
  char *note = allocate(sz);
  notes[note_count] = note;
  note_count++;
}

void remove_note() {
  int id;
  puts("Please give me an id.");
  fflush(stdout);
  scanf("%d%*c", &id);
  if (id > note_count || id < 0) return;
  if (notes[id]) deallocate(notes[id]);
  notes[id] = NULL;
}

void read_into_note() {
  puts("Please give me an id.");
  fflush(stdout);
  int id;
  scanf("%d%*c", &id);
  if (id > note_count || id < 0) return;
  if (!notes[id]) return;
  size_t sz;
  puts("Please give me a size.");
  fflush(stdout);
  scanf("%d%*c", &sz);
  puts("Please input your data.");
  fflush(stdout);
  read(STDIN_FILENO, notes[id], sz);
}

void print_note() {
  int id;
  puts("Please give me an id.");
  fflush(stdout);
  scanf("%d%*c", &id);
  if (id > note_count || id < 0) { return; }
  puts(notes[id]);
}

int read_menu_option() {
  int res;
  puts("Please choose an option.");
  fflush(stdout);
  scanf("%d%*c", &res);
  return res;
}

void print_menu() {
  puts("Please enter one of the following:");
  puts("1 to add a note.");
  puts("2 to remove a note.");
  puts("3 to change a note.");
  puts("4 to print a note.");
  puts("5 to quit.");
  fflush(stdout);
}

int main() {
  int opt = 0;
  while (opt != 5) {
    print_menu();
    opt = read_menu_option();
    switch (opt) {
    case 1:
      add_note(); break;
    case 2:
      remove_note(); break;
    case 3:
      read_into_note(); break;
    case 4:
      print_note(); break;
    case 5:
      break;
    default:
      exit(0); break;
    }
  }
  return 0;
}
