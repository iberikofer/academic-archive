public class Athlete {
private String name;
private int age;
private double weight;
private double energyLvl;

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public int getAge() {
        return age;
    }

    public void setAge(int age) {
        this.age = age;
    }

    public double getWeight() {
        return weight;
    }

    public void setWeight(double weight) {
        this.weight = weight;
    }

    public double getEnergyLvl() {
        return energyLvl;
    }

    public void setEnergyLvl(double energyLvl) {
        this.energyLvl = energyLvl;
    }

    static {
        System.out.println("=== Static initialization block of Athlete class executed ===");
    }

    {
        System.out.println("=== Non-static initialization block executed ===");
    }

    public  Athlete() {
        this("Gym bro", 20, 100.0, 100.0);
        System.out.println("=== Default Constructor without arguments was called (Delegation) ===");
    }

    public Athlete(String name, int age, double weight, double energyLvl) {
        this.name = name;
        this.age = age;
        this.weight = weight;
        this.energyLvl = energyLvl;

        System.out.println(
                "=== Athlete Constructor was called ==="
//                + "- Name:" + name + "\n" +
//                "- Age: " + age +"\n" +
//                "- Weight: " + weight + "\n" +
//                "- Energy level: " + energyLvl + "\n"
        );
    }

    public void train() {
        if (this.energyLvl >= 5){
            this.energyLvl -= 5;

            double weightLoss = java.util.concurrent.ThreadLocalRandom.current().nextDouble(5, 10);
            this.weight -= weightLoss;

            System.out.println(name + " потренувався. Енергія: " + energyLvl + "%, Вага: " + String.format("%.2f", weight));
        } else {
            System.out.println(name + " занадто виснажений для тренування!");
        }
    }

    public void eat() {
        if (this.energyLvl < 100){
            this.energyLvl = Math.min(this.energyLvl + 2.5, 100.0);

            double weightGain = java.util.concurrent.ThreadLocalRandom.current().nextDouble(1, 5);
            this.weight += weightGain;

            System.out.println(name + " відновився. Енергія: " + energyLvl + "%, Вага: " + String.format("%.2f", weight));
        } else {
            System.out.println(name + " повністю відновився!");
        }
    }

    public void rest() {
        if (this.energyLvl <= 100){
            this.energyLvl = Math.min(this.energyLvl + 2.5, 100.0);

            double weightGain = java.util.concurrent.ThreadLocalRandom.current().nextDouble(1, 5);
            this.weight += weightGain;

            System.out.println(name + " відновився. Енергія: " + energyLvl + "%, Вага: " + String.format("%.2f", weight));
        } else {
            System.out.println(name + " повністю відновився!");
        }
    }

    public void takeSelfie() {
        if (this.energyLvl < 5) {
            System.out.println(name + " Занадто виснажений що б робити селфі!");
        } else {
            this.energyLvl -= 1.0;
            System.out.println(name + " виглядає круто!");
        }

    }

    public void athletePrint() {
        System.out.printf("=== Стан атлета: %s ===\n", name);
        System.out.printf("Вік: %d років | Вага: %.2f кг | Енергія: %.1f%%\n\n",
                age, weight, energyLvl);
    }

    @Override
    public boolean equals(Object obj) {
        if (obj == null || getClass() != obj.getClass()) return false;
        if (this == obj) return true;

        Athlete other = (Athlete) obj;

        if (this.name.equals(other.name) && this.age == other.age && this.weight == other.weight)
            return true;

        return false;
    }

    @Override
    public String toString() {
        return "- Name: " + name + "\n" + "- Age: " + age + "\n" + "- Weight: " + weight + "\n" + "- Energy Lvl: " + energyLvl + "\n";
    }
}