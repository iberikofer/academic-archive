import java.util.Scanner;

public class Main {

    public static Athlete athlete1;
    private Athlete athlete2;
    String n = ""; int a = 0; double w = 0.0;
    Scanner sc = new Scanner(System.in);
    Integer athleteChoice = 0;

    static {
        System.out.println("Static initialization block!");
        athlete1 = new Athlete("Yaroslav Sych", 16, 70.0, 100.0);
//        System.out.println(athlete1);
    }

    {
        System.out.println("Non-static initialization block!");
        athlete2 = new Athlete();
//        System.out.println(athlete2);
    }

    void athleteParamsSet(Athlete target) {
        System.out.println("Enter athlete NAME.");
        n = sc.nextLine();
        System.out.println("Enter athlete AGE.");
        a = Integer.parseInt(sc.nextLine());
        System.out.println("Enter athlete WEIGHT.");
        w = Double.parseDouble(sc.nextLine());

        target.setName(n);
        target.setAge(a);
        target.setWeight(w);
    }

     void main() {
        System.out.println("Program started.\n");
        boolean exitProgram = false;
         Athlete athlete3 = new Athlete("Denys Katelnikov", 45, 80.0, 100.0);
         System.out.println(athlete3);

         do {
             System.out.println("===== MENU: =====");
             System.out.println("1 - Print Athlete1");
             System.out.println("2 - Change Athlete1");
             System.out.println("3 - Print Athlete2");
             System.out.println("4 - Change Athlete2");
             System.out.println("5 - Print Athlete3");
             System.out.println("6 - Change Athlete3");
             System.out.println("7 - Athlete interaction (Training)");
             System.out.println("8 - EXIT");
             Integer choice = Integer.parseInt(sc.nextLine());

             switch (choice) {
                 case (1): {
                    athlete1.athletePrint();
                 }
                 break;
                 case (2): {
                     athleteChoice = 1;
                     athleteParamsSet(athlete1);
                 }
                 break;
                 case (3): {
                     athlete2.athletePrint();
                 }
                 break;
                 case (4): {
                     athleteChoice = 2;
                     athleteParamsSet(athlete2);
                 }
                 break;
                 case (5): {
                     athlete3.athletePrint();
                 }
                 break;
                 case (6): {
                     athleteChoice = 3;
                     athleteParamsSet(athlete3);
                 }
                 break;
                 case (7): {
                     System.out.println("Choose TWO athletes (1-3) to interact");
                     System.out.println("First: ");
                     int first = Integer.parseInt(sc.nextLine());
                     System.out.println("Second: ");
                     int second = Integer.parseInt(sc.nextLine());

                     Athlete firstTarget = null;
                     Athlete secondTarget = null;

                     if (first == 1) firstTarget = athlete1;
                     else if (first == 2) firstTarget = athlete2;
                     else if (first == 3) firstTarget = athlete3;

                     if (second == 1) secondTarget = athlete1;
                     else if (second == 2) secondTarget = athlete2;
                     else if (second == 3) secondTarget = athlete3;

                     if (firstTarget != null && secondTarget != null && firstTarget != secondTarget) {
                         System.out.println("\n--- INTERACTION: " + firstTarget.getName() + " & " + secondTarget.getName() + " ---");

                         System.out.println(firstTarget.getName() + " motivates " + secondTarget.getName() + " to work harder!");
                         secondTarget.train();
                         firstTarget.takeSelfie();

                         System.out.println("Interaction completed successfully.\n");
                     } else if (firstTarget == secondTarget && firstTarget != null) {
                         System.out.println("An athlete cannot interact with themselves in this mode!");
                     } else {
                         System.out.println("Invalid input! Please choose numbers from 1 to 3.");
                     }
                 }
                 break;
                 case (8): {
                    exitProgram = true;
                 }
                 break;
             }
         } while(!exitProgram);

        if (athlete1.equals(athlete2)) {
            System.out.println("EQUALS!");
        } else {
            System.out.println("NOT EQUAL!");
        }

        System.out.println("Program ended.\n");
    }
}