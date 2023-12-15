

/*
===============================================================================
    Lost: An Epic Journey
    Authors: KARAMI Anir (C Group), PREVOST Louis (C Group)

    File: main.c
    Description: This file contains the main logic and execution flow for the
                 entire Lost: An Epic Journey game. It orchestrates player
                 interactions, combat mechanics, character personalization, and
                 progression through the immersive storyline.
===============================================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>

// Maximum length allowed for a player's name
#define MAX_NAME_LENGTH 50

// Maximum number of monsters that can be registered
#define MAX_MONSTERS 10

// Language code for French
#define LANG_FR "fr"

// Language code for English
#define LANG_EN "en"

// Class code for Chasseur
#define CLASS_CHASSEUR 'C'

// Class code for Arcaniste
#define CLASS_ARCANISTE 'A'

#define ANSWER_CODE 1101
#define ANSWER_CODE_2 36

#define MAX_HINTS 3

#define EXP_PER_LEVEL 10000

#define MAX_ROWS 100
#define MAX_COLS 100

#define PROGRESSION_TOTAL 6

// Structure to represent a key-value pair for language texts
typedef struct
{
    char *key;   // Key for the text entry
    char *value; // Value associated with the key
} TextEntry;

// Structure to store language texts as an array of TextEntry
typedef struct
{
    TextEntry *entries; // Array of text entries
    int count;          // Number of text entries
} LanguageTexts;

// Structure to represent the characteristics of a player class
typedef struct
{
    const char *name;   // Name of the class
    int healthPoints;   // Initial health points of the class
    int combatPoints;   // Combat points (not currently used)
    int simpleAttack;   // Damage points for a simple attack
    int criticalAttack; // Damage points for a critical attack
    int ultimateAttack; // Damage points for an ultimate attack
} AttackerClass;

// Structure to represent an attack move
typedef struct
{
    char *name;             // Name of the attack move
    int successProbability; // Probability of success (percentage)
    int chasseurDamage;     // Damage points for Chasseur class
    int arcanisteDamage;    // Damage points for Arcaniste class
} Attack;

// Structure to represent a monster
typedef struct
{
    int tier;          // Tier or level of the monster
    int healthPoints;  // Health points of the monster
    int attackDamage;  // Attack damage of the monster
    char dropItem[50]; // Item dropped by the monster
} Monster;

// Structure to represent the player's inventory
typedef struct
{
    int poison; // Weapon in the inventory
    int healthPotion;
    int shield;
} Inventory;

// Structure to represent a player
typedef struct
{
    char name[MAX_NAME_LENGTH]; // Name of the player
    int healthPoints;           // Health points of the player
    int playerLevel;            // Player Level
    int playerExp;              // Player experience
    int progression;            // Player progression
    AttackerClass playerClass;  // Class of the player (Hunter or or arcanist)
    Inventory inventory;        // Player's inventory
} Player;

/**
 * @struct MapData
 * @brief Structure to represent map data.
 */
struct MapData
{
    char id[20];  /**< The ID of the map. */
    int startRow; /**< The starting row of the map. */
    int endRow;   /**< The ending row of the map. */
};

/**
 * @var maps
 * @brief Array of map data.
 */
struct MapData maps[] = {
    {"EPISODE_1", 1, 14},
    {"path_1", 16, 29},
    {"path_2", 31, 44},
    {"LABORATOIRE", 46, 59},
    {"CAMP_NORD", 61, 74}};

/**
 * @var map
 * @brief 2D array to represent the game map.
 */
char map[MAX_ROWS][MAX_COLS];

LanguageTexts texts; /**< Structure to store language texts for the game. */

Player player; /**< Structure to store player information. */

Monster registeredMonsters[MAX_MONSTERS]; /**< Array to store registered monsters. */

int numRegisteredMonsters; /**< Number of registered monsters. */

char currentLevel[20]; /**< Current level of the game. */

Attack simpleAttack, criticalAttack, ultimateAttack; /**< Structures to store different types of attacks. */

/**
 * @brief Loads texts from a file and populates the texts structure.
 *
 * This function opens the specified file for reading and reads each line from the file.
 * Each line is tokenized using semicolon as the delimiter, and the key-value pairs are stored in the texts structure.
 * Memory is dynamically allocated for each TextEntry in the texts structure.
 *
 * @param filename The name of the file to load texts from.
 */
