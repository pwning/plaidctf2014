#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "art.h"
typedef enum {GRASS, HEAL, INSPECT, RELEASE, ART, NONE} choice_t;
typedef enum {RATATA, CHARIZARD, KAKUNA, PIDGEOT} pokemon_t;
int battle_count = 0;
static char * gust = "Gust";
static char * blaze = "Blaze";
static char * tackle = "Tackle";
static void * pokemon_list[5] = {NULL, NULL, NULL, NULL, NULL};
static pokemon_t pokemon_type[5] = {RATATA, RATATA, RATATA, RATATA, RATATA};


typedef struct Charizard {
    char name[15];
    char art[sizeof(charizard_art)+1];
    int health;
    int attack;
    char ** attack_name; 
    void (*print_pokemon) (struct Charizard *);
} Charizard;
void print_charizard(Charizard *c);
typedef struct Kakuna {
    char name[15];
    char art[sizeof(kakuna_art)+1];
    int health;
    int attack;
    char ** attack_name; 
    void (*print_pokemon) (struct Kakuna *);
} Kakuna;
typedef struct Pidgeot {
    char name[15];
    char art[sizeof(pidgeot_art)+1];
    int health;
    int attack;
    char ** attack_name; 
    void (*print_pokemon) (struct Pidgeot *);
} Pidgeot;

void print_welcome(){
    printf("%s\n", "Thank you for helping test CTF plays Pokemon! Keep in mind that this is currently in alpha which means that we will only support one person playing at a time. You will be provided with several options once the game begins, as well as several hidden options for those true CTF Plays Pokemon fans ;). We hope to expand this in the coming months to include even more features!  Enjoy! :)");
}

void print_charizard(Charizard *p){
    printf("Name: %s\n", p->name);
    printf("%s\n", p->art);
    printf("Current Health: %d\n", p->health);
    printf("Attack Power: %d\n", p->attack);
    printf("Attack: %s\n", *p->attack_name);
}
void print_kakuna(Kakuna *p){
    printf("Name: %s\n", p->name);
    printf("%s\n", p->art);
    printf("Current Health: %d\n", p->health);
    printf("Attack Power: %d\n", p->attack);
    printf("Attack: %s\n", *p->attack_name);
}
void print_pidgeot(Pidgeot *p){
    printf("Name: %s\n", p->name);
    printf("%s\n", p->art);
    printf("Current Health: %d\n", p->health);
    printf("Attack Power: %d\n", p->attack);
    printf("Attack: %s\n", *p->attack_name);
}
size_t get_choice(){
    int choice = getchar();
    getchar();
    choice -= '0';
    return (size_t) choice;
}
choice_t menu_prompt(){
    printf("Choose an Option:\n1. Go into the Grass\n\
2. Heal your Pokemon\n\
3. Inpect your Pokemon\n\
4. Release a Pokemon\n\
5. Change Pokemon artwork\n\n");
    choice_t choice = NONE;  
    while(choice == NONE) {
        switch (get_choice()) {
            case 1:
                return GRASS;
            case 2:
                return HEAL;
            case 3:
                return INSPECT;
            case 4:
                return RELEASE;
            case 5:
                return ART;
        }
    }
}

size_t list_pokemon_choices() {
    size_t i;
    printf("%s\n", "Choose a Pokemon!");
    for (i=0;i<5; i++) {
        if (pokemon_list[i]) {
            printf("%d. %s\n", i+1, (char *) pokemon_list[i]);
        }
        else {
            break;
        }
    }
    size_t choice = get_choice();
    if (choice > i) {
        exit(1);
    }
    return choice-1;
}

size_t get_battle_choice(){
    printf("Choose an Option:\n1. Attack\n\
2. Throw Pokeball\n\
3. Run\n");
    return get_choice();
}

