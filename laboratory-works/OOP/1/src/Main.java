import java.util.Scanner;

public class Main {
    public static void main(String[] args) {
        Scanner in = new Scanner(System.in);
        System.out.print("Enter your number N please: ");
        int N = in.nextInt();
        in.close();

        for (int i = N; i >= 2; i--) {
            boolean isPrime = true;

            for (int j = 2; j <= Math.sqrt(i); j++) {
                if (i % j == 0) {
                    isPrime = false;
                    break;
                }
            }
            if (isPrime) {
                System.out.println("Найбільше просте число: " + i);
                break;
            }
        }
    }
}