void loadTexts(const char *filename)
{
    // Open the file for reading
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        // Print an error message and exit the program if the file cannot be opened
        fprintf(stderr, "Error opening the file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    char line[512];
    // Read each line from the file
    while (fgets(line, sizeof(line), file) != NULL)
    {
        // Remove the newline character from the end of the line
        line[strcspn(line, "\n")] = '\0';
        // Tokenize the line using semicolon as the delimiter
        char *key = strtok(line, ";");
        char *value = strtok(NULL, ";");

        // Check if both key and value are not NULL
        if (key != NULL && value != NULL)
        {
            // Allocate memory for a new TextEntry in the texts structure
            texts.entries = realloc(texts.entries, (texts.count + 1) * sizeof(TextEntry));
            // Check for memory allocation failure
            if (texts.entries == NULL)
            {
                fprintf(stderr, "c'est pas bon ca\n");
                exit(EXIT_FAILURE);
            }
            // Copy the key and value into the new TextEntry
            texts.entries[texts.count].key = strdup(key);
            texts.entries[texts.count].value = strdup(value);
            texts.count++;
        }
    }

    // Close the file after reading
    fclose(file);
}

// Function to retrieve a text entry based on a key
/**
 * Retrieves the corresponding text value for a given key.
 *
 * @param key The key to search for in the texts structure.
 * @return The corresponding text value if a match is found, otherwise "Text not found".
 */
const char *getText(const char *key)
{
    // Iterate through each text entry in the texts structure
    for (int i = 0; i < texts.count; ++i)
    {
        // Check if the current entry's key matches the specified key (case-insensitive comparison)
        if (strcasecmp(texts.entries[i].key, key) == 0)
        {
            // Return the corresponding value if a match is found
            return texts.entries[i].value;
        }
    }
    // Return a default message if the specified key is not found
    return "Text not found";
}

/**
 * Registers a new monster with the specified attributes.
 *
 * @param tier The tier of the monster.
 * @param healthPoints The health points of the monster.
 * @param attackDamage The attack damage of the monster.
 * @param dropItem The item dropped by the monster.
 */
void registerMonster(int tier, int healthPoints, int attackDamage, const char *dropItem)
{
    if (numRegisteredMonsters < MAX_MONSTERS)
    {
        registeredMonsters[numRegisteredMonsters].tier = tier;
        registeredMonsters[numRegisteredMonsters].healthPoints = healthPoints;
        registeredMonsters[numRegisteredMonsters].attackDamage = attackDamage;
        strcpy(registeredMonsters[numRegisteredMonsters].dropItem, dropItem);

        numRegisteredMonsters++;
    }
    else
    {
        fprintf(stderr, "Max number of monsters reached\n");
    }
}

// Function to create a player with specified characteristics

/**
 * @brief Adds experience points (xp) to the player and updates the player's level if necessary.
 *
 * @param player Pointer to the Player structure.
 * @param xpToAdd The amount of experience points to add.
 */
void addXp(Player *player, int xpToAdd)
{
    player->playerExp += xpToAdd;

    int levelsGained = 0;

    while (player->playerExp >= EXP_PER_LEVEL)
    {
        player->playerExp -= EXP_PER_LEVEL;
        player->playerLevel++;
        levelsGained++;
    }
}

/**
 * Creates a player with the given name, health points, and player class.
 *
 * @param name The name of the player.
 * @param healthPoints The initial health points of the player.
 * @param playerClass The class of the player.
 * @return The created player.
 */
Player createPlayer(const char *name, AttackerClass playerClass)
{
    Player player;
    strcpy(player.name, name);
    player.healthPoints = playerClass.healthPoints;
    player.playerClass = playerClass;
    player.playerExp = 0;
    player.playerLevel = 1;
    player.progression = 0;
    player.inventory.healthPotion = 0;
    player.inventory.poison = 0;
    player.inventory.shield = 0;

    return player;
}

/**
 * @brief Increases the progression of a player by 1.
 *
 * This function adds 1 to the player's progression if it is less than the total progression.
 *
 * @param player A pointer to the Player structure.
 */
void addProgressionBar(Player *player)
{
    if (player->progression < PROGRESSION_TOTAL)
    {
        player->progression += 1;
    }
}

/**
 * Displays the progression bar for a player.
 *
 * @param player A pointer to the Player struct.
 */
void showProgressionBar(Player *player)
{
    printf("%s: %d/%d\n", getText("progression"), player->progression, PROGRESSION_TOTAL);

    // Show a bar with progression
    printf("\n——————————————————————————————————————————————————\n");
    for (int i = 0; i < player->progression; i++)
    {
        printf("███");
    }
    // Show a
    printf("\n——————————————————————————————————————————————————\n\n");
}

/**
 * Prints a portion of a map file based on the specified start and end rows.
 * The map file is identified by the given filename.
 * The identifier is used to display additional information about the map.
 *
 * @param filename   The name of the map file.
 * @param startRow   The starting row to print.
 * @param endRow     The ending row to print.
 * @param identifier The identifier for the map.
 */
void printMap(const char *filename, int startRow, int endRow, const char *identifier)
{
    // Open the map file
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Error opening file");
        return;
    }

    // Clear the console screen
    system("clear");

    int currentRow = 1;
    // Read each line of the map file and print the specified rows
    while (fgets(map[currentRow - 1], MAX_COLS, file) != NULL && currentRow <= endRow)
    {
        if (currentRow >= startRow)
        {
            printf("%s", map[currentRow - 1]);
        }
        currentRow += 1;
    }

    // Close the map file
    fclose(file);

    // Print the identifier and legend
    printf("\n\n—————————————————— %s —————————————————————\n", getText("map_legend"));
    printf("* : %s.\n", getText("map_legend_hills"));
    printf("& : %s\n", getText("map_legend_player"));
    printf("# : %s\n", getText("map_legend_interestpoint"));
    printf("| - / \\ : %s\n", getText("map_legend_path"));
    printf("——————————————————————————————————————————————————\n\n");

    printf("\n> %s", getText("ctc"));
    fflush(stdout);

    // Clear input buffer
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
        ;

    getchar();
}

/**
 * Prints a map based on the specified identifier.
 * If a map with the given identifier is found, it calls the printMap function.
 * Otherwise, it prints an error message.
 *
 * @param identifier The identifier for the map.
 */
void printMapByIdentifier(const char *identifier)
{
    // Iterate through the maps array to find the map with the given identifier
    for (size_t i = 0; i < sizeof(maps) / sizeof(maps[0]); i++)
    {
        if (strcmp(maps[i].id, identifier) == 0)
        {
            // Call the printMap function with the found map's details
            printMap("map.txt", maps[i].startRow, maps[i].endRow, identifier);
            return;
        }
    }

    // Print error message if map is not found
    printf("Map for %s not found.\n", identifier);
}

// Function to free memory allocated for text entries

/**
 * @brief Frees the memory allocated for texts.
 *
 * This function iterates over the texts.entries array and frees the memory allocated for each key and value.
 * Finally, it frees the memory allocated for the texts.entries array itself.
 */
void freeTexts()
{
    for (int i = 0; i < texts.count; ++i)
    {
        free(texts.entries[i].key);
        free(texts.entries[i].value);
    }
    free(texts.entries);
}

// Function to print a header with a stylized text