void fight(void * enemy, pokemon_t type, int enemy_health, int enemy_attack, char * enemy_attack_name, char * enemy_name){
    printf("%s %s %s\n", "A wild", enemy_name, "appears!");
    Pidgeot * birdJesus = pokemon_list[0];
    while(1) {
        if (birdJesus->health <= 0) {
            printf("%s is incapacitated! You cannot battle like this!\n", birdJesus->name);
            free(enemy);
            return;
        }
        //list 1. attack, 2 catch, 3. Run
        size_t choice = get_battle_choice();
        switch (choice){
        case 1:
            printf("%s used %s. It did %d damage to %s!\n", birdJesus->name, *birdJesus->attack_name, birdJesus->attack, enemy_name);
            enemy_health -= birdJesus->attack;
            if (enemy_health <=0) {
                printf("%s %s\n", enemy_name, "has been defeated!");
                free(enemy);
                return;
            }
            printf("%s used %s. It did %d damage to %s!\n", enemy_name, enemy_attack_name, enemy_attack, birdJesus->name);
            birdJesus->health -= enemy_attack;
            if (birdJesus->health <=0) {
                printf("%s %s\n", birdJesus->name, "has been defeated! You are forced to runaway!");
                free(enemy);
                return;
            }
            break;
        case 2:
            printf("You throw a Pokeball!\n");
            if (enemy_health <= 20) {
                printf("You successfully caught %s!\n", enemy_name); 
                printf("What would you like to name this Pokemon?\n");
                //read 14 char name
                size_t current = 0;
                char buf[15];
                int len = read(STDIN_FILENO, enemy_name, 14);
                enemy_name[len] = '\0';
                int i;
                for (i=0;i<5;i++) {
                    if (pokemon_list[i] == NULL) {
                        pokemon_list[i] = enemy;
                        pokemon_type[i] = type;
                        return;
                    }
                }
                printf("Oh no! you don't have any more room for a Pokemon! Choose a pokemon to replace!\n");
                size_t choice = list_pokemon_choices();
                if (choice == 0) {
                    printf("%s can't be freed!\n", (char *)pokemon_list[0]);
                    return;
                }
                if (choice > 4) {
                    printf("Invalid Choice!\n");    
                    return;
                }
                free(pokemon_list[choice]);
                pokemon_list[choice] = enemy;
                //THIS IS THE BUG 
                //pokemon_type[i] = type;
                return;
            }
            printf("You couldn't catch %s!\n", enemy_name);
            break;
        case 3:
            printf("You run away!\n");
            free(enemy);
            return;
        case 4:
            printf("Which Pokemon do you want to switch in?\n");
            size_t swap = list_pokemon_choices();
            birdJesus = pokemon_list[swap];
        }
    }

}
void battle(pokemon_t enemyType){
    if (enemyType == CHARIZARD) {
        Charizard * enemy = (Charizard *) malloc(sizeof(Charizard)); 
        strcpy(enemy->name, "Charizard");
        strcpy(enemy->art, charizard_art);
        enemy->attack_name = &blaze;
        enemy->health = 100;
        enemy->attack = 10;
        enemy->print_pokemon = &print_charizard;
        fight(enemy, CHARIZARD, enemy->health, enemy->attack, *enemy->attack_name, enemy->name); 
    } 
    else if (enemyType == KAKUNA) {
        Kakuna * enemy = (Kakuna *) malloc(sizeof(Kakuna)); 
        strcpy(enemy->name, "Kakuna");
        strcpy(enemy->art, kakuna_art);
        enemy->attack_name = &tackle;
        enemy->health = 20;
        enemy->attack = 1;
        enemy->print_pokemon = &print_kakuna;
        fight(enemy, KAKUNA, enemy->health, enemy->attack, *enemy->attack_name, enemy->name); 
    }
    else {
        return;
    }

}
void grass(){
    printf("%s\n", "You walk into the tall grass!");
    sleep(1);
    printf("%s\n", ".");
    sleep(1);
    printf("%s\n", ".");
    sleep(1);
    printf("%s\n", ".");
    sleep(1);
    battle_count++;
    if ((battle_count % 13) == 0){
        battle(CHARIZARD);
    }
    else if ((battle_count % 2) == 0){
        battle(KAKUNA);
    }
    else {
        printf("%s\n","You failed to find any Pokemon!");
        return;
    }
    return;
};
void heal_all_pokemon(){
    int i;
    for (i=0;i<5; i++) {
        if (pokemon_list[i]) {
            if (pokemon_type[i] == PIDGEOT) {
                ( (Pidgeot *) pokemon_list[i])->health = 1000;
            }
            else if (pokemon_type[i] == CHARIZARD) {
                ( (Charizard *) pokemon_list[i])->health = 100;
            }
            else if (pokemon_type[i] == KAKUNA) {
                ( (Kakuna *) pokemon_list[i])->health = 30;
            }
        }
        else {
            return;
        }
    }
    return;
}
void heal(){
    printf("%s\n", "Welcome to the Pokemon Center, where we will heal your Pokemon back to full health. Would you like me to take your Pokemon?");
    printf("1. Yes\n2. No\n");
    //read...
    size_t choice = get_choice();
    if (choice == 1){
        heal_all_pokemon();
        printf("%s\n","Okay, I'll take your Pokemon for a few seconds.");
        sleep(5);
        printf("%s\n","Your Pokemon are now healed. We hope to see you again.");
    }
    else {
        printf("%s\n", "We hope to see you again!");
    }
    return;
};


