# Lost: An Epic Journey

## Authors
- Anir Karami (C Group)
- Louis Prevost (C Group)

## Game Overview
Embark on an extraordinary quest in a realm veiled in enigma. Awaken as a protagonist with no recollection in an unfamiliar city. A mysterious device on your arm reveals an ID and a name. At a pivotal juncture, you must choose to either embrace the path of a Hunter, skilled in the art of combat, or become an Arcanist, harnessing arcane powers and wielding unique weapons. Confront ominous monsters, navigate through decisions that mold your fate, and unravel the secrets of your concealed identity. Are you prepared to meet the forthcoming challenges with unwavering courage and resolute determination?

## Key Features and Implementation

### Advanced Combat Mechanism
We implemented a combat system where the player can choose from different attacks based on their class. This was achieved by creating a switch case for each attack type and calculating the damage based on the player's class and attack choice.

### Character Personalization
We allowed the player to choose their class, name, and weapons. This was done by prompting the user for input and storing their choices in variables.

### Multiple Levels and Monsters
We created different levels and monsters for the player to fight against. This was achieved by using a loop that continues until the player or the monster's health points reach zero.

### Inventory System
We implemented an inventory system where the player can collect various items. This was done by creating an inventory array and functions to add and remove items.

### Quest System
We designed a quest system with hints to guide the player. This was achieved by storing quest information in text files and displaying them based on the player's progress.

### Map System
We created a map system to guide the player through different paths. This was done by using conditional statements to determine the player's location based on their choices.

### Experience and Leveling
We added a feature for the player to gain experience and level up. This was achieved by incrementing a variable each time the player wins a battle and updating the player's level based on their experience points.

### Reading Text Files Based on User Input
We implemented a feature to load text from a file based on the user's language choice. The text files are structured as key-value pairs, separated by a semicolon. We have a function that loads the text entries from the file into a structure, and another function that retrieves a text entry based on a key.

## Compilation and Execution 
  To compile and executethe program, navigate to the directory containing the source code and run the following command:

   > For MacOS / Linux based operation systems:

   ```bash 
   gcc -o game main.c
   ```
   ```bash 
   ./game 
   ```

   > For Windows operation systems:
   ```
   gcc -o game main_windows.c
   ```
   ```bash 
   game 
   ```


  ## Source Files
The source files contain the names of the authors and are commented in English. The main logic of the game is contained in  main.c and     ```main.c``` The text for the game's storyline and prompts is stored in     ```french.txt``` and  ```english.txt``` for the map it's in ```map.txt```