void printHeader()
{
    printf("\n\n ▄█        ▄██████▄     ▄████████     ███     \n");
    printf("███       ███    ███   ███    ███ ▀█████████▄  \n");
    printf("███       ███    ███   ███    █▀     ▀███▀▀██  \n");
    printf("███       ███    ███   ███            ███   ▀  \n");
    printf("███       ███    ███ ▀███████████     ███      \n");
    printf("███       ███    ███          ███     ███      \n");
    printf("███▌    ▄ ███    ███    ▄█    ███     ███      \n");
    printf("█████▄▄██  ▀██████▀   ▄████████▀     ▄████▀  \n\n");
}

// Function to choose the language for the game

/**
 * Prompts the user to choose a language and returns the chosen language.
 * The user is presented with options to choose between French and English.
 * The function validates the user's input and keeps prompting until a valid language is chosen.
 *
 * @return A pointer to a string representing the chosen language.
 */
const char *chooseLanguage()
{
    char language[3];
    do
    {
        printHeader();
        printf("\n> Choisisez votre language / Choose your language:\n\n");
        printf(" FR - Français / French\n");
        printf(" EN - Anglais / English\n");
        printf("\n——————————————————————————————————————————————————\n\n");
        printf("> Votre choix / Your choice: ");
        scanf("%s", language);

        if ((strcasecmp(language, LANG_FR) != 0) && (strcasecmp(language, LANG_EN) != 0))
        {
            fprintf(stderr, "\n\n> Invalid language choice / Choix de language invalide\n");
            sleep(1);
            clearerr(stdin);
            fflush(stdin);
            system("clear");
        }
    } while ((strcasecmp(language, LANG_FR) != 0) && (strcasecmp(language, LANG_EN) != 0));

    return strdup(language);
}

// Function to choose a name for the player

/**
 * @brief Prompts the user to enter a name and returns it.
 *
 * This function displays an introduction text and prompts the user to enter a name.
 * The entered name is stored in dynamically allocated memory and returned.
 * If there is an error in memory allocation or reading the name, the function exits with an error message.
 *
 * @param introText The text to be displayed before prompting for the name.
 * @return char* The entered name.
 */
char *chooseName(const char *introText)
{
    char *name = (char *)malloc(MAX_NAME_LENGTH);
    if (name == NULL)
    {
        fprintf(stderr, "ah c'est pas buenoo\n");
        exit(EXIT_FAILURE);
    }
    system("clear");

    printHeader();

    printf("\n%s\n", getText("intro_chunk1"));
    printf("\n%s\n", getText("intro_chunk2"));
    printf("\n\n[+] %s ", introText);
    if (scanf("%s", name) != 1)
    {
        fprintf(stderr, "err reading name\n");
        exit(EXIT_FAILURE);
    }

    return name;
}

// Function to register basic attacks (simple, critical, ultimate)

/**
 * @brief Registers the attacks by assigning values to the provided Attack pointers.
 *
 * This function assigns values to the provided Attack pointers for simple, critical, and ultimate attacks.
 * The values include the attack name, damage, energy cost, and cooldown.
 *
 * @param simpleAttack Pointer to the simple attack structure.
 * @param criticalAttack Pointer to the critical attack structure.
 * @param ultimateAttack Pointer to the ultimate attack structure.
 */
void registerAttacks(Attack *simpleAttack, Attack *criticalAttack, Attack *ultimateAttack)
{
    *simpleAttack = (Attack){"Simple Attack", 100, 10, 12};
    *criticalAttack = (Attack){"Critical Attack", 60, 15, 18};
    *ultimateAttack = (Attack){"Ultimate Attack", 20, 30, 35};
}

/**
 * @brief Function to prompt the user for an exit choice and handle the exit logic.
 *
 * This function displays a menu to the user with options to exit the game or continue playing.
 * The user's choice is read from the input and based on the choice, the function either exits the game or continues.
 *
 * @return 1 if the user chooses to exit the game, 0 otherwise.
 */
int exitGame()
{

    int exitChoice;

    do
    {
        system("clear");

        printf("\n——————————————————————————————————————————————————\n");
        printf("[+] %s\n", getText("exit_game"));
        printf("——————————————————————————————————————————————————\n\n");
        printf("[+] 1 - %s\n", getText("oui"));
        printf("[+] 2 - %s\n", getText("non"));
        printf("\n——————————————————————————————————————————————————\n\n");
        printf("> %s: ", getText("exit_choice"));

        scanf("%d", &exitChoice);

        switch (exitChoice)
        {
        case 1:
            printf("\n——————————————————————————————————————————————————\n");
            printf("[+] %s\n", getText("yes_exit_game"));
            printf("——————————————————————————————————————————————————\n\n");
            sleep(2);
            system("clear");

            return 1;
            break;
        case 2:
            printf("\n——————————————————————————————————————————————————\n");
            printf("[+] %s\n", getText("no_exit_game"));
            printf("——————————————————————————————————————————————————\n\n");
            sleep(2);
            system("clear");

            return 0;
            break;
        default:
            fprintf(stderr, "\n%s\n\n", getText("invalid_choice"));
            return 0;
            break;
        }

    } while (exitChoice != 1 && exitChoice != 2);

    return 0;
}

// Function to choose the player's class
// Function to allow the player to choose their character class (Chasseur or Arcaniste)
/**
 * Allows the user to choose an attacker class.
 *
 * @return AttackerClass - The chosen attacker class.
 */
