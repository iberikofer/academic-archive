import java.util.Scanner;
import java.util.Arrays;
import java.util.Random;
import java.util.Comparator;
  
public class Main {

    private static int readInt(Scanner scanner, String message) {
        while (true) {
            System.out.print(message);
            try {
                return Integer.parseInt(scanner.nextLine());
            } catch (NumberFormatException e) {
                System.out.println("Error: Please enter a valid integer number!");
            }
        }
    }

    private static double readDouble(Scanner scanner, String message) {
        while (true) {
            System.out.print(message);
            try {
                return Double.parseDouble(scanner.nextLine());
            } catch (NumberFormatException e) {
                System.out.println("Error: Please enter a valid decimal number!");
            }
        }
    }

    private static String readName(Scanner scanner, String message) {
        while (true) {
            System.out.print(message);
            String input = scanner.nextLine();
            if (input.matches(".*\\d.*")) {
                System.out.println("Error: Name cannot contain numbers! Try again.");
            } else if (input.trim().isEmpty()) {
                System.out.println("Error: Name cannot be empty! Try again.");
            } else {
                return input;
            }
        }
    }

    private static Athlete[] createAthletePool() {
        Athlete[] pool = new Athlete[50];
        String[] names = {
            "Liam", "Noah", "Oliver", "Elijah", "James", "William", "Benjamin", "Lucas", "Henry", "Theodore",
            "Jack", "Levi", "Alexander", "Jackson", "Mateo", "Daniel", "Michael", "Mason", "Sebastian", "Ethan",
            "Logan", "Owen", "Samuel", "Jacob", "Asher", "Aiden", "John", "Joseph", "Wyatt", "David",
            "Leo", "Luke", "Julian", "Hudson", "Grayson", "Matthew", "Ezra", "Gabriel", "Carter", "Isaac",
            "Jayden", "Luca", "Lincoln", "Anthony", "Dylan", "Jaxon", "Thomas", "Charles", "Christopher", "Josiah"
        };
        String[] eqNames = {"Dumbbell", "Barbell", "Kettlebell", "Jump Rope", "Exercise Band"};
        Random rnd = new Random();

        for (int i = 0; i < 50; i++) {
            String name = names[rnd.nextInt(names.length)];
            int age = 18 + rnd.nextInt(40);
            double weight = 40.0 + (rnd.nextDouble() * 60.0);
            double energy = 5.0 + (rnd.nextDouble() * 95.0);
            String eqName = eqNames[rnd.nextInt(eqNames.length)];
            int eqValue = 5 + rnd.nextInt(45);
            Equipment eq = new Equipment(eqName, eqValue, "kg");

            pool[i] = new Athlete(name, age, weight, energy, eq);
        }
        return pool;
    }

    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);

        int n = readInt(scanner, "Enter the default number of Athletes (up to 50): ");
        while (n < 0 || n > 50) {
            System.out.println("Please enter a number between 0 and 50.");
            n = readInt(scanner, "Enter the default number of Athletes (up to 50): ");
        }
        Athlete[] athletes = new Athlete[n];
        Athlete[] pool = createAthletePool();
        for (int i = 0; i < n; i++) {
            athletes[i] = pool[i];
        }
        System.out.println("\nSuccessfully loaded " + n + " random athletes into the array.\n");

        TrainingZone cardioZone = new TrainingZone("Cardio Zone");
        TrainingZone weightZone = new TrainingZone("Weight Zone");
        boolean exit = false;

        while (!exit) {
            printMainMenu();
            int choice = readInt(scanner, "Your choice: ");
            System.out.print("\n");

            switch (choice) {
                case 1:  handleShowAll(athletes); break;
                case 2:  handleShowSpecific(scanner, athletes); break;
                case 3:  athletes = handleAddAthlete(scanner, athletes); break;
                case 4:  athletes = handleDeleteAthlete(scanner, athletes); break;
                case 5:  handleEditAthlete(scanner, athletes); break;
                case 6:  handleSortAthletes(scanner, athletes); break;
                case 7:  athletes = handleDeepCopy(scanner, athletes); break;
                case 8:  handleBinarySearch(scanner, athletes); break;
                case 9:  athletes = handleCategoryDelete(scanner, athletes); break;
                case 10: handleTeamCompetition(athletes); break;
                case 11: athletes = handleAddMoveZone(scanner, athletes, cardioZone, weightZone); break;
                case 12: handleShowAllData(athletes, cardioZone, weightZone); break;
                case 13: athletes = handleBenchPressCompetition(scanner, athletes, cardioZone, weightZone); break;
                case 14: handleZoneWar(cardioZone, weightZone); break;
                case 15: handleCountCriteria(scanner, athletes, cardioZone, weightZone); break;
                case 16: athletes = handleDeleteFromZone(scanner, athletes, cardioZone, weightZone); break;
                case 0:  exit = true; break;
                default: System.out.println("Unknown command.");
            }
        }
        System.out.println("Program terminated.");
    }

    private static void printMainMenu() {
        System.out.println("\n===== MAIN MENU =====");
        System.out.println("--- Universal array ---");
        System.out.println("1 - Show all athletes");
        System.out.println("2 - Show specific athlete");
        System.out.println("3 - Add athlete");
        System.out.println("4 - Delete athlete");
        System.out.println("5 - Edit athlete");
        System.out.println("6 - Sort athletes");
        System.out.println("7 - Deep copy an athlete");
        System.out.println("8 - Binary athlete search");
        System.out.println("9 - Divide and delete by category");
        System.out.println("10 - Teams Competition");
        System.out.println("--- Macro objects area ---");
        System.out.println("11 - Add/Move athlete to a specific zone");
        System.out.println("12 - Show Universal Array and All Zones");
        System.out.println("13 - Athlete bench press Interaction");
        System.out.println("14 - Area War");
        System.out.println("15 - Count athletes by criteria");
        System.out.println("16 - Delete athlete from a specific zone");
        System.out.println("0 - Exit");
    }

    private static void handleShowAll(Athlete[] athletes) {
        if (athletes.length == 0) {
            System.out.print("Athlete list is empty!\n");
        } else {
            for (Athlete athlete : athletes) {
                athlete.athletePrint();
            }
        }
    }

    private static void handleShowSpecific(Scanner scanner, Athlete[] athletes) {
        int viewIndex = readInt(scanner, "Enter athlete index (0 to " + (athletes.length - 1) + "): ");
        if (viewIndex >= 0 && viewIndex < athletes.length) {
            athletes[viewIndex].athletePrint();
        } else {
            System.out.println("Error: Invalid index!");
        }
    }

    private static Athlete[] handleAddAthlete(Scanner scanner, Athlete[] athletes) {
        int insertIndex = readInt(scanner, "Enter index to add (0 to " + athletes.length + "): ");
        if (insertIndex >= 0 && insertIndex <= athletes.length) {
            athletes = Arrays.copyOf(athletes, athletes.length + 1);
            for (int i = athletes.length - 1; i > insertIndex; i--) {
                athletes[i] = athletes[i - 1];
            }
            String newName = readName(scanner, "Enter athlete's name: ");
            int newAge = readInt(scanner, "Enter age: ");
            while (newAge < 5 || newAge > 120) {
                System.out.println("Please enter a valid age!");
                newAge = readInt(scanner, "Enter valid age: ");
            }
            double newWeight = readDouble(scanner, "Enter weight (kg): ");
            while (newWeight < 20 || newWeight > 600) {
                System.out.println("Please enter a valid weight!");
                newWeight = readDouble(scanner, "Enter valid weight: ");
            }
            double newEnergy = readDouble(scanner, "Enter energy level (0-100): ");
            while (newEnergy < 0.0 || newEnergy > 100.0) {
                System.out.println("Please enter a valid energy level!");
                newEnergy = readDouble(scanner, "Enter energy level (0-100): ");
            }
            String eqName = readName(scanner, "Enter equipment name: ");
            int eqValue = readInt(scanner, "Enter equipment condition/weight (int): ");
            System.out.print("Enter unit type ('kg', '%', etc.): ");
            String eqUnit = scanner.nextLine();

            Equipment newEquipment = new Equipment(eqName, eqValue, eqUnit);
            athletes[insertIndex] = new Athlete(newName, newAge, newWeight, newEnergy, newEquipment);

            System.out.println("Athlete added successfully!\n");
        } else {
            System.out.println("Error: Index out of bounds!");
        }
        return athletes;
    }

    private static Athlete[] handleDeleteAthlete(Scanner scanner, Athlete[] athletes) {
        int deleteIndex = readInt(scanner, "Enter index to delete (0 to " + (athletes.length - 1) + "): ");
        if (deleteIndex >= 0 && deleteIndex < athletes.length) {
            for (int i = deleteIndex; i < athletes.length - 1; i++) {
                athletes[i] = athletes[i + 1];
            }
            athletes = Arrays.copyOf(athletes, athletes.length - 1);
            System.out.println("Athlete deleted successfully!");
        } else {
            System.out.println("Error: Index out of bounds!");
        }
        return athletes;
    }

    private static void handleEditAthlete(Scanner scanner, Athlete[] athletes) {
        int editIndex = readInt(scanner, "Enter athlete index to edit (0 to " + (athletes.length - 1) + "): ");
        if (editIndex >= 0 && editIndex < athletes.length) {
            System.out.println("--- Current Data ---");
            athletes[editIndex].athletePrint();

            String newName = readName(scanner, "Enter new name: ");
            athletes[editIndex].setName(newName);

            int newAge = readInt(scanner, "Enter new age: ");
            athletes[editIndex].setAge(newAge);

            double newWeight = readDouble(scanner, "Enter new weight (kg): ");
            athletes[editIndex].setWeight(newWeight);

            double newEnergy = readDouble(scanner, "Enter new energy level (0-100): ");
            while (newEnergy < 0.0 || newEnergy > 100.0) {
                System.out.println("Please enter a valid energy level!");
                newEnergy = readDouble(scanner, "Enter energy level (0-100): ");
            }
            athletes[editIndex].setEnergyLvl(newEnergy);

            String newEqName = readName(scanner, "Enter new equipment name: ");
            int newEqValue = readInt(scanner, "Enter new equipment condition/weight (int): ");
            System.out.print("Enter new unit type ('kg', '%', etc.): ");
            String newEqUnit = scanner.nextLine();

            athletes[editIndex].setEquipment(new Equipment(newEqName, newEqValue, newEqUnit));

            System.out.println("Athlete data updated successfully!");
            athletes[editIndex].athletePrint();
        } else {
            System.out.println("Error: Invalid index!");
        }
    }

    private static void handleSortAthletes(Scanner scanner, Athlete[] athletes) {
        System.out.println("--- Sort Options ---");
        System.out.println("1 - By Name (Natural Ordering / Comparable)");
        System.out.println("2 - By Age (Nested Class Comparator)");
        System.out.println("3 - By Weight (Anonymous Class Comparator)");

        int sortChoice = readInt(scanner, "Choose sorting method: ");
        if (sortChoice == 1) {
            Arrays.sort(athletes);
            System.out.println("Sorted by Name!");
        } else if (sortChoice == 2) {
            Arrays.sort(athletes, new Athlete.AgeComparator());
            System.out.println("Sorted by Age!");
        } else if (sortChoice == 3) {
            Arrays.sort(athletes, new Comparator<Athlete>() {
                @Override
                public int compare(Athlete a1, Athlete a2) {
                    return Double.compare(a1.getWeight(), a2.getWeight());
                }
            });
            System.out.println("Sorted by Weight!");
        } else {
            System.out.println("Error: Invalid choice!");
        }
    }

    private static Athlete[] handleDeepCopy(Scanner scanner, Athlete[] athletes) {
        int cloneIndex = readInt(scanner, "Enter index of the object you want to copy (0-" + (athletes.length - 1) + "): ");
        if (cloneIndex >= 0 && cloneIndex < athletes.length) {
            try {
                Athlete original = athletes[cloneIndex];
                Athlete cloneObj = (Athlete) original.clone();

                athletes = Arrays.copyOf(athletes, athletes.length + 1);
                athletes[athletes.length - 1] = cloneObj;

                if (original.getEquipment() != null) {
                    original.getEquipment().name = "Broken " + original.getEquipment().name;
                }

                System.out.print("Do you want to set parameters to a clone? Y/N\n");
                String copyChoice = scanner.nextLine();
                if (copyChoice.equalsIgnoreCase("y")) {
                    String newName = readName(scanner, "Enter athlete's name: ");
                    int newAge = readInt(scanner, "Enter age: ");
                    while (newAge < 5 || newAge > 120) {
                        System.out.println("Please enter a valid age!");
                        newAge = readInt(scanner, "Enter valid age: ");
                    }
                    double newWeight = readDouble(scanner, "Enter weight (kg): ");
                    while (newWeight < 20 || newWeight > 600) {
                        System.out.println("Please enter a valid weight!");
                        newWeight = readDouble(scanner, "Enter valid weight: ");
                    }
                    double newEnergy = readDouble(scanner, "Enter energy level (0-100): ");
                    while (newEnergy < 0.0 || newEnergy > 100.0) {
                        System.out.println("Please enter a valid energy level!");
                        newEnergy = readDouble(scanner, "Enter energy level (0-100): ");
                    }

                    cloneObj.setName(newName + " (Cloned from " + original.getName() + ")");
                    cloneObj.setAge(newAge);
                    cloneObj.setWeight(newWeight);
                    cloneObj.setEnergyLvl(newEnergy);
                } else if (copyChoice.equalsIgnoreCase("n")) {
                    cloneObj.setName(cloneObj.getName() + " (Cloned from '" + original.getName() + "')");
                }

                System.out.println("\n--- Original (modified) ---");
                original.athletePrint();

                System.out.println("\n--- Clone (in the end of array) ---");
                cloneObj.athletePrint();

                System.out.println("Deep copy was successful!");
            } catch (CloneNotSupportedException e) {
                System.out.println("Cloning error!");
            }
        } else {
            System.out.println("Error: Invalid index!");
        }
        return athletes;
    }

    private static void handleBinarySearch(Scanner scanner, Athlete[] athletes) {
        System.out.println("\nSorting the array by Name (Ascending) for Binary Search...");
        Arrays.sort(athletes);
        System.out.println("Array sorted successfully!");

        String searchName = readName(scanner, "Enter the name of the athlete to search for: ");
        Athlete searchDummy = new Athlete(searchName);
        int searchResult = Arrays.binarySearch(athletes, searchDummy);

        if (searchResult >= 0) {
            System.out.println("\nEXACT MATCH FOUND at index: " + searchResult);
            athletes[searchResult].athletePrint();

            int left = searchResult - 1;
            while (left >= 0 && athletes[left].compareTo(searchDummy) == 0) {
                System.out.println("\nADDITIONAL MATCH FOUND at index: " + left);
                athletes[left].athletePrint();
                left--;
            }

            int right = searchResult + 1;
            while (right < athletes.length && athletes[right].compareTo(searchDummy) == 0) {
                System.out.println("\nADDITIONAL MATCH FOUND at index: " + right);
                athletes[right].athletePrint();
                right++;
            }
        } else {
            System.out.println("\nAthlete with name '" + searchName + "' was not found in the array!");
            System.out.println("(If added, it should be placed at index: " + (-(searchResult + 1)) + ")");
        }
    }

    private static Athlete[] handleCategoryDelete(Scanner scanner, Athlete[] athletes) {
        double threshold = readDouble(scanner, "Divide athletes into two categories based on their Energy Level. \nEnter energy threshold (0-100): ");

        int exhaustedCount = 0;
        int energeticCount = 0;
        for (Athlete a : athletes) {
            if (a.getEnergyLvl() < threshold) exhaustedCount++;
            else energeticCount++;
        }

        System.out.println("\nCategory 1 (Exhausted - Energy < " + threshold + "%): " + exhaustedCount + " athletes");
        System.out.println("Category 2 (Energetic - Energy >= " + threshold + "%): " + energeticCount + " athletes");

        if (athletes.length == 0) {
            System.out.println("Array is empty. Nothing to delete.");
            return athletes;
        }

        System.out.println("\nWhich category do you want to DELETE?");
        System.out.println("1 - Delete Category 1 (Exhausted)");
        System.out.println("2 - Delete Category 2 (Energetic)");
        int categoryChoice = readInt(scanner, "Your choice (1 or 2): ");

        if (categoryChoice != 1 && categoryChoice != 2) {
            System.out.println("Error: Invalid category choice. Operation canceled.");
            return athletes;
        }

        int keepCount = (categoryChoice == 1) ? energeticCount : exhaustedCount;
        Athlete[] newAthletes = new Athlete[keepCount];
        int insertPos = 0;

        for (Athlete a : athletes) {
            boolean isExhausted = a.getEnergyLvl() < threshold;
            if (categoryChoice == 1 && !isExhausted) {
                newAthletes[insertPos++] = a;
            } else if (categoryChoice == 2 && isExhausted) {
                newAthletes[insertPos++] = a;
            }
        }

        athletes = newAthletes;
        System.out.println("\nCategory deleted successfully! Remaining athletes in array: " + athletes.length);
        return athletes;
    }

    private static void handleTeamCompetition(Athlete[] athletes) {
        System.out.println("You chose to do the competition between athletes.\nAll of them will be divided in 2 teams.\n");
        if (athletes.length < 2) {
            System.out.println("Error: Not enough athletes for a competition. Need at least 2!");
            return;
        }

        int compSize = athletes.length;
        if (compSize % 2 != 0) {
            System.out.println("Notice: Odd number of athletes (" + compSize + "). The last athlete will sit on the bench.");
            compSize--;
        }

        int teamSize = compSize / 2;
        Athlete[] team1 = Arrays.copyOfRange(athletes, 0, teamSize);
        Athlete[] team2 = Arrays.copyOfRange(athletes, teamSize, compSize);

        double team1Score = 0;
        System.out.println("\n=== TEAM 1 ===");
        for (Athlete a : team1) {
            System.out.print("- " + a.getName() + " (Power: " + String.format("%.1f", a.getEnergyLvl() + a.getWeight()) + ")\n");
            team1Score += (a.getEnergyLvl() + a.getWeight());
        }

        double team2Score = 0;
        System.out.println("\n=== TEAM 2 ===");
        for (Athlete a : team2) {
            System.out.print("- " + a.getName() + " (Power: " + String.format("%.1f", a.getEnergyLvl() + a.getWeight()) + ")\n");
            team2Score += (a.getEnergyLvl() + a.getWeight());
        }

        System.out.println("\n--- COMPETITION RESULTS ---");
        System.out.printf("Team 1 Total Power: %.2f\n", team1Score);
        System.out.printf("Team 2 Total Power: %.2f\n", team2Score);

        if (team1Score > team2Score) {
            System.out.println("TEAM 1 WINS THE COMPETITION!");
        } else if (team2Score > team1Score) {
            System.out.println("TEAM 2 WINS THE COMPETITION!");
        } else {
            System.out.println("IT'S A DRAW!");
        }
    }

    private static Athlete[] handleAddMoveZone(Scanner scanner, Athlete[] athletes, TrainingZone cardioZone, TrainingZone weightZone) {
        System.out.println("\n--- Add/Move Athlete to a Zone ---");
        System.out.println("1 - Create a NEW athlete");
        System.out.println("2 - MOVE an existing athlete from Universal Array (Lobby)");
        int addChoice = readInt(scanner, "Your choice: ");

        Athlete athleteToAdd = null;

        if (addChoice == 1) {
            String newName = readName(scanner, "Enter athlete's name: ");
            int newAge = readInt(scanner, "Enter age: ");
            double newWeight = readDouble(scanner, "Enter weight (kg): ");
            double newEnergy = readDouble(scanner, "Enter energy level (0-100): ");
            String eqName = readName(scanner, "Enter equipment name: ");
            int eqValue = readInt(scanner, "Enter equipment condition/weight (int): ");
            System.out.print("Enter unit type ('kg', '%', etc.): ");
            String eqUnit = scanner.nextLine().trim();

            athleteToAdd = new Athlete(newName, newAge, newWeight, newEnergy, new Equipment(eqName, eqValue, eqUnit));
        } else if (addChoice == 2) {
            if (athletes.length == 0) {
                System.out.println("Lobby is empty! Cannot move anyone.");
                return athletes;
            }
            int moveIndex = readInt(scanner, "Enter index of athlete in Lobby (0 to " + (athletes.length - 1) + "): ");
            if (moveIndex >= 0 && moveIndex < athletes.length) {
                athleteToAdd = athletes[moveIndex];

                for (int i = moveIndex; i < athletes.length - 1; i++) {
                    athletes[i] = athletes[i + 1];
                }
                athletes = Arrays.copyOf(athletes, athletes.length - 1);
            } else {
                System.out.println("Invalid index!");
                return athletes;
            }
        } else {
            System.out.println("Invalid choice.");
            return athletes;
        }

        if (athleteToAdd != null) {
            System.out.println("\nWhere should we add " + athleteToAdd.getName() + "?");
            System.out.println("1 - Cardio Zone");
            System.out.println("2 - Weight Zone");
            int destChoice = readInt(scanner, "Your choice: ");

            if (destChoice == 1) {
                cardioZone.addAthlete(athleteToAdd);
                System.out.println("Added to Cardio Zone!");
            } else if (destChoice == 2) {
                weightZone.addAthlete(athleteToAdd);
                System.out.println("Added to Weight Zone!");
            } else {
                System.out.println("Invalid zone. Athlete got lost.");
            }
        }
        return athletes;
    }

    private static void handleShowAllData(Athlete[] athletes, TrainingZone cardioZone, TrainingZone weightZone) {
        System.out.println("\n--- Show All Data ---");
        System.out.println("=== UNIVERSAL ARRAY (Lobby) ===");
        if (athletes.length == 0) {
            System.out.println("  [Empty]");
        } else {
            for (int i = 0; i < athletes.length; i++) {
                System.out.print("  [" + i + "] ");
                athletes[i].athletePrint();
            }
        }
        System.out.println("===============================\n");

        cardioZone.printZone();
        weightZone.printZone();
    }

    private static Athlete[] handleBenchPressCompetition(Scanner scanner, Athlete[] athletes, TrainingZone cardioZone, TrainingZone weightZone) {
        System.out.println("\n--- Athlete bench press competition ---");
        String fighter1Name = readName(scanner, "Enter name of FIRST athlete: ");
        String fighter2Name = readName(scanner, "Enter name of SECOND athlete: ");

        if (fighter1Name.equalsIgnoreCase(fighter2Name)) {
            System.out.println("An athlete cannot compete with themselves!");
            return athletes;
        }

        Athlete fighter1 = null;
        Athlete fighter2 = null;

        int totalAthletes = athletes.length + cardioZone.getZoneAthletes().length + weightZone.getZoneAthletes().length;
        Athlete[] allAthletes = new Athlete[totalAthletes];
        int index = 0;
        for (Athlete a : athletes) allAthletes[index++] = a;
        for (Athlete a : cardioZone.getZoneAthletes()) allAthletes[index++] = a;
        for (Athlete a : weightZone.getZoneAthletes()) allAthletes[index++] = a;

        for (Athlete a : allAthletes) {
            if (a.getName().equalsIgnoreCase(fighter1Name)) fighter1 = a;
            if (a.getName().equalsIgnoreCase(fighter2Name)) fighter2 = a;
        }

        if (fighter1 == null || fighter2 == null) {
            System.out.println("Error: One or both athletes were not found in any gym zone!");
            return athletes;
        }

        System.out.println("\nBENCH PRESS COMPETITION " + fighter1.getName() + " VS " + fighter2.getName());
        double p1Power = fighter1.getEnergyLvl() + fighter1.getWeight();
        double p2Power = fighter2.getEnergyLvl() + fighter2.getWeight();

        Athlete winner = null;
        Athlete loser = null;

        if (p1Power > p2Power) {
            winner = fighter1; loser = fighter2;
        } else if (p2Power > p1Power) {
            winner = fighter2; loser = fighter1;
        }

        if (winner != null) {
            System.out.println(winner.getName() + " wins the bench press competition!");
            winner.setEnergyLvl(Math.min(100.0, winner.getEnergyLvl() + 20.0));
            loser.setEnergyLvl(loser.getEnergyLvl() - 30.0);

            if (loser.getEnergyLvl() < 0) {
                loser.setEnergyLvl(0);
            }

            System.out.println(winner.getName() + " gains +20 Energy. Current Energy: " + winner.getEnergyLvl() + "%");
            System.out.println(loser.getName() + " loses -30 Energy. Current Energy: " + loser.getEnergyLvl() + "%");

            if (loser.getEnergyLvl() <= 0) {
                System.out.println(loser.getName() + " is completely exhausted and goes home (Removed from the gym)!");
                Athlete[] newUniversal = new Athlete[athletes.length];
                int uIdx = 0;
                for (Athlete a : athletes) if (a != loser) newUniversal[uIdx++] = a;
                if (uIdx < athletes.length) athletes = Arrays.copyOf(newUniversal, uIdx);

                for (int i = 0; i < cardioZone.getZoneAthletes().length; i++) {
                    if (cardioZone.getZoneAthletes()[i] == loser) cardioZone.removeAthlete(i);
                }
                for (int i = 0; i < weightZone.getZoneAthletes().length; i++) {
                    if (weightZone.getZoneAthletes()[i] == loser) weightZone.removeAthlete(i);
                }
            }
        } else {
            System.out.println("It's a tie! Both athletes learned a lot.");
        }
        return athletes;
    }

    private static void handleZoneWar(TrainingZone cardioZone, TrainingZone weightZone) {
        System.out.println("\n--- Zone War: Cardio vs Weight ---");

        if (cardioZone.getZoneAthletes().length == 0 || weightZone.getZoneAthletes().length == 0) {
            System.out.println("Error: Both zones must have at least one athlete to compete!");
            return;
        }

        double cardioPower = 0;
        for (Athlete a : cardioZone.getZoneAthletes()) cardioPower += (a.getEnergyLvl() + a.getWeight());

        double weightPower = 0;
        for (Athlete a : weightZone.getZoneAthletes()) weightPower += (a.getEnergyLvl() + a.getWeight());

        System.out.printf("Cardio Zone Total Power: %.1f\n", cardioPower);
        System.out.printf("Weight Zone Total Power: %.1f\n", weightPower);

        TrainingZone winningZone = null;
        TrainingZone losingZone = null;

        if (cardioPower > weightPower) {
            winningZone = cardioZone;
            losingZone = weightZone;
        } else if (weightPower > cardioPower) {
            winningZone = weightZone;
            losingZone = cardioZone;
        }

        if (winningZone != null) {
            System.out.println(winningZone.getName() + " WON THE ZONE WAR!");
            System.out.println("Penalty: The weakest athlete from " + losingZone.getName() + " will go to the " + winningZone.getName() + "!");

            Athlete[] losers = losingZone.getZoneAthletes();
            int weakestIndex = 0;
            double minPower = losers[0].getEnergyLvl() + losers[0].getWeight();

            for (int i = 1; i < losers.length; i++) {
                double p = losers[i].getEnergyLvl() + losers[i].getWeight();
                if (p < minPower) {
                    minPower = p;
                    weakestIndex = i;
                }
            }

            Athlete defeated = losers[weakestIndex];
            System.out.println(defeated.getName() + " is switching sides! (Transferred to " + winningZone.getName() + ")");

            losingZone.removeAthlete(weakestIndex);
            winningZone.addAthlete(defeated);
        } else {
            System.out.println("It's a tie! Both zones respect each other.");
        }
    }

    private static void handleCountCriteria(Scanner scanner, Athlete[] athletes, TrainingZone cardioZone, TrainingZone weightZone) {
        System.out.println("\n--- Count Athletes by Criteria ---");
        System.out.println("1 - Energy is LESS than N (Find exhausted)");
        System.out.println("2 - Weight is GREATER than N (Find heavyweights)");
        System.out.println("3 - Age EXACTLY equals N (Find peers)");
        int critChoice = readInt(scanner, "Choose criterion (1-3): ");

        double checkValue = 0;
        if (critChoice == 1 || critChoice == 2) {
            checkValue = readDouble(scanner, "Enter the value for comparison: ");
        } else if (critChoice == 3) {
            checkValue = readInt(scanner, "Enter exact age: ");
        } else {
            System.out.println("Invalid choice!");
            return;
        }

        int totalFound = 0;
        System.out.println("\n--- SEARCH RESULTS ---");

        for (Athlete a : athletes) {
            boolean match = (critChoice == 1 && a.getEnergyLvl() < checkValue) ||
                    (critChoice == 2 && a.getWeight() > checkValue) ||
                    (critChoice == 3 && a.getAge() == (int)checkValue);
            if (match) {
                System.out.println("[Lobby] " + a.getName() + " (Age: " + a.getAge() + ", Weight: " + String.format("%.1f", a.getWeight()) + ", Energy: " + String.format("%.1f", a.getEnergyLvl()) + ")");
                totalFound++;
            }
        }

        for (Athlete a : cardioZone.getZoneAthletes()) {
            boolean match = (critChoice == 1 && a.getEnergyLvl() < checkValue) ||
                    (critChoice == 2 && a.getWeight() > checkValue) ||
                    (critChoice == 3 && a.getAge() == (int)checkValue);
            if (match) {
                System.out.println("[Cardio Zone] " + a.getName() + " (Age: " + a.getAge() + ", Weight: " + String.format("%.1f", a.getWeight()) + ", Energy: " + String.format("%.1f", a.getEnergyLvl()) + ")");
                totalFound++;
            }
        }

        for (Athlete a : weightZone.getZoneAthletes()) {
            boolean match = (critChoice == 1 && a.getEnergyLvl() < checkValue) ||
                    (critChoice == 2 && a.getWeight() > checkValue) ||
                    (critChoice == 3 && a.getAge() == (int)checkValue);
            if (match) {
                System.out.println("[Weight Zone] " + a.getName() + " (Age: " + a.getAge() + ", Weight: " + String.format("%.1f", a.getWeight()) + ", Energy: " + String.format("%.1f", a.getEnergyLvl()) + ")");
                totalFound++;
            }
        }

        System.out.println("----------------------");
        System.out.println("Total athletes matching criterion: " + totalFound);
    }

    private static Athlete[] handleDeleteFromZone(Scanner scanner, Athlete[] athletes, TrainingZone cardioZone, TrainingZone weightZone) {
        System.out.println("\n--- Delete Athlete from Specific Zone ---");
        System.out.println("Select the zone to delete from:");
        System.out.println("1 - Universal Array (Lobby)");
        System.out.println("2 - Cardio Zone");
        System.out.println("3 - Weight Zone");
        int delZoneChoice = readInt(scanner, "Your choice: ");

        if (delZoneChoice < 1 || delZoneChoice > 3) {
            System.out.println("Invalid zone selected.");
            return athletes;
        }

        String delName = readName(scanner, "Enter the exact NAME of the athlete to delete: ");
        if (delName.equals("CANCEL_OPERATION")) return athletes;

        boolean deleted = false;

        if (delZoneChoice == 1) {
            for (int i = 0; i < athletes.length; i++) {
                if (athletes[i].getName().equalsIgnoreCase(delName)) {
                    for (int j = i; j < athletes.length - 1; j++) {
                        athletes[j] = athletes[j + 1];
                    }
                    athletes = Arrays.copyOf(athletes, athletes.length - 1);
                    deleted = true;
                    break;
                }
            }
        } else if (delZoneChoice == 2) {
            for (int i = 0; i < cardioZone.getZoneAthletes().length; i++) {
                if (cardioZone.getZoneAthletes()[i].getName().equalsIgnoreCase(delName)) {
                    cardioZone.removeAthlete(i);
                    deleted = true;
                    break;
                }
            }
        } else if (delZoneChoice == 3) {
            for (int i = 0; i < weightZone.getZoneAthletes().length; i++) {
                if (weightZone.getZoneAthletes()[i].getName().equalsIgnoreCase(delName)) {
                    weightZone.removeAthlete(i);
                    deleted = true;
                    break;
                }
            }
        }

        if (deleted) {
            System.out.println("Athlete '" + delName + "' was successfully removed from the selected zone.");
        } else {
            System.out.println("Athlete '" + delName + "' was NOT FOUND in the selected zone.");
        }
        return athletes;
    }
}