void inspect_all(){
    printf("%s\n", "Here are all of the stats on your current Pokemon!");
    int i;
    for (i=0;i<5;i++) {
        if (pokemon_list[i]) {
            if (pokemon_type[i] == PIDGEOT) {
                ( (Pidgeot *) pokemon_list[i])->print_pokemon(pokemon_list[i]);
            }
            else if (pokemon_type[i] == CHARIZARD) {
                ( (Charizard *) pokemon_list[i])->print_pokemon(pokemon_list[i]);
            }
            else if (pokemon_type[i] == KAKUNA) {
                ( (Kakuna *) pokemon_list[i])->print_pokemon(pokemon_list[i]);
            }
        }
        else {
            return;
        }
    }
    return;
};
void change_art(){
    char buf[4000];
    size_t choice = list_pokemon_choices();
    Pidgeot * p = (Pidgeot *)pokemon_list[choice];
    size_t len = strlen(p->art);
    size_t current = 0;
    while (current < len) {
        current += read(STDIN_FILENO, buf+current, len-current);
    }
    memcpy(p->art, buf, len);
    p->art[len] = '\0';
    printf("%s\n", "I'm sure you'll love to show this new art to your friends!");
    return;

}

void release() {
    size_t choice = list_pokemon_choices();
    if (choice == 0) {
        printf("%s cannot be set free!\n", pokemon_list[0]);
        return;
    }
    printf("%s has been set free!\n", (char *) pokemon_list[choice]);
    free(pokemon_list[choice]);
    pokemon_list[choice] = NULL;
    pokemon_type[choice] = RATATA; //TODO: bug here?
    for (;choice < 4; choice++) {
        if (pokemon_list[choice+1]) {
            pokemon_list[choice] = pokemon_list[choice+1];
            pokemon_type[choice] = pokemon_type[choice+1];
        }
        else {
            break;
        }
    }
    
}
void init_game() {
    Pidgeot * birdJesus = (Pidgeot *) malloc(sizeof(Pidgeot)); 
    strcpy(birdJesus->name, "Bird Jesus");
    strcpy(birdJesus->art, pidgeot_art);
    birdJesus->attack_name = &gust;
    birdJesus->health = 1000;
    birdJesus->attack = 20;
    birdJesus->print_pokemon = &print_pidgeot;
    pokemon_list[0] = birdJesus;
    pokemon_type[0] = PIDGEOT;
    return;
}
int main(int argc, char ** argv){
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);
    sleep(1);
    init_game();
    print_welcome();
    while(1) {
        choice_t choice = menu_prompt();
        switch (choice)
        {
            case GRASS:
                grass(); 
                break;
            case HEAL:
                heal();
                break;
            case INSPECT:
                inspect_all();
                break;
            case RELEASE:
                release();
                break;
            case ART:
                change_art(); 
                break;
        }
    }
    return 0;
}