AttackerClass chooseClass()
{
    // Declare a variable to store the chosen attacker class
    AttackerClass attackerClass;

    // Declare a variable to store the user's input for class choice
    char classChoice;

    // Clear the console screen and print the game header
    system("clear");

    printHeader();

    // Display the class selection prompt and descriptions for Chasseur and Arcaniste
    printf("\n——————————————————————————————————————————————————\n");
    printf("[+] %s\n", getText("class_prompt"));
    printf("——————————————————————————————————————————————————\n\n");
    printf("[+] %s\n\n", getText("chasseur_description"));
    printf(" > %s\n", getText("chasseur_weapon_1"));
    printf(" > %s\n", getText("chasseur_weapon_2"));
    printf(" > %s\n\n", getText("chasseur_weapon_3"));
    printf("[+] %s\n\n", getText("arcaniste_description"));
    printf(" > %s\n", getText("arcaniste_weapon_1"));
    printf(" > %s\n", getText("arcaniste_weapon_2"));
    printf(" > %s\n\n", getText("arcaniste_weapon_3"));
    // Allow the user to choose a class until a valid choice is made
    do
    {
        // Prompt the user to enter their class choice
        printf("[+] %s ", getText("class_choice"));
        scanf(" %c", &classChoice);

        // Process the user's choice and initialize the attackerClass accordingly
        switch (toupper(classChoice))
        {
        case CLASS_CHASSEUR:
            attackerClass.name = getText("chasseur_name");
            attackerClass.healthPoints = 100;
            attackerClass.combatPoints = 0;
            attackerClass.simpleAttack = 10;
            attackerClass.criticalAttack = 15;
            attackerClass.ultimateAttack = 30;
            printf("\n> %s\n", getText("class_chasseur_message"));
            player.inventory.healthPotion += 1; // Add a health potion for Chasseur
            break;
        case CLASS_ARCANISTE:
            attackerClass.name = getText("arcaniste_name");
            attackerClass.healthPoints = 80;
            attackerClass.combatPoints = 0;
            attackerClass.simpleAttack = 12;
            attackerClass.criticalAttack = 18;
            attackerClass.ultimateAttack = 35;
            printf("\n> %s\n", getText("class_arcaniste_message"));
            player.inventory.healthPotion += 1; // Add a health potion for Arcaniste
            break;
        default:
            // Display an error message for an invalid class choice
            fprintf(stderr, "\n> %s\n\n", getText("class_invalid"));
            break;
        }
    } while (toupper(classChoice) != CLASS_CHASSEUR && toupper(classChoice) != CLASS_ARCANISTE);

    // Pause for a moment to display the chosen class message
    sleep(2);

    // Return the chosen attacker class
    return attackerClass;
}

/**
 * @brief Displays the player's statistics on the screen.
 *
 * This function clears the screen and prints the player's name, level, health points, experience points,
 * health potions, poison, and shield. It also shows a progression bar indicating the player's progress.
 *
 * @param None
 * @return None
 */
void showPlayerStats()
{
    system("clear");

    printf("——————————————————————————————————————————————————\n");
    printf(" %s\n", getText("player_stats"));
    printf("——————————————————————————————————————————————————\n");
    printf(" %s: %s\n", getText("player_name"), player.name);
    printf(" %s: %d\n", getText("player_level"), player.playerLevel);
    printf(" %s: %d\n", getText("player_health"), player.healthPoints);
    printf(" %s: %d\n", getText("player_exp"), player.playerExp);
    printf(" %s: %d\n", getText("player_health_potion"), player.inventory.healthPotion);
    printf(" %s: %d\n", getText("player_poison"), player.inventory.poison);
    printf(" %s: %d\n", getText("player_shield"), player.inventory.shield);
    printf("——————————————————————————————————————————————————\n\n");
    showProgressionBar(&player);

    printf("> Cliquez sur enter pour continuer");
    fflush(stdout);

    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
        ;

    getchar();
}

/**
 * Performs an attack between a player and a monster.
 *
 * @param attack The attack to be performed.
 * @param attackerClass The class of the attacker.
 * @param monster The monster being attacked.
 * @param player The player performing the attack.
 */
void performAttack(const Attack *attack, AttackerClass *attackerClass, Monster *monster, Player *player)
{
    // Generate a random number between 1 and 100 to determine attack success probability
    int probability = rand() % 100 + 1;

    // Player's attack
    if (probability <= attack->successProbability)
    {
        // Check if the attacker is still alive
        if (attackerClass->healthPoints > 0)
        {
            // Check the attacker's class and apply corresponding damage to the monster
            if (strcmp(attackerClass->name, getText("chasseur_name")) == 0)
            {
                monster->healthPoints -= attack->chasseurDamage;
                printf("\n > %s %dHP", getText("attack_success"), attack->chasseurDamage);
                printf("\n");
            }
            else if (strcmp(attackerClass->name, getText("arcaniste_name")) == 0)
            {
                monster->healthPoints -= attack->arcanisteDamage;
                printf("\n > %s %dHP", getText("attack_success"), attack->arcanisteDamage);
                printf("\n");
            }
        }
    }
    else
    {
        // Display a message indicating the attack was missed
        printf("\n > %s\n", getText("missed_attack"));
    }

    // Monster's attack independent of the player's attack result
    if (monster->healthPoints > 0 && attackerClass->healthPoints > 0)
    {
        // Calculate the monster's attack considering poison effect
        int monsterAttack = monster->attackDamage;

        // Subtract the monster's damage from the player's health points
        player->healthPoints -= monsterAttack;
        printf("\n > %s %dHP\n", getText("monster_attack"), monsterAttack);

        // Display a message indicating the start of the next round
        printf("\n > %s\n", getText("next_round"));
    }
}
/**
 * Function to throw poison at a monster.
 * Reduces the monster's health points by 10.
 *
 * @param monster Pointer to the monster object.
 */
void throwPoison(Monster *monster)
{
    printf("\n > %s\n", getText("poison_success"));
    monster->healthPoints -= 10;
}

/**
 * Function to add an item to the player's inventory.
 *
 * @param player Pointer to the player object.
 * @param item   Name of the item to be added.
 */
void addToInventory(Player *player, const char *item)
{
    if (strcmp(item, "health_potion") == 0)
    {
        player->inventory.healthPotion++;
    }
    else if (strcmp(item, "poison") == 0)
    {
        player->inventory.poison++;
    }
    else if (strcmp(item, "shield") == 0)
    {
        player->inventory.shield++;
    }
    else
    {
        fprintf(stderr, "Invalid item\n");
    }
}

/**
 * Function to use an item from the player's inventory.
 *
 * @param player Pointer to the player object.
 * @param item   Name of the item to be used.
 */

void useItem(Player *player, const char *item)
{
    if (strcmp(item, "health_potion") == 0)
    {
        if (player->inventory.healthPotion > 0)
        {
            player->inventory.healthPotion--;
            player->healthPoints += 20;
            printf("\n > %s\n", getText("used_hp"));
        }
        else
        {
            printf("\n > %s\n", getText("no_hp"));
        }
    }
    else if (strcmp(item, "poison") == 0)
    {
        if (player->inventory.poison > 0)
        {
            player->inventory.poison--;
            printf("\n > %s\n", getText("use_poison"));
        }
        else
        {
            printf("\n > %s\n", getText("no_poison"));
        }
    }
    else
    {
        fprintf(stderr, "Invalid item\n");
    }
}

// Function to print a health bar based on health points

/**
 * Prints a health bar based on the given health points.
 * Each health point is represented by a filled block character.
 * The health bar is displayed as a sequence of these block characters.
 * The number of block characters is determined by the health points divided by 5.
 * The health points value is also displayed at the end of the health bar.
 *
 * @param health_points The number of health points to display.
 */
void print_health_bar(int health_points)
{
    int num_chars = health_points / 5;
    for (int i = 0; i < num_chars; i++)
    {
        printf("█");
    }
    printf(" %d\n", health_points);
}

/**
 * Resets the player's health points based on the player's class and inventory.
 * If the player's class is "chasseur", the health points are set to 100 plus extra health from the shield.
 * If the player's class is "arcaniste", the health points are set to 80 plus extra health from the shield.
 * If the player's class is invalid, an error message is printed to stderr.
 */
void resetPlayerHealth()
{
    int extraHealth = player.inventory.shield * 20;

    if (strcasecmp(player.playerClass.name, getText("chasseur_name")) == 0)
    {
        player.healthPoints = 100 + extraHealth;
    }
    else if (strcasecmp(player.playerClass.name, getText("arcaniste_name")) == 0)
    {
        player.healthPoints = 80 + extraHealth;
    }
    else
    {
        fprintf(stderr, "Invalid player class\n");
    }
}

/**
 * Uses a poison item from the player's inventory on the given monster.
 * If the player has a poison item in their inventory, the poison count is decreased by 1,
 * and a message is printed indicating that the poison is used.
 * The poison effect is then applied to the monster by calling the throwPoison function.
 * If the player does not have a poison item in their inventory, a message is printed indicating that there is no poison.
 *
 * @param monster A pointer to the monster on which to use the poison.
 */
void usePoison(Monster *monster)
{
    if (player.inventory.poison > 0)
    {
        player.inventory.poison--;
        printf("\n > %s\n", getText("use_poison"));
        throwPoison(monster);
    }
    else
    {
        printf("\n > %s\n", getText("no_poison_in_inventory"));
    }
}

/**
 * Function: useHealthPotion
 * -------------------------
 * This function is used to simulate the usage of a health potion by the player.
 * It checks the player's class and determines the maximum health points based on the class.
 * If the player has a health potion in their inventory, it increases the player's health points by 20,
 * up to the maximum health points allowed for the player's class.
 * If the player's health points are already at the maximum, it displays a message indicating so.
 * If the player does not have a health potion, it displays a message indicating so.
 */

void useHealthPotion()
{

    // based on player class get max health

    int maxHealth = 0;

    if (strcasecmp(player.playerClass.name, getText("chasseur_name")) == 0)
    {
        maxHealth = 100;
    }
    else if (strcasecmp(player.playerClass.name, getText("arcaniste_name")) == 0)
    {
        maxHealth = 80;
    }
    else
    {
        fprintf(stderr, "Invalid player class\n");
    }

    if (player.inventory.healthPotion > 0)
    {
        if (player.healthPoints < maxHealth)
        {
            player.inventory.healthPotion--;
            player.healthPoints += 20;
            if (player.healthPoints > maxHealth)
            {
                player.healthPoints = maxHealth;
            }
            printf("\n > %s\n", getText("used_hp"));
        }
        else
        {
            printf("\n > %s\n", getText("max_hp"));
        }
    }
    else
    {
        printf("\n > %s\n", getText("no_hp"));
    }
}
/**
 * @brief Manages the inventory by displaying the inventory items and allowing the user to choose options.
 *
 * This function displays a menu to the user and prompts for their choice. If the user chooses to show the inventory items,
 * it displays the current quantities of different items in the player's inventory. The function continues to loop until
 * the user chooses to exit.
 */

void manageInventory()
{

    int inventoryChoice;

    do
    {
        system("clear");

        printf("\n——————————————————————————————————————————————————\n");
        printf("[+] %s\n", getText("manage_inventory"));
        printf("——————————————————————————————————————————————————\n\n");
        printf("[+] 1 - %s\n", getText("show_inventory_items"));
        printf("\n——————————————————————————————————————————————————\n\n");
        printf("[+] %s ", getText("which_choice"));

        scanf("%d", &inventoryChoice);

        switch (inventoryChoice)
        {
        case 1:
            system("clear");

            printf("\n——————————————————————————————————————————————————\n");
            printf("[+] %s\n", getText("inventory_items"));
            printf("——————————————————————————————————————————————————\n\n");
            printf("[+] %s: %d\n", getText("player_health_potion"), player.inventory.healthPotion);
            printf("[+] %s: %d\n", getText("player_poison"), player.inventory.poison);
            printf("[+] %s: %d\n", getText("player_shield"), player.inventory.shield);
            printf("\n——————————————————————————————————————————————————\n\n");
            printf("> %s", getText("ctc"));
            fflush(stdout);

            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF)
                ;

            getchar();

            break;
        default:
            fprintf(stderr, "\n%s\n\n", getText("invalid_choice"));
            break;
        }

    } while (inventoryChoice != 1);
}

// Function to initialize the game, registering monsters and attacks

void initGame()
{

    registerAttacks(&simpleAttack, &criticalAttack, &ultimateAttack);

    registerMonster(1, 60, 10, "poison");
    registerMonster(2, 80, 20, "shield");
    registerMonster(3, 100, 30, "monster head");
}

/**
 * @brief Initializes the battle between the player and a monster.
 *
 * This function displays the battle stats, including the health points of the player and the monster.
 * It presents the player with a menu of attack options and other actions they can take during the battle.
 * The function then performs the selected attack or action based on the user's input.
 * The battle continues until either the player or the monster's health points reach zero.
 *
 * @param monster A pointer to the Monster struct representing the monster in the battle.
 */

void initBattle(Monster *monster)
{
    int attackChoice;

    resetPlayerHealth();

    do
    {
        do
        {
            // Display battle stats
            printf("\n%s\n", getText("battle_stats"));
            printf("%s\n", getText("your_hp_points"));
            print_health_bar(player.healthPoints);
            printf("%s\n", getText("monster_hp_points"));
            print_health_bar(monster->healthPoints);
            printf("——————————————————————————————————————————————————\n\n");

            // Display attack options and other actions
            printf("\n——————————————————————————————————————————————————\n");
            printf("[+] %s\n", getText("use_attack"));
            printf("——————————————————————————————————————————————————\n");
            printf("[+] 1 - %s\n", getText("simple_attack"));
            printf("[+] 2 - %s\n", getText("critical_attack"));
            printf("[+] 3 - %s\n", getText("ultimate_attack"));
            printf("——————————————————————————————————————————————————\n");
            printf("[+] 4 - %s\n", getText("use_hp"));
            printf("[+] 5 - %s\n", getText("use_poison"));
            printf("——————————————————————————————————————————————————\n");
            printf("[+] 6 - %s\n", getText("show_map"));
            printf("[+] 7 - %s\n", getText("show_player_stats"));
            printf("[+] 8 - %s\n", getText("manage_inventory"));
            printf("[+] 9 - %s\n", getText("exit_game"));
            printf("——————————————————————————————————————————————————\n\n");

            printf("[+] %s ", getText("which_choice"));

            scanf("%d", &attackChoice);

            switch (attackChoice)
            {
            case 1:
                performAttack(&simpleAttack, &player.playerClass, monster, &player);
                break;
            case 2:
                performAttack(&criticalAttack, &player.playerClass, monster, &player);
                break;
            case 3:
                performAttack(&ultimateAttack, &player.playerClass, monster, &player);
                break;
            case 4:
                useHealthPotion();
                break;
            case 5:
                usePoison(monster);
                break;
            case 6:
                printMapByIdentifier(currentLevel);
                break;
            case 7:
                showPlayerStats();
                break;
            case 8:
                manageInventory();
                break;
            case 9:
                if (exitGame() == 1)
                {
                    exit(EXIT_SUCCESS);
                }
                break;
            default:
                system("clear");

                printf("——————————————————————————————————————————————————\n");
                fprintf(stderr, "%s\n", getText("attack_choice_invalid"));
                printf("——————————————————————————————————————————————————\n");
                break;
            }
            sleep(2);
            system("clear");

        } while ((attackChoice < 1 || attackChoice > 8 || floor(attackChoice) != attackChoice));
        system("clear");

    } while (player.healthPoints > 0 && monster->healthPoints > 0);
}

/**
 * @brief Executes the final episode of the game.
 *
 * This function displays a series of text messages to progress the final episode of the game.
 * It also handles the battle between the player and a monster.
 * If the player's health points reach 0, a death message is displayed and the final episode is restarted.
 * If the monster's health points reach 0, a final chapter message is displayed.
 *
 * @param None
 * @return None
 */
void finalEpisode()
{

    system("clear");

    Monster monster = registeredMonsters[2];

    addProgressionBar(&player);

    resetPlayerHealth();

    strcpy(currentLevel, "CAMP_NORD");

    printf("\n——————————————————————————————————————————————————\n");
    printf("%s\n", getText("final_episode_1"));
    sleep(3);
    printf("%s\n", getText("final_episode_2"));
    sleep(3);
    printf("%s\n", getText("final_episode_3"));
    sleep(3);
    printf("%s\n", getText("final_episode_4"));
    sleep(3);
    printf("%s\n", getText("final_episode_5"));
    sleep(3);
    printf("%s\n", getText("final_episode_6"));
    sleep(3);
    printf("%s\n", getText("final_episode_7"));
    sleep(3);
    printf("%s: %s\n", getText("final_episode_8"), player.name);
    sleep(3);
    printf("%s\n", getText("final_episode_9"));
    sleep(3);
    printf("%s\n", getText("final_episode_10"));
    printf("——————————————————————————————————————————————————\n\n");

    sleep(5);
    system("clear");

    initBattle(&monster);

    if (player.healthPoints <= 0)
    {
        printf("\n——————————————————————————————————————————————————\n");
        printf("[+] %s\n", getText("death_message"));
        printf("——————————————————————————————————————————————————\n\n");
        sleep(1);
        system("clear");

        addToInventory(&player, "health_potion");
        addToInventory(&player, "health_potion");
        addToInventory(&player, "health_potion");
        addToInventory(&player, "health_potion");

        addToInventory(&player, "poison");
        addToInventory(&player, "poison");
        addToInventory(&player, "poison");

        finalEpisode();
    }
    else if (monster.healthPoints <= 0)
    {
        printf("\n——————————————————————————————————————————————————\n");
        printf("\n> %s\n", getText("final_chapter_1"));
        sleep(2);
        printf("\n> %s\n", getText("final_chapter_2"));
        sleep(2);
        printf("\n> %s\n", getText("end"));
        printf("\n——————————————————————————————————————————————————\n\n");

        sleep(5);
        exit(EXIT_SUCCESS);
    }

    addProgressionBar(&player);
    addXp(&player, 5000);
}

/**
 * Function: path2
 * ----------------
 * This function represents the second path in the game. It prompts the player to make a choice
 * whether to open a door or not. If the player chooses to open the door, they will be prompted
 * to enter a code. If the code is correct, the player will receive rewards and proceed to the
 * final episode. If the code is incorrect, the player will be given hints and can try again.
 * If the player chooses not to open the door, they will proceed to the final episode directly.
 */

void path2()
{
    int choice_open_chest;

    const char *hints[MAX_HINTS] = {
        getText("path2_code_hint1"),
        getText("path2_code_hint2"),
        getText("path2_code_hint3")};

    const char *hintKeys[MAX_HINTS] = {
        "path2_code_hint1",
        "path2_code_hint2",
        "path2_code_hint3"};

    int current_hint = 0;
    int code;

    system("clear");

    strcpy(currentLevel, "path_2");

    resetPlayerHealth();

    addProgressionBar(&player);

    do
    {

        printf("\n————————————————————— Episode 2 —————————————————————\n");
        printf("%s\n", getText("path2_line_1"));
        printf("%s\n", getText("path2_line_2"));
        printf("%s\n", getText("path2_line_3"));
        printf("%s\n", getText("path2_line_4"));
        printf("\n——————————————————————————————————————————————————\n");

        printf("> %s\n", getText("open_door"));
        printf("——————————————————————————————————————————————————\n");
        printf(" 1 - %s\n", getText("oui"));
        printf(" 2 - %s\n", getText("non"));
        printf("——————————————————————————————————————————————————\n\n");
        printf("> %s", getText("which_choice"));
        if (scanf(" %d", &choice_open_chest) != 1 || (choice_open_chest != 1 && choice_open_chest != 2))
        {
            printf("\n> Invalid input. Please enter 1 or 2.\n");
            sleep(2);
            system("clear");
        }
    } while (choice_open_chest != 1 && choice_open_chest != 2);

    if (choice_open_chest == 1)
    {
        do
        {
            system("clear");

            printf("\n——————————————————————————————————————————————————\n");
            printf("%s\n", getText("path2_line_5"));
            printf("%s\n", getText("path2_line_6"));
            for (int i = 0; i < current_hint; i++)
            {
                printf("——————————————————————————————————————————————————\n");
                printf("> %s %d: %s \n", getText("hint"), i + 1, getText(hintKeys[i]));
            }
            printf("——————————————————————————————————————————————————\n\n");

            printf("> %s ", getText("input_code"));

            if (scanf("%d", &code) != 1)
            {
                while (getchar() != '\n')
                    ;
                printf("\n> %s.\n", getText("wrong_code"));
                printf("  %s\n", getText("tips_shown"));
                sleep(2);

                if (current_hint < MAX_HINTS)
                {
                    current_hint++;
                }
            }
            printf("——————————————————————————————————————————————————\n\n");
        } while (code != ANSWER_CODE_2);

        addXp(&player, 1500);

        printf("\n> %s\n", getText("path2_line_7"));
        printf("\n> %s\n", getText("path2_line_8"));
        addToInventory(&player, "poison");
        addToInventory(&player, "shield");
        printf("\n> %s\n", getText("path2_line_9"));

        sleep(4);

        finalEpisode();
    }
    else
    {
        printf("\n> %s\n", getText("path2_line_9"));
        sleep(4);
        finalEpisode();
    }
}

// Function for the first branch of the episode 2

/**
 * Executes path 1 of the game.
 * This function initializes the necessary variables and starts the battle with a monster.
 * If the player's health points reach 0, the player is considered dead and the function is called recursively.
 * If the monster's health points reach 0, the player gains experience points, receives a health potion, and proceeds to the next mission.
 * After the battle, the function presents an enigma to the player and prompts for a code input.
 * If the input code is incorrect, hints are shown and the player can try again.
 * If the input code is correct, the player gains experience points and receives a poison item.
 * Finally, the function calls path2() to proceed to the next path in the game.
 */
void path1()
{

    Monster monster = registeredMonsters[0];
    int attackChoice;
    const char *hints[MAX_HINTS] = {
        getText("path1_code_hint1"),
        getText("path1_code_hint2"),
        getText("path1_code_hint3")};

    const char *hintKeys[MAX_HINTS] = {
        "path1_code_hint1",
        "path1_code_hint2",
        "path1_code_hint3"};

    strcpy(currentLevel, "path_1");
    resetPlayerHealth();

    system("clear");

    addProgressionBar(&player);

    printf("\n————————————————————— path 1 —————————————————————\n");
    printf("%s \n", getText("intro_path1_line1"));
    printf("%s \n", getText("intro_path1_line2"));
    printf("%s \n", getText("intro_path1_line3"));
    printf("——————————————————————————————————————————————————\n\n");

    sleep(3);

    initBattle(&monster);

    if (player.healthPoints <= 0)
    {
        printf("\n——————————————————————————————————————————————————\n");
        printf("[+] %s\n", getText("death_message"));
        printf("——————————————————————————————————————————————————\n\n");
        sleep(1);
        system("clear");

        path1();
    }
    else if (monster.healthPoints <= 0)
    {
        printf("\n——————————————————————————————————————————————————\n");
        printf("[+] %s\n", getText("monster_kill"));
        printf("[+] %s\n", getText("get_health_potion"));
        printf("——————————————————————————————————————————————————\n\n");

        addToInventory(&player, "health_potion");

        printf("\n> %s\n", getText("next_mission"));

        sleep(4);

        addXp(&player, 1500);
    }

    system("clear");

    int current_hint = 0;
    int code;

    do
    {
        system("clear");

        printf("\n————————————————————— Enigme ———————————————————————\n");
        printf(" %s \n", getText("path1_p2_line1"));
        printf(" %s \n", getText("path1_p2_line4"));
        printf(" %s \n\n", getText("path1_p2_line2"));
        printf(" %s \n", getText("path1_p2_line3"));

        for (int i = 0; i < current_hint; i++)
        {
            printf("——————————————————————————————————————————————————\n");
            printf("> Incide %d: %s \n", i + 1, getText(hintKeys[i]));
        }

        printf("——————————————————————————————————————————————————\n\n");

        printf("> %s ", getText("input_code"));

        if (scanf("%d", &code) != 1)
        {
            while (getchar() != '\n')
                ;
            continue;
        }

        if (code != ANSWER_CODE)
        {
            printf("\n> %s.\n", getText("wrong_code"));
            printf("  %s\n", getText("tips_shown"));
            sleep(2);

            if (current_hint < MAX_HINTS)
            {
                current_hint++;
            }
        }

    } while (code != ANSWER_CODE);

    addXp(&player, 1500);

    addToInventory(&player, "poison");

    printf("\n> %s\n", getText("correct_answer"));
    printf("\n> %s", getText("new_poison_in_inventory"));

    sleep(2);

    path2();
}

// Function for the second episode of the game

/**
 * Executes Episode 2 of the game.
 * This function allows the player to choose between two paths and progresses the game accordingly.
 * It displays the episode introduction, presents the choices, and executes the corresponding path based on the player's input.
 *
 * @param None
 * @return None
 */
void episode2()
{

    int choosenPath;

    strcpy(currentLevel, "EPISODE_2");
    addProgressionBar(&player);
    resetPlayerHealth();
    system("clear");

    printf("\n————————————————————— Episode 2 —————————————————————\n");
    printf("%s\n", getText("episode2_intro_line1"));
    printf("%s\n", getText("episode2_intro_line2"));

    do
    {

        printf("\n——————————————————————————————————————————————————\n");
        printf("[1] %s\n", getText("choice_path1"));
        printf("[2] %s\n", getText("choice_path2"));
        printf("——————————————————————————————————————————————————\n\n");

        printf("[+] %s ?: ", getText("which_path"));

        scanf("%d", &choosenPath);

        switch (choosenPath)
        {
        case 1:
            system("clear");

            printf("\n——————————————————————————————————————————————————\n");
            printf("[+] %s\n", getText("path1"));
            printf("——————————————————————————————————————————————————\n\n");
            sleep(2);
            system("clear");

            path1();
            break;
        case 2:
            system("clear");

            printf("\n——————————————————————————————————————————————————\n");
            printf("[+] %s\n", getText("path2"));
            printf("——————————————————————————————————————————————————\n\n");
            sleep(2);
            system("clear");

            path2();
            break;
        default:
            fprintf(stderr, "\n%s\n\n", getText("invalid_choice"));
            break;
        }

    } while (choosenPath != 1 && choosenPath != 2);
}

// Function for the first episode of the game

/**
 * @brief Executes Episode 1 of the game.
 *
 * This function initializes the necessary variables and progresses the player through Episode 1.
 * It displays the episode introduction, initiates a battle with a monster, and handles the outcome.
 * If the player's health points reach 0, the function displays a death message and restarts Episode 1.
 * If the monster's health points reach 0, the function rewards the player with items, progresses to Episode 2,
 * and adds experience points to the player's character.
 */
void episode1()
{

    int attackChoice;
    Monster monster = registeredMonsters[0];

    strcpy(currentLevel, "EPISODE_1");

    system("clear");

    addProgressionBar(&player);

    // bonus

    addToInventory(&player, "health_potion");
    addToInventory(&player, "poison");

    resetPlayerHealth();
    printf("\n————————————————————— Episode 1 —————————————————————\n");
    printf("%s ", getText("episode1_intro_line1"));
    printf("%s ", getText("episode1_intro_line2"));
    printf("%s\n", getText("episode1_intro_line3"));
    printf("——————————————————————————————————————————————————\n\n");

    initBattle(&monster);

    if (player.healthPoints <= 0)
    {
        printf("\n——————————————————————————————————————————————————\n");
        printf("[+] %s\n", getText("death_message"));
        printf("——————————————————————————————————————————————————\n\n");
        sleep(1);
        system("clear");

        episode1();
    }
    else if (monster.healthPoints <= 0)
    {
        printf("\n——————————————————————————————————————————————————\n");
        printf("[+] %s\n", getText("monster_kill"));
        printf("[+] %s\n", getText("get_health_potion"));
        printf("——————————————————————————————————————————————————\n\n");

        addToInventory(&player, "poison");
        addToInventory(&player, "health_potion");
        printf("\n> %s\n", getText("next_mission"));

        sleep(4);
        system("clear");

        addXp(&player, 1000);

        episode2();
    }
}

/**
 * @file main_mac.c
 * @brief Main function that serves as the entry point of the game.
 *
 * This file contains the main function that initializes the game, prompts the user for input,
 * and executes the game episodes. It also handles memory allocation and deallocation for the
 * player's name and loaded texts.
 */
// Main function that serves as the entry point of the game
int main()
{
    // Seed the random number generator based on the current time
    srand(time(NULL));
    // Declare variables to store the player's name and chosen language
    char *name;
    char language[3];
    system("clear");

    strcpy(language, chooseLanguage());
    // Load text entries based on the chosen language from the corresponding text file
    loadTexts((strcasecmp(language, "fr") == 0) ? "french.txt" : "english.txt");
    // Prompt the user to enter their name and store it in the 'name' variable
    name = chooseName(getText("name_intro"));
    // Choose the player's class (Chasseur or Arcaniste) and store the information in 'attackerClass'
    AttackerClass attackerClass = chooseClass();
    // Create the player based on the entered name, default health points (100), and chosen class
    player = createPlayer(name, attackerClass);

    // Initialize the game by registering attacks, monsters, and performing other setup
    initGame();
    // Start Episode 1 of the game
    episode1();
    // // Clear the console screen before starting Episode 2
    system("clear");

    // Free the dynamically allocated memory for the player's name and loaded texts
    free(name);
    freeTexts();
    // Return 0 to indicate successful execution of the program
    return 0;
}